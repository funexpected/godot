// #ifdef MODULE_FNXEXT_ENABLED

#include <core/object/class_db.h>
#include "register_types.h"
#include "stylebox_bordered_texture.h"
#include "visibility_controller_2d.h"
#include "mesh_line_2d.h"
#include "future.h"
#include "zip_tool.h"

#ifdef TOOLS_ENABLED
#include <editor/editor_node.h>
#include "canvas_layers_editor_plugin.h"
#endif

#ifdef TOOLS_ENABLED
static void _editor_init() {
	EditorNode::get_singleton()->add_editor_plugin(memnew(CanvasLayersEditorPlugin));
}
#endif

void initialize_fnxext_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	GDREGISTER_CLASS(StyleBoxBorderedTexture);
	GDREGISTER_CLASS(VisibilityController2D);
	GDREGISTER_CLASS(MeshLine2D);
	// GDREGISTER_CLASS(Future);
	GDREGISTER_CLASS(ZipTool);
	
// #ifdef TOOLS_ENABLED
	// Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
	// Control* gui = EditorNode::get_singleton()->get_gui_base();
	// Ref<Texture> vcicon = theme->get_icon("VisibilityEnabler2D", "EditorIcons");
	// gui->add_theme_icon_override("VisibilityController2D", vcicon);
	
	EditorNode::add_init_callback(_editor_init);
	
// #endif
}
void uninitialize_fnxext_module(ModuleInitializationLevel p_level) {
}

