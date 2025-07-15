/**
 * @file      gerium-core.h
 * @brief     Core functionality.
 * @details   Fundamental system interfaces.
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_CORE_H
#define GERIUM_CORE_H

#include "gerium-structs.h"

GERIUM_BEGIN

/**
 * @brief   Current library version as packed 32-bit value.
 * @details Format: (major << 16) | (minor << 8) | micro.
 * @return  Return packed version number
 */
gerium_api gerium_uint32_t
gerium_version(void);

/**
 * @brief   Current library version as human-readable string.
 * @details Format: "major.minor.micro", eg: "1.0.0".
 * @return  Return version string.
 */
gerium_api gerium_utf8_t
gerium_version_string(void);

/**
 * @brief     Converts error code to descriptive string.
 * @details   Provides error message for debugging.
 * @param[in] result Error code value.
 * @return    Corresponding error message.
 */
gerium_api gerium_utf8_t
gerium_result_to_string(gerium_result_t result);

/**
 * @name Functions of Logger.
 * @brief Functions for opaque type ::gerium_logger_t.
 * @{
 */

/**
 * @brief      Creates new logger instance
 * @details    Initializes logging subsystem with specific tag.
 * @param[in]  tag Tag string.
 * @param[out] logger New logger instance.
 * @return     Operation result
 * @note       The tag string is specified in the format: "mygame:logic:health".
 * @sa         See ::gerium_logger_set_level_by_tag to enable or disable specific nested tags.
 */
gerium_api gerium_result_t
gerium_logger_create(gerium_utf8_t tag,
                     gerium_logger_t* logger);

/**
 * @brief     Increments reference count.
 * @details   Manages logger instance lifetime.
 * @param[in] logger Target logger instance.
 * @return    Reference to same logger.
 * @sa        ::gerium_logger_destroy
 */
gerium_api gerium_logger_t
gerium_logger_reference(gerium_logger_t logger);

/**
 * @brief     Releases logger instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] logger Logger to destroy.
 * @sa        ::gerium_logger_reference
 */
gerium_api void
gerium_logger_destroy(gerium_logger_t logger);

/**
 * @brief     Gets current severity level.
 * @details   Returns minimum level for logged messages.
 * @param[in] logger Target logger.
 * @return    Current severity threshold.
 * @sa        ::gerium_logger_set_level
 */
gerium_api gerium_logger_level_t
gerium_logger_get_level(gerium_logger_t logger);

/**
 * @brief     Sets logging severity level.
 * @details   Filters messages below specified level.
 * @param[in] logger Target logger.
 * @param[in] level New severity threshold.
 * @sa        ::gerium_logger_get_level
 */
gerium_api void
gerium_logger_set_level(gerium_logger_t logger,
                        gerium_logger_level_t level);

/**
 * @brief     Configures level by category tag.
 * @details   Applies level to all loggers with matching prefix.
 * @param[in] tag Logger category prefix.
 * @param[in] level New severity level.
 * @note      The tag string is specified in the format: "mygame:logic".
 */
gerium_api void
gerium_logger_set_level_by_tag(gerium_utf8_t tag,
                               gerium_logger_level_t level);

/**
 * @brief     Print message.
 * @details   Outputs message if level meets threshold.
 * @param[in] logger Target logger.
 * @param[in] level Message severity.
 * @param[in] message Log message.
 */
gerium_api void
gerium_logger_print(gerium_logger_t logger,
                    gerium_logger_level_t level,
                    gerium_utf8_t message);

/** @} */

/**
 * @name Functions of File.
 * @brief Functions for opaque type ::gerium_file_t.
 * @{
 */

/**
 * @brief   Gets cache directory path.
 * @details Returns writable per-user cache location.
 * @return  Absolute cache path.
 */
gerium_api gerium_utf8_t
gerium_file_get_cache_dir(void);

/**
 * @brief   Gets application directory.
 * @details Returns the installation location of the executable or the directory where application support files are stored, depending on the system.
 * @return  Absolute application path.
 */
gerium_api gerium_utf8_t
gerium_file_get_app_dir(void);

/**
 * @brief     Checks file existence.
 * @details   Verifies file at specified path.
 * @param[in] path File path to check.
 * @return    *TRUE* if file exists.
 */
gerium_api gerium_bool_t
gerium_file_exists_file(gerium_utf8_t path);

/**
 * @brief     Checks directory existence.
 * @details   Verifies directory at specified path.
 * @param[in] path Directory path to check.
 * @return    *TRUE* if directory exists.
 */
gerium_api gerium_bool_t
gerium_file_exists_dir(gerium_utf8_t path);

/**
 * @brief     Deletes file.
 * @details   Removes file at specified path.
 * @param[in] path File to delete.
 */
gerium_api void
gerium_file_delete_file(gerium_utf8_t path);

/**
 * @brief      Opens file.
 * @details    Accesses file with specified mode.
 * @param[in]  path File location.
 * @param[in]  read_only Open in read-only mode.
 * @param[out] file New file instance.
 * @return     Operation result.
 * @sa         For creating temporary files see ::gerium_file_create_temp.
 */
gerium_api gerium_result_t
gerium_file_open(gerium_utf8_t path,
                 gerium_bool_t read_only,
                 gerium_file_t* file);

/**
 * @brief      Create file.
 * @details    Create file with the possibility of reserving space
 * @param[in]  path Path to create file
 * @param[in]  size Initial size in bytes. If 0, the space is not reserved.
 * @param[out] file New file instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_file_create(gerium_utf8_t path,
                   gerium_uint64_t size,
                   gerium_file_t* file);

/**
 * @brief      Creates temporary file.
 * @details    Once the application is terminated, the file is considered invalid.
 * @param[in]  size Initial size in bytes. If 0, the space is not reserved.
 * @param[out] file New file instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_file_create_temp(gerium_uint64_t size,
                        gerium_file_t* file);

/**
 * @brief     Increments reference count.
 * @details   Manages file instance lifetime.
 * @param[in] file Target file.
 * @return    Reference to same file.
 * @sa        ::gerium_file_destroy
 */
gerium_api gerium_file_t
gerium_file_reference(gerium_file_t file);

/**
 * @brief     Releases file instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] file File to close.
 * @sa        ::gerium_file_reference
 */
gerium_api void
gerium_file_destroy(gerium_file_t file);

/**
 * @brief     Gets current file size.
 * @details   Returns size in bytes.
 * @param[in] file Target file.
 * @return    File size.
 */
gerium_api gerium_uint64_t
gerium_file_get_size(gerium_file_t file);

/**
 * @brief     Moves file pointer.
 * @details   Changes read/write position.
 * @param[in] file Target file.
 * @param[in] offset Position offset.
 * @param[in] seek Seek mode.
 */
gerium_api void
gerium_file_seek(gerium_file_t file,
                 gerium_uint64_t offset,
                 gerium_file_seek_t seek);

/**
 * @brief     Writes data to file.
 * @details   Outputs binary data at current position.
 * @param[in] file Target file.
 * @param[in] data Source data.
 * @param[in] size Data size.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_file_write(gerium_file_t file,
                  gerium_cdata_t data,
                  gerium_uint32_t size);

/**
 * @brief     Reads file contents.
 * @details   Inputs binary data from current position.
 * @param[in] file Target file.
 * @param[in] data Destination buffer.
 * @param[in] size Buffer size in bytes.
 * @return    Bytes read.
 */
