/*************************************************************************/
/*  host_input_queue.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/

#ifndef HOST_INPUT_QUEUE_H
#define HOST_INPUT_QUEUE_H

#ifdef IPHONE_ENABLED

#include <atomic>
#include <stdint.h>

// Single-producer / single-consumer ring buffer used to ferry UIKit-thread
// input (touch/keyboard/joypad) into the engine worker thread that drives
// Main::iteration. Events are POD; the consumer rebuilds Ref<InputEvent>s
// locally so reference-counting stays thread-confined.
//
// Producer: any UIKit-callback context (touchesBegan/Moved/Ended,
// UITextView delegate, GCController valueChangedHandler — all main-queue
// by default).
// Consumer: OSIPhone::iterate(), which calls drain() before Main::iteration.
//
// Capacity is fixed and modest: at 60Hz, even rapid multi-touch sessions
// don't exceed a few dozen events per frame. Overflow drops events.
struct HostInputEvent {
	enum Kind {
		KIND_TOUCH_PRESS,
		KIND_TOUCH_DRAG,
		KIND_TOUCH_CANCEL,
		KIND_KEY,
	};
	Kind kind;
	int idx;
	float x, y, prev_x, prev_y;
	bool pressed;
	bool doubleclick;
	uint32_t key;
};

class HostInputQueue {
public:
	static HostInputQueue &get();

	// Push from the producer thread. Returns false if the ring is full.
	bool push(const HostInputEvent &e);

	// Drain everything currently queued, dispatching each event to OSIPhone.
	// Must be called from the engine worker thread (the one that owns
	// InputDefault::parse_input_event).
	void drain();

private:
	HostInputQueue();
	HostInputQueue(const HostInputQueue &) = delete;
	HostInputQueue &operator=(const HostInputQueue &) = delete;

	static const uint32_t CAPACITY = 1024;
	HostInputEvent ring[CAPACITY];
	std::atomic<uint32_t> head; // next slot the producer writes
	std::atomic<uint32_t> tail; // next slot the consumer reads
};

// Convenience producer entry points for UIKit callsites. These build a
// HostInputEvent and push it; safe to call from any thread.
namespace HostInput {

void touch_press(int idx, float x, float y, bool pressed, bool doubleclick);
void touch_drag(int idx, float prev_x, float prev_y, float x, float y);
void touch_cancel(int idx);
void key(uint32_t scancode, bool pressed);

} // namespace HostInput

#endif // IPHONE_ENABLED

#endif // HOST_INPUT_QUEUE_H
