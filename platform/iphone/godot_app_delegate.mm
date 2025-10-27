/*************************************************************************/
/*  godot_app_delegate.m                                                 */
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

#import "godot_app_delegate.h"

#import "app_delegate.h"
#include "os_iphone.h"
@interface GodotApplicalitionDelegate ()

@end

@implementation GodotApplicalitionDelegate

static NSMutableArray<ApplicationDelegateService *> *services = nil;

+ (NSArray<ApplicationDelegateService *> *)services {
	return services;
}

+ (void)load {
	services = [NSMutableArray new];
	[services addObject:[AppDelegate new]];
}

+ (void)addService:(ApplicationDelegateService *)service {
	if (!services || !service) {
		return;
	}
	[services addObject:service];
}

// UIApplicationDelegate documantation can be found here: https://developer.apple.com/documentation/uikit/uiapplicationdelegate

// MARK: Window

- (UIWindow *)window {
	UIWindow *result = nil;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		UIWindow *value = [service window];

		if (value) {
			result = value;
		}
	}
	result.backgroundColor = [UIColor 
		colorWithRed:0.42
		green:0.35
		blue:0.79
		alpha:1.0
	];
	return result;
}

// MARK: Initializing

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey, id> *)launchOptions {
	NSLog(@"[delegate] willFinishLaunchingWithOptions");
	// Temporarily disable animations during launch to prevent flicker
	[UIView setAnimationsEnabled:NO];
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application willFinishLaunchingWithOptions:launchOptions]) {
			result = YES;
		}
	}

	return result;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey, id> *)launchOptions {
	NSLog(@"[delegate] didFinishLaunchingWithOptions");
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application didFinishLaunchingWithOptions:launchOptions]) {
			result = YES;
		}
	}
	
	// Re-enable animations after launch is complete to allow proper orientation transitions
	dispatch_async(dispatch_get_main_queue(), ^{
		[UIView setAnimationsEnabled:YES];
	});

	return result;
}

// MARK: Life-Cycle

- (void)applicationDidBecomeActive:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidBecomeActive:application];
	}
}

- (void)applicationWillResignActive:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillResignActive:application];
	}
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidEnterBackground:application];
	}
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillEnterForeground:application];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillTerminate:application];
	}
}

// MARK: Environment Changes

- (void)applicationProtectedDataDidBecomeAvailable:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationProtectedDataDidBecomeAvailable:application];
	}
}

- (void)applicationProtectedDataWillBecomeUnavailable:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationProtectedDataWillBecomeUnavailable:application];
	}
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidReceiveMemoryWarning:application];
	}
}

- (void)applicationSignificantTimeChange:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationSignificantTimeChange:application];
	}
}

// MARK: App State Restoration

- (BOOL)application:(UIApplication *)application shouldSaveSecureApplicationState:(NSCoder *)coder API_AVAILABLE(ios(13.2)) {
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application shouldSaveSecureApplicationState:coder]) {
			result = YES;
		}
	}

	return result;
}

- (BOOL)application:(UIApplication *)application shouldRestoreSecureApplicationState:(NSCoder *)coder API_AVAILABLE(ios(13.2)) {
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application shouldRestoreSecureApplicationState:coder]) {
			result = YES;
		}
	}

	return result;
}

- (UIViewController *)application:(UIApplication *)application viewControllerWithRestorationIdentifierPath:(NSArray<NSString *> *)identifierComponents coder:(NSCoder *)coder {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		UIViewController *controller = [service application:application viewControllerWithRestorationIdentifierPath:identifierComponents coder:coder];

		if (controller) {
			if (controller.view) {
				controller.view.backgroundColor = [UIColor 
					colorWithRed:0.42
					green:0.35
					blue:0.79
					alpha:1.0
				];
			}
			return controller;
		}
	}

	return nil;
}

- (void)application:(UIApplication *)application willEncodeRestorableStateWithCoder:(NSCoder *)coder {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application willEncodeRestorableStateWithCoder:coder];
	}
}

- (void)application:(UIApplication *)application didDecodeRestorableStateWithCoder:(NSCoder *)coder {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didDecodeRestorableStateWithCoder:coder];
	}
}

// MARK: Download Data in Background

- (void)application:(UIApplication *)application handleEventsForBackgroundURLSession:(NSString *)identifier completionHandler:(void (^)(void))completionHandler {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application handleEventsForBackgroundURLSession:identifier completionHandler:completionHandler];
	}

	completionHandler();
}

// MARK: Remote Notification

// Moved to the iOS Plugin

// MARK: User Activity and Handling Quick Actions