gerium_api gerium_uint32_t
gerium_file_read(gerium_file_t file,
                 gerium_data_t data,
                 gerium_uint32_t size);

/**
 * @brief     Memory-maps file contents.
 * @details   Provides direct access to file data.
 * @param[in] file Target file.
 * @return    Pointer to mapped memory region.
 * @note      The Mapping closes when the file is closed.
 */
gerium_api gerium_data_t
gerium_file_map(gerium_file_t file);

/** @} */

/**
 * @name Functions of Mutex.
 * @brief Functions for opaque type ::gerium_mutex_t.
 * @{
 */

/**
 * @brief      Creates new mutex instance.
 * @details    Initializes synchronization object.
 * @param[out] mutex New mutex instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_mutex_create(gerium_mutex_t* mutex);

/**
 * @brief     Increments reference count.
 * @details   Manages mutex instance lifetime.
 * @param[in] mutex Target mutex.
 * @return    Reference to same mutex.
 * @sa        ::gerium_mutex_destroy
 */
gerium_api gerium_mutex_t
gerium_mutex_reference(gerium_mutex_t mutex);

/**
 * @brief     Releases mutex instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] mutex Mutex to destroy.
 * @sa        ::gerium_mutex_reference
 */
gerium_api void
gerium_mutex_destroy(gerium_mutex_t mutex);

/**
 * @brief     Acquires mutex lock.
 * @details   Blocks until lock is obtained.
 * @param[in] mutex Mutex to lock.
 * @warning   May deadlock if recursively locked.
 * @sa        See also ::gerium_mutex_unlock.
 */
gerium_api void
gerium_mutex_lock(gerium_mutex_t mutex);

/**
 * @brief     Releases mutex lock.
 * @details   Allows other threads to proceed.
 * @param[in] mutex Mutex to unlock.
 * @sa        See also ::gerium_mutex_lock.
 */
gerium_api void
gerium_mutex_unlock(gerium_mutex_t mutex);

/** @} */

/**
 * @name Functions of Signal.
 * @brief Functions for opaque type ::gerium_signal_t.
 * @{
 */

/**
 * @brief      Creates new signal instance.
 * @details    Initializes in unsignaled state.
 * @param[out] signal New signal instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_signal_create(gerium_signal_t* signal);

/**
 * @brief     Increments reference count.
 * @details   Manages signal instance lifetime.
 * @param[in] signal Target signal.
 * @return    Reference to same signal.
 * @sa        ::gerium_signal_destroy
 */
gerium_api gerium_signal_t
gerium_signal_reference(gerium_signal_t signal);

/**
 * @brief     Releases signal instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] signal Signal to destroy.
 * @sa        ::gerium_signal_reference
 */
gerium_api void
gerium_signal_destroy(gerium_signal_t signal);

/**
 * @brief     Checks signal state.
 * @details   Tests if signal has been raised.
 * @param[in] signal Target signal.
 * @return    *TRUE* if signaled.
 */
gerium_api gerium_bool_t
gerium_signal_is_set(gerium_signal_t signal);

/**
 * @brief     Raises the signal.
 * @details   Wakes all waiting threads.
 * @param[in] signal Signal to raise.
 */
gerium_api void
gerium_signal_set(gerium_signal_t signal);

/**
 * @brief     Blocks until signaled.
 * @details   Waits for signal to be raised.
 * @param[in] signal Signal to wait for.
 */
gerium_api void
gerium_signal_wait(gerium_signal_t signal);

/**
 * @brief     Resets signal state.
 * @details   Returns to unsignaled condition.
 * @param[in] signal Signal to reset.
 */
gerium_api void
gerium_signal_clear(gerium_signal_t signal);

/** @} */

/**
 * @name Functions of Application.
 * @brief Functions for opaque type ::gerium_application_t.
 * @{
 */

/**
 * @brief      Creates new application instance.
 * @details    Initializes window system and core subsystems.
 * @param[in]  title Window title.
 * @param[in]  width Initial window width.
 * @param[in]  height Initial window height.
 * @param[out] application Created application instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_application_create(gerium_utf8_t title,
                          gerium_uint32_t width,
                          gerium_uint32_t height,
                          gerium_application_t* application);

/**
 * @brief     Increments reference count.
 * @details   Manages application instance lifetime.
 * @param[in] application Target application
 * @return    Reference to same application.
 * @sa        ::gerium_application_destroy
 */
gerium_api gerium_application_t
gerium_application_reference(gerium_application_t application);

/**
 * @brief     Releases application instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] application Application to destroy.
 * @sa        ::gerium_application_reference
 */
gerium_api void
gerium_application_destroy(gerium_application_t application);

/**
 * @brief     Gets current platform.
 * @details   Identifies operating system.
 * @param[in] application Target application.
 * @return    Runtime platform identifier.
 */
gerium_api gerium_runtime_platform_t
gerium_application_get_platform(gerium_application_t application);

/**
 * @brief      Gets current frame callback.
 * @details    Retrieves registered rendering handler.
 * @param[in]  application Target application.
 * @param[out] data Callback user data pointer.
 * @return     Current frame callback function.
 */
gerium_api gerium_application_frame_callback_t
gerium_application_get_frame_callback(gerium_application_t application,
                                      gerium_data_t* data);

/**
 * @brief     Sets frame rendering callback.
 * @details   Registers handler for frame rendering events.
 * @param[in] application Target application.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 * @note      Called every frame.
 */
gerium_api void
gerium_application_set_frame_callback(gerium_application_t application,
                                      gerium_application_frame_callback_t callback,
                                      gerium_data_t data);

/**
 * @brief      Gets application state callback.
 * @details    Retrieves registered state change handler.
 * @param[in]  application Target application.
 * @param[out] data Callback user data pointer.
 * @return     Current state callback function.
 */
gerium_api gerium_application_state_callback_t
gerium_application_get_state_callback(gerium_application_t application,
                                      gerium_data_t* data);

/**
 * @brief     Sets state change callback.
 * @details   Registers handler for application state changes.
 * @param[in] application Target application.
 * @param[in] callback State handler function.
 * @param[in] data Callback user data.
 * @note      Called when minimizing/resizing/etc.
 */
gerium_api void
gerium_application_set_state_callback(gerium_application_t application,
                                      gerium_application_state_callback_t callback,
                                      gerium_data_t data);

/**
 * @brief         Gets display information.
 * @details       Retrieves connected display configurations.
 * @param[in]     application Target application.
 * @param[in,out] display_count Receives display count (if *displays* is null, then *display_count* will be set to the available count).
 * @param[in]     displays Display info array.
 * @return        Operation result.
 */
gerium_api gerium_result_t
gerium_application_get_display_info(gerium_application_t application,
                                    gerium_uint32_t* display_count,
                                    gerium_display_info_t* displays);

/**
 * @brief     Checks fullscreen state.
 * @details   Determines if window is fullscreen.
 * @param[in] application Target application.
 * @return    *TRUE* if in fullscreen mode.
 */
gerium_api gerium_bool_t
gerium_application_is_fullscreen(gerium_application_t application);

/**
 * @brief     Change fullscreen state.
 * @details   Change fullscreen state and display mode.
 * @param[in] application Target application.
 * @param[in] fullscreen Change to fullscreen.
 * @param[in] display_id Display id.
 * @param[in] mode Mode, may be null.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_application_fullscreen(gerium_application_t application,
                              gerium_bool_t fullscreen,
                              gerium_uint32_t display_id,
                              const gerium_display_mode_t* mode);

/**
 * @brief     Gets window style.
 * @details   Retrieves current window decoration style.
 * @param[in] application Target application.
 * @return    Current window style flags.
 */
