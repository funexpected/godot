
// #include "future.h"

// Variant Yield::complete(const Variant **p_args, int p_argcount, Variant::Fstatic void interpolate &r_error) {
//     if (p_argcount == 0) {
//         Variant args[1] = { Variant() };
//         const Variant *argptrs[1] = { &args[0] };
//         emit_signal("completed", (const Variant **)argptrs, 1);
//     } else if (p_argcount == 1) {
//         emit_signal("completed", p_args, 1);
//     } else {
//         Array result;
//         for (int i=0; i<p_argcount; i++) {
//             result.push_back(p_args[i]);
//         }
//         emit_signal("completed", result);
//         Variant args[1] = { result };
//         const Variant *argptrs[4] = { &args[0] };
//         emit_signal("completed", (const Variant **)argptrs, 1);
//     }
//     return Variant();
// }

// void Yield::keep(Ref<RefCounted> obj) {
//     keeped = obj;
// }

// void Yield::_bind_methods() {
//     {
//         MethodInfo mi;
//         mi.name = "complete";
//         Vector<Variant> defargs;
//         ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "complete", &Yield::complete, mi, defargs);
//     }
//     ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::NIL, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
// }

// Ref<Yield> Future::yield(Object *obj, const String &signal) {
//     Ref<Yield> yields;
//     yields.instance();
//     obj->connect(signal, yields.ptr(), "complete");
//     RefCounted *objref = Object::cast_to<RefCounted>(obj);
//     if (objref != NULL) {
//         yields->keep(Ref<RefCounted>(objref));
//     }
//     return yields;
// }

// Future* Future::start() {
//     Future* future = memnew(Future);
//     future->call_deferred("_continue_execution", Variant(), false);
//     return future;
// }


// Future* Future::_then(std::function<Variant (Variant, Resolver)> p_callback) {
//     if (continuation == NULL) {
//         continuation = p_callback;
//         return this;
//     } else {
//         Future* future = memnew(Future);
//         future->prev = Ref<Future>(this);
//         next = future;
//         future->continuation = p_callback;
//         return future;
//     }
// }

// Future* Future::_wait(std::function<void (Variant, Resolver)> p_callback) {
//     return _then([p_callback](Variant arg, Resolver _) {
//         Ref<Yield> yields;
//         yields.instance();
//         Yield* yieldptr = yields.ptr();
//         p_callback(arg, [yieldptr](Variant result) {
//             yieldptr->call_deferred("complete", result);
//         });
//         return yields;
//     });
// }


// Future* Future::then(std::function<Variant (Variant, Resolver)> p_callback) {
//     return _then(p_callback);
// }

// Future* Future::then(std::function<Variant (Variant)> p_callback) {
//     return _then([p_callback](Variant arg, Resolver _r) -> Variant {
//         return p_callback(arg);
//     });
// }

// Future* Future::then(std::function<Variant (Resolver)> p_callback) {
//     return _then([p_callback](Variant _v, Resolver resolver) -> Variant {
//         return p_callback(resolver);
//     });
// }

// Future* Future::then(std::function<Variant ()> p_callback) {
//     return _then([p_callback](Variant _v, Resolver _r) -> Variant {
//         return p_callback();
//     });
// }

// Future* Future::wait(std::function<void (Variant, Resolver)> p_callback) {
//     return _wait(p_callback);
// }

// Future* Future::wait(std::function<void (Resolver)> p_callback) {
//     return _wait([p_callback](Variant _v, Resolver resolver) {
//         p_callback(resolver);
//     });
// }

// // Ref<Future> Future::then(std::function<void (Variant, Resolver)> p_callback) {
// //     return _then([p_callback](Variant arg, Resolver resolver) -> Variant {
// //         p_callback(arg, resolver);
// //         return Variant();
// //     });
// // }

// // Ref<Future> Future::then(std::function<void (Variant)> p_callback) {
// //     return _then([p_callback](Variant arg, Resolver resolver) -> Variant {
// //         p_callback(arg);
// //         return Variant();
// //     });
// // }

// // Ref<Future> Future::then(std::function<void (Resolver)> p_callback) {
// //     return _then([p_callback](Variant arg, Resolver resolver) -> Variant {
// //         p_callback(resolver);
// //         return Variant();
// //     });
// // }

// // Ref<Future> Future::then(std::function<void ()> p_callback) {
// //     return _then([p_callback](Variant arg, Resolver resolver) -> Variant {
// //         p_callback();
// //         return Variant();
// //     });
// // }

// void Future::_continue_execution(Variant p_arg, bool p_resolved) {
//     if (prev.is_valid()) {
//         prev.unref();
//     }
//     if (p_resolved) {
//         if (next != NULL) {
//             //_release();
//             next->_continue_execution(p_arg, true);
//         } else {
//             _complete(p_arg);
//         }
//         return;
//     }
//     bool resolved = false;
//     Variant resolve;
//     Variant arg = continuation(p_arg, [&resolved, &resolve](Variant result){
//         resolved = true;
//         resolve = result;
//     });
//     arg = resolved ? resolve : arg;
//     yield_instruction = arg;
//     if (yield_instruction.is_valid()) {
//         if (next != NULL) {
//             yield_instruction->connect("completed", next, "_continue_execution", varray(resolved), CONNECT_DEFERRED);
//         } else {
//             yield_instruction->connect("completed", this, "_complete");
//         }
//     } else {
//         if (next != NULL) {
//             next->_continue_execution(arg, resolved);
//         } else {
//             _complete(arg);
//         }
//     }
// }

// void Future::_complete(Variant arg) {    
//     emit_signal("completed", arg);
// }

// Ref<Yield> Future::await() {
//     return yield(this, "completed");
// }

// void Future::_release(Variant arg) {
//     retained = false;
//     unRefCounted();
// }

// Future* Future::finally(std::function<void ()> p_finalizer) {
//     finalizer = p_finalizer;
//     return this;
// }

// Ref<Future> Future::retain() {
//     Ref<Future> result = Ref<Future>(this);
//     if (!retained) {
//         result->RefCounted();
//         result->connect("completed", this, "_release", varray(), CONNECT_DEFERRED);
//         retained = true;
//     }
//     return result;
// }


// void Future::_bind_methods() {
//     ClassDB::bind_method(D_METHOD("_complete", "arg"), &Future::_complete);
//     ClassDB::bind_method(D_METHOD("_continue_execution", "arg", "resolved"), &Future::_continue_execution, DEFVAL(Variant()), DEFVAL(false));
//     ClassDB::bind_method(D_METHOD("_release", "arg"), &Future::_release, DEFVAL(Variant()));
   
//     ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::NIL, "result", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NIL_IS_VARIANT)));
// }

// Future::Future(){
//     finalizer = NULL;
//     next = NULL;
//     continuation = NULL;
//     retained = false;
// }
// Future::~Future(){
//     if (finalizer != NULL) {
//         finalizer();
//     }
//     //print_line("destroing future");
// }