- (BOOL)application:(UIApplication *)application willContinueUserActivityWithType:(NSString *)userActivityType {
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application willContinueUserActivityWithType:userActivityType]) {
			result = YES;
		}
	}

	return result;
}

- (BOOL)application:(UIApplication *)application continueUserActivity:(NSUserActivity *)userActivity restorationHandler:(void (^)(NSArray<id<UIUserActivityRestoring> > *restorableObjects))restorationHandler {
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application continueUserActivity:userActivity restorationHandler:restorationHandler]) {
			result = YES;
		}
	}

	return result;
}

- (void)application:(UIApplication *)application didUpdateUserActivity:(NSUserActivity *)userActivity {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didUpdateUserActivity:userActivity];
	}
}

- (void)application:(UIApplication *)application didFailToContinueUserActivityWithType:(NSString *)userActivityType error:(NSError *)error {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didFailToContinueUserActivityWithType:userActivityType error:error];
	}
}

- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void (^)(BOOL succeeded))completionHandler {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application performActionForShortcutItem:shortcutItem completionHandler:completionHandler];
	}
}

// MARK: WatchKit

- (void)application:(UIApplication *)application handleWatchKitExtensionRequest:(NSDictionary *)userInfo reply:(void (^)(NSDictionary *replyInfo))reply {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application handleWatchKitExtensionRequest:userInfo reply:reply];
	}
}

// MARK: HealthKit

- (void)applicationShouldRequestHealthAuthorization:(UIApplication *)application {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationShouldRequestHealthAuthorization:application];
	}
}

// MARK: Opening an URL

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey, id> *)options {
	BOOL result = NO;
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:app openURL:url options:options]) {
			result = YES;
		}
	}
	return result;
}

// MARK: Disallowing Specified App Extension Types

- (BOOL)application:(UIApplication *)application shouldAllowExtensionPointIdentifier:(UIApplicationExtensionPointIdentifier)extensionPointIdentifier {
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application shouldAllowExtensionPointIdentifier:extensionPointIdentifier]) {
			result = YES;
		}
	}

	return result;
}

// MARK: SiriKit

- (id)application:(UIApplication *)application handlerForIntent:(INIntent *)intent API_AVAILABLE(ios(14.0)) {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		id result = [service application:application handlerForIntent:intent];

		if (result) {
			return result;
		}
	}

	return nil;
}

// MARK: CloudKit

- (void)application:(UIApplication *)application userDidAcceptCloudKitShareWithMetadata:(CKShareMetadata *)cloudKitShareMetadata {
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application userDidAcceptCloudKitShareWithMetadata:cloudKitShareMetadata];
	}
}

// /* Handled By Info.plist file for now

// MARK: Interface Geometry

- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {
	NSLog(@"[AppDelegate] supportedInterfaceOrientationsForWindow called for window: %@", window);
	if (!OS::get_singleton()) {
		NSLog(@"[AppDelegate] OS singleton not available, returning Portrait");
		return UIInterfaceOrientationMaskPortrait;
	}
	OS::ScreenOrientation orientation = OS::get_singleton()->get_screen_orientation();
	NSLog(@"[AppDelegate] Current orientation setting: %d (PORTRAIT=1, LANDSCAPE=0, SENSOR_LANDSCAPE=4)", orientation);
	switch (orientation) {
		case OS::SCREEN_PORTRAIT:
			return UIInterfaceOrientationMaskPortrait;
		case OS::SCREEN_REVERSE_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscapeRight;
		case OS::SCREEN_REVERSE_PORTRAIT:
			return UIInterfaceOrientationMaskPortraitUpsideDown;
		case OS::SCREEN_SENSOR_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscape;
		case OS::SCREEN_SENSOR_PORTRAIT:
			return UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
		case OS::SCREEN_SENSOR:
			return UIInterfaceOrientationMaskAll;
		case OS::SCREEN_LANDSCAPE:
			return UIInterfaceOrientationMaskLandscapeLeft;
	}
}

// */

// MARK: Scene

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options API_AVAILABLE(ios(13.0)) {
	NSLog(@"[delegate] configurationForConnectingSceneSession");
	UISceneConfiguration *configuration = [UISceneConfiguration configurationWithName:@"main"
																		  sessionRole:UIWindowSceneSessionRoleApplication];

    configuration.delegateClass = GodotSceneDelegate.class;
    
    // For iOS 16+, ensure we request full screen from the start
    if (@available(iOS 16.0, *)) {
    	NSLog(@"[delegate] Configuring scene for full-screen mode");
    }

    return configuration;
}

//- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {}

/* */

@end

API_AVAILABLE(ios(13.0))
@implementation GodotSceneDelegate