gerium_api gerium_application_style_flags_t
gerium_application_get_style(gerium_application_t application);

/**
 * @brief     Sets window style.
 * @details   Changes window decoration and behavior.
 * @param[in] application Target application.
 * @param[in] style New style flags.
 */
gerium_api void
gerium_application_set_style(gerium_application_t application,
                             gerium_application_style_flags_t style);

/**
 * @brief      Gets minimum window size
 * @details    Retrieves smallest allowed window dimensions.
 * @param[in]  application Target application.
 * @param[out] width Receives minimum width.
 * @param[out] height Receives minimum height.
 */
gerium_api void
gerium_application_get_min_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

/**
 * @brief     Sets minimum window size.
 * @details   Constrains window resizing below these dimensions.
 * @param[in] application Target application.
 * @param[in] width Minimum width in pixels.
 * @param[in] height Minimum height in pixels.
 */
gerium_api void
gerium_application_set_min_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

/**
 * @brief      Gets maximum window size.
 * @details    Retrieves largest allowed window dimensions.
 * @param[in]  application Target application.
 * @param[out] width Receives maximum width.
 * @param[out] height Receives maximum height.
 */
gerium_api void
gerium_application_get_max_size(gerium_application_t application,
                                gerium_uint16_t* width,
                                gerium_uint16_t* height);

/**
 * @brief     Sets maximum window size.
 * @details   Constrains window resizing above these dimensions.
 * @param[in] application Target application.
 * @param[in] width Maximum width in pixels.
 * @param[in] height Maximum height in pixels.
 */
gerium_api void
gerium_application_set_max_size(gerium_application_t application,
                                gerium_uint16_t width,
                                gerium_uint16_t height);

/**
 * @brief      Gets current window size.
 * @details    Retrieves actual window dimensions.
 * @param[in]  application Target application.
 * @param[out] width Receives current width.
 * @param[out] height Receives current height.
 */
gerium_api void
gerium_application_get_size(gerium_application_t application,
                            gerium_uint16_t* width,
                            gerium_uint16_t* height);

/**
 * @brief     Changes window size.
 * @details   Resizes window to specified dimensions.
 * @param[in] application Target application.
 * @param[in] width New width in pixels.
 * @param[in] height New height in pixels.
 * @note      May be constrained by min/max sizes.
 */
gerium_api void
gerium_application_set_size(gerium_application_t application,
                            gerium_uint16_t width,
                            gerium_uint16_t height);

/**
 * @brief     Gets window title.
 * @details   Retrieves current window title text.
 * @param[in] application Target application instance.
 * @return    Current title string.
 */
gerium_api gerium_utf8_t
gerium_application_get_title(gerium_application_t application);

/**
 * @brief     Sets window title.
 * @details   Updates the window title bar text.
 * @param[in] application Target application.
 * @param[in] title New title text.
 */
gerium_api void
gerium_application_set_title(gerium_application_t application,
                             gerium_utf8_t title);

/**
 * @brief     Gets background wait policy.
 * @details   Checks if application sleeps when inactive.
 * @param[in] application Target application.
 * @return    *TRUE* if suspends when backgrounded
 */
gerium_api gerium_bool_t
gerium_application_get_background_wait(gerium_application_t application);

/**
 * @brief     Sets background wait policy.
 * @details   Controls whether application sleeps when inactive.
 * @param[in] application Target application.
 * @param[in] enable *TRUE* to enable background waiting.
 */
gerium_api void
gerium_application_set_background_wait(gerium_application_t application,
                                       gerium_bool_t enable);

/**
 * @brief     Checks cursor visibility.
 * @details   Determines if system cursor is visible.
 * @param[in] application Target application.
 * @return    *TRUE* if cursor is shown.
 */
gerium_api gerium_bool_t
gerium_application_is_show_cursor(gerium_application_t application);

/**
 * @brief     Shows/hides system cursor.
 * @details   Controls cursor visibility in application window.
 * @param[in] application Target application.
 * @param[in] show *TRUE* to show cursor
 */
gerium_api void
gerium_application_show_cursor(gerium_application_t application,
                               gerium_bool_t show);

/**
 * @brief     Gets display density.
 * @details   Retrieves screen DPI scaling factor.
 * @param[in] application Target application.
 * @return    Density ratio.
 */
gerium_api gerium_float32_t
gerium_application_get_density(gerium_application_t application);

/**
 * @brief     Converts dimension units.
 * @details   Translates between pixels and other units.
 * @param[in] application Target application.
 * @param[in] unit The unit to convert from.
 * @param[in] value The value to apply the unit to.
 * @return    Converted value in pixels.
 */
gerium_api gerium_float32_t
gerium_application_get_dimension(gerium_application_t application,
                                 gerium_dimension_unit_t unit,
                                 gerium_float32_t value);

/**
 * @brief     Starts main event loop.
 * @details   Begins processing system events and callbacks.
 * @param[in] application Target application.
 * @return    Execution result code.
 */
gerium_api gerium_result_t
gerium_application_run(gerium_application_t application);

/**
 * @brief     Requests application exit.
 * @details   Initiates graceful shutdown sequence.
 * @param[in] application Target application.
 */
gerium_api void
gerium_application_exit(gerium_application_t application);

/**
 * @brief      Polls system events.
 * @details    Retrieves next pending event without blocking.
 * @param[in]  application Target application.
 * @param[out] event Receives event data if available.
 * @return     *TRUE* if event was available.
 */
gerium_api gerium_bool_t
gerium_application_poll_events(gerium_application_t application,
                               gerium_event_t* event);

/**
 * @brief     Checks key state by scancode.
 * @details   Tests if specified key is currently pressed.
 * @param[in] application Target application.
 * @param[in] scancode Platform-independent key identifier.
 * @return    *TRUE* if key is pressed
 */
gerium_api gerium_bool_t
gerium_application_is_press_scancode(gerium_application_t application,
                                     gerium_scancode_t scancode);

/**
 * @brief     Executes callback in fiber.
 * @details   Schedules function for thread execution.
 * @param[in] application Target application.
 * @param[in] callback Function to execute.
 * @param[in] data Callback context data.
 */
gerium_api void
gerium_application_execute(gerium_application_t application,
                           gerium_application_executor_callback_t callback,
                           gerium_data_t data);

/**
 * @brief     Shows system message dialog.
 * @details   Displays modal alert to user.
 * @param[in] application Target application.
 * @param[in] title Dialog title.
 * @param[in] message Message content.
 * @note      Blocks current thread until dismissed.
 */
gerium_api void
gerium_application_show_message(gerium_application_t application,
                                gerium_utf8_t title,
                                gerium_utf8_t message);

/** @} */

/**
 * @name Functions of Renderer.
 * @brief Functions for opaque type ::gerium_renderer_t.
 * @{
 */

/**
 * @brief      Creates renderer instance.
 * @details    Initializes graphics subsystem.
 * @param[in]  application Host application.
 * @param[in]  features Requested rendering features.
 * @param[in]  options Configuration parameters.
 * @param[out] renderer New renderer.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_create(gerium_application_t application,
                       gerium_feature_flags_t features,
                       const gerium_renderer_options_t* options,
                       gerium_renderer_t* renderer);

/**
 * @brief     Increments reference count.
 * @details   Manages renderer instance lifetime.
 * @param[in] renderer Target renderer.
 * @return    Reference to same renderer.
 * @sa        ::gerium_renderer_destroy
 */
