/******************************************************************************
 * Spine Runtimes Software License v2.5
 *
 * Copyright (c) 2013-2016, Esoteric Software
 * All rights reserved.
 *
 * You are granted a perpetual, non-exclusive, non-sublicensable, and
 * non-transferable license to use, install, execute, and perform the Spine
 * Runtimes software and derivative works solely for personal or internal
 * use. Without the written permission of Esoteric Software (see Section 2 of
 * the Spine Software License Agreement), you may not (a) modify, translate,
 * adapt, or develop new applications using the Spine Runtimes or otherwise
 * create derivative works or improvements of the Spine Runtimes or (b) remove,
 * delete, alter, or obscure any trademarks or any copyright, trademark, patent,
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 *
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
 * USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifdef MODULE_FNXEXT_ENABLED

#include <core/class_db.h>
#include "register_types.h"
#include "stylebox_bordered_texture.h"
#include "visibility_controller_2d.h"

#ifdef TOOLS_ENABLED
#include <editor/editor_node.h>
#include "canvas_layers_editor_plugin.h"
#endif

#ifdef TOOLS_ENABLED
static void _editor_init() {
	EditorNode::get_singleton()->add_editor_plugin(memnew(CanvasLayersEditorPlugin));
}
#endif

void register_fnxext_types() {
	ClassDB::register_class<StyleBoxBorderedTexture>();
	ClassDB::register_class<VisibilityController2D>();

#ifdef TOOLS_ENABLED
	// Control* gui = EditorNode::get_singleton()->get_gui_base();
	// Ref<Texture> vcicon = gui->get_icon("VisibilityEnabler2D", "EditorIcons");
	// gui->add_icon_override("VisibilityController2D", vcicon);
	EditorNode::add_init_callback(_editor_init);
#endif
}

void unregister_fnxext_types() {

}

#else

void register_fnxext_types() {}
void unregister_fnxext_types() {}

#endif // MODULE_FNXEXT_ENABLED
