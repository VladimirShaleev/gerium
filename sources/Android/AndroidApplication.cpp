#include "AndroidApplication.hpp"

namespace gerium::android {

ANativeWindow* AndroidApplication::nativeWindow() noexcept {
    return onNativeWindow();
}

} // namespace gerium::android

typedef gerium_result_t
(*ApplicationCreateFunc)(gerium_utf8_t title,
                         gerium_uint32_t width,
                         gerium_uint32_t height,
                         gerium_application_t* application);

static ApplicationCreateFunc applicationCreateFunc = nullptr;

#ifdef GERIUM_ANDROID_HAS_NATIVE_APP_GLUE

static android_app* app = nullptr;

namespace gerium::android {

NativeAppGlueApplication::NativeAppGlueApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
        _application(app),
        _initialized(false),
        _activated(false),
        _focused(false),
        _exit(false),
        _callbackError(false),
        _isInMultiWindowMode(nullptr)  {
    assert(_application);
    _application->userData = alias_cast<void*>(this);
    _application->onAppCmd = onAppCmd;

    auto activity = _application->activity;
    if (activity->sdkVersion >= 24) {
        if (JNIEnv* env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            auto classActivity = env->FindClass("android/app/NativeActivity");
            _isInMultiWindowMode = env->GetMethodID(classActivity, "isInMultiWindowMode", "()Z");
            activity->vm->DetachCurrentThread();
        }
    }
}

gerium_runtime_platform_t NativeAppGlueApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_ANDROID;
}

void NativeAppGlueApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    displayCount = 0;
}

bool NativeAppGlueApplication::onIsFullscreen() const noexcept {
    auto activity = _application->activity;
    auto result = true;

    if (_isInMultiWindowMode) {
        if (JNIEnv* env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            result = !env->CallBooleanMethod(_application->activity->clazz, _isInMultiWindowMode);
            activity->vm->DetachCurrentThread();
        }
    }
    return result;
}

void NativeAppGlueApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {

}

gerium_application_style_flags_t NativeAppGlueApplication::onGetStyle() const noexcept {
    return {}; // TODO:
}

void NativeAppGlueApplication::onSetStyle(gerium_application_style_flags_t style) noexcept {

}

void NativeAppGlueApplication::onGetMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {

}

void NativeAppGlueApplication::onGetMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {

}

void NativeAppGlueApplication::onGetSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    if (width) {
        *width = gerium_uint16_t(_application->contentRect.right - _application->contentRect.left);
    }
    if (height) {
        *height = gerium_uint16_t(_application->contentRect.bottom - _application->contentRect.top);
    }
}

void NativeAppGlueApplication::onSetMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}

void NativeAppGlueApplication::onSetMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}

void NativeAppGlueApplication::onSetSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {

}

gerium_utf8_t NativeAppGlueApplication::onGetTitle() const noexcept {
    return ""; // TODO:
}

void NativeAppGlueApplication::onSetTitle(gerium_utf8_t title) noexcept {

}

void NativeAppGlueApplication::onRun() {
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
            // auto currentTime = getCurrentTime();
            auto elapsed = 0.0f; // (galaxy_float32_t) (currentTime - _lastTime);
            // _lastTime = currentTime;

            if (!callFrameFunc(elapsed)) {
                onExit();
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
        }
    }
}

void NativeAppGlueApplication::onExit() noexcept {
    _exit = true;
    ANativeActivity_finish(_application->activity);
}

ANativeWindow* NativeAppGlueApplication::onNativeWindow() noexcept {
    return _application->window;
}

void NativeAppGlueApplication::initialize() {
    if (!_initialized && _application->window) {
        _initialized = true;
        changeState(GERIUM_APPLICATION_STATE_INITIALIZE);
    }
    active();
}

void NativeAppGlueApplication::uninitialize() {
    if (_initialized) {
        _initialized = false;
        changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE);
    }
}

void NativeAppGlueApplication::active() {
    if (!_activated && _application->window) {
        _activated = true;
        changeState(GERIUM_APPLICATION_STATE_VISIBLE);
    }
}

void NativeAppGlueApplication::deactive() {
    if (_activated) {
        _activated = false;
        changeState(GERIUM_APPLICATION_STATE_INVISIBLE);
    }
}

void NativeAppGlueApplication::gotFocus() {
    if (!_focused) {
        _focused = true;
        changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS);
    }
}

void NativeAppGlueApplication::lostFocus() {
    if (_focused) {
        _focused = false;
        changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS);
    }
}

bool NativeAppGlueApplication::isPause() const noexcept {
    return !_activated && getBackgroundWait();
}

void NativeAppGlueApplication::onAppCmd(int32_t cmd) noexcept {
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

void NativeAppGlueApplication::onAppCmd(android_app* application, int32_t cmd) {
    auto app = alias_cast<NativeAppGlueApplication*>(application->userData);
    app->onAppCmd(cmd);
}

} // namespace gerium::android

extern "C" int main(int argc, char *argv[]);

static gerium_result_t applicationCreate(gerium_utf8_t title,
                                         gerium_uint32_t width,
                                         gerium_uint32_t height,
                                         gerium_application_t* application) {
    using namespace gerium;
    using namespace gerium::android;
    return Object::create<NativeAppGlueApplication>(*application, title, width, height);
}

gerium_public void android_main(android_app* state) {
    app = state;
    applicationCreateFunc = applicationCreate;

    int pid = getpid();
    char fname[256]{};
    char cmdline[256]{};
    snprintf(fname, sizeof fname, "/proc/%d/cmdline", pid);
    FILE *fp = fopen(fname, "rb");
    fgets(cmdline, sizeof cmdline, fp);
    fclose(fp);

    char* argv = cmdline;
    main(1, &argv);
}

#endif

gerium_result_t gerium_application_create(gerium_utf8_t title,
                                          gerium_uint32_t width,
                                          gerium_uint32_t height,
                                          gerium_application_t* application) {
    return applicationCreateFunc
        ? applicationCreateFunc(title, width, height, application)
        : GERIUM_RESULT_ERROR_NOT_IMPLEMENTED;
}
