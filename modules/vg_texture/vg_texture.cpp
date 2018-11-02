#include "vg_texture.h"

// VGPath

void VGPath::set_bounds(const Rect2 &p_bounds) {
    bounds = p_bounds;
};

Rect2 VGPath::get_bounds() const {
    return bounds;
};

void VGPath::set_closed(bool p_closed) {
    closed = p_closed;
};
bool VGPath::is_closed() const {
    return closed;
};
void VGPath::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_bounds", "bounds"), &VGPath::set_bounds);
    ClassDB::bind_method(D_METHOD("get_bounds"), &VGPath::get_bounds);
    ClassDB::bind_method(D_METHOD("set_closed", "closed"), &VGPath::set_closed);
    ClassDB::bind_method(D_METHOD("is_closed"), &VGPath::is_closed);
    ADD_PROPERTY(PropertyInfo(Variant::RECT2, "bounds"), "set_bounds", "get_bounds");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "closed"), "set_closed", "is_closed");
}


// VGShape

void VGShape::_set_pathes(const Array &p_pathes) {
    pathes.resize(p_pathes.size());
    for (int i=0; i<p_pathes.size(); i++){
        pathes.write[i] = p_pathes[i];
    }
}

Array VGShape::_get_pathes() const {
    Array res;
    res.resize(pathes.size());
    for (int i=0; i<pathes.size(); i++){
        res[i] = pathes[i];
    }
    return res;
};
void VGShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_set_pathes", "pathes"), &VGShape::_set_pathes);
    ClassDB::bind_method(D_METHOD("_get_pathes"), &VGShape::_get_pathes);
    ClassDB::bind_method(D_METHOD("set_color", "color"), &VGShape::set_color);
    ClassDB::bind_method(D_METHOD("get_color"), &VGShape::get_color);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "pathes"), "_set_pathes", "_get_pathes");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
}

void VGShape::set_color(const Color &p_color){
    color = p_color;
}

Color VGShape::get_color() const {
    return color;
}
void VGShape::add_path(const Ref<VGPath> &p_path) {
    pathes.push_back(p_path);
}

int VGShape::get_path_count() const {
    return pathes.size();
}

Ref<VGPath> VGShape::get_path(int idx) const {
    ERR_FAIL_COND_V(pathes.size()<=idx, Ref<VGPath>());
    return pathes[idx];
}

void VGShape::set_path(int idx, const Ref<VGPath> &p_path){
    ERR_FAIL_COND(pathes.size()<=idx);
    pathes.write[idx] = p_path;
}


int VGTexture::get_width() const {
    return size.x;
}

int VGTexture::get_height() const {
    return size.y;
}

Size2 VGTexture::get_size() const {
    return Size2(get_width(), get_height());
}

Vector2 VGTexture::_get_size() const {
    return size;
}
void VGTexture::_set_size(const Vector2 &p_size){
    size = p_size;
}

RID VGTexture::get_rid() const {
    return RID();
}

bool VGTexture::has_alpha() const {
    return true;
}
void VGTexture::set_flags(uint32_t p_flags) {

}
uint32_t VGTexture::get_flags() const {
    return 0;
};

void VGTexture::draw(RID p_canvas_item, const Point2 &p_pos, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map) const {
    print_line("VGTexture::draw");
}

void VGTexture::draw_rect(RID p_canvas_item, const Rect2 &p_rect, bool p_tile, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map) const {
    /*
    print_line(String("VGTexture::draw_rect") + " rect:" + Variant(p_rect) + ", tile: " + Variant(p_tile) + ", transpose: " + Variant(p_transpose));
    float r = 300;
    float k = r*0.552284749831;
    Curve2D c;
    c.add_point(Vector2(0,-r), Vector2(0, 0), Vector2(k, 0));
	c.add_point(Vector2(r, 0), Vector2(0, -k), Vector2(0, k));
	c.add_point(Vector2(0, r), Vector2(k, 0), Vector2(-k, 0));
	c.add_point(Vector2(-r, 0), Vector2(0, k), Vector2(0, -k));
	c.add_point(Vector2(0,-r), Vector2(-k, 0), Vector2(0, 0));

    PoolVector2Array pp = c.tessellate(6, 2.0);
    
    Vector<Vector2> points;
    Vector<Color> colors;
    for (int i=0; i<pp.size(); i++){
        points.push_back(pp[i]);
        colors.push_back(Color(1,1,1));
    }
    VS::get_singleton()->canvas_item_add_polygon(p_canvas_item, points, colors);
    //VS::get_singleton()->canvas_item_add_triangle_array()
    */
   //VS::get_singleton()->canvas_item_add_triangle_array(p_canvas_item, triangles, vertices, colors);
   for (int i=0; i<shapes.size();i++){
       Ref<VGShape> shape = shapes[i];
       for (int j=0; j<shape->get_path_count(); j++){
           Ref<VGPath> path = shape->get_path(j);
           PoolVector2Array pool_points = path->tessellate(4, 2);
           Vector<Vector2> points;
           Vector<Color> colors;
           points.resize(pool_points.size());
           colors.resize(pool_points.size());
           for (int k=0; k<pool_points.size();k++){
               colors.write[k] = shape->get_color();
               points.write[k] = pool_points[k];
           }
           VS::get_singleton()->canvas_item_add_polygon(p_canvas_item, points, colors, Vector<Vector2>(), RID(), RID(), true);
       }
   }
}

void VGTexture::draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map, bool p_clip_uv) const {
    print_line("VGTexture::draw_rect_region");
}

void VGTexture::_bind_methods(){
    ClassDB::bind_method(D_METHOD("_set_size", "size"), &VGTexture::_set_size);
    ClassDB::bind_method(D_METHOD("_get_size"), &VGTexture::_get_size);
    ClassDB::bind_method(D_METHOD("_set_shapes", "shapes"), &VGTexture::_set_shapes);
    ClassDB::bind_method(D_METHOD("_get_shapes"), &VGTexture::_get_shapes);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size"), "_set_size", "_get_size");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "shapes"), "_set_shapes", "_get_shapes");
}

void VGTexture::_set_shapes(const Array &p_shapes) {
    shapes.resize(p_shapes.size());
    for (int i=0; i<p_shapes.size(); i++){
        shapes.write[i] = p_shapes[i];
    }
}

Array VGTexture::_get_shapes() const {
    Array res;
    res.resize(shapes.size());
    for (int i=0; i<shapes.size(); i++){
        res[i] = shapes[i];
    }
    return res;
};

int VGTexture::add_shape(const Ref<VGShape> &p_shape) {
    int idx = shapes.size();
    shapes.push_back(p_shape);
    return idx;
}

int VGTexture::get_shapes_count() const {
    return shapes.size();
}

Ref<VGShape> VGTexture::get_shape(int idx) const {
    ERR_FAIL_COND_V(shapes.size()<=idx, Ref<VGShape>());
    return shapes[idx];
}

void VGTexture::set_shape(int idx, const Ref<VGShape> &p_shape) {
    ERR_FAIL_COND(shapes.size()<=idx);
    shapes.write[idx] = p_shape;
}