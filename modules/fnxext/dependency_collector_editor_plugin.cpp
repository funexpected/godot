#ifdef TOOLS_ENABLED

#include "dependency_collector_editor_plugin.h"
#include <core/engine.h>
#include <core/os/os.h>
#include <editor/editor_settings.h>
#include <editor/editor_file_system.h>

// This module allow to collect all project depenencies into single json file.
// Usage: 
//  godot -e --path <project> --collect-dependencies path/to/result.json
//
// Resulting json format is array of file entry. Each file entry is 
// array of strings where first element is file path and the rest is 
// its dependencies:
//
// [[
//   "res://ui/onboarding/minigames/geobaord.tscn",
//   "res://ui/onboarding/minigames/geoboard.gd",
//   "res://ui/onboarding/minigames/board.gd",
//   "res://egypt/geoboard/logic/level.tscn"
// ],[
//   "res://ui/onboarding/minigames/geoboard.gd",
//   "res://egypt/geoboard/logic/level.gd"
// ],[
//   "res://ui/onboarding/minigames/monkey.gd",
//   "res://japan/monkey/logic/bush.tscn",
//   "res://japan/monkey/logic/bush.gd",
//   "res://japan/monkey/config.gd",
//   "res://japan/monkey/logic/textures.gd",
//   "res://audio/japan/monkey_eating.ogg",
//   "res://audio/japan/monkey_happy.ogg",
//   "res://audio/japan/japan_monkey_happy_smile.ogg",
//   "res://audio/japan/monkey_itches.ogg"
// ]]

DependencyCollectorEditorPlugin* DependencyCollectorEditorPlugin::instance = NULL;

void DependencyCollectorEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_collect_dependencies"), &DependencyCollectorEditorPlugin::_collect_dependencies);

}

void DependencyCollectorEditorPlugin::_collect_dependencies(bool p_exist) {
    if (collected) {
        return;
    }
    collected = true;
    FileAccess *file = FileAccess::open(result_path, FileAccess::WRITE);
    if (file) {
        // write json directly to file
        // as it is faster and easier
        // then build struct and then serialize json
        file->store_string("[");
        _populate_deps(EditorFileSystem::get_singleton()->get_filesystem(), file);
        file->store_string("]");
        file->close();
    } else {
        print_error(String("Invalid file path for collecting dependencies: ") + result_path);
    }
    get_tree()->quit(0);
}

void DependencyCollectorEditorPlugin::_populate_deps(EditorFileSystemDirectory *p_dir, FileAccess *file) {
    for (int i = 0; i < p_dir->get_subdir_count(); i++) {
		_populate_deps(p_dir->get_subdir(i), file);
	}

	for (int i = 0; i < p_dir->get_file_count(); i++) {
        
        String path = p_dir->get_file_path(i);
        if (first_entry_added) {
            file->store_string(",");
        } else {
            first_entry_added = true;
        }
        file->store_string("[\n  \"");
        file->store_string(path);
        file->store_string("\"");
        List<String> deps;
        ResourceLoader::get_dependencies(path, &deps);
        for (List<String>::Element *E = deps.front(); E; E = E->next()) {
            file->store_string(",\n  \"");
            file->store_string(E->get());
            file->store_string("\"");
        }
        file->store_string("\n]");
	}
}

DependencyCollectorEditorPlugin::DependencyCollectorEditorPlugin() {
    collected = false;
    result_path = "";
    first_entry_added = false;
    List<String> args = OS::get_singleton()->get_cmdline_args();
    bool collect_required = false;
    for (List<String>::Element *E = args.front(); E; E = E->next()) {
        if (E->get() == "--collect-dependencies") {
            EditorFileSystem::get_singleton()->connect("sources_changed", this, "_collect_dependencies");
            collect_required = true;
        } else if (collect_required) {
            result_path = E->get();
            break;
        }
    }
}
#endif