/*************************************************************************/
/*  library_entry.mm                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifdef IOS_LIBRARY_MODE

#import "library_entry.h"

#import "app_delegate.h"
#import "godot_view.h"
#import "view_controller.h"

#include "core/project_settings.h"
#include "core/ustring.h"
#include "os_iphone.h"

#define kRenderingFrequency 60

extern int iphone_main(int, char **, String);
extern void iphone_finish();

static BOOL g_started = NO;

// Backing store for the synthesized argv. iphone_main does not take ownership —
// it reads argv[0] for chdir and hands the rest to Main::setup. We keep the
// buffer alive for the process lifetime since the engine assumes argv outlives
// startup, same as the UIApplicationMain path.
static char **g_argv = NULL;
static int g_argc = 0;

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

int godot_library_start(NSArray<NSString *> *cmdline, NSString *dataDir) {
	NSLog(@"[godot-lib] godot_library_start called, g_started=%d", g_started);
	if (g_started) {
		return 0;
	}
	if (dataDir == nil) {
		return -1;
	}

	// Flip the guard BEFORE calling into iphone_main. iphone_main /
	// Main::setup / register_core_types mutates process-wide singletons
	// (StringName::configured, IP::singleton, InputMap::singleton, ...);
	// a crash or exception thrown anywhere inside would leave the engine
	// half-initialized. If the host retries enter() we must NOT reach
	// register_core_types again — the ERR_FAIL_COND messages are harmless
	// but a second IP::create / InputMap() hits an abort in some
	// configurations.
	g_started = YES;
	build_argv_from_array(cmdline);

	int err = iphone_main(g_argc, g_argv, String::utf8([dataDir UTF8String]));
	NSLog(@"[godot-lib] iphone_main returned err=%d", err);
	if (err != 0) {
		return err;
	}

	bool keep_screen_on = bool(GLOBAL_DEF("display/window/energy_saving/keep_screen_on", true));
	NSLog(@"[godot-lib] keep_screen_on=%d", keep_screen_on);
	OSIPhone::get_singleton()->set_keep_screen_on(keep_screen_on);

	NSLog(@"[godot-lib] godot_library_start done");
	return 0;
}

UIViewController *godot_library_make_view_controller(void) {
	ViewController *vc = [[ViewController alloc] init];
	vc.godotView.useCADisplayLink = bool(GLOBAL_DEF("display.iOS/use_cadisplaylink", true)) ? YES : NO;
	vc.godotView.renderingInterval = 1.0 / kRenderingFrequency;
	vc.modalPresentationStyle = UIModalPresentationFullScreen;

	[AppDelegate setViewController:vc];

	// In the standalone Godot iOS flow, OSIPhone::on_focus_in() (triggered
	// by the Godot AppDelegate's applicationDidBecomeActive) is what starts
	// the CADisplayLink and drives drawView → setupView → Main::start().
	// Library-mode hosts (e.g. a React Native shell) have their own app
	// delegate and never forward applicationDidBecomeActive here, so we
	// kick off rendering manually. The host is still responsible for
	// forwarding subsequent focus-in/out events; for this prototype a
	// one-shot kick at mount time is enough.
	OSIPhone::get_singleton()->on_focus_in();

	return vc;
}

void godot_library_stop(void) {
	if (!g_started) {
		return;
	}
	iphone_finish();
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
#if !defined(GODOT_IOS_PLUGINS_CUSTOM)

void godot_ios_plugins_initialize() {
}

void godot_ios_plugins_deinitialize() {
}

#endif // !GODOT_IOS_PLUGINS_CUSTOM

#endif // IOS_LIBRARY_MODE
