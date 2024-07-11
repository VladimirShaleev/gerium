#ifndef GERIUM_PROFILER_HPP
#define GERIUM_PROFILER_HPP

#include "ObjectPtr.hpp"

struct _gerium_profiler : public gerium::Object {};

namespace gerium {

class Profiler : public _gerium_profiler {
public:
};

} // namespace gerium

#endif
