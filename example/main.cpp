#include <gerium/gerium.h>
#include <iostream>

int main() {
    gerium_application_t application;
    auto state = gerium_windows_application_create(
        "test", 800, 600, GERIUM_APPLICATION_MODE_RESIZABLE_BIT, nullptr, &application);
    if (state != GERIUM_RESULT_SUCCESS) {
        std::cerr << gerium_result_to_string(state) << std::endl;
        return 1;
    }

    gerium_application_run(application);

    gerium_application_destroy(application);
    return 0;
}
