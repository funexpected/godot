#ifndef ZIPTOOL_H
#define ZIPTOOL_H

#include "core/object/ref_counted.h"

class ZipTool: public RefCounted {
    GDCLASS(ZipTool, RefCounted)

protected:
    static void _bind_methods();

public:
    Array list_files(const String &p_path);
};

#endif