@synthesize window = _window;

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions API_AVAILABLE(ios(13.0)) {
	NSLog(@"[delegate] willConnectToSession");
	if (session != nil && (session.role == UIWindowSceneSessionRoleExternalDisplay || session.role == UIWindowSceneSessionRoleExternalDisplayNonInteractive)) {
		NSLog(@"[delegate] external session");
		return;
	}
	
	// Request full-screen geometry
	if ([scene isKindOfClass:[UIWindowScene class]]) {
		UIWindowScene *windowScene = (UIWindowScene *)scene;
		
		if (@available(iOS 16.0, *)) {
			// NOTE: UIRequiresFullScreen is DEPRECATED in iPadOS 16+
			// Apple now requires apps to support multitasking
			// We can request full-screen as default, but can't disable multitasking entirely
			
			UIWindowSceneGeometryPreferencesIOS *geometryPreferences = [[UIWindowSceneGeometryPreferencesIOS alloc] init];
			geometryPreferences.interfaceOrientations = UIInterfaceOrientationMaskAll;
			
			NSLog(@"[GODOT_SCENE] 🎯 Requesting full-screen geometry (interfaceOrientations=All)");
			NSLog(@"[GODOT_SCENE] Screen bounds: %@", NSStringFromCGSize(windowScene.screen.bounds.size));
			
			[windowScene requestGeometryUpdateWithPreferences:geometryPreferences
												  errorHandler:^(NSError * _Nonnull error) {
				NSLog(@"[GODOT_SCENE] ❌ Geometry update error: %@", error.localizedDescription);
			}];
		}
	}
	
	UIApplication *app = [UIApplication sharedApplication];
    self.window = app.delegate.window;
	self.window.windowScene = scene;

	for (NSUserActivity *acticity in connectionOptions.userActivities) {
		NSURL *url = acticity.webpageURL;
		if (url != nil) {
			[app.delegate application:app openURL:url options:@{}];
			break;
		}
	}

	for (UIOpenURLContext *ctx in connectionOptions.URLContexts) {
		NSLog(@"[sd] ctx %@", ctx);
		NSURL *url = ctx.URL;
		if (url != nil) {
			[app.delegate application:app openURL:url options:@{}];
			// [app.delegate application:app openURL:url options:@{
			// 	UIApplicationOpenURLOptionsSourceApplicationKey: ctx.options.sourceApplication,
			// 	UIApplicationOpenURLOptionsAnnotationKey: ctx.options.annotation
			// }];
			break;
		}
	}
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationWillEnterForeground:app];
}

- (void)sceneDidBecomeActive:(UIScene *)scene {
	NSLog(@"[GODOT_SCENE] sceneDidBecomeActive");
	
	// Check if UIRequiresFullScreen is set in Info.plist
	NSNumber *requiresFullScreen = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UIRequiresFullScreen"];
	NSLog(@"[GODOT_SCENE] Info.plist UIRequiresFullScreen = %@", requiresFullScreen);
	
	// Force full-screen geometry when scene becomes active
	// This ensures we're not stuck in Slide Over/Split View mode
	if (@available(iOS 16.0, *)) {
		if ([scene isKindOfClass:[UIWindowScene class]]) {
			UIWindowScene *windowScene = (UIWindowScene *)scene;
			
			CGSize windowSize = self.window.bounds.size;
			CGSize screenSize = windowScene.screen.bounds.size;
			
			NSLog(@"[GODOT_SCENE] sceneDidBecomeActive - Window: %.0fx%.0f, Screen: %.0fx%.0f", 
				  windowSize.width, windowSize.height,
				  screenSize.width, screenSize.height);
			
			// Request full-screen geometry
			NSLog(@"[GODOT_SCENE] 🎯 Requesting full-screen geometry");
			
			UIWindowSceneGeometryPreferencesIOS *geometryPreferences = [[UIWindowSceneGeometryPreferencesIOS alloc] init];
			geometryPreferences.interfaceOrientations = UIInterfaceOrientationMaskAll;
			
			[windowScene requestGeometryUpdateWithPreferences:geometryPreferences
												  errorHandler:^(NSError * _Nonnull error) {
				NSLog(@"[GODOT_SCENE] ❌ Geometry update error: %@", error.localizedDescription);
			}];
			
			// iOS 16+ with deprecated UIRequiresFullScreen: user can still manually resize
			// But we force full-screen as much as possible
			dispatch_async(dispatch_get_main_queue(), ^{
				CGRect screenBounds = windowScene.screen.bounds;
				if (!CGRectEqualToRect(self.window.frame, screenBounds)) {
					NSLog(@"[GODOT_SCENE] Window (%@) != Screen (%@), forcing to screen bounds",
						  NSStringFromCGRect(self.window.frame),
						  NSStringFromCGRect(screenBounds));
					self.window.frame = screenBounds;
					[self.window setNeedsLayout];
					[self.window layoutIfNeeded];
				}
			});
		}
	}
	
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationDidBecomeActive:app];
}

