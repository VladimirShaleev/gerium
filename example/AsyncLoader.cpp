#include "AsyncLoader.hpp"

AsyncLoader::~AsyncLoader() {
    destroy();
}

void AsyncLoader::create(gerium_application_t application, gerium_renderer_t renderer) {
    _renderer = renderer;

    check(gerium_signal_create(&_shutdownSignal));
    check(gerium_signal_create(&_waitTaskSignal));
    check(gerium_mutex_create(&_mutex));

    gerium_application_execute(application, [](auto app, auto data) {
        ((AsyncLoader*) data)->loader();
    }, this);
}

void AsyncLoader::destroy() {
    if (_renderer) {
        gerium_signal_set(_waitTaskSignal);
        gerium_signal_set(_shutdownSignal);
        _renderer = nullptr;
    }
}

void AsyncLoader::loadTexture(gerium_texture_h texture, gerium_file_t file, gerium_cdata_t data) {
    gerium_mutex_lock(_mutex);
    _tasks.push(new Task{ texture, file, data, nullptr });
    gerium_mutex_unlock(_mutex);
    gerium_signal_set(_waitTaskSignal);
}

void AsyncLoader::loader() noexcept {
    while (!gerium_signal_is_set(_shutdownSignal)) {
        gerium_signal_wait(_waitTaskSignal);

        std::vector<Task*> tasks;
        gerium_mutex_lock(_mutex);
        tasks.reserve(_tasks.size());
        while (!_tasks.empty()) {
            tasks.push_back(_tasks.front());
            _tasks.pop();
        }
        gerium_signal_clear(_waitTaskSignal);
        gerium_mutex_unlock(_mutex);

        for (auto task : tasks) {
            int widht, height, comp;
            task->image = (gerium_cdata_t) stbi_load_from_memory(
                (const stbi_uc*) task->data, (int) gerium_file_get_size(task->file), &widht, &height, &comp, 4);

            auto renderer = _renderer.load();
            if (renderer) {
                gerium_renderer_async_upload_texture_data(
                    renderer, task->texture, task->image, [](auto _, auto texture, auto data) {
                    auto task = (Task*) data;
                    stbi_image_free((void*) task->image);
                    gerium_file_destroy(task->file);
                    delete task;
                }, task);
            }
        }
    }

    gerium_mutex_destroy(_mutex);
    gerium_signal_destroy(_waitTaskSignal);
    gerium_signal_destroy(_shutdownSignal);
}
