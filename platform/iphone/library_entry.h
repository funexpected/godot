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

// Initialize the Godot engine for host-embedded use. Host apps (e.g. a React Native
// shell) call this once before asking for a view controller. cmdline may be nil
// (equivalent to no arguments). dataDir must be a writable path, typically
// NSDocumentDirectory. Returns 0 on success. Subsequent calls no-op.
int godot_library_start(NSArray<NSString *> *_Nullable cmdline, NSString *_Nonnull dataDir);

// Build and return a GodotViewController configured the same way the non-library
// path does it (CADisplayLink enabled, 60Hz, fullscreen presentation style). Also
// registers the VC with AppDelegate so the rest of the engine can reach it via
// AppDelegate.viewController. Ownership: caller retains the returned VC. Safe to
// call multiple times; the most recently returned VC becomes the registered one.
UIViewController *_Nonnull godot_library_make_view_controller(void);

// Placeholder teardown. iphone_finish() is called but OSIPhone/Main in the 3.3 fork
// are not designed for re-init, so this is effectively one-shot (process-end only).
// Kept as a symbol so hosts can wire it up now and we can harden the teardown path
// later without breaking their code.
void godot_library_stop(void);

#ifdef __cplusplus
}
#endif

#endif // IOS_LIBRARY_MODE
