#include "vg_texture.h"


// VGGradient

VGGradient::SpreadMethod VGGradient::get_spread_method() const {
    return spread_method;
}

void VGGradient::set_spread_method(VGGradient::SpreadMethod p_method) {
    spread_method = p_method;
}

VGGradient::GradientType VGGradient::get_gradient_type() const {
    return gradient_type;
}

void VGGradient::set_gradient_type(VGGradient::GradientType p_type) {
    gradient_type = p_type;
}

Transform2D VGGradient::get_gradient_transform() const {
    return transform;
}

void VGGradient::set_gradient_transform(Transform2D p_transform) {
    transform = p_transform;
}

Vector2 VGGradient::get_focal_point() const {
    return focal_point;
}

void VGGradient::set_focal_point(Vector2 p_point) {
    focal_point = p_point;
}

void VGGradient::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_spread_method", "method"), &VGGradient::set_spread_method);
    ClassDB::bind_method(D_METHOD("get_spread_method"), &VGGradient::get_spread_method);
    ClassDB::bind_method(D_METHOD("set_gradient_type", "type"), &VGGradient::set_gradient_type);
    ClassDB::bind_method(D_METHOD("get_gradient_type"), &VGGradient::get_gradient_type);
    ClassDB::bind_method(D_METHOD("set_transform", "transform"), &VGGradient::set_gradient_transform);
    ClassDB::bind_method(D_METHOD("get_transform"), &VGGradient::get_gradient_transform);
    ClassDB::bind_method(D_METHOD("set_focal_point", "focal_point"), &VGGradient::set_focal_point);
    ClassDB::bind_method(D_METHOD("get_focal_point"), &VGGradient::get_focal_point);


    ADD_PROPERTY(PropertyInfo(Variant::INT, "spread_method"), "set_spread_method", "get_spread_method");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "gradient_type"), "set_gradient_type", "get_gradient_type");
    ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "gradient_transform"), "set_transform", "get_transform");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "focal_point"), "set_focal_point", "get_focal_point");
    BIND_ENUM_CONSTANT(PAD);
    BIND_ENUM_CONSTANT(REPEAT);
    BIND_ENUM_CONSTANT(REFLECT);
    BIND_ENUM_CONSTANT(SOLID);
    BIND_ENUM_CONSTANT(LINEAR);
    BIND_ENUM_CONSTANT(RADIAL);
};

VGGradient::VGGradient() {
    gradient_type = VGGradient::SOLID;
    spread_method = VGGradient::PAD;
    transform = Transform2D();
    focal_point = Vector2();
}

VGGradient::~VGGradient() {

}


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

PoolColorArray VGShape::get_config() const {
    PoolColorArray raw;
    Ref<VGGradient> gr = get_color();
    int colors_count = gr->get_gradient_type() == VGGradient::SOLID ? 2 : gr->get_points_count();
    raw.push_back(Color(
        1.0,                                            // alpha
        (float)gr->get_gradient_type(),                        // gradient type
        (float)colors_count,                                   // gradient points count
        0                                               // textures count
    ));
    if (gr->get_gradient_type() != VGGradient::SOLID){
        Transform2D gtr = gr->get_gradient_transform();
        raw.push_back(Color(
            gtr.elements[0].x,                          // gradient transform axes
            gtr.elements[0].y,
            gtr.elements[1].x,
            gtr.elements[1].y
        ));
        if (gr->get_gradient_type() == VGGradient::LINEAR){
            raw.push_back(Color(
                gtr.elements[2].x,                      // linear gradient offset
                gtr.elements[2].y,
                0.0, 0.0                                    // zero padded      
            ));
        } else {
            raw.push_back(Color(
                gtr.elements[2].x,                      // radial gradient position
                gtr.elements[2].y,
                gr->get_focal_point().x,                // with focal point
                gr->get_focal_point().y
            ));
        }

    } 
    for (int i=0; i<colors_count; i++){
        Color p = gr->get_color(i);
        float w = gr->get_offset(i);
        raw.push_back(Color(p.r, p.g, p.b, w));
    }
    return raw;
}

void VGShape::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_set_pathes", "pathes"), &VGShape::_set_pathes);
    ClassDB::bind_method(D_METHOD("_get_pathes"), &VGShape::_get_pathes);
    ClassDB::bind_method(D_METHOD("set_color", "color"), &VGShape::set_color);
    ClassDB::bind_method(D_METHOD("get_color"), &VGShape::get_color);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "pathes"), "_set_pathes", "_get_pathes");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
}

