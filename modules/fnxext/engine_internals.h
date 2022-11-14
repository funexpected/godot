#ifndef MODULE_ENGINE_INTERNALS_H
#define MODULE_ENGINE_INTERNALS_H

#include "core/object.h"

class EngineInternals: public Object {
    GDCLASS(EngineInternals, Object);
    static EngineInternals *singleton;

protected:
    static void _bind_methods();

public:
    void add_resource_format_loader(const String &script_path);
    EngineInternals();
};
#endif