#ifndef DEPENDENCY_COLLECTOR_EDITOR_PLUGIN_H
#define DEPENDENCY_COLLECTOR_EDITOR_PLUGIN_H

#include <editor/editor_plugin.h>
#include <editor/editor_file_system.h>
#include <scene/gui/menu_button.h>

class DependencyCollectorEditorPlugin: public EditorPlugin {
    GDCLASS(DependencyCollectorEditorPlugin, EditorPlugin);

    MenuButton* menu_btn;
    Dictionary layers;

    static DependencyCollectorEditorPlugin* instance;
    bool first_entry_added;
    bool collected;
    String result_path;

    void _populate_deps(EditorFileSystemDirectory *p_dir, FileAccess *result);

protected:
    static void _bind_methods();

public:
    void _collect_dependencies();
    DependencyCollectorEditorPlugin();
};



#endif // DEPENDENCY_COLLECTOR_EDITOR_PLUGIN_H