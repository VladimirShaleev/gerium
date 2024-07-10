#include "AndroidApplication.hpp"

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

        auto displayServiceField =
                env->GetStaticFieldID(contextClass, "DISPLAY_SERVICE", "Ljava/lang/String;");
        auto displayService = env->GetStaticObjectField(contextClass, displayServiceField);
        _displayManager =
                env->NewGlobalRef(env->CallObjectMethod(activityClazz, getSystemServiceMethod, displayService));
        auto displayManagerClass = env->GetObjectClass(_displayManager);
        _getDisplays = env->GetMethodID(displayManagerClass, "getDisplays", "()[Landroid/view/Display;");
        auto displayClass = (jclass) env->FindClass("android/view/Display");
        auto displayModeClass = (jclass) env->FindClass("android/view/Display$Mode");
        _getDisplayName = env->GetMethodID(displayClass, "getName", "()Ljava/lang/String;");
        _getDisplayId = env->GetMethodID(displayClass, "getDisplayId", "()I");
        _getSupportedModes = env->GetMethodID(displayClass, "getSupportedModes", "()[Landroid/view/Display$Mode;");
        _getPhysicalHeight = displayModeClass ? env->GetMethodID(displayModeClass, "getPhysicalHeight", "()I") : nullptr;
        _getPhysicalWidth = displayModeClass ? env->GetMethodID(displayModeClass, "getPhysicalWidth", "()I") : nullptr;
        _getRefreshRate = displayModeClass ? env->GetMethodID(displayModeClass, "getRefreshRate", "()F") : nullptr;

        _getTitle = env->GetMethodID(activityClass, "getTitle", "()Ljava/lang/CharSequence;");
        _setTitle = env->GetMethodID(activityClass, "setTitle", "(Ljava/lang/CharSequence;)V");

        activity->vm->DetachCurrentThread();
    }
}

ANativeWindow* AndroidApplication::nativeWindow() noexcept {
    return _application->window;
}

gerium_runtime_platform_t AndroidApplication::onGetPlatform() const noexcept {
    return GERIUM_RUNTIME_PLATFORM_ANDROID;
}

