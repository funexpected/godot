#ifndef VISIBILITY_CONTROLLER_2D_H
#define VISIBILITY_CONTROLLER_2D_H

#include "scene/gui/control.h"
#include "scene/2d/parallax_layer.h"

class VisibilityController2D: public Control {
    GDCLASS(VisibilityController2D, Control);

    enum VisibilityType {
        UNKNOWN,
        VISIBLE,
        INVISIBLE
    };

protected:
    static Rect2 visible_rect;

    VisibilityType visibility;
    ParallaxLayer *layer;
    NodePath controlled_node;
    bool control_visibility;
    bool control_activity;

    void _notification(int p_what);
    static void _bind_methods();

    void setup_controlled_node();
    void update_visibility(bool forced=false);
    VisibilityType detect_visibility_outside_parallax();
    VisibilityType detect_visibility_inside_parallax();

public:
    bool _get(const String &p_name, Variant &r_ret) const;

    Rect2 get_bounding_box();

    NodePath get_controlled_node() const;
    void set_controlled_node(const NodePath &p_node);
    bool should_control_visibility() const;
    void set_control_visibility(bool p_value);
    bool should_control_activity() const;
    void set_control_activity(bool p_value);
    

    bool is_visible_on_screen();
    VisibilityController2D();
    ~VisibilityController2D();

};

#endif