#include "signal_processor_utils.h"

//#include "csharp_script.h"
#include "mono_gd/gd_mono_cache.h"

Ref<SignalReceiverHandle> SignalReceiverHandle::instance;

Variant SignalReceiverHandle::_signal_callback(const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
    if (p_argcount < 2) {
        r_error.error = Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
        r_error.argument = 2;
        return Variant();
    }

    Object *target = *p_args[p_argcount - 2];
    if (target == NULL) {
        r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
        r_error.argument = p_argcount - 2;
        r_error.expected = Variant::OBJECT;
        return Variant();
    }
    
    mono_byte processor_index = *p_args[p_argcount - 1];
    MonoObject *mono_target = GDMonoMarshal::variant_to_mono_object(*p_args[p_argcount-2]);
    MonoArray *signal_args = mono_array_new(mono_domain_get(), CACHED_CLASS_RAW(MonoObject), p_argcount - 1);

    for (int i = 0; i < p_argcount-2; i++) {
        MonoObject *boxed = GDMonoMarshal::variant_to_mono_object(*p_args[i]);
        mono_array_setref(signal_args, i, boxed);
    }

    MonoException *exc = NULL;
    GD_MONO_BEGIN_RUNTIME_INVOKE;
    CACHED_METHOD_THUNK(SignalProcessor, ProcessSignal).invoke(mono_target, processor_index, signal_args, &exc);
    GD_MONO_END_RUNTIME_INVOKE;
    if (exc) {
        GDMonoUtils::set_pending_exception(exc);
        ERR_FAIL_V(Variant());
    }

    return Variant();
}

void SignalReceiverHandle::_deferred_call(int p_id) {
    MonoException *exc = NULL;
    GD_MONO_BEGIN_RUNTIME_INVOKE;
    CACHED_METHOD_THUNK(SignalProcessor, ProcessDeferredCall).invoke(p_id, &exc);
    GD_MONO_END_RUNTIME_INVOKE;
    if (exc) {
        GDMonoUtils::set_pending_exception(exc);
    }

}

void SignalReceiverHandle::_bind_methods() {
    ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "_signal_callback", &SignalReceiverHandle::_signal_callback, MethodInfo("_signal_callback"));
    ClassDB::bind_method(D_METHOD("_deferred_call", "id"), &SignalReceiverHandle::_deferred_call);
}

SignalReceiverHandle *SignalReceiverHandle::get_instance() {
    if (SignalReceiverHandle::instance.is_null()) {
        SignalReceiverHandle::instance.instance();
    }
    return SignalReceiverHandle::instance.ptr();
}

void SignalReceiverHandle::finish() {
    if (SignalReceiverHandle::instance.is_valid()) {
        SignalReceiverHandle::instance.unref();
    }
}