gerium_api gerium_renderer_t
gerium_renderer_reference(gerium_renderer_t renderer);

/**
 * @brief     Releases renderer instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] renderer Renderer to destroy
 * @note      Destroys all GPU resources and contexts.
 * @sa        ::gerium_renderer_reference
 */
gerium_api void
gerium_renderer_destroy(gerium_renderer_t renderer);

/**
 * @brief     Gets active features.
 * @details   Retrieves actually supported features.
 * @param[in] renderer Target renderer.
 * @return    Bitmask of enabled features.
 */
gerium_api gerium_feature_flags_t
gerium_renderer_get_enabled_features(gerium_renderer_t renderer);

/**
 * @brief     Checks profiling state.
 * @details   Determines if performance metrics are collected.
 * @param[in] renderer Target renderer.
 * @return    *TRUE* if profiling enabled.
 */
gerium_api gerium_bool_t
gerium_renderer_get_profiler_enable(gerium_renderer_t renderer);

/**
 * @brief     Enables/disables profiling.
 * @details   Controls performance metric collection.
 * @param[in] renderer Target renderer.
 * @param[in] enable profiling state.
 */
gerium_api void
gerium_renderer_set_profiler_enable(gerium_renderer_t renderer,
                                    gerium_bool_t enable);

/**
 * @brief     Gets Number of frames in flight.
 * @details   Returns number of simultaneous in-flight frames.
 * @param[in] renderer Target renderer.
 * @return    Frames-in-flight count.
 */
gerium_api gerium_uint32_t
gerium_renderer_get_frames_in_flight(gerium_renderer_t renderer);

/**
 * @brief     Checks format support.
 * @details   Verifies if texture format is supported.
 * @param[in] renderer Target renderer.
 * @param[in] format Texture format to check.
 * @return    *TRUE* if format is supported.
 */
gerium_api gerium_bool_t
gerium_renderer_is_supported_format(gerium_renderer_t renderer,
                                    gerium_format_t format);

/**
 * @brief      Retrieves texture information.
 * @details    Gets metadata for existing texture.
 * @param[in]  renderer Target renderer.
 * @param[in]  handle Texture to inspect.
 * @param[out] info Output texture info.
 */
gerium_api void
gerium_renderer_get_texture_info(gerium_renderer_t renderer,
                                 gerium_texture_h handle,
                                 gerium_texture_info_t* info);

/**
 * @brief      Creates GPU buffer.
 * @details    Allocates and initializes buffer resource.
 * @param[in]  renderer Target renderer.
 * @param[in]  buffer_usage Usage flags.
 * @param[in]  dynamic Allocate the dynamic buffer.
 * @param[in]  name Debug name.
 * @param[in]  data Initial data. May be null.
 * @param[in]  size Buffer size in bytes.
 * @param[out] handle New buffer handle.
 * @return     Operation result.
 * @note       A dynamic buffer is available only for one frame. It is effective for frequently changing data or
 *             as an intermediate buffer. Dynamic buffer is allocated much more efficiently than static. A large
 *             number of buffers can be allocated per frame. For maximum sizes, use limits settings with
 *             gerium_renderer_options_t::dynamic_ubo_size and gerium_renderer_options_t::dynamic_ssbo_size.
 */
gerium_api gerium_result_t
gerium_renderer_create_buffer(gerium_renderer_t renderer,
                              gerium_buffer_usage_flags_t buffer_usage,
                              gerium_bool_t dynamic,
                              gerium_utf8_t name,
                              gerium_cdata_t data,
                              gerium_uint32_t size,
                              gerium_buffer_h* handle);

/**
 * @brief      Creates texture resource.
 * @details    Allocates and initializes texture.
 * @param[in]  renderer Target renderer.
 * @param[in]  info Texture properties.
 * @param[in]  data Initial data.
 * @param[out] handle New texture handle.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_create_texture(gerium_renderer_t renderer,
                               const gerium_texture_info_t* info,
                               gerium_cdata_t data,
                               gerium_texture_h* handle);

/**
 * @brief      Creates texture view.
 * @details    Makes alternative view of existing texture.
 * @param[in]  renderer Target renderer.
 * @param[in]  texture Source texture.
 * @param[in]  type View type.
 * @param[in]  mip_base_level First mip level.
 * @param[in]  mip_level_count Number of mip levels.
 * @param[in]  layer_base First array layer.
 * @param[in]  layer_count Number of layers.
 * @param[in]  name Debug identifier.
 * @param[out] handle New texture view handle.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_create_texture_view(gerium_renderer_t renderer,
                                    gerium_texture_h texture,
                                    gerium_texture_type_t type,
                                    gerium_uint16_t mip_base_level,
                                    gerium_uint16_t mip_level_count,
                                    gerium_uint16_t layer_base,
                                    gerium_uint16_t layer_count,
                                    gerium_utf8_t name,
                                    gerium_texture_h* handle);

/**
 * @brief      Creates rendering technique.
 * @details    Compiles shader pipelines for rendering.
 * @param[in]  renderer Target renderer.
 * @param[in]  frame_graph Frame graph.
 * @param[in]  name Debug name.
 * @param[in]  pipeline_count Number of pipelines.
 * @param[in]  pipelines Pipeline configurations.
 * @param[out] handle New technique handle.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_create_technique(gerium_renderer_t renderer,
                                 gerium_frame_graph_t frame_graph,
                                 gerium_utf8_t name,
                                 gerium_uint32_t pipeline_count,
                                 const gerium_pipeline_t* pipelines,
                                 gerium_technique_h* handle);

/**
 * @brief      Creates descriptor set.
 * @details    Allocates resource binding points.
 * @param[in]  renderer Target renderer.
 * @param[in]  global *TRUE* for globally set.
 * @param[out] handle New descriptor set handle.
 * @return     Operation result.
 * @note       A non-global set descriptor differs from a global one in that it is allocated anew
 *             each frame. This is useful if we need to change the bindings each frame.
 */
gerium_api gerium_result_t
gerium_renderer_create_descriptor_set(gerium_renderer_t renderer,
                                      gerium_bool_t global,
                                      gerium_descriptor_set_h* handle);

