#include "InputService.hpp"
#include "../Application.hpp"
#include "../components/Camera.hpp"

bool InputService::isPressScancode(gerium_scancode_t scancode) const noexcept {
    return gerium_application_is_press_scancode(application().handle(), scancode);
}

void InputService::start() {
}

void InputService::stop() {
    _events.clear();
}

void InputService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t elapsed) {
    auto view = entityRegistry().view<Camera>();

    Camera* camera = nullptr;
    for (auto entity : view) {
        auto& c = view.get<Camera>(entity);
        if (c.active) {
            camera = &c;
        }
    }

    auto app            = application().handle();
    bool swapFullscreen = false;
    bool showCursor     = gerium_application_is_show_cursor(app);
    bool invY           = gerium_application_get_platform(app) == GERIUM_RUNTIME_PLATFORM_MAC_OS;

    gerium_float32_t pitch = 0.0f;
    gerium_float32_t yaw   = 0.0f;
    gerium_float32_t zoom  = 0.0f;

    auto move = 1.0f;
    if (isPressScancode(GERIUM_SCANCODE_SHIFT_LEFT) || isPressScancode(GERIUM_SCANCODE_SHIFT_RIGHT)) {
        move *= 2.0f;
    }
    if (isPressScancode(GERIUM_SCANCODE_CONTROL_LEFT) || isPressScancode(GERIUM_SCANCODE_CONTROL_RIGHT)) {
        move /= 2.0f;
    }

    gerium_event_t event;
    while (gerium_application_poll_events(app, &event)) {
        // _events.push_back(event);
        if (event.type == GERIUM_EVENT_TYPE_KEYBOARD) {
            if (event.keyboard.scancode == GERIUM_SCANCODE_ENTER && event.keyboard.state == GERIUM_KEY_STATE_RELEASED &&
                (event.keyboard.modifiers & GERIUM_KEY_MOD_LALT)) {
                swapFullscreen = true;
            } else if (event.keyboard.scancode == GERIUM_SCANCODE_ESCAPE &&
                       event.keyboard.state == GERIUM_KEY_STATE_RELEASED) {
                showCursor = true;
            }
        } else if (event.type == GERIUM_EVENT_TYPE_MOUSE) {
            constexpr auto buttonsDown = GERIUM_MOUSE_BUTTON_LEFT_DOWN | GERIUM_MOUSE_BUTTON_RIGHT_DOWN |
                                         GERIUM_MOUSE_BUTTON_MIDDLE_DOWN | GERIUM_MOUSE_BUTTON_4_DOWN |
                                         GERIUM_MOUSE_BUTTON_5_DOWN;
            if (event.mouse.buttons & buttonsDown) {
                showCursor = false;
            }
            if (!gerium_application_is_show_cursor(app) ||
                gerium_application_get_platform(app) == GERIUM_RUNTIME_PLATFORM_ANDROID) {
                const auto delta = 1.0f;
                pitch += event.mouse.raw_delta_y * (invY ? delta : -delta);
                yaw += event.mouse.raw_delta_x * -delta;
                zoom += event.mouse.wheel_vertical * move * -0.1f;
            }
        }
    }

    if (camera) {
        auto speed = camera->fov / glm::radians(30.0f);
        camera->pitch += pitch * camera->rotationSpeed * speed;
        camera->yaw += yaw * camera->rotationSpeed * speed;
        camera->fov += zoom;

        auto velocity = camera->movementSpeed * elapsed;

        if (isPressScancode(GERIUM_SCANCODE_A) || isPressScancode(GERIUM_SCANCODE_ARROW_LEFT)) {
            camera->position += camera->right * move * velocity;
        }
        if (isPressScancode(GERIUM_SCANCODE_D) || isPressScancode(GERIUM_SCANCODE_ARROW_RIGHT)) {
            camera->position += camera->right * -move * velocity;
        }
        if (isPressScancode(GERIUM_SCANCODE_W) || isPressScancode(GERIUM_SCANCODE_ARROW_UP)) {
            camera->position += camera->front * move * velocity;
        }
        if (isPressScancode(GERIUM_SCANCODE_S) || isPressScancode(GERIUM_SCANCODE_ARROW_DOWN)) {
            camera->position += camera->front * -move * velocity;
        }
        if (isPressScancode(GERIUM_SCANCODE_SPACE) || isPressScancode(GERIUM_SCANCODE_PAGE_UP)) {
            camera->position += glm::vec3(0.0f, 1.0f, 0.0f) * move * velocity;
        }
        if (isPressScancode(GERIUM_SCANCODE_C) || isPressScancode(GERIUM_SCANCODE_PAGE_DOWN)) {
            camera->position += glm::vec3(0.0f, -1.0f, 0.0f) * move * velocity;
        }
    }

    if (swapFullscreen) {
        gerium_application_fullscreen(app, !gerium_application_is_fullscreen(app), 0, nullptr);
    }

    if (gerium_application_get_platform(app) != GERIUM_RUNTIME_PLATFORM_ANDROID) {
        gerium_application_show_cursor(app, showCursor);
    }
}
