#include "core/custom_error_handler.h"
#include "core/script_language.h"

void CustomErrorHandler::_handle_error(void *p_self, const char *p_func, const char *p_file, int p_line, const char *p_error, const char *p_errorexp, ErrorHandlerType p_type) {
    CustomErrorHandler *self = (CustomErrorHandler*)p_self;

    if (p_type == ERR_HANDLER_SCRIPT) {
        self->handle_error(p_func, p_file, p_line, p_error, p_errorexp);
    }
}

void CustomErrorHandler::handle_error(const String &p_func, const String &p_file, int p_line, const String &p_error, const String &p_errorexp) {
    if (get_script_instance() && get_script_instance()->has_method("handle_error")) {
		get_script_instance()->call("handle_error", p_func, p_file, p_line, p_error, p_errorexp);
    }
}

CustomErrorHandler::CustomErrorHandler() {
    eh.userdata = this;
    eh.errfunc = _handle_error;
    add_error_handler(&eh);
}

CustomErrorHandler::~CustomErrorHandler() {
    remove_error_handler(&eh);
}