/**
 * @brief      Asynchronously loads texture.
 * @details    Loads texture file in background fiber.
 * @param[in]  renderer Target renderer.
 * @param[in]  filename Image file path.
 * @param[in]  callback Completion handler.
 * @param[in]  data User context data.
 * @param[out] handle New texture handle.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_async_load_texture(gerium_renderer_t renderer,
                                   gerium_utf8_t filename,
                                   gerium_texture_loaded_callback_t callback,
                                   gerium_data_t data,
                                   gerium_texture_h* handle);

/**
 * @brief     Asynchronously uploads texture data.
 * @details   Transfers texture data in background.
 * @param[in] renderer Target renderer.
 * @param[in] handle Destination texture.
 * @param[in] texture_data Pixel data to upload.
 * @param[in] callback Completion handler.
 * @param[in] data Callback data.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_async_upload_texture_data(gerium_renderer_t renderer,
                                          gerium_texture_h handle,
                                          gerium_cdata_t texture_data,
                                          gerium_texture_loaded_callback_t callback,
                                          gerium_data_t data);

/**
 * @brief     Configures texture sampler.
 * @details   Sets filtering and addressing modes.
 * @param[in] renderer Target renderer.
 * @param[in] handle Target texture.
 * @param[in] min_filter Minification filter.
 * @param[in] mag_filter Magnification filter.
 * @param[in] mip_filter Mipmap filter.
 * @param[in] address_mode_u U addressing mode.
 * @param[in] address_mode_v V addressing mode.
 * @param[in] address_mode_w W addressing mode.
 * @param[in] reduction_mode Sample reduction mode (to use ::GERIUM_REDUCTION_MODE_MIN or ::GERIUM_REDUCTION_MODE_MAX, you must request the ::GERIUM_FEATURE_SAMPLER_FILTER_MINMAX_BIT feature).
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_texture_sampler(gerium_renderer_t renderer,
                                gerium_texture_h handle,
                                gerium_filter_t min_filter,
                                gerium_filter_t mag_filter,
                                gerium_filter_t mip_filter,
                                gerium_address_mode_t address_mode_u,
                                gerium_address_mode_t address_mode_v,
                                gerium_address_mode_t address_mode_w,
                                gerium_reduction_mode_t reduction_mode);

/**
 * @brief      Gets buffer by name.
 * @details    Get buffer from frame graph.
 * @param[in]  renderer Target renderer.
 * @param[in]  resource Resource name.
 * @param[out] handle Found buffer.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_get_buffer(gerium_renderer_t renderer,
                           gerium_utf8_t resource,
                           gerium_buffer_h* handle);

/**
 * @brief      Gets texture by name.
 * @details    Get texture from frame graph.
 * @param[in]  renderer Target renderer.
 * @param[in]  resource Resource name.
 * @param[in]  from_previous_frame Take resource from previous frame.
 * @param[out] handle Found texture.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_get_texture(gerium_renderer_t renderer,
                            gerium_utf8_t resource,
                            gerium_bool_t from_previous_frame,
                            gerium_texture_h* handle);

/**
 * @brief     Destroys buffer.
 * @details   Releases GPU buffer resources.
 * @param[in] renderer Target renderer.
 * @param[in] handle Buffer to destroy.
 * @note      You can release a resource directly in the frame where it was used. The rendering
 *            system will release the resource when the GPU stops using it.
 */
gerium_api void
gerium_renderer_destroy_buffer(gerium_renderer_t renderer,
                               gerium_buffer_h handle);

/**
 * @brief     Destroys texture.
 * @details   Releases GPU texture resources.
 * @param[in] renderer Target renderer.
 * @param[in] handle Texture to destroy.
 * @note      You can release a resource directly in the frame where it was used. The rendering
 *            system will release the resource when the GPU stops using it.
 */
gerium_api void
gerium_renderer_destroy_texture(gerium_renderer_t renderer,
                                gerium_texture_h handle);

/**
 * @brief     Destroys technique.
 * @details   Releases pipeline resources.
 * @param[in] renderer Target renderer.
 * @param[in] handle Technique to destroy.
 * @note      You can release a resource directly in the frame where it was used. The rendering
 *            system will release the resource when the GPU stops using it.
 */
gerium_api void
gerium_renderer_destroy_technique(gerium_renderer_t renderer,
                                  gerium_technique_h handle);

/**
 * @brief     Destroys descriptor set.
 * @details   Releases resource binding points.
 * @param[in] renderer Target renderer.
 * @param[in] handle Descriptor set to destroy.
 * @note      You can release a resource directly in the frame where it was used. The rendering
 *            system will release the resource when the GPU stops using it.
 */
gerium_api void
gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer,
                                       gerium_descriptor_set_h handle);

/**
 * @brief     Binds buffer to descriptor set.
 * @details   Attaches buffer to binding point.
 * @param[in] renderer Target renderer.
 * @param[in] handle Descriptor set.
 * @param[in] binding Binding index.
 * @param[in] buffer Buffer to bind.
 */
gerium_api void
gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                            gerium_descriptor_set_h handle,
                            gerium_uint16_t binding,
                            gerium_buffer_h buffer);

/**
 * @brief     Binds texture to descriptor set.
 * @details   Attaches texture to binding point.
 * @param[in] renderer Target renderer.
 * @param[in] handle Descriptor set.
 * @param[in] binding Binding index.
 * @param[in] element Array element.
 * @param[in] texture Texture to bind.
 */
gerium_api void
gerium_renderer_bind_texture(gerium_renderer_t renderer,
                             gerium_descriptor_set_h handle,
                             gerium_uint16_t binding,
                             gerium_uint16_t element,
                             gerium_texture_h texture);

/**
 * @brief     Binds resource by name.
 * @details   Attaches named resource to binding point.
 * @param[in] renderer Target renderer.
 * @param[in] handle Descriptor set.
 * @param[in] binding Binding index.
 * @param[in] resource_input Resource name.
 * @param[in] from_previous_frame Take resource from previous frame.
 */
gerium_api void
gerium_renderer_bind_resource(gerium_renderer_t renderer,
                              gerium_descriptor_set_h handle,
                              gerium_uint16_t binding,
                              gerium_utf8_t resource_input,
                              gerium_bool_t from_previous_frame);

/**
 * @brief     Maps buffer memory.
 * @details   Provides CPU access to GPU buffer.
 * @param[in] renderer Target renderer.
 * @param[in] handle Buffer to map.
 * @param[in] offset Byte offset.
 * @param[in] size Mapping size.
 * @return    Mapped memory pointer.
 */
gerium_api gerium_data_t
gerium_renderer_map_buffer(gerium_renderer_t renderer,
                           gerium_buffer_h handle,
                           gerium_uint32_t offset,
                           gerium_uint32_t size);

/**
 * @brief     Unmaps buffer memory.
 * @details   Finalizes CPU buffer modifications.
 * @param[in] renderer Target renderer.
 * @param[in] handle Buffer to unmap.
 */
gerium_api void
gerium_renderer_unmap_buffer(gerium_renderer_t renderer,
                             gerium_buffer_h handle);

/**
 * @brief     Begins new frame.
 * @details   Prepares renderer for new frame.
 * @param[in] renderer Target renderer.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_new_frame(gerium_renderer_t renderer);

/**
 * @brief     Executes frame rendering.
 * @details   Performs passes on the frame graph.
 * @param[in] renderer Target renderer.
 * @param[in] frame_graph Frame graph.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_renderer_render(gerium_renderer_t renderer,
                       gerium_frame_graph_t frame_graph);

/**
 * @brief     Presents frame.
 * @details   Displays rendered frame.
 * @param[in] renderer Target renderer.
 * @return    Operation result.
 * @note      Note that the ::gerium_renderer_present does not wait for the GPU frame to be rendered. Instead,
 *            the rendering system allows the next frame to be rendered (you can call ::gerium_renderer_new_frame).
 */
gerium_api gerium_result_t
gerium_renderer_present(gerium_renderer_t renderer);

/** @} */

/**
 * @name Functions of Command Buffer.
 * @brief Functions for opaque type ::gerium_command_buffer_t.
 * @{
 */

