/*************************************************************************/
/*  godot_view_renderer.mm                                               */
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

#import "godot_view_renderer.h"

#include "core/os/keyboard.h"
#include "core/project_settings.h"
#include "main/main.h"
#include "os_iphone.h"
#include "servers/audio_server.h"

#import <AudioToolbox/AudioServices.h>
#import <CoreMotion/CoreMotion.h>
#import <GameController/GameController.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

@interface GodotViewRenderer ()

@property(assign, nonatomic) BOOL hasFinishedProjectDataSetup;
@property(assign, nonatomic) BOOL hasStartedMain;
@property(assign, nonatomic) BOOL hasFinishedSetup;

@end

@implementation GodotViewRenderer

- (BOOL)setupView:(UIView *)view {
	if (self.hasFinishedSetup) {
		return NO;
	}

	if (!OS::get_singleton()) {
		exit(0);
	}

	if (!self.hasFinishedProjectDataSetup) {
		[self setupProjectData];
		return YES;
	}

	if (!self.hasStartedMain) {
		self.hasStartedMain = YES;
		// Instrumentation for off-main-boot investigation. OSIPhone::start()
		// calls Main::start() which loads the main scene + runs first-pass
		// GDScript _ready, allocating GL resources. Candidate for running
		// off main in Option 1b (requires GL context handoff).
		NSDate *t0 = [NSDate date];
		OSIPhone::get_singleton()->start();
		NSTimeInterval dt = [[NSDate date] timeIntervalSinceDate:t0];
		NSLog(@"[godot-lib] OSIPhone::start (→ Main::start, scene load) took %.3fs", dt);
		return YES;
	}

	self.hasFinishedSetup = YES;

	return NO;
}

- (void)setupProjectData {
	self.hasFinishedProjectDataSetup = YES;

	// Instrumentation for off-main-boot investigation. Main::setup2 inits
	// VisualServer / rasterizer, which means GL calls — moving this off
	// main requires an EAGLContext current on the boot thread.
	NSDate *t0 = [NSDate date];
	Main::setup2();
	NSTimeInterval dt = [[NSDate date] timeIntervalSinceDate:t0];
	NSLog(@"[godot-lib] Main::setup2 took %.3fs", dt);

	// this might be necessary before here
	NSDictionary *dict = [[NSBundle mainBundle] infoDictionary];
	for (NSString *key in dict) {
		NSObject *value = [dict objectForKey:key];
		String ukey = String::utf8([key UTF8String]);

		// we need a NSObject to Variant conversor

		if ([value isKindOfClass:[NSString class]]) {
			NSString *str = (NSString *)value;
			String uval = String::utf8([str UTF8String]);

			ProjectSettings::get_singleton()->set("Info.plist/" + ukey, uval);

		} else if ([value isKindOfClass:[NSNumber class]]) {
			NSNumber *n = (NSNumber *)value;
			double dval = [n doubleValue];

			ProjectSettings::get_singleton()->set("Info.plist/" + ukey, dval);
		};
		// do stuff
	}
}

- (void)renderOnView:(UIView *)view {
	if (!OSIPhone::get_singleton()) {
		return;
	}

	OSIPhone::get_singleton()->iterate();
}

@end
