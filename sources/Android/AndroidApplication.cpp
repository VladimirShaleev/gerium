#include "AndroidApplication.hpp"

namespace gerium::android {

static android_app* app = nullptr;

AndroidApplication::AndroidApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
    _application(app),
    _initialized(false),
    _activated(false),
    _focused(false),
    _exit(false),
    _callbackError(false),
    _isInMultiWindowMode(nullptr) {
    assert(_application);
    _application->userData = alias_cast<void*>(this);
    _application->onAppCmd = onAppCmd;

    auto activity = _application->activity;
    if (activity->sdkVersion >= 24) {
        if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            auto classActivity   = env->FindClass("android/app/NativeActivity");
            _isInMultiWindowMode = env->GetMethodID(classActivity, "isInMultiWindowMode", "()Z");
            activity->vm->DetachCurrentThread();
        }
    }
}

ANativeWindow* AndroidApplication::nativeWindow() noexcept {
    return _application->window;
}

gerium_runtime_platform_t AndroidApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_ANDROID;
}

void AndroidApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    displayCount = 0;
}

bool AndroidApplication::onIsFullscreen() const noexcept {
    auto activity = _application->activity;
    auto result   = true;

    if (_isInMultiWindowMode) {
        if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            result = !env->CallBooleanMethod(_application->activity->clazz, _isInMultiWindowMode);
            activity->vm->DetachCurrentThread();
        }
    }
    return result;
}

void AndroidApplication::onFullscreen(bool fullscreen,
                                            gerium_uint32_t displayId,
                                            const gerium_display_mode_t* mode) {
}

gerium_application_style_flags_t AndroidApplication::onGetStyle() const noexcept {
    return {}; // TODO:
}

void AndroidApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {
}

void AndroidApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
}

void AndroidApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
}

void AndroidApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = gerium_uint16_t(_application->contentRect.right - _application->contentRect.left);
    }
    if (height) {
        *height = gerium_uint16_t(_application->contentRect.bottom - _application->contentRect.top);
    }
}

void AndroidApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

void AndroidApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

void AndroidApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
}

gerium_utf8_t AndroidApplication::onGetTitle() const noexcept {
    return ""; // TODO:
}

void AndroidApplication::onSetTitle(gerium_utf8_t title) noexcept {
}

void AndroidApplication::onRun() {
    if (_application->destroyRequested) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_initialized) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }

    ANativeActivity_setWindowFormat(_application->activity, WINDOW_FORMAT_RGBX_8888);

    try {
        changeState(GERIUM_APPLICATION_STATE_CREATE);
    } catch (...) {
        onExit();
        throw;
    }

    int events;
    android_poll_source* source;

    _prevTime = getCurrentTime();

    while (true) {
        while (ALooper_pollAll(isPause() ? -1 : 0, nullptr, &events, alias_cast<void**>(&source)) >= 0) {
            if (source != nullptr) {
                source->process(_application, source);
            }

            if (_application->destroyRequested) {
                if (_callbackError) {
                    error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
                }
                return;
            }
        }

        if (!isPause() && _initialized && !_exit) {
            auto currentTime = getCurrentTime();

            const std::chrono::duration<float, std::milli> delta = currentTime - _prevTime;

            const auto elapsed = delta.count();
            if (elapsed == 0.0f) {
                continue;
            }
            _prevTime = currentTime;

            if (!callFrameFunc(elapsed)) {
                onExit();
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
        }
    }
}

void AndroidApplication::onExit() noexcept {
    _exit = true;
    ANativeActivity_finish(_application->activity);
}

bool AndroidApplication::onIsRunning() const noexcept {
    return _initialized;
}

void AndroidApplication::onInitImGui() {

}

void AndroidApplication::onShutdownImGui() {

}

void AndroidApplication::onNewFrameImGui() {

}

void AndroidApplication::initialize() {
    if (!_initialized && _application->window) {
        _initialized = true;
        changeState(GERIUM_APPLICATION_STATE_INITIALIZE);
    }
    active();
}

void AndroidApplication::uninitialize() {
    if (_initialized) {
        _initialized = false;
        changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
    }
}

void AndroidApplication::active() {
    if (!_activated && _application->window) {
        _activated = true;
        changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    }
}

void AndroidApplication::deactive() {
    if (_activated) {
        _activated = false;
        changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
}

void AndroidApplication::gotFocus() {
    if (!_focused) {
        _focused = true;
        changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
    }
}

void AndroidApplication::lostFocus() {
    if (_focused) {
        _focused = false;
        changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);
    }
}

bool AndroidApplication::isPause() const noexcept {
    return !_activated && getBackgroundWait();
}

void AndroidApplication::onAppCmd(int32_t cmd) noexcept {
    try {
        switch (cmd) {
            case APP_CMD_INPUT_CHANGED:
                break;

            case APP_CMD_INIT_WINDOW:
                initialize();

                if (!onIsFullscreen()) {
                    active();
                }
                break;

            case APP_CMD_TERM_WINDOW:
                if (!onIsFullscreen()) {
                    deactive();
                    uninitialize();
                }
                break;

            case APP_CMD_WINDOW_RESIZED:
                break;

            case APP_CMD_WINDOW_REDRAW_NEEDED:
                break;

            case APP_CMD_CONTENT_RECT_CHANGED:
                break;

            case APP_CMD_GAINED_FOCUS:
                if (onIsFullscreen()) {
                    active();
                }
                gotFocus();
                break;

            case APP_CMD_LOST_FOCUS:
                break;

            case APP_CMD_CONFIG_CHANGED:
                break;

            case APP_CMD_LOW_MEMORY:
                break;

            case APP_CMD_RESUME:
                break;

            case APP_CMD_SAVE_STATE:
                deactive();
                // changeState(GERIUM_APPLICATION_STATE_SAVE);
                uninitialize();
                break;

            case APP_CMD_PAUSE:
                lostFocus();

                if (onIsFullscreen()) {
                    deactive();
                }
                break;

            case APP_CMD_DESTROY:
                uninitialize();
                // changeState(GERIUM_APPLICATION_STATE_DESTROY);
                break;
        }
    } catch (...) {
        _callbackError = true;
        onExit();
    }
}

void AndroidApplication::onAppCmd(android_app* application, int32_t cmd) {
    auto app = alias_cast<AndroidApplication*>(application->userData);
    app->onAppCmd(cmd);
}

std::chrono::high_resolution_clock::time_point AndroidApplication::getCurrentTime() noexcept {
    return std::chrono::high_resolution_clock::now();
}

} // namespace gerium::android

gerium_public void android_main(android_app* state) {
    typedef int (* mainFunc)(int argc, char* argv[]);

    gerium::android::app = state;

    if (auto main = (mainFunc) dlsym(RTLD_DEFAULT, "main"); main) {
        int pid = getpid();
        char fname[256]{};
        char cmdline[256]{};
        snprintf(fname, sizeof fname, "/proc/%d/cmdline", pid);
        FILE* fp = fopen(fname, "rb");
        fgets(cmdline, sizeof cmdline, fp);
        fclose(fp);
        char* argv = cmdline;

        main(1, &argv);
    }
}

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::android;
    return gerium::android::app
        ? Object::create<AndroidApplication>(*application, title, width, height)
        : GERIUM_RESULT_ERROR_NOT_IMPLEMENTED;
}