/**
 * @brief     Set the viewport.
 * @details   Set the viewport transformation parameters.
 * @param[in] command_buffer Target command buffer.
 * @param[in] x Left viewport coordinate.
 * @param[in] y Top viewport coordinate.
 * @param[in] width Viewport width.
 * @param[in] height Viewport height.
 * @param[in] min_depth Minimum depth value (typically `0.0`).
 * @param[in] max_depth Maximum depth value (typically `1.0`).
 */
gerium_api void
gerium_command_buffer_set_viewport(gerium_command_buffer_t command_buffer,
                                   gerium_uint16_t x,
                                   gerium_uint16_t y,
                                   gerium_uint16_t width,
                                   gerium_uint16_t height,
                                   gerium_float32_t min_depth,
                                   gerium_float32_t max_depth);

/**
 * @brief     Sets scissor rectangle.
 * @details   Defines the rectangular region where rendering can occur.
 * @param[in] command_buffer Target command buffer.
 * @param[in] x Left scissor coordinate.
 * @param[in] y Top scissor coordinate.
 * @param[in] width Scissor width.
 * @param[in] height Scissor height.
 */
gerium_api void
gerium_command_buffer_set_scissor(gerium_command_buffer_t command_buffer,
                                  gerium_uint16_t x,
                                  gerium_uint16_t y,
                                  gerium_uint16_t width,
                                  gerium_uint16_t height);

/**
 * @brief     Binds rendering technique.
 * @details   Sets the active shader pipeline state for render pass.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Technique to bind.
 */
gerium_api void
gerium_command_buffer_bind_technique(gerium_command_buffer_t command_buffer,
                                     gerium_technique_h handle);

/**
 * @brief     Bind vertex buffer.
 * @details   Bind an vertex buffer to a command buffer for use in subsequent drawing commands.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Vertex buffer handle.
 * @param[in] binding Binding index.
 * @param[in] offset Buffer offset.
 */
gerium_api void
gerium_command_buffer_bind_vertex_buffer(gerium_command_buffer_t command_buffer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t binding,
                                         gerium_uint32_t offset);

/**
 * @brief     Bind index buffer.
 * @details   Bind an index buffer to a command buffer.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Index buffer handle.
 * @param[in] offset Buffer offset.
 * @param[in] type Type of indices (16-bit or 32-bit).
 */
gerium_api void
gerium_command_buffer_bind_index_buffer(gerium_command_buffer_t command_buffer,
                                        gerium_buffer_h handle,
                                        gerium_uint32_t offset,
                                        gerium_index_type_t type);

/**
 * @brief     Bind descriptor set.
 * @details   Bind descriptor set to a command buffer.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Descriptor set handle.
 * @param[in] set Set number.
 */
gerium_api void
gerium_command_buffer_bind_descriptor_set(gerium_command_buffer_t command_buffer,
                                          gerium_descriptor_set_h handle,
                                          gerium_uint32_t set);

/**
 * @brief     Dispatches compute work.
 * @details   Executes compute shader with specified workgroup counts.
 * @param[in] command_buffer Target command buffer.
 * @param[in] group_x Number of local workgroups to dispatch in the X dimension.
 * @param[in] group_y Number of local workgroups to dispatch in the X dimension.
 * @param[in] group_z Number of local workgroups to dispatch in the X dimension.
 */
gerium_api void
gerium_command_buffer_dispatch(gerium_command_buffer_t command_buffer,
                               gerium_uint32_t group_x,
                               gerium_uint32_t group_y,
                               gerium_uint32_t group_z);

/**
 * @brief     Draw primitives.
 * @details   Draws non-indexed primitives.
 * @param[in] command_buffer Target command buffer.
 * @param[in] first_vertex Index of the first vertex to draw
 * @param[in] vertex_count Number of vertices to draw.
 * @param[in] first_instance Instance ID of the first instance to draw.
 * @param[in] instance_count Number of instances to draw.
 */
gerium_api void
gerium_command_buffer_draw(gerium_command_buffer_t command_buffer,
                           gerium_uint32_t first_vertex,
                           gerium_uint32_t vertex_count,
                           gerium_uint32_t first_instance,
                           gerium_uint32_t instance_count);

/**
 * @brief     Draw primitives with indirect parameters.
 * @details   Draws non-indexed primitives using indirect parameters.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Buffer containing draw parameters.
 * @param[in] offset Byte offset into *handle* where parameters begin.
 * @param[in] draw_count_handle Buffer containing the draw count
 * @param[in] draw_count_offset Byte offset into *draw_count_handle* where the draw count begins.
 * @param[in] draw_count Number of draws to execute, and can be zero.
 * @param[in] stride Byte stride between successive sets of draw parameters.
 * @parblock
 * @note      The *handle* buffer should contain a list of GPU commands starting at *offset*
 *            bytes and separated by *stride* bytes with the following structure:
 *            
 *                struct DrawIndirectCommand {
 *                    uint vertexCount;
 *                    uint instanceCount;
 *                    uint firstVertex;
 *                    uint firstInstance;
 *                };
 * @endparblock
 * @parblock
 * @note      If the value *draw_count_handle* is passed (i.e. not equal to 65535), then the
 *            number of draws will be read from *draw_count_handle* as uint by offset *draw_count_offset*
 * @endparblock
 * @parblock
 * @note      To use ::gerium_command_buffer_draw_indirect, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_BIT feature.
 * @endparblock
 * @parblock
 * @note      To use *draw_count_handle*, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT feature.
 * @endparblock
 * @parblock
 * @note      The *handle* and *draw_count_handle* buffers must be created with the ::GERIUM_BUFFER_USAGE_INDIRECT_BIT flag.
 * @endparblock
 */
gerium_api void
gerium_command_buffer_draw_indirect(gerium_command_buffer_t command_buffer,
                                    gerium_buffer_h handle,
                                    gerium_uint32_t offset,
                                    gerium_buffer_h draw_count_handle,
                                    gerium_uint32_t draw_count_offset,
                                    gerium_uint32_t draw_count,
                                    gerium_uint32_t stride);

/**
 * @brief     Draws indexed primitives.
 * @details   Draw primitives with indexed vertices.
 * @param[in] command_buffer Target command buffer.
 * @param[in] first_index Base index within the index buffer.
 * @param[in] index_count Number of vertices to draw.
 * @param[in] vertex_offset Value added to the vertex index before indexing into the vertex buffer.
 * @param[in] first_instance Instance ID of the first instance to draw.
 * @param[in] instance_count Number of instances to draw.
 */
gerium_api void
gerium_command_buffer_draw_indexed(gerium_command_buffer_t command_buffer,
                                   gerium_uint32_t first_index,
                                   gerium_uint32_t index_count,
                                   gerium_uint32_t vertex_offset,
                                   gerium_uint32_t first_instance,
                                   gerium_uint32_t instance_count);