void VGShape::set_color(const Ref<VGGradient> &p_color){
    color = p_color;
}

Ref<VGGradient> VGShape::get_color() const {
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

// VGTexture
Ref<Texture> VGTexture::create_config() {
    Vector<PoolColorArray> colors;
    int min_width = 0;
    for (int i=0; i<get_shapes_count(); i++){
        Ref<VGShape> shape = get_shape(i);
        PoolColorArray raw = shape->get_config();
        min_width = MAX(raw.size(), min_width);
        colors.push_back(raw);
        //for (int j=0; j<raw.size(); j++){
        //    print_line(String("  tex raw ") + Variant(i) + " " + Variant(raw[j]));
        //}
    }
    Ref<Image> img;
    img.instance();
    img->create(min_width, colors.size(), false, Image::FORMAT_RGBAF);
    img->resize_to_po2();
    img->lock();
    for (int i=0; i<colors.size(); i++){
        PoolColorArray raw = colors[i];
        for (int j=0; j<raw.size(); j++){
            Color c = raw[j];
            img->set_pixel(j, i, c);
        }
    }
    img->unlock();
    Ref<ImageTexture> tex;
    tex.instance();
    tex->create_from_image(img, 0);
    print_line(String("Creating config texture ") + Variant(img->get_size()));
    return tex;
}

Ref<Shader> VGTexture::create_shader() {
    Ref<Shader> shader;
    shader.instance();
    shader->set_code(String("shader_type canvas_item;\n") +
        "\n" +
        "uniform sampler2D config : hint_black;\n" +
        "\n" +
        
        // interpolate local vertex
        "varying noperspective vec2 point;\n" +
        "\n" +
        "void vertex(){\n" +
        "\tpoint = VERTEX;\n" +
        "}\n" +
        "\n" +

        "void fragment(){\n" +
        // setup 
        "\tfloat raw = floor(UV.x);\n" +
        "\tvec2 uv = UV - vec2(raw);\n" +
        "\tint cy = int(round(raw));\n"+
        "\tvec4 header = texelFetch(config, ivec2(0, cy), 0);\n" +
        "\tfloat gradient_type = header.g;\n" +
        "\tint colors_count = int(header.b);\n" +
        "\tint textures_count = int(header.a);\n" +
        "\tint cx = 1;\n" +
        "\tfloat gradient_offset=1.0;\n" +

        // calculate linear gradient
        "\tif (gradient_type == 1.0) { // linear\n" +
        "\t\tvec4 axes = texelFetch(config, ivec2(cx, cy), 0);\n" +
        "\t\tcx += 1;\n" +
        "\t\tvec4 origin = texelFetch(config, ivec2(cx, cy), 0);\n" +
        "\t\tcx += 1;\n" +
        "\t\tgradient_offset = point.x*axes.g + point.y*axes.a + origin.g;\n" +

        // calculate radial gradient
        "\t} else if (gradient_type == 2.0) { // radial\n" +
        "\t\tvec4 axes = texelFetch(config, ivec2(cx, cy), 0);\n" +
        "\t\tcx += 1;\n" +
        "\t\tvec4 origin = texelFetch(config, ivec2(cx, cy), 0);\n" +
        "\t\tcx += 1;\n" +
        "\t\tgradient_offset = length(vec2(\n" +
        "\t\t\tpoint.x*axes.r + point.y*axes.b + origin.r,\n" +
        "\t\t\tpoint.x*axes.g + point.y*axes.a + origin.g\n" +
        "\t\t));\n" +

        // fallback to solid color
        "\t} else {\n" +
        "\t\tgradient_offset = 0.0;\n" +
        "\t}\n" +

        // detect current point of gradient
        "\tvec3 back = vec3(1.0);\n"
        "\tvec4 from = vec4(1.0);\n"
        "\tvec4 to = vec4(1.0);\n"


        "\tfor (int i=0; i<colors_count; i++){\n" +
        "\t\tvec4 c = texelFetch(config, ivec2(cx, cy), 0);\n" +
        "\t\tcx += 1;\n" +
        "\t\tif (i==0) {\n" +
        "\t\t\tfrom = c;\n" +
        "\t\t\tto = c;\n" +
        "\t\t}\n" +
        "\t\tif (c.a >= gradient_offset || i+1==colors_count){\n" +
        "\t\t\tto = c;\n" +
        "\t\t\tbreak;\n" +
        "\t\t}\n"
        "\t\tfrom = c;\n" +
        "\t}\n" +

        // calcualte actual gradient
        "\tfloat delta = to.a - from.a;\n" +
        "\tif (delta > 0.0){\n"
        "\t\tCOLOR.rgb = mix(from.rgb, to.rgb, (gradient_offset - from.a)/delta);\n" +
        "\t} else {\n" +
        "\t\tCOLOR.rgb = from.rgb;\n" +
        "\t}" +
        
        "}"
    );
    print_line(String("generated shader code:\n") + shader->get_code());
    return shader;
}

Ref<ShaderMaterial> VGTexture::create_material() {
    if (material.is_valid()){
        return material;
    }
    material.instance();
    material->set_shader(create_shader());
    material->set_shader_param("config", create_config());
    return material;
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
    Transform2D tr;
    tr.scale(p_rect.size/_get_size());
    tr.set_origin(tr.get_origin()+p_rect.position);
    VS::get_singleton()->canvas_item_add_set_transform(p_canvas_item, tr);
    VS::get_singleton()->canvas_item_set_material(p_canvas_item, material->get_rid());

    print_line(String("VGTexture::draw_rect") + " rect:" + Variant(p_rect) + ", tile: " + Variant(p_tile) + ", transpose: " + Variant(p_transpose));
    for (int i=0; i<shapes.size();i++){
       Ref<VGShape> shape = shapes[i];
        for (int j=0; j<shape->get_path_count(); j++){
            Ref<VGPath> path = shape->get_path(j);
            PoolVector2Array pool_points = path->tessellate(4, 2);
            Vector<Vector2> points;
            Vector<Color> colors;
            Vector<Vector2> uvs;
            points.resize(pool_points.size());
            colors.resize(pool_points.size());
            uvs.resize(pool_points.size());
            for (int k=0; k<pool_points.size();k++){
                colors.write[k] = Color(1,1,1,1);
                points.write[k] = pool_points[k];
                uvs.write[k] = Vector2(i,i) + (pool_points[k] - path->get_bounds().position) / path->get_bounds().size;
                //print_line(String("UV for bounds: ") + Variant(path->get_bounds()) + " " + Variant(uvs[k]));
            }

            VS::get_singleton()->canvas_item_add_polygon(p_canvas_item, points, colors, uvs, RID(), RID(), true);
            //VS::get_singleton()->canvas_item_add_set_transform()
            //VS::get_singleton()->canvas_item_set_clip(p_canvas_item, true);
        }
    }
}

void VGTexture::draw_rect_region(RID p_canvas_item, const Rect2 &p_rect, const Rect2 &p_src_rect, const Color &p_modulate, bool p_transpose, const Ref<Texture> &p_normal_map, bool p_clip_uv) const {
    print_line(String("VGTexture::draw_rect_region") + " rect:" + Variant(p_rect) + ", p_src_rect: " + Variant(p_src_rect) + ", clip: " + Variant(p_clip_uv) + ", transpose: " + Variant(p_transpose));
    Transform2D tr;
    tr.scale(p_rect.size/p_src_rect.size);
    tr.set_origin(tr.get_origin()-p_src_rect.position*tr.get_scale() + p_rect.position);
    VS::get_singleton()->canvas_item_add_set_transform(p_canvas_item, tr);
    VS::get_singleton()->canvas_item_set_custom_rect(p_canvas_item, true, p_rect);
    VS::get_singleton()->canvas_item_set_clip(p_canvas_item, true);
    //
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
                colors.write[k] = Color(1,1,1,1);
                points.write[k] = pool_points[k];
            }

            VS::get_singleton()->canvas_item_add_polygon(p_canvas_item, points, colors, Vector<Vector2>(), RID(), RID(), true);
            //VS::get_singleton()->canvas_item_add_set_transform()
            //VS::get_singleton()->canvas_item_set_clip(p_canvas_item, true);
        }
    }
    Transform2D tr2;
    //tr.scale(p_rect.size/p_src_rect.size);
    //tr2.set_origin(tr2.get_origin()+p_rect.position);
    //VS::get_singleton()->canvas_item_add_set_transform(p_canvas_item, tr2);
    //VS::get_singleton()->rect
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
    material = Ref<ShaderMaterial>();
    create_material();
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