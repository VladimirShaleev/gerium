/**
 * @file      gerium-callbacks.h
 * @brief     Callback interfaces.
 * @details   Defines function signatures for event handling and custom operations.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_CALLBACKS_H
#define GERIUM_CALLBACKS_H

#include "gerium-enums.h"

GERIUM_BEGIN

/**
 * @brief     Application frame update callback.
 * @details   Called every frame to start rendering passes.
 * @param[in] application Application instance.
 * @param[in] data User data.
 * @param[in] elapsed_ms The elapsed time in milliseconds since the last call to the previous frame.
 * @return    If *FALSE* is returned, the application terminates the event loop.
 */
typedef gerium_bool_t
(*gerium_application_frame_callback_t)(gerium_application_t application,
                                       gerium_data_t data,
                                       gerium_uint64_t elapsed_ms);

/**
 * @brief     Application state change callback.
 * @details   Notifies about application state changes.
 * @param[in] application Application instance.
 * @param[in] data User data.
 * @param[in] state New application state.
 * @return    If *FALSE* is returned, the application terminates the event loop.
 */
typedef gerium_bool_t
(*gerium_application_state_callback_t)(gerium_application_t application,
                                       gerium_data_t data,
                                       gerium_application_state_t state);

/**
 * @brief     Application executor callback.
 * @details   Called to execute work in fiber.
 * @param[in] application Application instance.
 * @param[in] data User data.
 */
typedef void
(*gerium_application_executor_callback_t)(gerium_application_t application,
                                          gerium_data_t data);

/**
 * @brief     Frame graph preparation callback.
 * @details   Called when frame graph begins preparing render passes.
 * @param[in] frame_graph Frame graph instance.
 * @param[in] renderer Associated renderer.
 * @param[in] max_workers Maximum available worker threads.
 * @param[in] data User data.
 * @return    Should return number of worker threads to use.
 */
typedef gerium_uint32_t
(*gerium_frame_graph_prepare_callback_t)(gerium_frame_graph_t frame_graph,
                                         gerium_renderer_t renderer,
                                         gerium_uint32_t max_workers,
                                         gerium_data_t data);

/**
 * @brief     Frame graph resize callback.
 * @details   Notifies about render target size changes.
 * @param[in] frame_graph Frame graph instance.
 * @param[in] renderer Associated renderer.
 * @param[in] data User data.
 * @return    If *FALSE* is returned, the application terminates the event loop.
 */
typedef gerium_bool_t
(*gerium_frame_graph_resize_callback_t)(gerium_frame_graph_t frame_graph,
                                        gerium_renderer_t renderer,
                                        gerium_data_t data);

/**
 * @brief     Callback for the rendering pass.
 * @details   Executes rendering commands for a specific worker or compute work.
 * @param[in] frame_graph Frame graph instance.
 * @param[in] renderer Associated renderer.
 * @param[in] command_buffer Command buffer for recording.
 * @param[in] worker Current worker index.
 * @param[in] total_workers Total number of workers.
 * @param[in] data User data.
 * @return    If *FALSE* is returned, the application terminates the event loop.
 */
typedef gerium_bool_t
(*gerium_frame_graph_render_callback_t)(gerium_frame_graph_t frame_graph,
                                        gerium_renderer_t renderer,
                                        gerium_command_buffer_t command_buffer,
                                        gerium_uint32_t worker,
                                        gerium_uint32_t total_workers,
                                        gerium_data_t data);

/**
 * @brief     Texture loaded callback.
 * @details   Notifies when texture loading completes.
 * @param[in] renderer Associated renderer.
 * @param[in] texture Loaded texture handle.
 * @param[in] data User data.
 */
typedef void
(*gerium_texture_loaded_callback_t)(gerium_renderer_t renderer,
                                    gerium_texture_h texture,
                                    gerium_data_t data);

GERIUM_END

#endif /* GERIUM_CALLBACKS_H */
