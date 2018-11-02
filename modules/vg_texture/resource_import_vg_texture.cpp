
#include "resource_import_vg_texture.h"
#include "core/os/file_access.h"
#include "core/io/resource_saver.h"
#include "scene/resources/curve.h"
#include "core/math/geometry.h"
#include "vg_texture.h"
String ResourceImporterVGTexture::get_importer_name() const {
    return "vg_texture";
};

String ResourceImporterVGTexture::get_visible_name() const {
    return "Vector Texture";
};

void ResourceImporterVGTexture::get_recognized_extensions(List<String> *p_extensions) const {
    p_extensions->push_back("svg");
};

String ResourceImporterVGTexture::get_save_extension() const {
    return "res";
};

String ResourceImporterVGTexture::get_resource_type() const {
    return "VGTexture";
};

int ResourceImporterVGTexture::get_preset_count() const {
    return 1;
};

String ResourceImporterVGTexture::get_preset_name(int p_idx) const {
    return "Default";
};

void ResourceImporterVGTexture::get_import_options(List<ImportOption> *r_options, int p_preset) const {
    r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "tesselate/tolerance_degrees", PROPERTY_HINT_RANGE, "0.1,30,0.1"), 2));
    r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "tesselate/max_subdivisions", PROPERTY_HINT_RANGE, "2,16,1"), 5));
};

bool ResourceImporterVGTexture::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const{
    return true;
};

Error ResourceImporterVGTexture::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files){
    NSVGimage *svg_image;
    ERR_EXPLAIN(p_source_file + " doesn't exists");
    ERR_FAIL_COND_V(!FileAccess::exists(p_source_file), ERR_DOES_NOT_EXIST);
    FileAccess *f = FileAccess::open(p_source_file, FileAccess::READ);
    uint32_t size = f->get_len();
	PoolVector<uint8_t> src_image;
	src_image.resize(size + 1);
	PoolVector<uint8_t>::Write src_w = src_image.write();
	f->get_buffer(src_w.ptr(), size);
	src_w.ptr()[size] = '\0';
	PoolVector<uint8_t>::Read src_r = src_image.read();
	svg_image = nsvgParse((char *)src_r.ptr(), "px", 96);

    print_line(String("Importing ") + p_source_file + " " + Variant(svg_image->width) + "x" + Variant(svg_image->height));// + " " + Variant(svg_image->shapes));
    int subdivisions = p_options["tesselate/max_subdivisions"];
    float tolerance = p_options["tesselate/tolerance_degrees"];

    Ref<VGTexture> tex;
    tex.instance();
    tex->size.x = svg_image->width;
    tex->size.y = svg_image->height;
    
    for (NSVGshape *shape = svg_image->shapes; shape!= NULL; shape = shape->next){
        Ref<VGShape> vg_shape;
        vg_shape.instance();

        vg_shape->set_color(Color((0x0000FF&shape->fill.color)/255.0, ((0x00FF00&shape->fill.color)>>8)/255.0, ((0xFF0000&shape->fill.color)>>16)/255.0));
        print_line(String("Shape color: #") + vg_shape->get_color().to_html());
        tex->add_shape(vg_shape);
        NSVGpath *path = shape->paths;
        for (NSVGpath *path = shape->paths; path != NULL; path = path->next){
            Ref<VGPath> vg_path;
            vg_path.instance();
            vg_path->set_closed(path->closed);
            vg_path->set_bounds(Rect2(path->bounds[0], path->bounds[1], path->bounds[2], path->bounds[3]));
            float *pts = path->pts;
            Vector2 pt;
            print_line(String("create curve of ") + Variant(path->npts) + " points");
            //Vector2 last_point;
            //Vector2 last_out;
            //c.add_point(Vector2(pts[0], pts[1]), Vector2(), Vector2(pts[1], pts[2]]));
            for (int i=0; i<path->npts-1; i+=3){
                float* p = &path->pts[i*2];
                if (i==0){
                    pt = Vector2(p[0], p[1]);
                    print_line(String("add p0 ") + Variant(pt));
                    vg_path->add_point(pt);
                    
                }
                pt = Vector2(p[6], p[7]);
                print_line(String("add p ") + Variant(pt));
                vg_path->add_point(pt);
                pt = Vector2(p[2]-p[0], p[3]-p[1]);
                print_line(String("add out ") + Variant(pt));
                vg_path->set_point_out(i/3, pt);
                pt = Vector2(p[4]-p[6], p[5]-p[7]);
                print_line(String("add in ") + Variant(pt));
                vg_path->set_point_in(i/3+1, pt);
                //last_point = Vector2(p[6], p[7]);
                //last_out = Vector2(p)
            }
            vg_shape->add_path(vg_path);
        }
    }


    

    Error res = ResourceSaver::save(p_save_path + "." + get_save_extension(), tex, ResourceSaver::FLAG_COMPRESS);
    print_line(String("Saving to ") + p_save_path + "." + get_save_extension() + ", result: " + Variant(res));

    
    nsvgDelete(svg_image);
    return OK;
};
ResourceImporterVGTexture *ResourceImporterVGTexture::singleton = NULL;

ResourceImporterVGTexture* ResourceImporterVGTexture::get_singleton(){
    return singleton;
}
ResourceImporterVGTexture::ResourceImporterVGTexture(){
    singleton = this;
};
ResourceImporterVGTexture::~ResourceImporterVGTexture(){

};