/**
 * @brief     Draws indexed primitives with indirect parameters.
 * @details   Draw primitives with indirect parameters and indexed vertices.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Buffer containing draw parameters.
 * @param[in] offset Byte offset into buffer where parameters begin.
 * @param[in] draw_count_handle Buffer containing the draw count.
 * @param[in] draw_count_offset Byte offset into countBuffer where the draw count begins.
 * @param[in] draw_count Number of draws to execute, and can be zero.
 * @param[in] stride Byte stride between successive sets of draw parameters.
 * @parblock
 * @note      The *handle* buffer should contain a list of GPU commands starting at *offset*
 *            bytes and separated by *stride* bytes with the following structure:
 *            
 *                struct DrawIndexedIndirectCommand {
 *                    uint indexCount;
 *                    uint instanceCount;
 *                    uint firstIndex;
 *                    int  vertexOffset;
 *                    uint firstInstance;
 *                };
 * @endparblock
 * @parblock
 * @note      If the value *draw_count_handle* is passed (i.e. not equal to 65535), then the
 *            number of draws will be read from *draw_count_handle* as uint by offset *draw_count_offset*
 * @endparblock
 * @parblock
 * @note      To use ::gerium_command_buffer_draw_indexed_indirect, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_BIT feature.
 * @endparblock
 * @parblock
 * @note      To use *draw_count_handle*, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT feature.
 * @endparblock
 * @parblock
 * @note      The *handle* and *draw_count_handle* buffers must be created with the ::GERIUM_BUFFER_USAGE_INDIRECT_BIT flag.
 * @endparblock
 */
gerium_api void
gerium_command_buffer_draw_indexed_indirect(gerium_command_buffer_t command_buffer,
                                            gerium_buffer_h handle,
                                            gerium_uint32_t offset,
                                            gerium_buffer_h draw_count_handle,
                                            gerium_uint32_t draw_count_offset,
                                            gerium_uint32_t draw_count,
                                            gerium_uint32_t stride);

/**
 * @brief     Draw mesh task work items.
 * @details   Executes mesh shader with specified workgroup counts. When the command is
 *            executed, a global workgroup consisting of groupCountX × groupCountY ×
 *            groupCountZ local workgroups is assembled
 * @param[in] command_buffer Target command buffer.
 * @param[in] group_x Number of local workgroups to dispatch in the X dimension.
 * @param[in] group_y Number of local workgroups to dispatch in the Y dimension.
 * @param[in] group_z Number of local workgroups to dispatch in the Z dimension.
 * @note      To use ::gerium_command_buffer_draw_mesh_tasks, you need to request the ::GERIUM_FEATURE_MESH_SHADER_BIT feature.
 */
gerium_api void
gerium_command_buffer_draw_mesh_tasks(gerium_command_buffer_t command_buffer,
                                      gerium_uint32_t group_x,
                                      gerium_uint32_t group_y,
                                      gerium_uint32_t group_z);

/**
 * @brief     Dispatches mesh shading work with indirect parameters.
 * @details   Executes mesh shader with indirect workgroup counts.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Buffer containing draw parameters.
 * @param[in] offset Byte offset into buffer where parameters begin.
 * @param[in] draw_count_handle Buffer containing the draw count.
 * @param[in] draw_count_offset Byte offset into countBuffer where the draw count begins.
 * @param[in] draw_count Number of draws to execute, and can be zero.
 * @param[in] stride Byte stride between successive sets of draw parameters.
 * @parblock
 * @note      The *handle* buffer should contain a list of GPU commands starting at *offset*
 *            bytes and separated by *stride* bytes with the following structure:
 *            
 *                struct DrawMeshTasksIndirectCommand {
 *                    uint groupCountX;
 *                    uint groupCountY;
 *                    uint groupCountZ;
 *                };
 * @endparblock
 * @parblock
 * @note      If the value *draw_count_handle* is passed (i.e. not equal to 65535), then the
 *            number of draws will be read from *draw_count_handle* as uint by offset *draw_count_offset*
 * @endparblock
 * @parblock
 * @note      To use ::gerium_command_buffer_draw_mesh_tasks_indirect, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_BIT and ::GERIUM_FEATURE_MESH_SHADER_BIT features.
 * @endparblock
 * @parblock
 * @note      To use *draw_count_handle*, you need to request the ::GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT feature.
 * @endparblock
 * @parblock
 * @note      The *handle* and *draw_count_handle* buffers must be created with the ::GERIUM_BUFFER_USAGE_INDIRECT_BIT flag.
 * @endparblock
 */
gerium_api void
gerium_command_buffer_draw_mesh_tasks_indirect(gerium_command_buffer_t command_buffer,
                                               gerium_buffer_h handle,
                                               gerium_uint32_t offset,
                                               gerium_buffer_h draw_count_handle,
                                               gerium_uint32_t draw_count_offset,
                                               gerium_uint32_t draw_count,
                                               gerium_uint32_t stride);

/**
 * @brief         Draw profiler UI.
 * @details       Renders performance metrics overlay.
 * @param[in]     command_buffer Target command buffer.
 * @param[in,out] show Flag storing the visibility state of the metrics window; if null, the user will not be shown a cross to close the window.
 */
gerium_api void
gerium_command_buffer_draw_profiler(gerium_command_buffer_t command_buffer,
                                    gerium_bool_t* show);

/**
 * @brief     Fills buffer.
 * @details   Fill a region of a buffer with a fixed value.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Buffer to fill.
 * @param[in] offset Byte offset into the buffer at which to start filling, and must be a multiple of 4.
 * @param[in] size Number of bytes to fill, and must be either a multiple of 4.
 * @param[in] data The 4-byte word written repeatedly to the buffer to fill size bytes of data. The data word is written to memory according to the host endianness.
 */
gerium_api void
gerium_command_buffer_fill_buffer(gerium_command_buffer_t command_buffer,
                                  gerium_buffer_h handle,
                                  gerium_uint32_t offset,
                                  gerium_uint32_t size,
                                  gerium_uint32_t data);

/**
 * @brief     Copy data between buffer regions.
 * @details   To copy data between buffer objects.
 * @param[in] command_buffer Target command buffer.
 * @param[in] source_handle Source buffer.
 * @param[in] source_offset Source starting byte offset.
 * @param[in] dest_handle Destination buffer.
 * @param[in] dest_offset Destination starting byte offset.
 * @param[in] size Number of bytes to copy.
 */
gerium_api void
gerium_command_buffer_copy_buffer(gerium_command_buffer_t command_buffer,
                                  gerium_buffer_h source_handle,
                                  gerium_uint32_t source_offset,
                                  gerium_buffer_h dest_handle,
                                  gerium_uint32_t dest_offset,
                                  gerium_uint32_t size);

/**
 * @brief     Inserts write barrier for buffer.
 * @details   Ensures previous writes to buffer are completed.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Buffer to synchronize.
 * @note      Please note that the frame graph automatically places barriers in the right places,
 *            including for external resources. But there are situations when within one pass,
 *            for example, a computational shader, it is necessary to read and write to the
 *            texture several times in succession. For example, when computing a depth pyramid,
 *            which is quite acceptable to do within one computational pass.
 */
gerium_api void
gerium_command_buffer_barrier_buffer_write(gerium_command_buffer_t command_buffer,
                                           gerium_buffer_h handle);

/**
 * @brief     Inserts write barrier for texture.
 * @details   Ensures previous writes to texture are completed.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Texture to synchronize.
 * @note      Please note that the frame graph automatically places barriers in the right places,
 *            including for external resources. But there are situations when within one pass,
 *            for example, a computational shader, it is necessary to read and write to the
 *            texture several times in succession. For example, when computing a depth pyramid,
 *            which is quite acceptable to do within one computational pass.
 */
gerium_api void
gerium_command_buffer_barrier_buffer_read(gerium_command_buffer_t command_buffer,
                                          gerium_buffer_h handle);

/**
 * @brief     Inserts write barrier for texture.
 * @details   Ensures previous writes to texture are completed.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Texture to synchronize.
 * @note      Please note that the frame graph automatically places barriers in the right places,
 *            including for external resources. But there are situations when within one pass,
 *            for example, a computational shader, it is necessary to read and write to the
 *            texture several times in succession. For example, when computing a depth pyramid,
 *            which is quite acceptable to do within one computational pass.
 */
