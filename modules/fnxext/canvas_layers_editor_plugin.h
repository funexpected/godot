#ifndef CANVAS_LAYERS_EDITOR_PLUGIN_H
#define CANVAS_LAYERS_EDITOR_PLUGIN_H

#include <editor/editor_plugin.h>
#include <scene/gui/menu_button.h>

class CanvasLayersEditorPlugin: public EditorPlugin {
    GDCLASS(CanvasLayersEditorPlugin, EditorPlugin);

    MenuButton* menu_btn;
    Dictionary layers;

    static CanvasLayersEditorPlugin* instance;

protected:
    static void _bind_methods();
    void _notification(int p_what);

    void _check_node_for_layers(Node* node);
    void _select_item(int pressed_idx=0);
    String _get_layer_group_name(const String &layer) const;

public:
    virtual void save_external_data();

    static CanvasLayersEditorPlugin* get_singleton() { return instance; };

    void add_layer(const String &name, bool selected=true);
    bool is_layer_visible(const String &name);
    CanvasLayersEditorPlugin();
};



#endif // CANVAS_LAYERS_EDITOR_PLUGIN_H