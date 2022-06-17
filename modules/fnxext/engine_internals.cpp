#include "engine_internals.h"
#include "core/io/resource_loader.h"

EngineInternals* EngineInternals::singleton = NULL;

void EngineInternals::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_resource_format_loader", "script_path"), &EngineInternals::add_resource_format_loader);
}

void EngineInternals::add_resource_format_loader(const String &script_path) {
    ResourceLoader::remove_custom_resource_format_loader(script_path);
    ResourceLoader::add_custom_resource_format_loader(script_path);
}

EngineInternals::EngineInternals() {
    singleton = this;
}