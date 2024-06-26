#ifndef FUTURE_H
#define FUTURE_H

#include <functional>
#include <core/reference.h>

typedef std::function<void (Variant)> Resolver;

class Yield: public Reference {
    GDCLASS(Yield, Reference)
    Ref<Reference> keeped;

protected:
	static void _bind_methods();

public:
    void keep(Ref<Reference> obj);
    Variant complete(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
};

class Future: public Reference {
    GDCLASS(Future, Reference);

    Future *next;
    Ref<Future> prev;
    Ref<Yield> yield_instruction;
    std::function<Variant (Variant arg, Resolver resolve)> continuation;
    std::function<void ()> finalizer;
    bool retained;

protected:
	static void _bind_methods();
    Future* _then(std::function<Variant (Variant, Resolver)> p_callback);
    Future* _wait(std::function<void (Variant, Resolver)> p_callback);
    void _complete(Variant arg);
    void _continue_execution(Variant arg = Variant(), bool resolved = false);
    void _release(Variant arg=Variant());

public:
    static Ref<Yield> yield(Object *obj, const String &signal);
    static Future* start();
    Future* then(std::function<Variant (Variant, Resolver)> p_callback);
    Future* then(std::function<Variant (Variant)> p_callback);
    Future* then(std::function<Variant (Resolver)> p_callback);
    Future* then(std::function<Variant ()> p_callback);
    Future* wait(std::function<void (Variant, Resolver)> p_callback);
    Future* wait(std::function<void (Resolver)> p_callback);
    Future* finally(std::function<void ()> p_finalizer);
    Ref<Future> retain();
    Ref<Yield> await();

    Ref<Future> test();
    static Future* wait_seconds(float time);
    static Future* wait_three_times(float time);

    Future();
    ~Future();
};

#endif