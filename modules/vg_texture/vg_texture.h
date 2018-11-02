//#ifdef MODULE_VG_TEXTURE_ENABLED
#ifndef VG_TEXTURE_H
#define VG_TEXTURE_H

#include "scene/resources/texture.h"
#include "scene/resources/curve.h"

class VGPath: public Curve2D {
    GDCLASS(VGPath, Curve2D);
protected:
    Rect2 bounds;
    bool closed;
    static void _bind_methods();
public:
    void set_bounds(const Rect2 &bounds);
    Rect2 get_bounds() const;
    void set_closed(bool closed);
    bool is_closed() const;
};

class VGShape: public Resource {
    GDCLASS(VGShape, Resource);
protected:
    Color color;
    Vector< Ref<VGPath> > pathes;
    void _set_pathes(const Array &p_pathes);
    Array _get_pathes() const;
    static void _bind_methods();
public:
    void set_color(const Color &p_color);
    Color get_color() const;
    void add_path(const Ref<VGPath> &p_pathes);
    int get_path_count() const;
    Ref<VGPath> get_path(int idx) const;
    void set_path(int idx, const Ref<VGPath> &p_path);

};

class VGTexture: public Texture {
    GDCLASS(VGTexture, Texture);
    friend class ResourceImporterVGTexture;

protected:
    Size2i size;
    Vector< Ref<VGShape> > shapes;
    static void _bind_methods();
    void _set_shapes(const Array &p_shapes);
    Array _get_shapes() const;
    void _set_size(const Vector2 &p_size);
    Vector2  _get_size() const;

public:
    int get_width() const;
	int get_height() const;
	Size2 get_size() const;
	RID get_rid() const;

	//virtual bool is_pixel_opaque(int p_x, int p_y) const;

	bool has_alpha() const;

	void set_flags(uint32_t p_flags);
	uint32_t get_flags() const;

	virtual void draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>()) const;
	virtual void draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile = false, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>()) const;
	virtual void draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate = Color(1, 1, 1), bool p_transpose = false, const Ref<Texture> &p_normal_map = Ref<Texture>(), bool p_clip_uv = true) const;

    int add_shape(const Ref<VGShape> &p_shape);
    int get_shapes_count() const;
    Ref<VGShape> get_shape(int idx) const;
    void set_shape(int idx, const Ref<VGShape> &p_shape);
    //virtual bool get_rect_region(const Rect2 &p_rect, const Rect2 &p_src_rect, Rect2 &r_rect, Rect2 &r_src_rect) const;

    
};


#endif // VG_TEXTURE_H
//#endif // MODULE_VG_TEXTURE_ENABLED