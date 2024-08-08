#include "AndroidApplication.hpp"
#include "AndroidScanCodes.hpp"

#include <imgui_impl_android.h>

namespace gerium::android {

static android_app* app = nullptr;

AndroidApplication::AndroidApplication(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) :
    _application(app),
    _initialized(false),
    _activated(false),
    _focused(false),
    _exit(false),
    _isInMultiWindowMode(nullptr) {
    assert(_application);
    _application->userData     = alias_cast<void*>(this);
    _application->onAppCmd     = onAppCmd;
    _application->onInputEvent = onInputEvent;

    auto activity = _application->activity;

    if (JNIEnv * env; app->activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto activityClazz = app->activity->clazz;
        auto activityClass = env->GetObjectClass(activityClazz);

        if (activity->sdkVersion >= 24) {
            _isInMultiWindowMode = env->GetMethodID(activityClass, "isInMultiWindowMode", "()Z");
        }

        _keyEventClass         = (jclass) env->NewGlobalRef(env->FindClass("android/view/KeyEvent"));
        _keyEventCtor          = env->GetMethodID(_keyEventClass, "<init>", "(II)V");
        _getUnicodeCharMethod  = env->GetMethodID(_keyEventClass, "getUnicodeChar", "()I");
        _getUnicodeCharIMethod = env->GetMethodID(_keyEventClass, "getUnicodeChar", "(I)I");

        auto contextClass = env->FindClass("android/content/Context");
        auto windowClass  = env->FindClass("android/view/Window");
        auto viewClass    = env->FindClass("android/view/View");

        auto inputMethodServiceField =
            env->GetStaticFieldID(contextClass, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
        auto inputMethodService = env->GetStaticObjectField(contextClass, inputMethodServiceField);
        auto getSystemServiceMethod =
            env->GetMethodID(activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        _inputMethodManager =
            env->NewGlobalRef(env->CallObjectMethod(activityClazz, getSystemServiceMethod, inputMethodService));

        _getWindowMethod      = env->GetMethodID(activityClass, "getWindow", "()Landroid/view/Window;");
        _getDecorViewMethod   = env->GetMethodID(windowClass, "getDecorView", "()Landroid/view/View;");
        _getWindowTokenMethod = env->GetMethodID(viewClass, "getWindowToken", "()Landroid/os/IBinder;");

        auto inputMethodManagerClass = env->FindClass("android/view/inputmethod/InputMethodManager");
        _showSoftInputMethod = env->GetMethodID(inputMethodManagerClass, "showSoftInput", "(Landroid/view/View;I)Z");
        _hideSoftInputFromWindowMehtod =
            env->GetMethodID(inputMethodManagerClass, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");

        auto displayServiceField = env->GetStaticFieldID(contextClass, "DISPLAY_SERVICE", "Ljava/lang/String;");
        auto displayService      = env->GetStaticObjectField(contextClass, displayServiceField);
        _displayManager =
            env->NewGlobalRef(env->CallObjectMethod(activityClazz, getSystemServiceMethod, displayService));
        auto displayManagerClass = env->GetObjectClass(_displayManager);
        _getDisplays             = env->GetMethodID(displayManagerClass, "getDisplays", "()[Landroid/view/Display;");
        auto displayClass        = (jclass) env->FindClass("android/view/Display");
        auto displayModeClass    = (jclass) env->FindClass("android/view/Display$Mode");
        _getDisplayName          = env->GetMethodID(displayClass, "getName", "()Ljava/lang/String;");
        _getDisplayId            = env->GetMethodID(displayClass, "getDisplayId", "()I");
        _getSupportedModes = env->GetMethodID(displayClass, "getSupportedModes", "()[Landroid/view/Display$Mode;");
        _getPhysicalHeight =
            displayModeClass ? env->GetMethodID(displayModeClass, "getPhysicalHeight", "()I") : nullptr;
        _getPhysicalWidth = displayModeClass ? env->GetMethodID(displayModeClass, "getPhysicalWidth", "()I") : nullptr;
        _getRefreshRate   = displayModeClass ? env->GetMethodID(displayModeClass, "getRefreshRate", "()F") : nullptr;

        _getTitle = env->GetMethodID(activityClass, "getTitle", "()Ljava/lang/CharSequence;");
        _setTitle = env->GetMethodID(activityClass, "setTitle", "(Ljava/lang/CharSequence;)V");

        activity->vm->DetachCurrentThread();
    }
}

ANativeWindow* AndroidApplication::nativeWindow() noexcept {
    return _application->window;
}

android_app* AndroidApplication::instance() noexcept {
    return app;
}

gerium_runtime_platform_t AndroidApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_ANDROID;
}

void AndroidApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    auto activity = _application->activity;

    if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto jDisplays        = (jobjectArray) env->CallObjectMethod(_displayManager, _getDisplays);
        auto attachedDisplays = (gerium_uint32_t) env->GetArrayLength(jDisplays);
        displayCount          = displays ? std::min(displayCount, attachedDisplays) : attachedDisplays;
        if (displays) {
            _modes.clear();
            _names.clear();
            for (gerium_uint32_t i = 0; i < displayCount; ++i) {
                auto jDisplay = env->GetObjectArrayElement(jDisplays, (jsize) i);
                auto jName    = (jstring) env->CallObjectMethod(jDisplay, _getDisplayName);
                auto jId      = env->CallIntMethod(jDisplay, _getDisplayId);
                auto jModes =
                    _getSupportedModes ? (jobjectArray) env->CallObjectMethod(jDisplay, _getSupportedModes) : nullptr;
                auto jModeCount = jModes ? env->GetArrayLength(jModes) : 0;
                if (jName) {
                    auto jNameLength = env->GetStringUTFLength(jName);
                    std::string name;
                    name.resize(jNameLength + 1);
                    env->GetStringUTFRegion(jName, 0, jNameLength, name.data());
                    env->DeleteLocalRef(jName);
                    _names.push_back(std::move(name));
                } else {
                    _names.emplace_back("Unknown");
                }

                displays[i].id         = gerium_uint32_t(jId);
                displays[i].gpu_name   = "Unknown";
                displays[i].mode_count = gerium_uint32_t(jModeCount);
                displays[i].modes      = nullptr;

                for (jsize m = 0; m < jModeCount; ++m) {
                    auto jMode        = env->GetObjectArrayElement(jModes, m);
                    auto jWidth       = env->CallIntMethod(jMode, _getPhysicalWidth);
                    auto jHeight      = env->CallIntMethod(jMode, _getPhysicalHeight);
                    auto jRefreshRate = env->CallFloatMethod(jMode, _getRefreshRate);
                    _modes.emplace_back(
                        gerium_uint16_t(jWidth), gerium_uint16_t(jHeight), gerium_uint16_t(jRefreshRate));
                    env->DeleteLocalRef(jMode);
                }

                env->DeleteLocalRef(jModes);
                env->DeleteLocalRef(jDisplay);
            }
        }
        env->DeleteLocalRef(jDisplays);
        activity->vm->DetachCurrentThread();
    }

    if (displays) {
        gerium_uint32_t modeOffset = 0;
        for (gerium_uint32_t i = 0; i < displayCount; ++i) {
            displays[i].name        = _names[i].c_str();
            displays[i].device_name = _names[i].c_str();
            if (displays[i].mode_count) {
                displays[i].modes = &_modes[modeOffset];
                modeOffset += displays[i].mode_count;
            }
        }
    }
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

void AndroidApplication::onFullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
}

gerium_application_style_flags_t AndroidApplication::onGetStyle() const noexcept {
    return GERIUM_APPLICATION_STYLE_NONE_BIT;
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
    auto activity = _application->activity;

    if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto jTitle       = (jstring) env->CallObjectMethod(_application->activity->clazz, _getTitle);
        auto jTitleLength = env->GetStringLength(jTitle);
        _title.clear();
        _title.resize(jTitleLength + 1);
        env->GetStringUTFRegion(jTitle, 0, jTitleLength, _title.data());
        env->DeleteLocalRef(jTitle);
        activity->vm->DetachCurrentThread();
    }
    return _title.c_str();
}

