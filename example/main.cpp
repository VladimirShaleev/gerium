#include <gerium/gerium.h>
#include <iostream>

void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

int main() {
    gerium_application_t application = nullptr;

    try {
        check(gerium_windows_application_create("test", 800, 600, nullptr, &application));
        check(gerium_application_run(application));

    } catch (const std::runtime_error& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    } catch (...) {
        std::cerr << "Error: unknown error" << std::endl;
    }
    gerium_application_destroy(application);
    return 0;
}
