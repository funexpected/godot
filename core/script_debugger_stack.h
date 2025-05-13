/*************************************************************************/
/*  script_debugger_stack.h                                              */
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

#ifndef SCRIPT_DEBUGGER_STACK_H
#define SCRIPT_DEBUGGER_STACK_H

#include "core/list.h"
#include "core/script_language.h"

class ScriptDebuggerStack : public ScriptDebugger {
private:
    bool profiling;

public:
    virtual void debug(ScriptLanguage *p_script, bool p_can_continue = true, bool p_is_error_breakpoint = false);
    virtual void send_message(const String &p_message, const Array &p_args);
    virtual void send_error(const String &p_func, const String &p_file, int p_line, const String &p_err, const String &p_descr, ErrorHandlerType p_type, const Vector<ScriptLanguage::StackInfo> &p_stack_info);

    virtual bool is_profiling() const { return profiling; }
    virtual void add_profiling_frame_data(const StringName &p_name, const Array &p_data) {}
    virtual void profiling_start();
    virtual void profiling_end();
    virtual void profiling_set_frame_times(float p_frame_time, float p_idle_time, float p_physics_time, float p_physics_frame_time) {}

    ScriptDebuggerStack();
    ~ScriptDebuggerStack();
};

#endif // SCRIPT_DEBUGGER_STACK_H