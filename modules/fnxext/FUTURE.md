## Example usage of asyncronouse coding in native environment

```c++
// future_example.h
#ifndef FUTURE_EXAMPLE_H
#define FUTURE_EXAMPLE_H

#include "future.h"

class FutureExample: public RefCounted {
    GDCLASS(FutureExample, RefCounted);
protected:
	static void _bind_methods();
public:
    Ref<Future> test();
    static Future* wait_seconds(float time);
    static Future* wait_three_times(float time);
};

#endif
```

```c++
// future_example.cpp
#include "future_example.h"
#include <future>
#include <scene/main/scene_tree.h>
#include <scene/main/timer.h>
#include <core/io/resource_loader.h>

std::future<void> future_eample_future_holder;

void FutureExample::_bind_methods() {
    ClassDB::bind_method("test", &FutureExample::test);
}

Future* FutureExample::wait_three_times(float time) {
    return Future::start()->then([time](){
        return FutureExample::wait_seconds(time)->await();
    })->then([time](){
        return FutureExample::wait_seconds(time)->await();
    })->then([time](Resolver resolve){
        resolve("won't wait!");
        return FutureExample::wait_seconds(time)->await();
    })->then([time](){
        float result = time * 3;
        print_line(String() + "Waited " + (Variant)result + " seconds");
        return result;
    });
}

Future* FutureExample::wait_seconds(float time) {
    Timer *timer = memnew(Timer);
    timer->set_wait_time(time);
    timer->set_one_shot(true);
    SceneTree::get_singleton()->get_current_scene()->add_child(timer);
    return Future::start()->then([timer](Resolver resolve){
        timer->start();
        return Future::yield(timer, "timeout");
    })->finally([timer](){
        timer->queue_delete();
    });
}

Ref<Future> FutureExample::test() {
    return Future::start()->then([](){
        print_line(String() + "Start executing.");
        return Variant();
    })->then([]() {
        print_line(String() + "Waiting 1 sec");
        return FutureExample::wait_seconds(1)->await();
    })->then([]() {
        print_line(String() + "Wating 1.5 secs");
        return FutureExample::wait_three_times(0.5)->await();
    })->then([](Variant value) {
        print_line(String() + "wait_three_times returned " + (Variant)value);
        return Variant();
    })->wait([](Resolver resolve) {
        print_line("Starting thread, wait 3 seconds");
        future_eample_future_holder = std::async(std::launch::async, [resolve](){
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            print_line("returning 'hello thread' from thread");
            resolve(ResourceLoader::load("res://icon.png"));
        });
    })->then([](String from_thread){
        print_line(String() + "Incoming result from thread: " + from_thread);
        print_line(String() + "Returing 21");
        return 21;
    })->then([](int value) {
        print_line(String() + "Taking " + (Variant)value);
        print_line("Givint gexture");
        Ref<Texture> icon = ResourceLoader::load("res://icon.png");
        return icon;
    })->then([](Ref<Texture> icon, Resolver resolve) {
        if (icon.is_valid()) {
            print_line(String() + "Getting icon " + icon->get_path());
        } else {
            print_line("Icon not resolved");
        }
        return icon;
    })->then([](Array value) {
        print_line("Awating array. Size: " + itos(value.size()));
        return Variant();
    })->finally([]() {
        print_line(String() + "Finalizing");

    // don't forget to call `retain` before exporting 
    // future results to public api (gdscipt, gdnative, etc.) 
    })->retain();
}
```