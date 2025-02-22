#ifndef COMPONENTS_SETTINGS_HPP
#define COMPONENTS_SETTINGS_HPP

#include "../Common.hpp"

struct Settings {
    enum State {
        None,
        Save,
        Load
    };

    State state{};
};

#endif
