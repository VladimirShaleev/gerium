# gerium

A framework for cross-platform game development

> [!IMPORTANT]  
> PROJECT IS UNDER ACTIVE DEVELOPMENT AND NOT READY FOR USE!

The goal of this project is to implement a simple C interface (and later wrappers for other languages and environments) for constructing a frame graph. The frame graph represents a directed acyclic graph (DAG) of rendering passes. Modern rendering mechanisms require hundreds of passes to create a frame, and manual management of these passes is error-prone. However, by describing frame passes as a graph, we achieve the following advantages:

- Automation of the creation and management of resources necessary for rendering passes;
- Reducing memory consumption. By tracking resource usage within the graph, we can reuse memory for resources that are no longer needed in subsequent nodes using memory aliasing techniques;
- Automatic resource synchronization. Namely, due to the presence of a graph, we can correctly place barriers and state transitions of layouts without manual control;
- Simplified rendering code. Building frames becomes more straightforward because we operate with input and output data, and we can easily enable or disable specific passes as needed.

For a deeper understanding of frame graphs, consider the following resources:

- [Talk by Yuriy O’Donnell at GDC 2017. Electronic Arts / DICE. FrameGraph: Extensible Rendering Architecture in Frostbite](https://www.slideshare.net/slideshow/framegraph-extensible-rendering-architecture-in-frostbite/72795495)
- [Epic Games. Rendering Dependency Graph](https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/Rendering/RenderDependencyGraph/)

Examples created using this framework:

- Clustered shading:

  <img src="https://media.githubusercontent.com/media/VladimirShaleev/gerium/refs/heads/main/doc/clustered-shading.gif" alt="clustered shading" width="700">

- Scene with over 9 million polygons (Meshlet occlusion culling):
  
  <img src="https://media.githubusercontent.com/media/VladimirShaleev/gerium/refs/heads/main/doc/occlusion-culling.gif" alt="occlusion culling" width="700">

  <img src="https://media.githubusercontent.com/media/VladimirShaleev/gerium/refs/heads/main/doc/occlusion-culling-profiler.gif" alt="occlusion culling (profiler)" width="700">

  More videos on [youtube channel](https://www.youtube.com/playlist?list=PLlKoKIZLrr-kLSHPs_UtlwtW9MBpYxDnV)

Currently, the framework supports the following platforms and graphics APIs:

- Windows (Vulkan API);
- MacOS (Vulkan API via MoltenVK on Metal);
- Android (Vulkan API).

There are plans to add iOS via MoltenVK soon. And also Web (Emscripten) with WebGPU API.

Ultimately the plan is to implement the following platforms and graphics APIs so that they are all `yes`:

| OS               |            Vulkan | WebGPU | GLES 3.0 (for legacy platforms) |
|:---------------- | -----------------:| ------:| -------------------------------:|
| Windows          |             `yes` |   `no` |                   `no` (ANGLE*) |
| Linux            |             `yes` |   `no` |                   `no` (ANGLE*) |
| MacOS            | `yes` (MoltenVK*) |   `no` |                   `no` (ANGLE*) |
| Android          |             `yes` |   `no` |                            `no` |
| iOS              |              `no` |   `no` |                            `no` |
| Web (Emscripten) |              `no` |   `no` |                    `no` (WebGL) |

- [MoltenVK*](https://github.com/KhronosGroup/MoltenVK)
- [ANGLE*](https://github.com/google/angle)

It is also worth reading the following books from which you can get information on the Vulkan API and more:

1. [Graham Sellers. Vulkan Programming Guide: The Official Guide to Learning Vulkan. 1st Ed. — Reading, MA:.Addison-Wesley Professional, 2016.](https://www.amazon.co.uk/Vulkan-Programming-Guide-Official-Learning/dp/0134464540) (ISBN-13: 978-0-13-446454-1) – A book for learning the Vulkan API;
2. [Marco Castorina., Gabriel Sassone. Mastering Graphics Programming with Vulkan: Develop a modern rendering engine from first principles to state-of-the-art techniques. — Reading, MA: Packt Publishing, 2023](https://www.amazon.co.uk/Mastering-Graphics-Programming-Vulkan-state/dp/1803244798) (ISBN-13: 978-1-80324-479-2) – A highly recommended book to read for learning the Vulkan API. Perhaps the best I've read about the Vulkan API. It covers not only the practical use of the Vulkan API, but also the implementation of the frame graph and other techniques of modern graphics APIs such as `Variable-rate shading`, `Raytracing` and others;
3. [Jason Gregory. Game Engine Architecture. 3rd Ed. — Reading, MA: A K Peters/CRC Press, 2018](https://www.amazon.co.uk/Engine-Architecture-Third-Jason-Gregory/dp/1138035459) (ISBN-13: 978-1138035454) – In general about game engines. About engine components, organization of resources in memory, etc.
