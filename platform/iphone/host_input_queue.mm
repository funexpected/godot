/*************************************************************************/
/*  host_input_queue.mm                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/

#ifdef IPHONE_ENABLED

#include "host_input_queue.h"

#include "os_iphone.h"

HostInputQueue::HostInputQueue() :
		head(0), tail(0) {
}

HostInputQueue &HostInputQueue::get() {
	static HostInputQueue instance;
	return instance;
}

bool HostInputQueue::push(const HostInputEvent &e) {
	uint32_t h = head.load(std::memory_order_relaxed);
	uint32_t t = tail.load(std::memory_order_acquire);
	if (h - t >= CAPACITY) {
		// Full — drop. Touch backlog this deep means the engine thread is
		// stalled; further events would be stale by the time we drain.
		return false;
	}
	ring[h % CAPACITY] = e;
	head.store(h + 1, std::memory_order_release);
	return true;
}

void HostInputQueue::drain() {
	OSIPhone *os = OSIPhone::get_singleton();
	if (!os) {
		return;
	}

	uint32_t t = tail.load(std::memory_order_relaxed);
	uint32_t h = head.load(std::memory_order_acquire);
	while (t != h) {
		const HostInputEvent &e = ring[t % CAPACITY];
		switch (e.kind) {
			case HostInputEvent::KIND_TOUCH_PRESS:
				os->touch_press(e.idx, (int)e.x, (int)e.y, e.pressed, e.doubleclick);
				break;
			case HostInputEvent::KIND_TOUCH_DRAG:
				os->touch_drag(e.idx, (int)e.prev_x, (int)e.prev_y, (int)e.x, (int)e.y);
				break;
			case HostInputEvent::KIND_TOUCH_CANCEL:
				os->touches_cancelled(e.idx);
				break;
			case HostInputEvent::KIND_KEY:
				os->key(e.key, e.pressed);
				break;
		}
		t++;
	}
	tail.store(t, std::memory_order_release);
}

namespace HostInput {

void touch_press(int idx, float x, float y, bool pressed, bool doubleclick) {
	HostInputEvent e = {};
	e.kind = HostInputEvent::KIND_TOUCH_PRESS;
	e.idx = idx;
	e.x = x;
	e.y = y;
	e.pressed = pressed;
	e.doubleclick = doubleclick;
	HostInputQueue::get().push(e);
}

void touch_drag(int idx, float prev_x, float prev_y, float x, float y) {
	HostInputEvent e = {};
	e.kind = HostInputEvent::KIND_TOUCH_DRAG;
	e.idx = idx;
	e.prev_x = prev_x;
	e.prev_y = prev_y;
	e.x = x;
	e.y = y;
	HostInputQueue::get().push(e);
}

void touch_cancel(int idx) {
	HostInputEvent e = {};
	e.kind = HostInputEvent::KIND_TOUCH_CANCEL;
	e.idx = idx;
	HostInputQueue::get().push(e);
}

void key(uint32_t scancode, bool pressed) {
	HostInputEvent e = {};
	e.kind = HostInputEvent::KIND_KEY;
	e.key = scancode;
	e.pressed = pressed;
	HostInputQueue::get().push(e);
}

} // namespace HostInput

#endif // IPHONE_ENABLED
