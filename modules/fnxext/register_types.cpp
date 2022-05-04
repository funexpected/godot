#ifdef MODULE_FNXEXT_ENABLED

#include <core/class_db.h>
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

void register_fnxext_types() {
	ClassDB::register_class<StyleBoxBorderedTexture>();
	ClassDB::register_class<VisibilityController2D>();
	ClassDB::register_class<MeshLine2D>();
	ClassDB::register_class<Future>();
	ClassDB::register_class<ZipTool>();

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
