/*************************************************************************/
/*  view_controller.mm                                                   */
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

#import "view_controller.h"

#include "core/project_settings.h"
#import "godot_view.h"
#import "godot_view_renderer.h"
#import "keyboard_input_view.h"
#import "native_video_view.h"
#include "os_iphone.h"

@interface ViewController () <GodotViewDelegate>

@property(strong, nonatomic) GodotViewRenderer *renderer;
@property(strong, nonatomic) GodotNativeVideoView *videoView;
@property(strong, nonatomic) GodotKeyboardInputView *keyboardView;

@property(strong, nonatomic) UIView *godotLoadingOverlay;

@end

@implementation ViewController

- (GodotView *)godotView {
	return (GodotView *)self.view;
}

- (void)loadView {
	NSLog(@"[GODOT_ORIENTATION] loadView - creating GodotView");
	GodotView *view = [[GodotView alloc] init];
	[view initializeRendering];

	GodotViewRenderer *renderer = [[GodotViewRenderer alloc] init];

	self.renderer = renderer;
	self.view = view;

	view.renderer = self.renderer;
	view.delegate = self;
	
	// Ensure view has proper autoresizing mask for full-screen behavior
	view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	NSLog(@"[GODOT_ORIENTATION] GodotView created with autoresizing mask");
}

- (instancetype)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

	if (self) {
		[self godot_commonInit];
	}

	return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
	self = [super initWithCoder:coder];

	if (self) {
		[self godot_commonInit];
	}

	return self;
}

- (void)godot_commonInit {
	// Initialize view controller values.
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
	printf("*********** did receive memory warning!\n");
};

- (void)viewDidLoad {
	[super viewDidLoad];

	[self observeKeyboard];
	[self displayLoadingOverlay];
	

	if (@available(iOS 11.0, *)) {
		[self setNeedsUpdateOfScreenEdgesDeferringSystemGestures];
	}
	self.view.backgroundColor = [UIColor 
		colorWithRed:0.42
		green:0.35
		blue:0.79
		alpha:1.0
	];
}

- (void)viewWillAppear:(BOOL)animated {
	[super viewWillAppear:animated];
	
	NSLog(@"[GODOT_ORIENTATION] viewWillAppear");
	NSLog(@"[GODOT_ORIENTATION] View frame: %@", NSStringFromCGRect(self.view.frame));
	NSLog(@"[GODOT_ORIENTATION] Window bounds: %@", NSStringFromCGRect(self.view.window.bounds));
	
	// Force full-screen on iOS 16+ when view appears
	if (@available(iOS 16.0, *)) {
		UIWindowScene *windowScene = (UIWindowScene *)self.view.window.windowScene;
		if (windowScene) {
			UIWindowSceneGeometryPreferencesIOS *geometryPreferences = [[UIWindowSceneGeometryPreferencesIOS alloc] init];
			geometryPreferences.interfaceOrientations = [self supportedInterfaceOrientations];
			NSLog(@"[GODOT_ORIENTATION] viewWillAppear requesting geometry with mask: %lu", (unsigned long)[self supportedInterfaceOrientations]);
			[windowScene requestGeometryUpdateWithPreferences:geometryPreferences
												  errorHandler:^(NSError * _Nonnull error) {
				NSLog(@"[GODOT_ORIENTATION] ERROR in viewWillAppear: %@", error.localizedDescription);
			}];
		}
	}
	
	// Ensure view fills window bounds
	if (self.view.window) {
		self.view.frame = self.view.window.bounds;
		NSLog(@"[GODOT_ORIENTATION] Set view frame to window bounds: %@", NSStringFromCGRect(self.view.frame));
	}
}

- (void)observeKeyboard {
	printf("******** setting up keyboard input view\n");
	self.keyboardView = [GodotKeyboardInputView new];
	[self.view addSubview:self.keyboardView];

	printf("******** adding observer for keyboard show/hide\n");
	[[NSNotificationCenter defaultCenter]
			addObserver:self
			   selector:@selector(keyboardOnScreen:)
				   name:UIKeyboardDidShowNotification
				 object:nil];
	[[NSNotificationCenter defaultCenter]
			addObserver:self
			   selector:@selector(keyboardHidden:)
				   name:UIKeyboardDidHideNotification
				 object:nil];
}

