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
#define kAccelerometerFrequency 100.0 // Hz

Error _shell_open(String);
void _set_keep_screen_on(bool p_enabled);
Variant nsobject_to_variant(NSObject *object);
NSObject *variant_to_nsobject(Variant v);

//convert from apple's abstract type to godot's abstract type....
Variant nsobject_to_variant(NSObject *object) {
	if ([object isKindOfClass:[NSString class]]) {
		const char *str = [(NSString *)object UTF8String];
		return String::utf8(str != NULL ? str : "");
	} else if ([object isKindOfClass:[NSData class]]) {
		PoolByteArray ret;
		NSData *data = (NSData *)object;
		if ([data length] > 0) {
			ret.resize([data length]);
			{
				PoolByteArray::Write w = ret.write();
				copymem(w.ptr(), [data bytes], [data length]);
			}
		}
		return ret;
	} else if ([object isKindOfClass:[NSArray class]]) {
		Array result;
		NSArray *array = (NSArray *)object;
		for (unsigned int i = 0; i < [array count]; ++i) {
			NSObject *value = [array objectAtIndex:i];
			result.push_back(nsobject_to_variant(value));
		}
		return result;
	} else if ([object isKindOfClass:[NSDictionary class]]) {
		Dictionary result;
		NSDictionary *dic = (NSDictionary *)object;

		NSArray *keys = [dic allKeys];
		int count = [keys count];
		for (int i = 0; i < count; ++i) {
			NSObject *k = [keys objectAtIndex:i];
			NSObject *v = [dic objectForKey:k];

			result[nsobject_to_variant(k)] = nsobject_to_variant(v);
		}
		return result;
	} else if ([object isKindOfClass:[NSNumber class]]) {
		//Every type except numbers can reliably identify its type.  The following is comparing to the *internal* representation, which isn't guaranteed to match the type that was used to create it, and is not advised, particularly when dealing with potential platform differences (ie, 32/64 bit)
		//To avoid errors, we'll cast as broadly as possible, and only return int or float.
		//bool, char, int, uint, longlong -> int
		//float, double -> float
		NSNumber *num = (NSNumber *)object;
		if (strcmp([num objCType], @encode(BOOL)) == 0) {
			return Variant((int)[num boolValue]);
		} else if (strcmp([num objCType], @encode(char)) == 0) {
			return Variant((int)[num charValue]);
		} else if (strcmp([num objCType], @encode(int)) == 0) {
			return Variant([num intValue]);
		} else if (strcmp([num objCType], @encode(unsigned int)) == 0) {
			return Variant((int)[num unsignedIntValue]);
		} else if (strcmp([num objCType], @encode(long long)) == 0) {
			return Variant((int)[num longValue]);
		} else if (strcmp([num objCType], @encode(float)) == 0) {
			return Variant([num floatValue]);
		} else if (strcmp([num objCType], @encode(double)) == 0) {
			return Variant((float)[num doubleValue]);
		} else {
			return Variant();
		}
	} else if ([object isKindOfClass:[NSDate class]]) {
		//this is a type that icloud supports...but how did you submit it in the first place?
		//I guess this is a type that *might* show up, if you were, say, trying to make your game
		//compatible with existing cloud data written by another engine's version of your game
		WARN_PRINT("NSDate unsupported, returning null Variant");
		return Variant();
	} else if ([object isKindOfClass:[NSNull class]] or object == nil) {
		return Variant();
	} else {
		WARN_PRINT("Trying to convert unknown NSObject type to Variant");
		return Variant();
	}
}

NSObject *variant_to_nsobject(Variant v) {
	if (v.get_type() == Variant::STRING) {
		return [[NSString alloc] initWithUTF8String:((String)v).utf8().get_data()];
	} else if (v.get_type() == Variant::REAL) {
		return [NSNumber numberWithDouble:(double)v];
	} else if (v.get_type() == Variant::INT) {
		return [NSNumber numberWithLongLong:(long)(int)v];
	} else if (v.get_type() == Variant::BOOL) {
		return [NSNumber numberWithBool:BOOL((bool)v)];
	} else if (v.get_type() == Variant::DICTIONARY) {
		NSMutableDictionary *result = [[NSMutableDictionary alloc] init];
		Dictionary dic = v;
		Array keys = dic.keys();
		for (unsigned int i = 0; i < keys.size(); ++i) {
			NSString *key = [[NSString alloc] initWithUTF8String:((String)(keys[i])).utf8().get_data()];
			NSObject *value = variant_to_nsobject(dic[keys[i]]);

			if (key == NULL || value == NULL) {
				return NULL;
			}

			[result setObject:value forKey:key];
		}
		return result;
	} else if (v.get_type() == Variant::ARRAY) {
		NSMutableArray *result = [[NSMutableArray alloc] init];
		Array arr = v;
		for (unsigned int i = 0; i < arr.size(); ++i) {
			NSObject *value = variant_to_nsobject(arr[i]);
			if (value == NULL) {
				//trying to add something unsupported to the array. cancel the whole array
				return NULL;
			}
			[result addObject:value];
		}
		return result;
	} else if (v.get_type() == Variant::POOL_BYTE_ARRAY) {
		PoolByteArray arr = v;
		PoolByteArray::Read r = arr.read();
		NSData *result = [NSData dataWithBytes:r.ptr() length:arr.size()];
		return result;
	}
	WARN_PRINT(String("Could not add unsupported type to iCloud: '" + Variant::get_type_name(v.get_type()) + "'").utf8().get_data());
	return NULL;
}

Error _shell_open(String p_uri) {
	NSString *url = [[NSString alloc] initWithUTF8String:p_uri.utf8().get_data()];

	if (![[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:url]]) {
		return ERR_CANT_OPEN;
	}

	printf("opening url %ls\n", p_uri.c_str());
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:url]];
	return OK;
};

void _set_keep_screen_on(bool p_enabled) {
	[[UIApplication sharedApplication] setIdleTimerDisabled:(BOOL)p_enabled];
};

void _vibrate() {
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
};

extern int gargc;
extern char **gargv;

extern int iphone_main(int, char **, String);
extern void iphone_finish();

@implementation AppDelegate

static ViewController *mainViewController = nil;

+ (ViewController *)viewController {
	return mainViewController;
}

OS::VideoMode _get_video_mode() {
	int backingWidth;
	int backingHeight;
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES,
			GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES,
			GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);

	OS::VideoMode vm;
	vm.fullscreen = true;
	vm.width = backingWidth;
	vm.height = backingHeight;
	vm.resizable = false;
	return vm;
};

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	
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
	if (OS::get_singleton()->get_main_loop()) {
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
	OSIPhone::get_singleton()->on_focus_in();
	[[NSNotificationCenter defaultCenter] postNotificationName: 
                       @"applicationDidBecomeActive_finish" object:nil userInfo: @{}];
}

- (void)dealloc {
	self.window = nil;
}

@end