gerium_api void
gerium_command_buffer_barrier_texture_write(gerium_command_buffer_t command_buffer,
                                            gerium_texture_h handle);

/**
 * @brief     Inserts read barrier for texture.
 * @details   Ensures previous reads from texture are completed.
 * @param[in] command_buffer Target command buffer.
 * @param[in] handle Texture to synchronize.
 * @note      Please note that the frame graph automatically places barriers in the right places,
 *            including for external resources. But there are situations when within one pass,
 *            for example, a computational shader, it is necessary to read and write to the
 *            texture several times in succession. For example, when computing a depth pyramid,
 *            which is quite acceptable to do within one computational pass.
 */
gerium_api void
gerium_command_buffer_barrier_texture_read(gerium_command_buffer_t command_buffer,
                                           gerium_texture_h handle);

/** @} */

/**
 * @name Functions of Frame Graph.
 * @brief Functions for opaque type ::gerium_frame_graph_t.
 * @{
 */

/**
 * @brief      Creates frame graph instance.
 * @details    Initializes render pass dependency graph system.
 * @param[in]  renderer Associated renderer instance.
 * @param[out] frame_graph New frame graph instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_create(gerium_renderer_t renderer,
                          gerium_frame_graph_t* frame_graph);

/**
 * @brief     Increments reference count.
 * @details   Manages frame graph instance lifetime.
 * @param[in] frame_graph Target frame graph.
 * @return    Reference to same frame graph.
 * @sa        ::gerium_frame_graph_destroy
 */
gerium_api gerium_frame_graph_t
gerium_frame_graph_reference(gerium_frame_graph_t frame_graph);

/**
 * @brief     Releases frame graph instance.
 * @details   Destroys when reference count reaches zero.
 * @param[in] frame_graph Frame graph to destroy.
 * @sa        ::gerium_frame_graph_reference
 */
gerium_api void
gerium_frame_graph_destroy(gerium_frame_graph_t frame_graph);

/**
 * @brief     Adds render pass to graph.
 * @details   Registers a render pass.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Unique pass identifier.
 * @param[in] render_pass Render pass description.
 * @param[in] data User-defined pass data.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_add_pass(gerium_frame_graph_t frame_graph,
                            gerium_utf8_t name,
                            const gerium_render_pass_t* render_pass,
                            gerium_data_t data);

/**
 * @brief     Removes render pass from graph.
 * @details   Unregisters previously added render pass.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Name of pass to remove.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_remove_pass(gerium_frame_graph_t frame_graph,
                               gerium_utf8_t name);

/**
 * @brief     Adds node to graph.
 * @details   Creates a node with the specified inputs/outputs.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Node identifier.
 * @param[in] compute Whether node is compute.
 * @param[in] input_count Number of input resources.
 * @param[in] inputs Input resource descriptions.
 * @param[in] output_count Number of output resources.
 * @param[in] outputs Output resource descriptions.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                            gerium_utf8_t name,
                            gerium_bool_t compute,
                            gerium_uint32_t input_count,
                            const gerium_resource_input_t* inputs,
                            gerium_uint32_t output_count,
                            const gerium_resource_output_t* outputs);

/**
 * @brief     Enables or disables graph node.
 * @details   Controls whether node participates in graph execution.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Node identifier.
 * @param[in] enable New state.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_enable_node(gerium_frame_graph_t frame_graph,
                               gerium_utf8_t name,
                               gerium_bool_t enable);

/**
 * @brief     Registers external buffer resource.
 * @details   In order for the frame graph to be able to control the synchronization of an external resource, it must be added to the graph.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Resource name.
 * @param[in] handle Buffer handle.
 * @return    Operation result.
 * @note      Please note that, unlike resources created by the frame graph itself, the frame graph is not responsible for the creation and release of external resources.
 */
gerium_api gerium_result_t
gerium_frame_graph_add_buffer(gerium_frame_graph_t frame_graph,
                              gerium_utf8_t name,
                              gerium_buffer_h handle);

/**
 * @brief     Registers external texture resource.
 * @details   In order for the frame graph to be able to control the synchronization of an external resource, it must be added to the graph.
 * @param[in] frame_graph Target frame graph.
 * @param[in] name Resource name.
 * @param[in] handle Texture handle.
 * @return    Operation result.
 * @note      Please note that, unlike resources created by the frame graph itself, the frame graph is not responsible for the creation and release of external resources.
 */
gerium_api gerium_result_t
gerium_frame_graph_add_texture(gerium_frame_graph_t frame_graph,
                               gerium_utf8_t name,
                               gerium_texture_h handle);

/**
 * @brief     Clears frame graph.
 * @details   Removes all passes, nodes and resources.
 * @param[in] frame_graph Target frame graph.
 */
gerium_api void
gerium_frame_graph_clear(gerium_frame_graph_t frame_graph);

/**
 * @brief     Compiles frame graph.
 * @details   Performs dependency analysis and optimization.
 * @param[in] frame_graph Target frame graph.
 * @return    Operation result.
 */
gerium_api gerium_result_t
gerium_frame_graph_compile(gerium_frame_graph_t frame_graph);

/** @} */

/**
 * @name Functions of Profiler.
 * @brief Functions for opaque type ::gerium_profiler_t.
 * @{
 */

/**
 * @brief      Creates profiler instance.
 * @details    Initializes performance measurement system.
 * @param[in]  renderer Associated renderer.
 * @param[out] profiler New profiler instance.
 * @return     Operation result.
 */
gerium_api gerium_result_t
gerium_profiler_create(gerium_renderer_t renderer,
                       gerium_profiler_t* profiler);

/**
 * @brief     Increments reference count.
 * @details   Manages profiler instance lifetime.
 * @param[in] profiler Target profiler.
 * @return    Reference to same profiler.
 * @sa        ::gerium_profiler_destroy
 */
gerium_api gerium_profiler_t
gerium_profiler_reference(gerium_profiler_t profiler);

/**
 * @brief     Increments reference count.
 * @details   Manages profiler instance lifetime.
 * @param[in] profiler Profiler to destroy.
 * @return    Reference to same profiler.
 * @sa        ::gerium_profiler_reference
 */
gerium_api void
gerium_profiler_destroy(gerium_profiler_t profiler);

/**
 * @brief         Retrieves GPU timestamps.
 * @details       Gets detailed timing measurements.
 * @param[in]     profiler Target profiler.
 * @param[in,out] gpu_timestamps_count Receives timestamp count (if *gpu_timestamps* is null, then *gpu_timestamps_count* will be set to the available count).
 * @param[out]    gpu_timestamps Receives timing data.
 */
gerium_api void
gerium_profiler_get_gpu_timestamps(gerium_profiler_t profiler,
                                   gerium_uint32_t* gpu_timestamps_count,
                                   gerium_gpu_timestamp_t* gpu_timestamps);

/**
 * @brief     Gets total GPU memory usage.
 * @details   Returns current GPU memory consumption.
 * @param[in] profiler Target profiler.
 * @return    Memory used in bytes.
 */
gerium_api gerium_uint32_t
gerium_profiler_get_gpu_total_memory_used(gerium_profiler_t profiler);

/** @} */

GERIUM_END

#endif /* GERIUM_CORE_H */