- (void)displayLoadingOverlay {
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *storyboardName = @"Launch Screen";

	if ([bundle pathForResource:storyboardName ofType:@"storyboardc"] == nil) {
		return;
	}

	UIStoryboard *launchStoryboard = [UIStoryboard storyboardWithName:storyboardName bundle:bundle];

	UIViewController *controller = [launchStoryboard instantiateInitialViewController];
	self.godotLoadingOverlay = controller.view;
	self.godotLoadingOverlay.frame = self.view.bounds;
	self.godotLoadingOverlay.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;

	[self.view addSubview:self.godotLoadingOverlay];
}

- (BOOL)godotViewFinishedSetup:(GodotView *)view {
	[self.godotLoadingOverlay removeFromSuperview];
	self.godotLoadingOverlay = nil;

	return YES;
}

- (void)dealloc {
	[self.videoView stopVideo];
	self.videoView = nil;

	self.keyboardView = nil;

	self.renderer = nil;

	if (self.godotLoadingOverlay) {
		[self.godotLoadingOverlay removeFromSuperview];
		self.godotLoadingOverlay = nil;
	}

	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

// MARK: Orientation

- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures {
	return UIRectEdgeAll;
}

- (BOOL)shouldAutorotate {
	if (!OSIPhone::get_singleton()) {
		return NO;
	}
	switch (OS::get_singleton()->get_screen_orientation()) {
		case OS::SCREEN_SENSOR:
		case OS::SCREEN_SENSOR_LANDSCAPE:
		case OS::SCREEN_SENSOR_PORTRAIT:
			return YES;
		default:
			return NO;
	}
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
	if (!OSIPhone::get_singleton()) {
		NSLog(@"[GODOT_ORIENTATION] supportedInterfaceOrientations: No singleton, returning Portrait");
		return UIInterfaceOrientationMaskPortrait;
	}
	OS::ScreenOrientation orientation = OS::get_singleton()->get_screen_orientation();
	UIInterfaceOrientationMask mask;
	switch (orientation) {
		case OS::SCREEN_PORTRAIT:
			mask = UIInterfaceOrientationMaskPortrait;
			break;
		case OS::SCREEN_REVERSE_LANDSCAPE:
			mask = UIInterfaceOrientationMaskLandscapeRight;
			break;
		case OS::SCREEN_REVERSE_PORTRAIT:
			mask = UIInterfaceOrientationMaskPortraitUpsideDown;
			break;
		case OS::SCREEN_SENSOR_LANDSCAPE:
			mask = UIInterfaceOrientationMaskLandscape;
			break;
		case OS::SCREEN_SENSOR_PORTRAIT:
			mask = UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
			break;
		case OS::SCREEN_SENSOR:
			mask = UIInterfaceOrientationMaskAll;
			break;
		case OS::SCREEN_LANDSCAPE:
			mask = UIInterfaceOrientationMaskLandscapeLeft;
			break;
		default:
			mask = UIInterfaceOrientationMaskPortrait;
			break;
	}
	NSLog(@"[GODOT_ORIENTATION] supportedInterfaceOrientations: orientation=%d, mask=%lu", (int)orientation, (unsigned long)mask);
	return mask;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
	if (!OSIPhone::get_singleton()) {
		return UIInterfaceOrientationPortrait;
	}
	switch (OS::get_singleton()->get_screen_orientation()) {
		case OS::SCREEN_PORTRAIT:
			return UIInterfaceOrientationPortrait;
		case OS::SCREEN_REVERSE_LANDSCAPE:
			return UIInterfaceOrientationLandscapeRight;
		case OS::SCREEN_REVERSE_PORTRAIT:
			return UIInterfaceOrientationPortraitUpsideDown;
		case OS::SCREEN_SENSOR_LANDSCAPE:
		case OS::SCREEN_LANDSCAPE:
			return UIInterfaceOrientationLandscapeLeft;
		case OS::SCREEN_SENSOR:
		case OS::SCREEN_SENSOR_PORTRAIT:
		default:
			return UIInterfaceOrientationPortrait;
	}
}

- (BOOL)prefersStatusBarHidden {
	return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden {
	if (GLOBAL_GET("display/window/ios/hide_home_indicator")) {
		return YES;
	} else {
		return NO;
	}
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
	NSLog(@"[GODOT_ORIENTATION] ====== viewWillTransitionToSize ======");
	NSLog(@"[GODOT_ORIENTATION] Target size: %@", NSStringFromCGSize(size));
	NSLog(@"[GODOT_ORIENTATION] Current view frame: %@", NSStringFromCGRect(self.view.frame));
	NSLog(@"[GODOT_ORIENTATION] Current window bounds: %@", NSStringFromCGRect(self.view.window.bounds));
	NSLog(@"[GODOT_ORIENTATION] Current window frame: %@", NSStringFromCGRect(self.view.window.frame));
	NSLog(@"[GODOT_ORIENTATION] Device screen bounds: %@", NSStringFromCGRect([UIScreen mainScreen].bounds));
	NSLog(@"[GODOT_ORIENTATION] Device native bounds: %@", NSStringFromCGRect([UIScreen mainScreen].nativeBounds));
	NSLog(@"[GODOT_ORIENTATION] Device scale: %.2f", [UIScreen mainScreen].scale);
	
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
	
	// Ensure full-screen layout during orientation transitions
	[coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
		NSLog(@"[GODOT_ORIENTATION] >>> Transition animation block");
		
		// Update view frame to fill the entire screen during transition
		CGRect newFrame = CGRectMake(0, 0, size.width, size.height);
		NSLog(@"[GODOT_ORIENTATION] Setting view frame to: %@", NSStringFromCGRect(newFrame));
		self.view.frame = newFrame;
		
		// Also ensure subviews update
		for (UIView *subview in self.view.subviews) {
			if ([subview isKindOfClass:NSClassFromString(@"GodotView")]) {
				NSLog(@"[GODOT_ORIENTATION] Updating GodotView subview frame to: %@", NSStringFromCGRect(newFrame));
				subview.frame = newFrame;
			}
		}
		
		[self.view layoutIfNeeded];
		NSLog(@"[GODOT_ORIENTATION] View frame after layout: %@", NSStringFromCGRect(self.view.frame));
		
		// Request geometry update for iOS 16+ to maintain full-screen
		if (@available(iOS 16.0, *)) {
			UIWindowScene *windowScene = (UIWindowScene *)self.view.window.windowScene;
			if (windowScene) {
				UIWindowSceneGeometryPreferencesIOS *geometryPreferences = [[UIWindowSceneGeometryPreferencesIOS alloc] init];
				// Set orientation mask to force full-screen during transition
				geometryPreferences.interfaceOrientations = [self supportedInterfaceOrientations];
				NSLog(@"[GODOT_ORIENTATION] Requesting geometry update with mask: %lu", (unsigned long)[self supportedInterfaceOrientations]);
				[windowScene requestGeometryUpdateWithPreferences:geometryPreferences
													  errorHandler:^(NSError * _Nonnull error) {
					NSLog(@"[GODOT_ORIENTATION] ERROR during transition: %@", error.localizedDescription);
				}];
			}
		}
	} completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
		NSLog(@"[GODOT_ORIENTATION] <<< Transition completion block");
		NSLog(@"[GODOT_ORIENTATION] Window bounds: %@", NSStringFromCGRect(self.view.window.bounds));
		NSLog(@"[GODOT_ORIENTATION] Window frame: %@", NSStringFromCGRect(self.view.window.frame));
		
		// Force window to recalculate size (simulating manual resize behavior)
		if (@available(iOS 13.0, *)) {
			UIWindowScene *windowScene = (UIWindowScene *)self.view.window.windowScene;
			if (windowScene) {
				NSLog(@"[GODOT_ORIENTATION] Scene coordinate space bounds: %@", NSStringFromCGRect(windowScene.coordinateSpace.bounds));
				// Update window to match scene bounds
				self.view.window.frame = windowScene.coordinateSpace.bounds;
				NSLog(@"[GODOT_ORIENTATION] Updated window frame to scene bounds: %@", NSStringFromCGRect(self.view.window.frame));
			}
		}
		
		// Ensure final layout is correct after transition completes
		self.view.frame = self.view.window.bounds;
		NSLog(@"[GODOT_ORIENTATION] Set final view frame to: %@", NSStringFromCGRect(self.view.frame));
		
		// Update all subviews
		for (UIView *subview in self.view.subviews) {
			if ([subview isKindOfClass:NSClassFromString(@"GodotView")]) {
				subview.frame = self.view.bounds;
				NSLog(@"[GODOT_ORIENTATION] Set GodotView subview frame to: %@", NSStringFromCGRect(subview.frame));
				// Force GodotView to re-layout its content
				[subview setNeedsLayout];
				[subview layoutIfNeeded];
			}
		}
		
		[self.view setNeedsLayout];
		[self.view layoutIfNeeded];
		
		// Double-check and force one more time if needed
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
			NSLog(@"[GODOT_ORIENTATION] Post-transition check");
			NSLog(@"[GODOT_ORIENTATION] Final window frame: %@", NSStringFromCGRect(self.view.window.frame));
			NSLog(@"[GODOT_ORIENTATION] Final view frame: %@", NSStringFromCGRect(self.view.frame));
			
			// Force one final update if sizes don't match
			if (!CGRectEqualToRect(self.view.frame, self.view.window.bounds)) {
				NSLog(@"[GODOT_ORIENTATION] WARNING: Size mismatch detected, forcing correction");
				self.view.frame = self.view.window.bounds;
				[self.view setNeedsLayout];
				[self.view layoutIfNeeded];
			}
			
			// CRITICAL: Force the rendering layer to update its scale and bounds
			// This ensures Godot renders at the correct resolution
			NSLog(@"[GODOT_ORIENTATION] Checking view subviews, count: %lu", (unsigned long)self.view.subviews.count);
			
			// The view controller's view IS the GodotView itself!
			if ([self.view isKindOfClass:NSClassFromString(@"GodotView")]) {
				NSLog(@"[GODOT_ORIENTATION] Found GodotView (is self.view)");
				CAEAGLLayer *layer = (CAEAGLLayer *)self.view.layer;
				CGFloat scale = [UIScreen mainScreen].nativeScale;
				NSLog(@"[GODOT_ORIENTATION] Current layer contentsScale: %.2f", layer.contentsScale);
				NSLog(@"[GODOT_ORIENTATION] Setting layer contentsScale to: %.2f", scale);
				layer.contentsScale = scale;
				layer.bounds = self.view.bounds;
				NSLog(@"[GODOT_ORIENTATION] Layer bounds set to: %@", NSStringFromCGRect(layer.bounds));
				
				// Force layer to re-layout and recreate framebuffer
				[layer setNeedsLayout];
				[layer layoutIfNeeded];
				
				// Force view layout too
				[self.view setNeedsLayout];
				[self.view layoutIfNeeded];
				
				NSLog(@"[GODOT_ORIENTATION] Rendering layer forcefully updated");
			} else {
				NSLog(@"[GODOT_ORIENTATION] WARNING: self.view is not GodotView, it's %@", NSStringFromClass([self.view class]));
			}
		});
		
		NSLog(@"[GODOT_ORIENTATION] Final view frame: %@", NSStringFromCGRect(self.view.frame));
		
		// Notify the OS layer of the window size change
		if (OSIPhone::get_singleton()) {
			OSIPhone::get_singleton()->on_focus_in();
		}
		
		NSLog(@"[GODOT_ORIENTATION] ====== Transition complete ======");
	}];
}

