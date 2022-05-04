#ifndef ZIPTOOL_H
#define ZIPTOOL_H

#include "core/reference.h"

class ZipTool: public Reference {
    GDCLASS(ZipTool, Reference)

protected:
    static void _bind_methods();

public:
    Array list_files(const String &p_path);
};

#endif