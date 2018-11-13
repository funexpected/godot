//#ifdef MODULE_VG_TEXTURE_ENABLED

#include <core/class_db.h>
#include <core/project_settings.h>
#include <core/io/resource_import.h>
#include "register_types.h"

#include "register_types.h"
#include "vg_texture.h"
#include "resource_import_vg_texture.h"
void register_vg_texture_types() {
	print_line("register vg_texture");
	ClassDB::register_class<VGGradient>();
	ClassDB::register_class<VGPath>();
	ClassDB::register_class<VGShape>();
	ClassDB::register_class<VGTexture>();
	ClassDB::register_class<ResourceImporterVGTexture>();
//#ifdef TOOLS_ENABLED
	Ref<ResourceImporterVGTexture> importer;
	importer.instance();
	ResourceFormatImporter::get_singleton()->add_importer(importer);
//#endif
}
void unregister_vg_texture_types() {}
//#endif

