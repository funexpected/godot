

#include "stylebox_bordered_texture.h"

void StyleBoxBorderedTexture::set_border_width(Side p_margin, int p_width) {
	ERR_FAIL_INDEX((int)p_margin, 4);
	border_width[p_margin] = p_width;
	emit_changed();
}

int StyleBoxBorderedTexture::get_border_width(Side p_margin) const {
	ERR_FAIL_INDEX_V((int)p_margin, 4, 0);
	return border_width[p_margin];
}

void StyleBoxBorderedTexture::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_border_width", "margin", "width"), &StyleBoxBorderedTexture::set_border_width);
	ClassDB::bind_method(D_METHOD("get_border_width", "margin"), &StyleBoxBorderedTexture::get_border_width);

    ADD_GROUP("Border Width", "border_width_");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_left", PROPERTY_HINT_RANGE, "0,1024,1"), "set_border_width", "get_border_width", SIDE_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_top", PROPERTY_HINT_RANGE, "0,1024,1"), "set_border_width", "get_border_width", SIDE_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_right", PROPERTY_HINT_RANGE, "0,1024,1"), "set_border_width", "get_border_width", SIDE_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_bottom", PROPERTY_HINT_RANGE, "0,1024,1"), "set_border_width", "get_border_width", SIDE_BOTTOM);
}

void StyleBoxBorderedTexture::draw(RID p_canvas_item, const Rect2 &p_rect) const {
    Ref<Texture2D> tex = get_texture();
    if (tex.is_null())
		return;
    Rect2 rect = p_rect;
	Rect2 src_rect = get_region_rect();
	tex->get_rect_region(rect, src_rect, rect, src_rect);

    rect.position.x -= get_expand_margin(SIDE_LEFT);
	rect.position.y -= get_expand_margin(SIDE_TOP);
	rect.size.x += get_expand_margin(SIDE_LEFT) + get_expand_margin(SIDE_RIGHT);
	rect.size.y += get_expand_margin(SIDE_TOP) + get_expand_margin(SIDE_BOTTOM);

    float tx[4] = { rect.position.x, rect.position.x + border_width[SIDE_LEFT], rect.position.x + rect.size.width - border_width[SIDE_RIGHT], rect.position.x + rect.size.width };
    float ty[4] = { rect.position.y, rect.position.y + border_width[SIDE_TOP], rect.position.y + rect.size.height - border_width[SIDE_BOTTOM], rect.position.y + rect.size.height };
    Vector2 ts[16] = {
        Vector2(tx[0], ty[0]), Vector2(tx[1], ty[0]), Vector2(tx[2], ty[0]), Vector2(tx[3], ty[0]),
        Vector2(tx[0], ty[1]), Vector2(tx[1], ty[1]), Vector2(tx[2], ty[1]), Vector2(tx[3], ty[1]),
        Vector2(tx[0], ty[2]), Vector2(tx[1], ty[2]), Vector2(tx[2], ty[2]), Vector2(tx[3], ty[2]),
        Vector2(tx[0], ty[3]), Vector2(tx[1], ty[3]), Vector2(tx[2], ty[3]), Vector2(tx[3], ty[3])
    };

    float sx[4] = { src_rect.position.x, src_rect.position.x + get_expand_margin(SIDE_LEFT), src_rect.position.x + src_rect.size.x - get_expand_margin(SIDE_RIGHT), src_rect.position.x + src_rect.size.x };
    float sy[4] = { src_rect.position.y, src_rect.position.y + get_expand_margin(SIDE_TOP), src_rect.position.y + src_rect.size.y - get_expand_margin(SIDE_BOTTOM), src_rect.position.y + src_rect.size.y };
    Vector2 ss[16] = {
        Vector2(sx[0], sy[0]), Vector2(sx[1], sy[0]), Vector2(sx[2], sy[0]), Vector2(sx[3], sy[0]),
        Vector2(sx[0], sy[1]), Vector2(sx[1], sy[1]), Vector2(sx[2], sy[1]), Vector2(sx[3], sy[1]),
        Vector2(sx[0], sy[2]), Vector2(sx[1], sy[2]), Vector2(sx[2], sy[2]), Vector2(sx[3], sy[2]),
        Vector2(sx[0], sy[3]), Vector2(sx[1], sy[3]), Vector2(sx[2], sy[3]), Vector2(sx[3], sy[3])
    };
    for (int i=0; i<11; i++) {
        if ( (i+1)%4 == 0) continue;
        if (i == 5 && !is_draw_center_enabled()) continue;
        Rect2 rect_part = Rect2(ts[i], ts[i+5] - ts[i]);
        Rect2 src_rect_part = Rect2(ss[i], ss[i+5] - ss[i]);
        if (rect_part.size.width == 0 || rect_part.size.height == 0) continue;

        RenderingServer::get_singleton()->canvas_item_add_texture_rect_region(
            p_canvas_item,
            rect_part,
            tex->get_rid(),
            src_rect_part,
            get_modulate(), 
            false,
            false
        );
    }
}

StyleBoxBorderedTexture::StyleBoxBorderedTexture() {
    border_width[0] = 0;
    border_width[1] = 0;
    border_width[2] = 0;
    border_width[3] = 0;
}