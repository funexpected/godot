/*************************************************************************/
/*  app_delegate.mm                                                      */
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

#import "app_delegate.h"

#include "core/project_settings.h"
#include "drivers/coreaudio/audio_driver_coreaudio.h"
#import "godot_view.h"
#include "main/main.h"
#include "os_iphone.h"
#import "view_controller.h"

#import <AudioToolbox/AudioServices.h>

#define kRenderingFrequency 60

extern int gargc;
extern char **gargv;

extern int iphone_main(int, char **, String);
extern void iphone_finish();

@implementation AppDelegate

static ViewController *mainViewController = nil;

+ (ViewController *)viewController {
	return mainViewController;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	NSLog(@"[sd] willFinishLaunchingWithOptions %@", launchOptions);

	// Create a full-screen window
	CGRect windowBounds = [[UIScreen mainScreen] bounds];
	self.window = [[UIWindow alloc] initWithFrame:windowBounds];

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
			NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];

	int err = iphone_main(gargc, gargv, String::utf8([documentsDirectory UTF8String]));
	if (err != 0) {
		// bail, things did not go very well for us, should probably output a message on screen with our error code...
		exit(0);
		return FALSE;
	}

	// WARNING: We must *always* create the GodotView after we have constructed the
	// OS with iphone_main. This allows the GodotView to access project settings so
	// it can properly initialize the OpenGL context

	ViewController *viewController = [[ViewController alloc] init];
	viewController.godotView.useCADisplayLink = bool(GLOBAL_DEF("display.iOS/use_cadisplaylink", true)) ? YES : NO;
	viewController.godotView.renderingInterval = 1.0 / kRenderingFrequency;

	self.window.rootViewController = viewController;

	// Show the window
	[self.window makeKeyAndVisible];

	[[NSNotificationCenter defaultCenter]
			addObserver:self
			   selector:@selector(onAudioInterruption:)
				   name:AVAudioSessionInterruptionNotification
				 object:[AVAudioSession sharedInstance]];

	mainViewController = viewController;

	// prevent to stop music in another background app
	[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];

	bool keep_screen_on = bool(GLOBAL_DEF("display/window/energy_saving/keep_screen_on", true));
	OSIPhone::get_singleton()->set_keep_screen_on(keep_screen_on);

	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"didFinishLaunchingWithOptions_finish" object:nil userInfo:launchOptions];
	return TRUE;
}


- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options {
	NSMutableArray *ansArray = [NSMutableArray arrayWithObjects:[NSNumber numberWithBool:YES], nil];
	NSDictionary *data = @{ @"app" : app, @"url": url, @"options":options, @"ansArray" : ansArray};
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"appOpenUrlWithOptions_finish" object:nil userInfo:data];
	for (id tempObject in ansArray) {
    	if ([tempObject boolValue] == NO)
			return NO;
	}
	return YES;
}


- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url sourceApplication:(NSString*)sourceApplication annotation:(id)annotation {
	NSMutableArray *ansArray = [NSMutableArray arrayWithObjects:[NSNumber numberWithBool:YES], nil];
	NSDictionary *data = @{ 
		@"app" : app, 
		@"url" : url, 
		@"sourceApplication" : sourceApplication, 
		@"annotation" : annotation,
		@"ansArray" : ansArray
	};
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"appOpenUrlWithOptions_finish" object:nil userInfo:data];
	for (id tempObject in ansArray) {
    	if ([tempObject boolValue] == NO)
			return NO;
	}
	return YES;

}

- (BOOL)application:(UIApplication *)application continueUserActivity:(nonnull NSUserActivity *)userActivity restorationHandler:
#if defined(__IPHONE_12_0) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_12_0)
(nonnull void (^)(NSArray<id<UIUserActivityRestoring>> *_Nullable))restorationHandler {
#else
    (nonnull void (^)(NSArray *_Nullable))restorationHandler {
#endif  // __IPHONE_12_0
	NSMutableDictionary *mutDic = [[NSMutableDictionary alloc] initWithCapacity:2];
	[mutDic setValue:userActivity forKey:@"userActivity"];
	[mutDic setValue:restorationHandler forKey:@"restorationHandler"];

	[[NSNotificationCenter defaultCenter] postNotificationName: @"appContinueUserActivity_finish" object:nil userInfo: mutDic];
	return YES;
}


- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo {
  // handler for Push Notifications
  NSLog(@"appDidReceiveRemoteNotification_finish");
  [[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"appDidReceiveRemoteNotification_finish" object:nil userInfo: @{@"userInfo" :userInfo}];
}

- (void)application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
    fetchCompletionHandler:(void (^)(UIBackgroundFetchResult))completionHandler {
	NSLog(@"appDidReceiveRemoteNotification_finish with completion");
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"appDidReceiveRemoteNotification_finish" object:nil userInfo: @{@"userInfo" :userInfo}];



	completionHandler(UIBackgroundFetchResultNewData);
}

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken; {
  NSLog(@"didRegisterForRemoteNotificationsWithDeviceToken_finish");
  // handler for Push Notifications
  [[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"didRegisterForRemoteNotificationsWithDeviceToken_finish" object:nil userInfo: @{@"deviceToken":deviceToken}];
}







- (void)onAudioInterruption:(NSNotification *)notification {
	if ([notification.name isEqualToString:AVAudioSessionInterruptionNotification]) {
		if ([[notification.userInfo valueForKey:AVAudioSessionInterruptionTypeKey] isEqualToNumber:[NSNumber numberWithInt:AVAudioSessionInterruptionTypeBegan]]) {
			NSLog(@"Audio interruption began");
			OSIPhone::get_singleton()->on_focus_out();
		} else if ([[notification.userInfo valueForKey:AVAudioSessionInterruptionTypeKey] isEqualToNumber:[NSNumber numberWithInt:AVAudioSessionInterruptionTypeEnded]]) {
			NSLog(@"Audio interruption ended");
			OSIPhone::get_singleton()->on_focus_in();
		}
	}
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
	if (OS::get_singleton() && OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(
				MainLoop::NOTIFICATION_OS_MEMORY_WARNING);
	}
}

- (void)applicationWillTerminate:(UIApplication *)application {
	iphone_finish();
}

// When application goes to background (e.g. user switches to another app or presses Home),
// then applicationWillResignActive -> applicationDidEnterBackground are called.
// When user opens the inactive app again,
// applicationWillEnterForeground -> applicationDidBecomeActive are called.

// There are cases when applicationWillResignActive -> applicationDidBecomeActive
// sequence is called without the app going to background. For example, that happens
// if you open the app list without switching to another app or open/close the
// notification panel by swiping from the upper part of the screen.

- (void)applicationWillResignActive:(UIApplication *)application {
	OSIPhone::get_singleton()->on_focus_out();
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"applicationWillResignActive_finish" object:nil userInfo: @{}];
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	NSLog(@"[sd] applicationDidBecomeActive");
	OSIPhone::get_singleton()->on_focus_in();
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"applicationDidBecomeActive_finish" object:nil userInfo: @{}];
}

- (void)dealloc {
	// self.window = nil;
}

@end