// MARK: Keyboard

- (void)keyboardOnScreen:(NSNotification *)notification {
	NSDictionary *info = notification.userInfo;
	NSValue *value = info[UIKeyboardFrameEndUserInfoKey];

	CGRect rawFrame = [value CGRectValue];
	CGRect keyboardFrame = [self.view convertRect:rawFrame fromView:nil];

	if (OSIPhone::get_singleton()) {
		OSIPhone::get_singleton()->set_virtual_keyboard_height(keyboardFrame.size.height);
	}
}

- (void)keyboardHidden:(NSNotification *)notification {
	if (OSIPhone::get_singleton()) {
		OSIPhone::get_singleton()->set_virtual_keyboard_height(0);
	}
}

// MARK: Native Video Player

- (BOOL)playVideoAtPath:(NSString *)filePath volume:(float)videoVolume audio:(NSString *)audioTrack subtitle:(NSString *)subtitleTrack {
	// If we are showing some video already, reuse existing view for new video.
	if (self.videoView) {
		return [self.videoView playVideoAtPath:filePath volume:videoVolume audio:audioTrack subtitle:subtitleTrack];
	} else {
		// Create autoresizing view for video playback.
		GodotNativeVideoView *videoView = [[GodotNativeVideoView alloc] initWithFrame:self.view.bounds];
		videoView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		[self.view addSubview:videoView];

		self.videoView = videoView;

		return [self.videoView playVideoAtPath:filePath volume:videoVolume audio:audioTrack subtitle:subtitleTrack];
	}
}

@end
