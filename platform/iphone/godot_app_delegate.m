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

	return result;
}

// MARK: Initializing

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey, id> *)launchOptions {
	NSLog(@"[sd] willFinishLaunchingWithOptions");
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
	NSLog(@"[sd] didFinishLaunchingWithOptions");
	BOOL result = NO;

	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:application didFinishLaunchingWithOptions:launchOptions]) {
			result = YES;
		}
	}

	return result;
}

// MARK: Life-Cycle

- (void)applicationDidBecomeActive:(UIApplication *)application {
	NSLog(@"[sd] applicationDidBecomeActive");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidBecomeActive:application];
	}
}

- (void)applicationWillResignActive:(UIApplication *)application {
	NSLog(@"[sd] applicationWillResignActive");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillResignActive:application];
	}
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	NSLog(@"[sd] applicationDidEnterBackground");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidEnterBackground:application];
	}
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	NSLog(@"[sd] applicationWillEnterForeground");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillEnterForeground:application];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application {
	NSLog(@"[sd] applicationWillTerminate");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationWillTerminate:application];
	}
}

// MARK: Environment Changes

- (void)applicationProtectedDataDidBecomeAvailable:(UIApplication *)application {
	NSLog(@"[sd] applicationProtectedDataDidBecomeAvailable");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationProtectedDataDidBecomeAvailable:application];
	}
}

- (void)applicationProtectedDataWillBecomeUnavailable:(UIApplication *)application {
	NSLog(@"[sd] applicationProtectedDataWillBecomeUnavailable");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationProtectedDataWillBecomeUnavailable:application];
	}
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
	NSLog(@"[sd] applicationDidReceiveMemoryWarning");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationDidReceiveMemoryWarning:application];
	}
}

- (void)applicationSignificantTimeChange:(UIApplication *)application {
	NSLog(@"[sd] applicationSignificantTimeChange");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationSignificantTimeChange:application];
	}
}

// MARK: App State Restoration

- (BOOL)application:(UIApplication *)application shouldSaveSecureApplicationState:(NSCoder *)coder API_AVAILABLE(ios(13.2)) {
	NSLog(@"[sd] shouldSaveSecureApplicationState");
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
	NSLog(@"[sd] shouldRestoreSecureApplicationState");
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
	NSLog(@"[sd] viewControllerWithRestorationIdentifierPath");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		UIViewController *controller = [service application:application viewControllerWithRestorationIdentifierPath:identifierComponents coder:coder];

		if (controller) {
			return controller;
		}
	}

	return nil;
}

- (void)application:(UIApplication *)application willEncodeRestorableStateWithCoder:(NSCoder *)coder {
	NSLog(@"[sd] willEncodeRestorableStateWithCoder");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application willEncodeRestorableStateWithCoder:coder];
	}
}

- (void)application:(UIApplication *)application didDecodeRestorableStateWithCoder:(NSCoder *)coder {
	NSLog(@"[sd] didDecodeRestorableStateWithCoder");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didDecodeRestorableStateWithCoder:coder];
	}
}

// MARK: Download Data in Background

- (void)application:(UIApplication *)application handleEventsForBackgroundURLSession:(NSString *)identifier completionHandler:(void (^)(void))completionHandler {
	NSLog(@"[sd] handleEventsForBackgroundURLSession");
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
	NSLog(@"[sd] willContinueUserActivityWithType");
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
	NSLog(@"[sd] continueUserActivity");
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
	NSLog(@"[sd] didUpdateUserActivity");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didUpdateUserActivity:userActivity];
	}
}

- (void)application:(UIApplication *)application didFailToContinueUserActivityWithType:(NSString *)userActivityType error:(NSError *)error {
	NSLog(@"[sd] didFailToContinueUserActivityWithType");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application didFailToContinueUserActivityWithType:userActivityType error:error];
	}
}

- (void)application:(UIApplication *)application performActionForShortcutItem:(UIApplicationShortcutItem *)shortcutItem completionHandler:(void (^)(BOOL succeeded))completionHandler {
	NSLog(@"[sd] performActionForShortcutItem");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application performActionForShortcutItem:shortcutItem completionHandler:completionHandler];
	}
}

// MARK: WatchKit

- (void)application:(UIApplication *)application handleWatchKitExtensionRequest:(NSDictionary *)userInfo reply:(void (^)(NSDictionary *replyInfo))reply {
	NSLog(@"[sd] handleWatchKitExtensionRequest");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application handleWatchKitExtensionRequest:userInfo reply:reply];
	}
}

// MARK: HealthKit

- (void)applicationShouldRequestHealthAuthorization:(UIApplication *)application {
	NSLog(@"[sd] applicationShouldRequestHealthAuthorization");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service applicationShouldRequestHealthAuthorization:application];
	}
}

// MARK: Opening an URL

- (BOOL)application:(UIApplication *)app openURL:(NSURL *)url options:(NSDictionary<UIApplicationOpenURLOptionsKey, id> *)options {
	NSLog(@"[sd] openURL");
	BOOL result = NO;
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		if ([service application:app openURL:url options:options]) {
			result = YES;
		}
	}
	NSLog(@"[sd] openURL done");

	return result;
}

// MARK: Disallowing Specified App Extension Types

