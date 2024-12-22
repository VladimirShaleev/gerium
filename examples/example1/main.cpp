#include "Application.hpp"

int main() {
    try {
        Application application;
        application.run("sample", 1000, 800);
    } catch (...) {
        return 1;
    }
    return 0;
}
