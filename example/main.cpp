#include <gerium/gerium.h>
#include <iostream>

void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    return 1;
}

gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state) {
    switch (state) {
        case GERIUM_APPLICATION_STATE_CREATE:
            std::cout << "GERIUM_APPLICATION_STATE_CREATE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_DESTROY:
            std::cout << "GERIUM_APPLICATION_STATE_DESTROY" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            std::cout << "GERIUM_APPLICATION_STATE_INITIALIZE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            std::cout << "GERIUM_APPLICATION_STATE_UNINITIALIZE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
            std::cout << "GERIUM_APPLICATION_STATE_GOT_FOCUS" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
            std::cout << "GERIUM_APPLICATION_STATE_LOST_FOCUS" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_VISIBLE:
            std::cout << "GERIUM_APPLICATION_STATE_VISIBLE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            std::cout << "GERIUM_APPLICATION_STATE_INVISIBLE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_NORMAL:
            std::cout << "GERIUM_APPLICATION_STATE_NORMAL" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_MINIMIZE:
            std::cout << "GERIUM_APPLICATION_STATE_MINIMIZE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
            std::cout << "GERIUM_APPLICATION_STATE_MAXIMIZE" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
            std::cout << "GERIUM_APPLICATION_STATE_FULLSCREEN" << std::endl;
            break;
        case GERIUM_APPLICATION_STATE_RESIZE:
            std::cout << "GERIUM_APPLICATION_STATE_RESIZE" << std::endl;
            break;
    }
    return 1;
}

int main() {
    gerium_application_t application = nullptr;

    try {
        check(gerium_windows_application_create("test", 800, 600, nullptr, &application));

        gerium_application_set_frame_func(application, frame, nullptr);
        gerium_application_set_state_func(application, state, nullptr);
        check(gerium_application_run(application));

    } catch (const std::runtime_error& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    } catch (...) {
        std::cerr << "Error: unknown error" << std::endl;
    }
    gerium_application_destroy(application);
    return 0;
}
