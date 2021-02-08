
#include "visibility_controller_2d.h"
#include "core/engine.h"

#ifdef TOOLS_ENABLED
#include "canvas_layers_editor_plugin.h"
#endif

Rect2 VisibilityController2D::visible_rect = Rect2(Vector2(-50, -50), Vector2(100, 100));

// void VisibilityController2D::_edit_set_rect(const Rect2 &p_edit_rect) {
//     return Node2D::_edit_set_rect(p_edit_rect);
// 	// rect = p_edit_rect;
//     // // print_line(String("edit_set_rect ") + p_edit_rect, + " " +get_global_mouse_position());
//     // if (Engine::get_singleton()->is_editor_hint()) {
//     //     update();
//     //     item_rect_changed();
//     // }
//     // _change_notify("rect");
// }

Rect2 VisibilityController2D::_edit_get_rect() const {
    return visible_rect;
}

bool VisibilityController2D::_edit_use_rect() const {
	return true;
}

bool VisibilityController2D::_get(const String &p_name, Variant &r_ret) const {
    if (p_name == "editor/canvas_layer") {
        r_ret = "visibility_controller";
        return true;
    } else {
        return false;
    }
}

NodePath VisibilityController2D::get_controlled_node() const {
    return controlled_node;
}

void VisibilityController2D::set_controlled_node(const NodePath &p_node) {
    controlled_node = p_node;
    setup_controlled_node();
}

bool VisibilityController2D::should_control_visibility() const {
    return control_visibility;
}

void VisibilityController2D::set_control_visibility(bool p_value) {
    control_visibility = p_value;
    update_visibility(true);
}

bool VisibilityController2D::should_control_activity() const {
    return control_activity;
}

void VisibilityController2D::set_control_activity(bool p_value) {
    control_activity = p_value;
    update_visibility(true);
}

Rect2 VisibilityController2D::get_global_rect(){
    Rect2 rect = visible_rect;
    Transform2D tr = get_global_transform();
    Rect2 gr = Rect2(tr.xform(rect.position), Vector2());
    gr = gr.expand(tr.xform(rect.position + rect.size * Vector2(1, 0)));
    gr = gr.expand(tr.xform(rect.position + rect.size * Vector2(0, 1)));
    gr = gr.expand(tr.xform(rect.position + rect.size));
    return gr;
}

void VisibilityController2D::set_rect(const Rect2 &p_rect) {
    set_position(p_rect.position + p_rect.size*0.5);
    set_scale(p_rect.size / visible_rect.size);
}

Rect2 VisibilityController2D::get_rect() const {
    Vector2 size = visible_rect.size * get_scale();
    return Rect2(-size*0.5, size);
}

void VisibilityController2D::setup_controlled_node() {
    Node *parent = get_node(controlled_node);
    while (parent != NULL && Object::cast_to<ParallaxLayer>(parent) == NULL){
        parent = parent->get_parent();
    }
    if (parent != NULL){
        layer = Object::cast_to<ParallaxLayer>(parent);
    }
}

void VisibilityController2D::update_visibility(bool force) {
    VisibilityType active_visibility = 
        !is_inside_tree() ? VisibilityType::INVISIBLE :
        layer == NULL ? detect_visibility_outside_parallax() : detect_visibility_inside_parallax();
    
    if (!force && active_visibility == visibility) return;
    bool visible = active_visibility == VisibilityType::VISIBLE;

    if (visibility != active_visibility) {
        emit_signal(visible ? "screen_entered" : "screen_exited");
        visibility = active_visibility;
    }

    CanvasItem* item = Object::cast_to<CanvasItem>(get_node(controlled_node));
    if (item == NULL) return;

    if (control_visibility) item->set_visible(visible);
    if (control_activity) item->set_branch_paused(!visible);
}
VisibilityController2D::VisibilityType VisibilityController2D::detect_visibility_outside_parallax() {
    Rect2 vp_rect = get_viewport_rect();
    Rect2 global_rect = get_global_rect();
    return global_rect.intersects(vp_rect) ?  VisibilityType::VISIBLE : VisibilityType::INVISIBLE;
}

