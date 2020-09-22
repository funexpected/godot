#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "core/object.h"

class CustomErrorHandler : public Object {

    GDCLASS(CustomErrorHandler, Object);
    ErrorHandlerList eh;
    static void _handle_error(void *p_self, const char *p_func, const char *p_file, int p_line, const char *p_error, const char *p_errorexp, ErrorHandlerType p_type);

public:
    void handle_error(const String &p_func, const String &p_file, int p_line, const String &p_error, const String &p_errorexp);
    CustomErrorHandler();
    ~CustomErrorHandler();
};

#endif