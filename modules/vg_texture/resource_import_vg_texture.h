#ifndef RESOURCE_IMPORT_VG_TEXTURE_H
#define RESOURCE_IMPORT_VG_TEXTURE_H

#include "io/resource_import.h"
#include <nanosvg.h>

class ResourceImporterVGTexture : public ResourceImporter {
	GDCLASS(ResourceImporterVGTexture, ResourceImporter)

protected:
	//static void _texture_reimport_srgb(const Ref<StreamTexture> &p_tex);
	//static void _texture_reimport_3d(const Ref<StreamTexture> &p_tex);
	//static void _texture_reimport_normal(const Ref<StreamTexture> &p_tex);

	static ResourceImporterVGTexture *singleton;

public:
    static ResourceImporterVGTexture* get_singleton();
	//static ResourceImporterTexture *get_singleton() { return singleton; }
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;


	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;

	//void _save_stex(const Ref<Image> &p_image, const String &p_to_path, int p_compress_mode, float p_lossy_quality, Image::CompressMode p_vram_compression, bool p_mipmaps, int p_texture_flags, bool p_streamable, bool p_detect_3d, bool p_detect_srgb, bool p_force_rgbe, bool p_detect_normal, bool p_force_normal);

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = NULL);

	//void update_imports();

	ResourceImporterVGTexture();
	~ResourceImporterVGTexture();
};

#endif