VisibilityController2D::VisibilityType VisibilityController2D::detect_visibility_inside_parallax() {
    Rect2 vpr = get_viewport_rect();
    Rect2 fr = get_global_rect();
    Vector2 num_parts = fr.size / vpr.size;
    num_parts.x = ceil(num_parts.x);
    num_parts.y = ceil(num_parts.y);
    Vector2 part_size = Vector2(fr.size.x/num_parts.x, fr.size.y/num_parts.y);
    Vector<Vector2> points;
    for (int xp=0; xp<num_parts.x; xp++){
        for (int yp=0; yp<num_parts.y; yp++){
            Vector2 base = Vector2(fr.position.x + xp*part_size.x, fr.position.y + yp*part_size.y);
            points.push_back(base);
            if (xp == num_parts.x-1){
                points.push_back(base+Vector2(part_size.x,0));
            }
            if (yp == num_parts.y-1){
                points.push_back(base+Vector2(0, part_size.y));
            }
            if (xp == num_parts.x-1 && yp == num_parts.y-1){
                points.push_back(base+part_size);
            }
        }
    }

    Vector2 mo = layer == NULL ? Vector2() : layer->get_mirroring()*layer->get_global_scale();
    VisibilityType active_visibility = VisibilityType::INVISIBLE;
    for (int i=0; i<points.size(); i++){
        Vector2 gp = points[i];
        while (mo.x && gp.x < 0) gp.x += mo.x;
        while (mo.x && gp.x > mo.x) gp.x -= mo.x;
        while (mo.y && gp.y < 0) gp.y += mo.y;
        while (mo.y && gp.y > mo.y) gp.y -= mo.y;
        if (vpr.has_point(gp)){
            active_visibility = VisibilityType::VISIBLE;
            break;
        }
    }
    return active_visibility;
}

void VisibilityController2D::_notification(int p_what){
    if (Engine::get_singleton()->is_editor_hint()) {
#ifdef TOOLS_ENABLED
        if (p_what == NOTIFICATION_DRAW && CanvasLayersEditorPlugin::get_singleton()->is_layer_visible("visibility_controller")) {
            Rect2 rect = visible_rect;
            draw_rect(rect, Color::html("3219f064"), true);
        }
#endif
        return;
    }
    switch (p_what){
        case NOTIFICATION_ENTER_TREE: {
            setup_controlled_node();
            update_visibility();
            set_notify_transform(true);
        }break;
        case NOTIFICATION_TRANSFORM_CHANGED: {
            update_visibility();
        } break;
    }
}

bool VisibilityController2D::is_visible_on_screen(){ return visibility; }

void VisibilityController2D::_bind_methods(){
    ClassDB::bind_method(D_METHOD("is_visible_on_screen"), &VisibilityController2D::is_visible_on_screen);
    ClassDB::bind_method(D_METHOD("get_global_rect"), &VisibilityController2D::get_global_rect);
    ClassDB::bind_method(D_METHOD("get_rect"), &VisibilityController2D::get_rect);
    ClassDB::bind_method(D_METHOD("set_rect", "rect"), &VisibilityController2D::set_rect);
    ClassDB::bind_method(D_METHOD("get_controlled_node"), &VisibilityController2D::get_controlled_node);
    ClassDB::bind_method(D_METHOD("set_controlled_node", "path"), &VisibilityController2D::set_controlled_node);
    ClassDB::bind_method(D_METHOD("should_control_visibility"), &VisibilityController2D::should_control_visibility);
    ClassDB::bind_method(D_METHOD("set_control_visibility", "value"), &VisibilityController2D::set_control_visibility);
    ClassDB::bind_method(D_METHOD("should_control_activity"), &VisibilityController2D::should_control_activity);
    ClassDB::bind_method(D_METHOD("set_control_activity", "value"), &VisibilityController2D::set_control_activity);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "controlled_node"), "set_controlled_node", "get_controlled_node");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "control_visibility"), "set_control_visibility", "should_control_visibility");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "control_activity"), "set_control_activity", "should_control_activity");

    ADD_SIGNAL(MethodInfo("screen_entered"));
    ADD_SIGNAL(MethodInfo("screen_exited"));
}

VisibilityController2D::VisibilityController2D(){
    controlled_node = NodePath("..");
    control_visibility = true;
    control_activity = false;
    layer = NULL;
}

VisibilityController2D::~VisibilityController2D(){
}