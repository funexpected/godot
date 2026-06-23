/*************************************************************************/
/*  library_entry.mm                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/

#ifdef IOS_LIBRARY_MODE

#import "library_entry.h"

#import "app_delegate.h"
#import "godot_view.h"
#import "view_controller.h"

#include "core/os/thread.h"
#include "core/project_settings.h"
#include "core/ustring.h"
#include "os_iphone.h"

#include <atomic>
#include <pthread.h>

#define kRenderingFrequency 60

extern int iphone_main(int, char **, String);
extern void iphone_finish();

NSNotificationName const GodotLibraryReadyNotification = @"GodotLibraryReadyNotification";

// === Boot state ============================================================
//
// godot_library_start runs on the host's UIKit main thread and must return
// quickly so RN can keep its UI responsive — including showing a splash /
// spinner while the engine boots. The engine itself must not touch the UIKit
// main thread once running. Resolution: godot_library_start spawns a
// dedicated pthread (the "engine thread") and returns immediately. The
// engine thread runs Main::setup on itself, then signals readiness via:
//   - flipping g_ready_state to a non-zero value (poll API), and
//   - posting GodotLibraryReadyNotification (push API).
// godot_library_make_view_controller blocks on g_setup_done_sem so a host
// that doesn't poll/observe can just call it lazily and pay the wait cost
// only if setup hasn't finished yet (usually zero by then).
//
// From the moment focus_in fires onward, every engine-touching call is
// marshalled onto the engine thread's CFRunLoop. CADisplayLink attaches to
// that runloop too, so drawView / Main::iteration fire off-main.

static BOOL g_started = NO;
static pthread_t g_engine_thread;
static CFRunLoopRef g_engine_runloop = NULL;
static dispatch_semaphore_t g_setup_done_sem = NULL;
static int g_setup_result = -1;
// 0 = not done, 1 = success, <0 = -|err|. Atomic so the host's poll path
// doesn't need to take any locks.
static std::atomic<int> g_ready_state{ 0 };

// argv backing — same lifetime requirement as before.
static char **g_argv = NULL;
static int g_argc = 0;
static NSString *g_data_dir = nil;

static void build_argv_from_array(NSArray<NSString *> *cmdline) {
	NSString *exe_path = [[NSBundle mainBundle] executablePath] ?: @"godot";
	NSMutableArray<NSString *> *all = [NSMutableArray arrayWithObject:exe_path];
	if (cmdline != nil) {
		[all addObjectsFromArray:cmdline];
	}
	g_argc = (int)all.count;
	g_argv = (char **)calloc(g_argc + 1, sizeof(char *));
	for (int i = 0; i < g_argc; i++) {
		const char *src = [all[i] UTF8String];
		size_t n = strlen(src) + 1;
		g_argv[i] = (char *)malloc(n);
		memcpy(g_argv[i], src, n);
	}
	g_argv[g_argc] = NULL;
}

// CFRunLoopSource trampoline — empty perform; only here so the runloop has
// at least one source attached and CFRunLoopRun() doesn't return immediately
// before CADisplayLink gets installed via focus_in.
static void engine_runloop_keepalive_perform(void *) {
}

static void *engine_thread_main(void *) {
	pthread_setname_np("godot.engine");

	@autoreleasepool {
		// Pin Godot's notion of "main thread" to THIS pthread, so every
		// is_main_thread() check across the engine (GDScript, MessageQueue,
		// VisualServer wrap-MT, etc.) treats this thread as main. Done before
		// Main::setup so register_core_types runs with the correct identity.
		Thread::make_main_thread();

		g_engine_runloop = CFRunLoopGetCurrent();

		NSDate *t0 = [NSDate date];
		int err = iphone_main(g_argc, g_argv, String::utf8([g_data_dir UTF8String]));
		NSTimeInterval dt = [[NSDate date] timeIntervalSinceDate:t0];
		NSLog(@"[godot-lib] engine thread: iphone_main returned %d after %.3fs", err, dt);

		g_setup_result = err;

		if (err == 0) {
			bool keep_screen_on = bool(GLOBAL_DEF("display/window/energy_saving/keep_screen_on", true));
			NSLog(@"[godot-lib] keep_screen_on=%d", keep_screen_on);
			OSIPhone::get_singleton()->set_keep_screen_on(keep_screen_on);
		}

		// Publish readiness BEFORE the notification, so observers that re-
		// query state (e.g. via godot_library_is_ready) see the final value.
		// 1 = success; <0 = -|err| failure code.
		int ready_value;
		if (err == 0) {
			ready_value = 1;
		} else {
			int abs_err = err < 0 ? -err : err;
			ready_value = -abs_err;
			if (ready_value == 0) {
				ready_value = -1;
			}
		}
		g_ready_state.store(ready_value, std::memory_order_release);

		// Release godot_library_make_view_controller and any other callers
		// that chose the "block and wait" API. dispatch_semaphore_signal is
		// idempotent in the sense that subsequent waits return immediately
		// once the count is non-zero — we bump once per call by signaling
		// only here.
		dispatch_semaphore_signal(g_setup_done_sem);

		// Push notification so hosts that opted into observation get pinged.
		// Posted on the engine thread; observers should not assume main.
		[[NSNotificationCenter defaultCenter]
				postNotificationName:GodotLibraryReadyNotification
							  object:nil
							userInfo:@{ @"result" : @(err) }];

		if (err != 0) {
			return NULL;
		}

		// Keep the runloop alive even with no CADisplayLink attached yet.
		// Blocks posted via CFRunLoopPerformBlock+CFRunLoopWakeUp run here;
		// CADisplayLink callbacks attached to this runloop run here.
		CFRunLoopSourceContext ctx = { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &engine_runloop_keepalive_perform };
		CFRunLoopSourceRef src = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &ctx);
		CFRunLoopAddSource(g_engine_runloop, src, kCFRunLoopCommonModes);
		CFRelease(src);

		CFRunLoopRun();
	}

	return NULL;
}

// Post a block to the engine thread's runloop and wake it up. Safe to call
// from any thread; returns immediately.
static void godot_lib_dispatch_to_engine(dispatch_block_t block) {
	if (g_engine_runloop == NULL) {
		// Engine thread not started or aborted during setup. Drop.
		return;
	}
	CFRunLoopPerformBlock(g_engine_runloop, kCFRunLoopCommonModes, block);
	CFRunLoopWakeUp(g_engine_runloop);
}

int godot_library_start(NSArray<NSString *> *cmdline, NSString *dataDir) {
	NSLog(@"[godot-lib] godot_library_start called, g_started=%d", g_started);
	if (g_started) {
		// Idempotent: return current readiness so a host that polls only
		// after re-issuing start gets a sensible answer. 0 still means
		// "spawn ok" in the success case — readiness is a separate query.
		return 0;
	}
	if (dataDir == nil) {
		return -1;
	}

	// Flip the guard BEFORE spawning the engine thread. Re-entry into
	// register_core_types / Main::setup mutates process-wide singletons
	// (StringName::configured, IP::singleton, InputMap::singleton, ...);
	// any retry after failure would double-allocate them.
	g_started = YES;
	build_argv_from_array(cmdline);
	g_data_dir = dataDir;

	g_setup_done_sem = dispatch_semaphore_create(0);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// Bigger stack than the 512KB default — Main::setup recurses through
	// project parsing and registers a lot of types.
	pthread_attr_setstacksize(&attr, 4 * 1024 * 1024);
	int rc = pthread_create(&g_engine_thread, &attr, engine_thread_main, NULL);
	pthread_attr_destroy(&attr);
	if (rc != 0) {
		NSLog(@"[godot-lib] pthread_create failed: %d", rc);
		g_started = NO;
		return -2;
	}

	// Return immediately. The host gets back to its event loop / can render
	// a spinner. Readiness arrives via GodotLibraryReadyNotification or by
	// polling godot_library_is_ready(); make_view_controller blocks if the
	// host doesn't bother synchronising and just calls it eagerly.
	NSLog(@"[godot-lib] engine thread spawned; setup running off-main");
	return 0;
}

int godot_library_is_ready(void) {
	return g_ready_state.load(std::memory_order_acquire);
}

UIViewController *godot_library_make_view_controller(void) {
	// UIViewController/UIView creation MUST be on UIKit main. The host
	// (godot_library_make_view_controller's caller) is on main. We keep that
	// invariant. The engine thread will only later attach a CADisplayLink to
	// the GodotView via focus_in (marshalled onto the engine runloop).
	//
	// Setup-gating: this method reads ProjectSettings ("display.iOS/..."),
	// which only exists after Main::setup completes on the engine thread.
	// If the host calls this before readiness, block until then. Past
	// readiness the wait is a no-op (the semaphore stays signalled because
	// nothing else consumes it).
	if (g_ready_state.load(std::memory_order_acquire) == 0) {
		dispatch_semaphore_wait(g_setup_done_sem, DISPATCH_TIME_FOREVER);
		// Re-signal so a subsequent call (e.g. host re-creates the VC)
		// doesn't deadlock. The semaphore is intentionally used as a
		// one-way latch, not a counter.
		dispatch_semaphore_signal(g_setup_done_sem);
	}
	ViewController *vc = [[ViewController alloc] init];
	vc.godotView.useCADisplayLink = bool(GLOBAL_DEF("display.iOS/use_cadisplaylink", true)) ? YES : NO;
	vc.godotView.renderingInterval = 1.0 / kRenderingFrequency;
	vc.modalPresentationStyle = UIModalPresentationFullScreen;

	[AppDelegate setViewController:vc];

	// Rendering does NOT start here. Hosts call godot_library_focus_in()
	// after the VC becomes visible (root-VC swap, view-becomes-window).
	return vc;
}

void godot_library_focus_in(void) {
	if (!g_started) {
		return;
	}
	NSLog(@"[godot-lib] focus_in — hopping to engine thread");
	godot_lib_dispatch_to_engine(^{
		// Runs on engine thread. on_focus_in calls
		// [godotView startRendering], which adds the CADisplayLink to
		// [NSRunLoop currentRunLoop] — i.e. THIS thread's runloop. From now
		// on, drawView / Main::iteration fire here, off-main.
		OSIPhone::get_singleton()->on_focus_in();
	});
}

void godot_library_focus_out(void) {
	if (!g_started) {
		return;
	}
	NSLog(@"[godot-lib] focus_out — hopping to engine thread");
	godot_lib_dispatch_to_engine(^{
		OSIPhone::get_singleton()->on_focus_out();
	});
}

void godot_library_stop(void) {
	if (!g_started) {
		return;
	}
	// One-shot: OSIPhone/Main in 3.3 are not reinit-safe. Kept as a stub for
	// host code; full teardown means killing the process.
	godot_lib_dispatch_to_engine(^{
		iphone_finish();
	});
	g_started = NO;
}

// No-op stubs for the iOS plugin hooks. OSIPhone::start() /
// OSIPhone::finalize() in os_iphone.mm call these, expecting the Godot
// export pipeline to have generated concrete implementations
// (platform/iphone/export/export.cpp:1442-1452). In library-mode builds
// the host app isn't produced by that pipeline, so the symbols would be
// undefined at link time. Provide empty definitions here so hosts that
// don't ship any iOS plugins link cleanly. Declarations in os_iphone.h
// use default C++ linkage (no extern "C"), so these must too — matching
// the mangled symbol the caller expects.
//
// Marked __attribute__((weak)) so a host that DOES ship a generated
// dummy.cpp from the Godot iOS export pipeline (concrete implementations
// calling register_<plugin>_types) overrides these stubs at link time
// without a duplicate-symbol error, even when -force_load pulls every .o
// from the library archive.
__attribute__((weak)) void godot_ios_plugins_initialize() {
}

__attribute__((weak)) void godot_ios_plugins_deinitialize() {
}

#endif // IOS_LIBRARY_MODE
