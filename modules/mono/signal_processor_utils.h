#ifndef SIGNAL_PROCESSOR_UTILS_H
#define SIGNAL_PROCESSOR_UTILS_H

#include "core/object.h"
// not sure how to include MonoObject, so use mono_
#include "mono_gc_handle.h"

namespace SignalProcessorUtils {
    Error process_signals(MonoObject *p_target, const Variant **p_args, int p_argcount, Variant::CallError &r_error);
}

class SignalReceiverHandle : public Reference {

	GDCLASS(SignalReceiverHandle, Reference);

    static Ref<SignalReceiverHandle> instance;

	Variant _signal_callback(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
    void _deferred_call(int p_id);

protected:

	static void _bind_methods();

public:
    static SignalReceiverHandle* get_instance();
    static void finish();
};

#endif