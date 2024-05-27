#define  TOOLS_ENABLED
#ifdef TOOLS_ENABLED

#include "canvas_layers_editor_plugin.h"
#include "core/config/engine.h"
#include <editor/editor_settings.h>

CanvasLayersEditorPlugin* CanvasLayersEditorPlugin::instance = NULL;

void CanvasLayersEditorPlugin::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
            add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, menu_btn);
            get_tree()->connect("node_added", callable_mp(this, &CanvasLayersEditorPlugin::_check_node_for_layers));
            //connect("scene_changed", this, "_update_connections");
        } break;
    }
}

void CanvasLayersEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_check_node_for_layers"), &CanvasLayersEditorPlugin::_check_node_for_layers);
    ClassDB::bind_method(D_METHOD("_select_item"), &CanvasLayersEditorPlugin::_select_item);
    ClassDB::bind_method(D_METHOD("add_layer"), &CanvasLayersEditorPlugin::add_layer);
    ClassDB::bind_method(D_METHOD("is_layer_visible"), &CanvasLayersEditorPlugin::is_layer_visible);
}

String CanvasLayersEditorPlugin::_get_layer_group_name(const String &layer) const {
    return "__canvas_layer/" + layer;
}

void CanvasLayersEditorPlugin::_check_node_for_layers(Node* node) {
    Variant layer_var = node->get("editor/canvas_layer");
    if (layer_var == Variant()) {
        return;
    }

    String layer_name = layer_var;
    String group = _get_layer_group_name(layer_name);
    if (!node->is_in_group(group)) {
        node->add_to_group(group);
    }
    if (!layers.has(layer_name)) {
        add_layer(layer_name);
    }
}

void CanvasLayersEditorPlugin::_select_item(int pressed_item_idx) {
    PopupMenu* menu = menu_btn->get_popup();
    menu->set_item_checked(pressed_item_idx, !menu->is_item_checked(pressed_item_idx));
    for (int i=0; i < menu->get_item_count(); i++) {
        layers[menu->get_item_metadata(i)] = menu->is_item_checked(i);
    }
    String group = _get_layer_group_name(menu->get_item_metadata(pressed_item_idx));
    get_tree()->call_group(group, "update");
}

void CanvasLayersEditorPlugin::save_external_data() {
    EditorSettings::get_singleton()->set_project_metadata("canvas_layers", "layers", layers);
}

void CanvasLayersEditorPlugin::add_layer(const String &layer_name, bool selected) {
    if (layers.has(layer_name)) {
        return;
    }

    PopupMenu* menu = menu_btn->get_popup();
    menu->add_check_item(layer_name.capitalize());
    menu->set_item_metadata(menu->get_item_count()-1, layer_name);
    menu->set_item_checked(menu->get_item_count()-1, selected);

    layers[layer_name] = selected;
}

bool CanvasLayersEditorPlugin::is_layer_visible(const String &layer_name) {
    return layers.has(layer_name) && layers[layer_name];
}


CanvasLayersEditorPlugin::CanvasLayersEditorPlugin() {
    CanvasLayersEditorPlugin::instance = this;

    menu_btn = memnew(MenuButton);
    menu_btn->set_text("Layers");
    PopupMenu* menu = menu_btn->get_popup();
    menu->connect("id_pressed", callable_mp(this, &CanvasLayersEditorPlugin::_select_item));
    menu->set_hide_on_checkable_item_selection(false);
    menu->set_hide_on_item_selection(false);

    Engine::get_singleton()->add_singleton(Engine::Singleton("CanvasLayersEditorPlugin", this));

    Dictionary known_layers = EditorSettings::get_singleton()->get_project_metadata("canvas_layers", "layers", Dictionary());

    for (int i=0; i<known_layers.size(); i++) {
        add_layer(known_layers.get_key_at_index(i), known_layers.get_value_at_index(i));
    }


    List<StringName> classes;
	ClassDB::get_class_list(&classes);
	
    List<StringName> blocked_classes;
    // blocked_classes.push_back("PinJoint2D");
    // blocked_classes.push_back("PinJoint3D");
    // blocked_classes.push_back("RectangleShape2D");
    // blocked_classes.push_back("RectangleShape3D");
    // blocked_classes.push_back("RigidBody2D");
    // blocked_classes.push_back("RigidBody3D");
    // blocked_classes.push_back("ScriptCreateDialog");
    // blocked_classes.push_back("SegmentShape2D");
    // blocked_classes.push_back("SegmentShape3D");
    // blocked_classes.push_back("SceneTree");
    // blocked_classes.push_back("SeparationRayShape2D");
    // blocked_classes.push_back("SeparationRayShape3D");
    // blocked_classes.push_back("SliderJoint3D");
    // blocked_classes.push_back("SliderJoint2D");
    // blocked_classes.push_back("CallbackTweener");
    // blocked_classes.push_back("IntervalTweener");
    // blocked_classes.push_back("MethodTweener");
    // blocked_classes.push_back("PropertyTweener");
    // blocked_classes.push_back("EditorPaths");
    // blocked_classes.push_back("Popup");
    // blocked_classes.push_back("PopupMenu");
    // blocked_classes.push_back("PopupPanel");
    // blocked_classes.push_back("SoftBody3D");
    // blocked_classes.push_back("SoftBody2D");
    // blocked_classes.push_back("SphereShape3D");
    // blocked_classes.push_back("SphereShape2D");
    // blocked_classes.push_back("StaticBody2D");
    // blocked_classes.push_back("StaticBody3D");
    // blocked_classes.push_back("TouchScreenButton");
    // blocked_classes.push_back("VehicleBody3D");
    // blocked_classes.push_back("WebRTCPeerConnection");
    // blocked_classes.push_back("Window");
    // blocked_classes.push_back("World2D");
    // blocked_classes.push_back("World3D");
    // blocked_classes.push_back("WorldBoundaryShape2D");
    // blocked_classes.push_back("WorldBoundaryShape3D");


    // {StringName("PinJoint2D")};

	for (List<StringName>::Element *E = classes.front(); E; E = E->next()) {
        StringName cls = E->get();
        String class_name = cls;
        if (class_name.begins_with("_")) continue;
        if (blocked_classes.find(cls) != NULL) continue;
        if (class_name.find("Server") != -1) continue;
        Object *c = NULL;
		bool cleanup_c = false;
        print_line(class_name);
		if (Engine::get_singleton()->has_singleton(cls)) {
			c = Engine::get_singleton()->get_singleton_object(cls);
			cleanup_c = false;
		} else if (ClassDB::can_instantiate(cls)) {
			c = ClassDB::instantiate(cls);
			cleanup_c = true;
		}

		if (c && Object::cast_to<Node>(c)) {
            Variant layer = c->get("editor/canvas_layer");
            if (layer != Variant()) {
                add_layer(layer);
            }
        }
        if (cleanup_c) {
            memdelete(c);
        }
	}
}


#endif