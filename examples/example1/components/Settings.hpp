#ifndef COMPONENTS_SETTINGS_HPP
#define COMPONENTS_SETTINGS_HPP

#include "../Common.hpp"

struct Settings {
    enum State {
        None,
        Save,
        Load
    };

    enum Transfrom {
        Translate,
        Rotate,
        Scale
    };

    bool showProfiler{};
    bool transforming{ false };
    bool transformChilds{ true };
    bool snapToGrid{ false };
    Transfrom transform{ Translate };
    State state{};
};

#endif
