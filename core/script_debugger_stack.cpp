/*************************************************************************/
/*  script_debugger_stack.cpp                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "script_debugger_stack.h"
#include "core/os/os.h"
#include "core/custom_error_handler.h"
#include "core/error_macros.h"

void ScriptDebuggerStack::debug(ScriptLanguage *p_script, bool p_can_continue, bool p_is_error_breakpoint) {
    if (!p_is_error_breakpoint || p_can_continue) {
        return;
    }

    String error = p_script->debug_get_error();
    int line = p_script->debug_get_stack_level_line(0);
    String file = p_script->debug_get_stack_level_source(0);
    String func = p_script->debug_get_stack_level_function(0);

	ScriptInstance *second_script;
	if (p_script->debug_get_stack_level_count() >= 2) {
		second_script = p_script->debug_get_stack_level_instance(1);
		
	}

	List<CustomErrorHandler *> custom_error_handlers = Engine::get_singleton()->get_custom_error_handlers();

	for (List<CustomErrorHandler *>::Element *E = custom_error_handlers.front(); E; E = E->next()) {
		if (second_script == E->get()->get_script_instance()) {
			print_error("Recursion error in CustomErrorHandler; ignore error");
			return;
		}
		E->get()->handle_error( func.utf8().get_data(), file.utf8().get_data(), line, error.utf8().get_data(), "");
	}

}

void ScriptDebuggerStack::send_message(const String &p_message, const Array &p_args) {
}

void ScriptDebuggerStack::send_error(const String &p_func, const String &p_file, int p_line, const String &p_err, const String &p_descr, ErrorHandlerType p_type, const Vector<ScriptLanguage::StackInfo> &p_stack_info) {
}
	
void ScriptDebuggerStack::profiling_start() {
    profiling = true;
}

void ScriptDebuggerStack::profiling_end() {
    profiling = false;
}

ScriptDebuggerStack::ScriptDebuggerStack() {
    profiling = false;

    // These settings make sure that functions can be entered/exited without breakpoints/stepping
    set_lines_left(-1);  // -1 means unlimited lines can be executed
    set_depth(-1);       // -1 means unlimited stack depth is allowed
}

ScriptDebuggerStack::~ScriptDebuggerStack() {
}