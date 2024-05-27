#ifndef STYLE_BOX_BORDERED_TEXTURE_H
#define STYLE_BOX_BORDERED_TEXTURE_H

// #include <scene/resources/style_box.h>
#include "scene/resources/style_box_texture.h"

class StyleBoxBorderedTexture: public StyleBoxTexture {
    
    GDCLASS(StyleBoxBorderedTexture, StyleBoxTexture);
    int border_width[4];

protected:
    static void _bind_methods();

public:

    void set_border_width(Side p_margin, int p_width);
	int get_border_width(Side p_margin) const;

    virtual void draw(RID p_canvas_item, const Rect2 &p_rect) const;

    StyleBoxBorderedTexture();

};

#endif //STYLE_BOX_BORDERED_TEXTURE_H