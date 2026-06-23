/*************************************************************************/
/*  library_entry.h                                                      */
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

#import <UIKit/UIKit.h>

#ifdef __cplusplus
extern "C" {
#endif

// Spawn the engine worker thread that runs Main::setup off the calling
// (UIKit main) thread. Returns IMMEDIATELY, before Main::setup has finished —
// the host can render a spinner / progress UI while the engine boots.
//
// Returns 0 on successful spawn, !=0 on pthread_create failure. Note: a
// successful spawn does NOT mean engine setup succeeded — that may still
// fail asynchronously. Observe the GodotLibraryReadyNotification (below) or
// poll godot_library_is_ready() for completion + result code.
//
// Subsequent calls after a successful first call are no-ops.
//
// cmdline may be nil. Pass --path <dir> to point Main::setup at a project
// folder, or --main-pack <pack.pck> for an exported pack. dataDir must be a
// writable path, typically NSDocumentDirectory.
int godot_library_start(NSArray<NSString *> *_Nullable cmdline, NSString *_Nonnull dataDir);

// Build and return a GodotViewController configured the same way the non-library
// path does it (CADisplayLink enabled, 60Hz, fullscreen presentation style). Also
// registers the VC with AppDelegate so the rest of the engine can reach it via
// AppDelegate.viewController.
//
// IMPORTANT: this call BLOCKS the calling thread until Main::setup finishes
// on the engine worker, because it reads ProjectSettings (display.iOS/...).
// In practice that means: call godot_library_start eagerly at app launch,
// then call this just before swapping to the Godot view — by then setup is
// usually done and the wait is zero. If you call it sooner it just blocks
// until ready, no race.
//
// Ownership: caller retains the returned VC. Safe to call multiple times;
// the most recently returned VC becomes the registered one.
UIViewController *_Nonnull godot_library_make_view_controller(void);

// Returns:
//   0  Setup still in progress on the engine thread.
//   1  Setup finished successfully.
//  <0  Setup failed; absolute value is the error code from iphone_main.
// Cheap to poll. Reads an atomic int.
int godot_library_is_ready(void);

// Posted on the default NSNotificationCenter once the engine worker finishes
// Main::setup (or fails). userInfo[@"result"] is an NSNumber with the iphone_main
// return code (0 = success, !=0 = failure). Posted on the engine thread, so
// observers that touch UIKit must hop to dispatch_get_main_queue.
extern NSNotificationName _Nonnull const GodotLibraryReadyNotification;

// Placeholder teardown. iphone_finish() is called but OSIPhone/Main in the 3.3 fork
// are not designed for re-init, so this is effectively one-shot (process-end only).
// Kept as a symbol so hosts can wire it up now and we can harden the teardown path
// later without breaking their code.
void godot_library_stop(void);

// Start/stop the CADisplayLink, audio, and video playback. Hosts should call
// godot_library_focus_in() after making the Godot VC visible (root-VC swap,
// view-becomes-window), and godot_library_focus_out() before hiding it. The
// fork's standalone entry wires these to applicationWillResignActive /
// applicationDidBecomeActive; library-mode hosts own their own app delegate
// and must forward the calls themselves. Safe to call before engine start
// (no-op). Between focus_out and the next focus_in, Godot stops rendering
// and pauses its audio driver.
void godot_library_focus_in(void);
void godot_library_focus_out(void);

#ifdef __cplusplus
}
#endif

#endif // IOS_LIBRARY_MODE
