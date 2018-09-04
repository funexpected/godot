#ifndef PARALLAX_VISIBILITY_NOTIFIER_H
#define PARALLAX_VISIBILITY_NOTIFIER_H

#include "scene/2d/node_2d.h"
#include "scene/2d/parallax_layer.h"

class ParallaxVisibilityNotifier: public Node2D {
    GDCLASS(ParallaxVisibilityNotifier, Node2D);

    enum VisibilityType {
        UNKNOWN,
        VISIBLE,
        INVISIBLE
    };

protected:
    VisibilityType visibility;
    ParallaxLayer *layer;
    Rect2 rect;

    void _notification(int p_what);
    static void _bind_methods();

public:
    Rect2 get_global_rect();
    virtual void _edit_set_rect(const Rect2 &p_edit_rect);
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

    Rect2 get_rect() const;
    void set_rect(const Rect2 &p_rect);

    bool is_visible_on_screen();
    ParallaxVisibilityNotifier();
    ~ParallaxVisibilityNotifier();

};

#endif