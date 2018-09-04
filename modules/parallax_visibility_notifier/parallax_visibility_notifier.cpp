
#include "parallax_visibility_notifier.h"
#include "core/engine.cpp"

void ParallaxVisibilityNotifier::_edit_set_rect(const Rect2 &p_edit_rect) {
	rect = p_edit_rect;
    if (Engine::get_singleton()->is_editor_hint()) {
        update();
        item_rect_changed();
    }
    _change_notify("rect");
}

Rect2 ParallaxVisibilityNotifier::_edit_get_rect() const {
	return rect;
}

bool ParallaxVisibilityNotifier::_edit_use_rect() const {
	return true;
}

Rect2 ParallaxVisibilityNotifier::get_rect() const {
    return rect;
}

void ParallaxVisibilityNotifier::set_rect(const Rect2 &p_rect){
    rect = p_rect;
    if (Engine::get_singleton()->is_editor_hint()) {
        update();
        item_rect_changed();
    }
    _change_notify("rect");
}

Rect2 ParallaxVisibilityNotifier::get_global_rect(){
    Vector2 gp = get_global_position();
    Vector2 gs = get_global_scale();
    return Rect2(gp + rect.position * gs, rect.size * gs);
}



void ParallaxVisibilityNotifier::_notification(int p_what){
    switch (p_what){
        case NOTIFICATION_ENTER_TREE: {
            Node *parent = get_parent();
            while (parent != NULL && Object::cast_to<ParallaxLayer>(parent) == NULL){
                parent = parent->get_parent();
            }
            if (parent != NULL){
                layer = Object::cast_to<ParallaxLayer>(parent);
            }
        }break;
        case Node::NOTIFICATION_PROCESS: {
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
            VisibilityType now_visible = VisibilityType::INVISIBLE;
            for (int i=0; i<points.size(); i++){
                Vector2 gp = points[i];
                while (mo.x && gp.x < 0) gp.x += mo.x;
                while (mo.x && gp.x > mo.x) gp.x -= mo.x;
                while (mo.y && gp.y < 0) gp.y += mo.y;
                while (mo.y && gp.y > mo.y) gp.y -= mo.y;
                if (vpr.has_point(gp)){
                    now_visible = VisibilityType::VISIBLE;
                    break;
                }
            }
            if (now_visible != visibility) emit_signal(now_visible == VisibilityType::VISIBLE ? "screen_entered" : "screen_exited");
            //print_line(String("") + Variant(gp0) + " " + Variant(vpr));
            visibility = now_visible;
        } break;
    }
}

bool ParallaxVisibilityNotifier::is_visible_on_screen(){ return visibility; }

void ParallaxVisibilityNotifier::_bind_methods(){
    ClassDB::bind_method(D_METHOD("is_visible_on_screen"), &ParallaxVisibilityNotifier::is_visible_on_screen);
    ClassDB::bind_method(D_METHOD("get_global_rect"), &ParallaxVisibilityNotifier::get_global_rect);
    ClassDB::bind_method(D_METHOD("get_rect"), &ParallaxVisibilityNotifier::get_rect);
    ClassDB::bind_method(D_METHOD("set_rect", "rect"), &ParallaxVisibilityNotifier::set_rect);
    ADD_PROPERTY(PropertyInfo(Variant::RECT2, "rect"), "set_rect", "get_rect");
    ADD_SIGNAL(MethodInfo("screen_entered"));
    ADD_SIGNAL(MethodInfo("screen_exited"));
}

ParallaxVisibilityNotifier::ParallaxVisibilityNotifier(){
    print_line("create visibility notifier");
    layer = NULL;
}

ParallaxVisibilityNotifier::~ParallaxVisibilityNotifier(){
}