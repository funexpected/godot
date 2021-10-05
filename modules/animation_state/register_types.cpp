// MIT License

// Copyright (c) 2021 Yakov Borevich, Funexpected LLC

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifdef MODULE_ANIMATION_STATE_ENABLED

#include "register_types.h"
#include "animation_state_expression.h"
#include "animation_state_nodes.h"
#include <core/class_db.h>

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"

static void _editor_init() {
	// add custom themes for state machine nodes
#define ADD_FRAME_STYLE(name, r, g, b)																				\
	{																												\
		Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();											\
		Ref<StyleBoxFlat> default_frame = theme->get_stylebox("state_machine_frame", "GraphNode");					\
		Ref<StyleBoxFlat> default_selected_frame = theme->get_stylebox("state_machine_selectedframe", "GraphNode");	\
		if (default_frame.is_valid() && default_selected_frame.is_valid()) {										\
			Ref<StyleBoxFlat> new_frame = default_frame->duplicate(true);											\
			Ref<StyleBoxFlat> new_selected_frame = default_selected_frame->duplicate(true);							\
			new_frame->set_bg_color(Color(r, g, b, 0.7));															\
			new_selected_frame->set_bg_color(Color(r, g, b, 0.9));													\
			theme->set_stylebox("state_machine_frame", String(#name), new_frame);									\
			theme->set_stylebox("state_machine_selectedframe", String(#name), new_selected_frame);					\
		}																											\
	};																												\

	// monokai colors from https://gist.github.com/r-malon/8fc669332215c8028697a0bbfbfbb32a
	// grey (empty)
	ADD_FRAME_STYLE(AnimationNodeEmpty, 46.0/255.0, 46.0/255.0, 46.0/255.0);
	
	// green (animations)
	ADD_FRAME_STYLE(AnimationNodeAnimation, 180.0/255.0, 210.0/255.0, 115.0/255.0);
	ADD_FRAME_STYLE(AnimationNodeFlashClip, 180.0/255.0, 210.0/255.0, 115.0/255.0);
	ADD_FRAME_STYLE(AnimationNodeSpineAnimation, 180.0/255.0, 210.0/255.0, 115.0/255.0);

	// blue (complex animations)
	ADD_FRAME_STYLE(AnimationNodeStateMachine, 108.0/255.0, 153.0/255.0, 187.0/255.0);

	// orange (instants)
	ADD_FRAME_STYLE(AnimationNodeStateUpdate, 229.0/255.0, 181.0/255.0, 103.0/255.0);

	// purple (custom non-instants)
	ADD_FRAME_STYLE(AnimationNodeDelay, 158.0/255.0, 134.0/255.0, 200.0/255.0);
}
#endif

void register_animation_state_types() {
    ClassDB::register_class<AnimationStateExpression>();
    ClassDB::register_class<AnimationNodeEmpty>();
	ClassDB::register_class<AnimationNodeDelay>();
	ClassDB::register_class<AnimationNodeStateUpdate>();

    #ifdef TOOLS_ENABLED
	EditorNode::add_init_callback(_editor_init);
#endif
}

void unregister_animation_state_types() {
    
}

#endif