- (BOOL)application:(UIApplication *)application shouldAllowExtensionPointIdentifier:(UIApplicationExtensionPointIdentifier)extensionPointIdentifier {
	NSLog(@"[sd] shouldAllowExtensionPointIdentifier");
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
	NSLog(@"[sd] handlerForIntent");
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
	NSLog(@"[sd] userDidAcceptCloudKitShareWithMetadata");
	for (ApplicationDelegateService *service in services) {
		if (![service respondsToSelector:_cmd]) {
			continue;
		}

		[service application:application userDidAcceptCloudKitShareWithMetadata:cloudKitShareMetadata];
	}
}

/* Handled By Info.plist file for now

// MARK: Interface Geometry

- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window {}

*/

// MARK: Scene

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options API_AVAILABLE(ios(13.0)) {
	NSLog(@"[sd] configurationForConnectingSceneSession");
	UISceneConfiguration *configuration = [UISceneConfiguration configurationWithName:@"main"
																		  sessionRole:UIWindowSceneSessionRoleApplication];

    configuration.delegateClass = GodotSceneDelegate.class;

    return configuration;
}

//- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {}

/* */

@end

API_AVAILABLE(ios(13.0))
@implementation GodotSceneDelegate


@synthesize window = _window;

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions API_AVAILABLE(ios(13.0)) {
	NSLog(@"[sd] willConnectToSession");
	UIApplication *app = [UIApplication sharedApplication];
    self.window = app.delegate.window;
	self.window.windowScene = scene;

	for (NSUserActivity *acticity in connectionOptions.userActivities) {
		NSURL *url = acticity.webpageURL;
		if (url != nil) {
			NSLog(@"[sd] activity url %@", url);
			[app.delegate application:app openURL:url options:@{}];
			NSLog(@"[sd] ctx url %@ done", url);
			break;
		}
	}

	for (UIOpenURLContext *ctx in connectionOptions.URLContexts) {
		NSLog(@"[sd] ctx %@", ctx);
		NSURL *url = ctx.URL;
		if (url != nil) {
			NSLog(@"[sd] ctx url %@", url);
			[app.delegate application:app openURL:url options:@{}];
			// [app.delegate application:app openURL:url options:@{
			// 	UIApplicationOpenURLOptionsSourceApplicationKey: ctx.options.sourceApplication,
			// 	UIApplicationOpenURLOptionsAnnotationKey: ctx.options.annotation
			// }];
			NSLog(@"[sd] ctx url %@ done", url);
			break;
		}
	}
	//[app.delegate application:app openURL:[NSURL URLWithString:@"funexpected://math/auth/signin?fnxdeeplink=yes&token=0VUZFmhOoA3G7M8uiw07xcoe9YgP"] options:@{}];
	NSLog(@"[sd] willConnectToSession done");
}

- (void)sceneWillEnterForeground:(UIScene *)scene {
	NSLog(@"[sd] sceneWillEnterForeground");
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationWillEnterForeground:app];
	NSLog(@"[sd] sceneWillEnterForeground done");
}

- (void)sceneDidBecomeActive:(UIScene *)scene {
	NSLog(@"[sd] sceneDidBecomeActive");
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationDidBecomeActive:app];
}

- (void)sceneWillResignActive:(UIScene *)scene {
	NSLog(@"[sd] sceneWillResignActive");
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationWillResignActive:app];
}

- (void)sceneDidEnterBackground:(UIScene *)scene {
	NSLog(@"[sd] sceneDidEnterBackground");
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate applicationDidEnterBackground:app];
}

- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)URLContexts {
	NSLog(@"[sd] openURLContexts");
	UIApplication *app = [UIApplication sharedApplication];
	for (UIOpenURLContext *ctx in URLContexts) {
		NSLog(@"[sd] ctx %@", ctx);
		NSURL *url = ctx.URL;
		if (url != nil) {
			NSLog(@"[sd] ctx url %@", url);
			[app.delegate application:app openURL:url options:@{}];
			// [app.delegate application:app openURL:url options:@{
			// 	UIApplicationOpenURLOptionsSourceApplicationKey: ctx.options.sourceApplication,
			// 	UIApplicationOpenURLOptionsAnnotationKey: ctx.options.annotation
			// }];
			NSLog(@"[sd] ctx url %@ done", url);
			break;
		}
	}
	NSLog(@"[sd] openURLContexts done");
}



- (void)scene:(UIScene *)scene willContinueUserActivityWithType:(NSString *)userActivityType {
	NSLog(@"[sd] willContinueUserActivityWithType");
	UIApplication *app = [UIApplication sharedApplication];
	[app.delegate application: app willContinueUserActivityWithType:userActivityType];
}

- (void)scene:(UIScene *)scene continueUserActivity:(NSUserActivity *)userActivity {
	NSLog(@"[sd] continueUserActivity %@ %@", userActivity.webpageURL, userActivity.activityType);
	NSURL *url = userActivity.webpageURL;
	UIApplication *app = [UIApplication sharedApplication];
	if (url != nil) {
		NSLog(@"[sd] activity url %@", url);
		[app.delegate application:app openURL:url options:@{}];
		NSLog(@"[sd] activity url %@ done", url);
	}
}




@end