void AndroidApplication::onGetDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    auto activity = _application->activity;

    if (JNIEnv * env; activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto jDisplays = (jobjectArray) env->CallObjectMethod(_displayManager, _getDisplays);
        auto attachedDisplays = (gerium_uint32_t) env->GetArrayLength(jDisplays);
        displayCount = displays ? std::min(displayCount, attachedDisplays) : attachedDisplays;
        if (displays) {
            _modes.clear();
            _names.clear();
            for (gerium_uint32_t i = 0; i < displayCount; ++i) {
                auto jDisplay = env->GetObjectArrayElement(jDisplays, (jsize) i);
                auto jName = (jstring) env->CallObjectMethod(jDisplay, _getDisplayName);
                auto jId = env->CallIntMethod(jDisplay, _getDisplayId);
                auto jModes = _getSupportedModes ? (jobjectArray) env->CallObjectMethod(jDisplay, _getSupportedModes) : nullptr;
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

                displays[i].id = gerium_uint32_t(jId);
                displays[i].gpu_name = "Unknown";
                displays[i].mode_count = gerium_uint32_t(jModeCount);
                displays[i].modes = nullptr;

                for (jsize m = 0; m < jModeCount; ++m) {
                    auto jMode = env->GetObjectArrayElement(jModes, m);
                    auto jWidth = env->CallIntMethod(jMode, _getPhysicalWidth);
                    auto jHeight = env->CallIntMethod(jMode, _getPhysicalHeight);
                    auto jRefreshRate = env->CallFloatMethod(jMode, _getRefreshRate);
                    _modes.emplace_back(
                            gerium_uint16_t(jWidth),
                            gerium_uint16_t(jHeight),
                            gerium_uint16_t(jRefreshRate));
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
            displays[i].name = _names[i].c_str();
            displays[i].device_name  = _names[i].c_str();
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
        auto jTitle = (jstring) env->CallObjectMethod(_application->activity->clazz, _getTitle);
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
    if (JNIEnv * env; _application->activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        auto keyEvent = env->NewObject(_keyEventClass, _keyEventCtor, eventType, keyCode);

        int unicodeKey = metaState == 0 ? env->CallIntMethod(keyEvent, _getUnicodeCharMethod)
                                        : env->CallIntMethod(keyEvent, _getUnicodeCharIMethod, metaState);

        env->DeleteLocalRef(keyEvent);
        _application->activity->vm->DetachCurrentThread();

        return unicodeKey;
    }
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
    auto eventKeyCode  = AKeyEvent_getKeyCode(event);
    auto eventScanCode = AKeyEvent_getScanCode(event);

    if (io.WantTextInput && AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
        AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
        if (eventKeyCode <= AKEYCODE_ENDCALL || (eventKeyCode >= AKEYCODE_DPAD_UP && eventKeyCode <= AKEYCODE_CLEAR) ||
            (eventKeyCode >= AKEYCODE_ALT_LEFT && eventKeyCode <= AKEYCODE_SHIFT_RIGHT) ||
            eventKeyCode >= AKEYCODE_SYM) {
            return ImGui_ImplAndroid_HandleInputEvent(event);
        }
        auto meta = AKeyEvent_getMetaState(event);

        auto c = getUnicodeChar(AKEY_EVENT_ACTION_DOWN, eventKeyCode, meta);
        io.AddInputCharacter(c);
        return 0;
    }

    if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
        ImGuiKey key = keyCodeToImGuiKey(eventKeyCode);
        ImGui_ImplAndroid_HandleInputEvent(event);
        io.AddKeyEvent(key, false);
        io.SetKeyEventNativeData(key, eventKeyCode, eventScanCode);
        return 0;
    } else {
        return ImGui_ImplAndroid_HandleInputEvent(event);
    }
}

void AndroidApplication::onAppCmd(android_app* application, int32_t cmd) {
    auto androidApp = alias_cast<AndroidApplication*>(application->userData);
    androidApp->onAppCmd(cmd);
}

int32_t AndroidApplication::onInputEvent(android_app* application, AInputEvent* event) {
    auto androidApp = alias_cast<AndroidApplication*>(application->userData);
    return androidApp->onInputEvent(event);
}

ImGuiKey AndroidApplication::keyCodeToImGuiKey(int32_t keyCode) noexcept {
    switch (keyCode) {
        case AKEYCODE_TAB:
            return ImGuiKey_Tab;
        case AKEYCODE_DPAD_LEFT:
            return ImGuiKey_LeftArrow;
        case AKEYCODE_DPAD_RIGHT:
            return ImGuiKey_RightArrow;
        case AKEYCODE_DPAD_UP:
            return ImGuiKey_UpArrow;
        case AKEYCODE_DPAD_DOWN:
            return ImGuiKey_DownArrow;
        case AKEYCODE_PAGE_UP:
            return ImGuiKey_PageUp;
        case AKEYCODE_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case AKEYCODE_MOVE_HOME:
            return ImGuiKey_Home;
        case AKEYCODE_MOVE_END:
            return ImGuiKey_End;
        case AKEYCODE_INSERT:
            return ImGuiKey_Insert;
        case AKEYCODE_FORWARD_DEL:
            return ImGuiKey_Delete;
        case AKEYCODE_DEL:
            return ImGuiKey_Backspace;
        case AKEYCODE_SPACE:
            return ImGuiKey_Space;
        case AKEYCODE_ENTER:
            return ImGuiKey_Enter;
        case AKEYCODE_ESCAPE:
            return ImGuiKey_Escape;
        case AKEYCODE_APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case AKEYCODE_COMMA:
            return ImGuiKey_Comma;
        case AKEYCODE_MINUS:
            return ImGuiKey_Minus;
        case AKEYCODE_PERIOD:
            return ImGuiKey_Period;
        case AKEYCODE_SLASH:
            return ImGuiKey_Slash;
        case AKEYCODE_SEMICOLON:
            return ImGuiKey_Semicolon;
        case AKEYCODE_EQUALS:
            return ImGuiKey_Equal;
        case AKEYCODE_LEFT_BRACKET:
            return ImGuiKey_LeftBracket;
        case AKEYCODE_BACKSLASH:
            return ImGuiKey_Backslash;
        case AKEYCODE_RIGHT_BRACKET:
            return ImGuiKey_RightBracket;
        case AKEYCODE_GRAVE:
            return ImGuiKey_GraveAccent;
        case AKEYCODE_CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case AKEYCODE_SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case AKEYCODE_NUM_LOCK:
            return ImGuiKey_NumLock;
        case AKEYCODE_SYSRQ:
            return ImGuiKey_PrintScreen;
        case AKEYCODE_BREAK:
            return ImGuiKey_Pause;
        case AKEYCODE_NUMPAD_0:
            return ImGuiKey_Keypad0;
        case AKEYCODE_NUMPAD_1:
            return ImGuiKey_Keypad1;
        case AKEYCODE_NUMPAD_2:
            return ImGuiKey_Keypad2;
        case AKEYCODE_NUMPAD_3:
            return ImGuiKey_Keypad3;
        case AKEYCODE_NUMPAD_4:
            return ImGuiKey_Keypad4;
        case AKEYCODE_NUMPAD_5:
            return ImGuiKey_Keypad5;
        case AKEYCODE_NUMPAD_6:
            return ImGuiKey_Keypad6;
        case AKEYCODE_NUMPAD_7:
            return ImGuiKey_Keypad7;
        case AKEYCODE_NUMPAD_8:
            return ImGuiKey_Keypad8;
        case AKEYCODE_NUMPAD_9:
            return ImGuiKey_Keypad9;
        case AKEYCODE_NUMPAD_DOT:
            return ImGuiKey_KeypadDecimal;
        case AKEYCODE_NUMPAD_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case AKEYCODE_NUMPAD_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case AKEYCODE_NUMPAD_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case AKEYCODE_NUMPAD_ADD:
            return ImGuiKey_KeypadAdd;
        case AKEYCODE_NUMPAD_ENTER:
            return ImGuiKey_KeypadEnter;
        case AKEYCODE_NUMPAD_EQUALS:
            return ImGuiKey_KeypadEqual;
        case AKEYCODE_CTRL_LEFT:
            return ImGuiKey_LeftCtrl;
        case AKEYCODE_SHIFT_LEFT:
            return ImGuiKey_LeftShift;
        case AKEYCODE_ALT_LEFT:
            return ImGuiKey_LeftAlt;
        case AKEYCODE_META_LEFT:
            return ImGuiKey_LeftSuper;
        case AKEYCODE_CTRL_RIGHT:
            return ImGuiKey_RightCtrl;
        case AKEYCODE_SHIFT_RIGHT:
            return ImGuiKey_RightShift;
        case AKEYCODE_ALT_RIGHT:
            return ImGuiKey_RightAlt;
        case AKEYCODE_META_RIGHT:
            return ImGuiKey_RightSuper;
        case AKEYCODE_MENU:
            return ImGuiKey_Menu;
        case AKEYCODE_0:
            return ImGuiKey_0;
        case AKEYCODE_1:
            return ImGuiKey_1;
        case AKEYCODE_2:
            return ImGuiKey_2;
        case AKEYCODE_3:
            return ImGuiKey_3;
        case AKEYCODE_4:
            return ImGuiKey_4;
        case AKEYCODE_5:
            return ImGuiKey_5;
        case AKEYCODE_6:
            return ImGuiKey_6;
        case AKEYCODE_7:
            return ImGuiKey_7;
        case AKEYCODE_8:
            return ImGuiKey_8;
        case AKEYCODE_9:
            return ImGuiKey_9;
        case AKEYCODE_A:
            return ImGuiKey_A;
        case AKEYCODE_B:
            return ImGuiKey_B;
        case AKEYCODE_C:
            return ImGuiKey_C;
        case AKEYCODE_D:
            return ImGuiKey_D;
        case AKEYCODE_E:
            return ImGuiKey_E;
        case AKEYCODE_F:
            return ImGuiKey_F;
        case AKEYCODE_G:
            return ImGuiKey_G;
        case AKEYCODE_H:
            return ImGuiKey_H;
        case AKEYCODE_I:
            return ImGuiKey_I;
        case AKEYCODE_J:
            return ImGuiKey_J;
        case AKEYCODE_K:
            return ImGuiKey_K;
        case AKEYCODE_L:
            return ImGuiKey_L;
        case AKEYCODE_M:
            return ImGuiKey_M;
        case AKEYCODE_N:
            return ImGuiKey_N;
        case AKEYCODE_O:
            return ImGuiKey_O;
        case AKEYCODE_P:
            return ImGuiKey_P;
        case AKEYCODE_Q:
            return ImGuiKey_Q;
        case AKEYCODE_R:
            return ImGuiKey_R;
        case AKEYCODE_S:
            return ImGuiKey_S;
        case AKEYCODE_T:
            return ImGuiKey_T;
        case AKEYCODE_U:
            return ImGuiKey_U;
        case AKEYCODE_V:
            return ImGuiKey_V;
        case AKEYCODE_W:
            return ImGuiKey_W;
        case AKEYCODE_X:
            return ImGuiKey_X;
        case AKEYCODE_Y:
            return ImGuiKey_Y;
        case AKEYCODE_Z:
            return ImGuiKey_Z;
        case AKEYCODE_F1:
            return ImGuiKey_F1;
        case AKEYCODE_F2:
            return ImGuiKey_F2;
        case AKEYCODE_F3:
            return ImGuiKey_F3;
        case AKEYCODE_F4:
            return ImGuiKey_F4;
        case AKEYCODE_F5:
            return ImGuiKey_F5;
        case AKEYCODE_F6:
            return ImGuiKey_F6;
        case AKEYCODE_F7:
            return ImGuiKey_F7;
        case AKEYCODE_F8:
            return ImGuiKey_F8;
        case AKEYCODE_F9:
            return ImGuiKey_F9;
        case AKEYCODE_F10:
            return ImGuiKey_F10;
        case AKEYCODE_F11:
            return ImGuiKey_F11;
        case AKEYCODE_F12:
            return ImGuiKey_F12;
        default:
            return ImGuiKey_None;
    }
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
