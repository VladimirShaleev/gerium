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

    gerium_uint32_t version{ GERIUM_VERSION_ENCODE(1, 0, 0) };
    bool showProfiler{};
    bool developerMode{ true };
    bool transforming{ false };
    bool transformChilds{ true };
    bool snapToGrid{ false };
    Transfrom transform{ Translate };
    State state{};
};

#endif