- (void)sceneWillResignActive:(UIScene *)scene {
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationWillResignActive:app];
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationDidEnterBackground:app];
}

- (void)windowScene:(UIWindowScene *)windowScene didUpdateCoordinateSpace:(id<UICoordinateSpace>)previousCoordinateSpace interfaceOrientation:(UIInterfaceOrientation)previousInterfaceOrientation traitCollection:(UITraitCollection *)previousTraitCollection API_AVAILABLE(ios(13.0)) {
	NSLog(@"[GODOT_ORIENTATION] *** windowScene didUpdateCoordinateSpace ***");
	NSLog(@"[GODOT_ORIENTATION] Scene bounds: %@", NSStringFromCGRect(windowScene.coordinateSpace.bounds));
	NSLog(@"[GODOT_ORIENTATION] Previous orientation: %ld, new orientation: %ld", (long)previousInterfaceOrientation, (long)windowScene.interfaceOrientation);
	
	// Ensure window fills the entire scene bounds - THIS IS CRITICAL
	if (self.window) {
		NSLog(@"[GODOT_ORIENTATION] Current window frame: %@", NSStringFromCGRect(self.window.frame));
		
		// Force window to match scene bounds (like manual resize does)
		self.window.frame = windowScene.coordinateSpace.bounds;
		NSLog(@"[GODOT_ORIENTATION] Set window frame to scene bounds: %@", NSStringFromCGRect(self.window.frame));
		
		// Force immediate layout pass
		[self.window setNeedsLayout];
		[self.window layoutIfNeeded];
		
		// Update root view to match new window size
		if (self.window.rootViewController) {
			self.window.rootViewController.view.frame = self.window.bounds;
			NSLog(@"[GODOT_ORIENTATION] Set root view frame to: %@", NSStringFromCGRect(self.window.rootViewController.view.frame));
			
			[self.window.rootViewController.view setNeedsLayout];
			[self.window.rootViewController.view layoutIfNeeded];
			NSLog(@"[GODOT_ORIENTATION] Root view frame after layout: %@", NSStringFromCGRect(self.window.rootViewController.view.frame));
		}
	}
	
	// Request full-screen geometry on iOS 16+
	if (@available(iOS 16.0, *)) {
		UIWindowSceneGeometryPreferencesIOS *geometryPreferences = [[UIWindowSceneGeometryPreferencesIOS alloc] init];
		if (self.window.rootViewController) {
			geometryPreferences.interfaceOrientations = [self.window.rootViewController supportedInterfaceOrientations];
			NSLog(@"[GODOT_ORIENTATION] Requesting geometry with mask: %lu", (unsigned long)[self.window.rootViewController supportedInterfaceOrientations]);
		} else {
			geometryPreferences.interfaceOrientations = UIInterfaceOrientationMaskAll;
			NSLog(@"[GODOT_ORIENTATION] Requesting geometry with mask: All");
		}
		[windowScene requestGeometryUpdateWithPreferences:geometryPreferences
											  errorHandler:^(NSError * _Nonnull error) {
			NSLog(@"[GODOT_ORIENTATION] ERROR in didUpdateCoordinateSpace: %@", error.localizedDescription);
		}];
	}
}

- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)URLContexts {
	UIApplication *app = [UIApplication sharedApplication];
	for (UIOpenURLContext *ctx in URLContexts) {
		NSURL *url = ctx.URL;
		if (url != nil) {
			[app.delegate application:app openURL:url options:@{}];
			// [app.delegate application:app openURL:url options:@{
			// 	UIApplicationOpenURLOptionsSourceApplicationKey: ctx.options.sourceApplication,
			// 	UIApplicationOpenURLOptionsAnnotationKey: ctx.options.annotation
			// }];
			break;
		}
	}
}



- (void)scene:(UIScene *)scene willContinueUserActivityWithType:(NSString *)userActivityType {
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate application: app willContinueUserActivityWithType:userActivityType];
}

- (void)scene:(UIScene *)scene continueUserActivity:(NSUserActivity *)userActivity {
	NSURL *url = userActivity.webpageURL;
	UIApplication *app = [UIApplication sharedApplication];
	if (url != nil) {
		[app.delegate application:app openURL:url options:@{}];
	}
}




@end