void AndroidApplication::onSetTitle(gerium_utf8_t title) noexcept {
    auto activity = _application->activity;

    if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto jTitle = env->NewStringUTF(title);
        env->CallVoidMethod(_application->activity->clazz, _setTitle, jTitle);
        env->DeleteLocalRef(jTitle);
        activity->vm->DetachCurrentThread();
    }
}

void AndroidApplication::onShowCursor(bool show) noexcept {
}

void AndroidApplication::onRun() {
    if (_application->destroyRequested) {
        error(GERIUM_RESULT_ERROR_APPLICATION_TERMINATED);
    }
    if (_initialized) {
        error(GERIUM_RESULT_ERROR_APPLICATION_ALREADY_RUNNING);
    }

    ANativeActivity_setWindowFormat(_application->activity, WINDOW_FORMAT_RGBX_8888);

    changeState(GERIUM_APPLICATION_STATE_CREATE, true);

    if (callbackStateFailed()) {
        onExit();
        error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
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
                if (callbackStateFailed()) {
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
    ImGui_ImplAndroid_Init(_application->window);
}

void AndroidApplication::onShutdownImGui() {
    ImGui_ImplAndroid_Shutdown();
}

void AndroidApplication::onNewFrameImGui() {
    auto& io = ImGui::GetIO();

    static bool wantTextInputLast = false;

    if (io.WantTextInput && !wantTextInputLast) {
        showSoftKeyboard(io.WantTextInput);
    } else if (!io.WantTextInput && wantTextInputLast) {
        showSoftKeyboard(io.WantTextInput);
    }
    wantTextInputLast = io.WantTextInput;

    ImGui_ImplAndroid_NewFrame();
}

void AndroidApplication::initialize() {
    if (!_initialized && _application->window) {
        _initialized = true;
        changeState(GERIUM_APPLICATION_STATE_INITIALIZE, true);
    }
    active();
}

void AndroidApplication::uninitialize() {
    if (_initialized) {
        _initialized = false;
        changeState(GERIUM_APPLICATION_STATE_UNINITIALIZE, true);
    }
}

void AndroidApplication::active() {
    if (!_activated && _application->window) {
        _activated = true;
        changeState(GERIUM_APPLICATION_STATE_VISIBLE, true);
    }
}

void AndroidApplication::deactive() {
    if (_activated) {
        _activated = false;
        changeState(GERIUM_APPLICATION_STATE_INVISIBLE, true);
    }
}

void AndroidApplication::gotFocus() {
    if (!_focused) {
        _focused = true;
        changeState(GERIUM_APPLICATION_STATE_GOT_FOCUS, true);
    }
}

void AndroidApplication::lostFocus() {
    if (_focused) {
        _focused = false;
        changeState(GERIUM_APPLICATION_STATE_LOST_FOCUS, true);
    }
}

bool AndroidApplication::isPause() const noexcept {
    return !_activated && getBackgroundWait();
}

int AndroidApplication::getUnicodeChar(int eventType, int keyCode, int metaState) const {
    const auto key = (gerium_uint64_t(keyCode) << 32) | gerium_uint64_t(metaState);

    if (auto it = _symbols.find(key); it != _symbols.end()) {
        return (int) it->second;
    }

    if (JNIEnv * env; _application->activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto keyEvent = env->NewObject(_keyEventClass, _keyEventCtor, eventType, keyCode);

        int unicodeKey = metaState == 0 ? env->CallIntMethod(keyEvent, _getUnicodeCharMethod)
                                        : env->CallIntMethod(keyEvent, _getUnicodeCharIMethod, metaState);

        env->DeleteLocalRef(keyEvent);
        _application->activity->vm->DetachCurrentThread();

        _symbols[key] = unicodeKey;
        return unicodeKey;
    }
    _symbols[key] = 0;
    return 0;
}

bool AndroidApplication::showSoftKeyboard(bool show) {
    auto result = false;
    if (JNIEnv * env; _application->activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto window    = env->CallObjectMethod(_application->activity->clazz, _getWindowMethod);
        auto decorView = env->CallObjectMethod(window, _getDecorViewMethod);

        if (show) {
            result = env->CallBooleanMethod(_inputMethodManager, _showSoftInputMethod, decorView, 0) != 0;
        } else {
            auto binder = env->CallObjectMethod(decorView, _getWindowTokenMethod);
            result      = env->CallBooleanMethod(_inputMethodManager, _hideSoftInputFromWindowMehtod, binder, 0) != 0;
        }

        app->activity->vm->DetachCurrentThread();
    }
    return result;
}

void AndroidApplication::onAppCmd(int32_t cmd) noexcept {
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
            changeState(GERIUM_APPLICATION_STATE_DESTROY, true);
            break;
    }

    if (callbackStateFailed()) {
        onExit();
    }
}

int32_t AndroidApplication::onInputEvent(AInputEvent* event) noexcept {
    auto& io           = ImGui::GetIO();
    auto eventType     = AInputEvent_getType(event);
    auto eventSource   = AInputEvent_getSource(event);
    auto eventAction   = AKeyEvent_getAction(event);
    auto eventKeyCode  = AKeyEvent_getKeyCode(event);
    auto eventScanCode = AKeyEvent_getScanCode(event);
    auto eventMeta     = AKeyEvent_getMetaState(event);

    auto imguiResult = ImGui_ImplAndroid_HandleInputEvent(event);

    if (ImGui::GetCurrentContext() && !isShowCursor() && !io.WantCaptureMouse) {
        ImGui::GetIO().AddFocusEvent(false);
    }

    if (io.WantTextInput && eventType == AINPUT_EVENT_TYPE_KEY && eventAction == AKEY_EVENT_ACTION_UP) {
        auto symbol = getUnicodeChar(AKEY_EVENT_ACTION_DOWN, eventKeyCode, eventMeta);
        if (symbol) {
            io.AddInputCharacter(symbol);
        }
        return imguiResult;
    }

    const auto timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(getCurrentTime().time_since_epoch());

    if (eventType == AINPUT_EVENT_TYPE_KEY && !io.WantTextInput) {
        const auto scancode  = toScanCode(eventScanCode);
        const auto keyUp     = eventAction == AKEY_EVENT_ACTION_UP;
        const auto prevKeyUp = !isPressScancode(scancode);

        if (keyUp != prevKeyUp) {
            gerium_event_t newEvent{};
            newEvent.type               = GERIUM_EVENT_TYPE_KEYBOARD;
            newEvent.timestamp          = timestamp.count();
            newEvent.keyboard.scancode  = scancode;
            newEvent.keyboard.code      = toKeyCode(eventKeyCode);
            newEvent.keyboard.state     = keyUp ? GERIUM_KEY_STATE_RELEASED : GERIUM_KEY_STATE_PRESSED;
            newEvent.keyboard.modifiers = toModifiers(eventMeta);

            if (newEvent.keyboard.code != GERIUM_KEY_CODE_UNKNOWN) {
                const auto symbol = getUnicodeChar(AKEY_EVENT_ACTION_DOWN, eventKeyCode, eventMeta);
                const auto bytes  = (gerium_uint8_t*) &symbol;

                newEvent.keyboard.symbol[0] = (gerium_char_t) bytes[0];
                newEvent.keyboard.symbol[1] = (gerium_char_t) bytes[1];
                newEvent.keyboard.symbol[2] = (gerium_char_t) bytes[2];
                newEvent.keyboard.symbol[3] = (gerium_char_t) bytes[3];
            }

            setKeyState(scancode, !keyUp);

            if (!io.WantCaptureKeyboard) {
                addEvent(newEvent);
            }
            return 1;
        }
    } else if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        if ((eventSource & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) {

        } else if ((eventSource & AINPUT_SOURCE_CLASS_POINTER) == AINPUT_SOURCE_CLASS_POINTER) {
            auto flags = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
            auto count = AMotionEvent_getPointerCount(event);

            for (size_t i = 0; i < count; ++i) {
                auto id = AMotionEvent_getPointerId(event, i);
                auto x = AMotionEvent_getX(event, i);
                auto y = AMotionEvent_getY(event, i);

                gerium_event_t newEvent{};
                newEvent.type             = GERIUM_EVENT_TYPE_MOUSE;
                newEvent.timestamp        = timestamp.count();
                newEvent.mouse.buttons    = GERIUM_MOUSE_BUTTON_NONE;
                newEvent.mouse.absolute_x = (gerium_sint16_t) x;
                newEvent.mouse.absolute_y = (gerium_sint16_t) y;
                newEvent.mouse.delta_x    = 0;
                newEvent.mouse.delta_y    = 0;

                if (flags == AMOTION_EVENT_ACTION_DOWN || flags == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                    newEvent.mouse.buttons |= GERIUM_MOUSE_BUTTON_LEFT_DOWN;
                    _pointers[id] = {x,y};
                } else if (flags == AMOTION_EVENT_ACTION_UP || flags == AMOTION_EVENT_ACTION_POINTER_UP) {
                    newEvent.mouse.buttons |= GERIUM_MOUSE_BUTTON_LEFT_UP;
                } else if (flags == AMOTION_EVENT_ACTION_MOVE) {
                    const auto [prevX, prevY] = _pointers[id];
                    newEvent.mouse.delta_x = gerium_sint16_t(x - prevX);
                    newEvent.mouse.delta_y = gerium_sint16_t(y - prevY);
                    _pointers[id] = {x,y};
                } else if (flags == AMOTION_EVENT_ACTION_CANCEL) {
                    newEvent.mouse.buttons |= GERIUM_MOUSE_BUTTON_LEFT_UP;
                } else {
                    continue;
                }

                if (!io.WantCaptureMouse) {
                    addEvent(newEvent);
                }
            }
            return 1;
        }
    }

    return imguiResult;
}

void AndroidApplication::onAppCmd(android_app* application, int32_t cmd) {
    auto androidApp = alias_cast<AndroidApplication*>(application->userData);
    androidApp->onAppCmd(cmd);
}

int32_t AndroidApplication::onInputEvent(android_app* application, AInputEvent* event) {
    auto androidApp = alias_cast<AndroidApplication*>(application->userData);
    return androidApp->onInputEvent(event);
}

std::chrono::high_resolution_clock::time_point AndroidApplication::getCurrentTime() noexcept {
    return std::chrono::high_resolution_clock::now();
}

} // namespace gerium::android

gerium_public void android_main(android_app* state) {
    typedef int (*mainFunc)(int argc, char* argv[]);

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
    return gerium::android::app ? Object::create<AndroidApplication>(*application, title, width, height)
                                : GERIUM_RESULT_ERROR_NOT_IMPLEMENTED;
}
