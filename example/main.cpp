#include "Application.hpp"

int main() {
    try {
        Application application;
        application.run("sample", 800, 600);
    } catch (...) {
        return 1;
    }
    return 0;
}
