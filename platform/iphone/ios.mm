/*************************************************************************/
/*  ios.mm                                                               */
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

#include "ios.h"

#import "app_delegate.h"
#import "view_controller.h"
#import <UIKit/UIKit.h>

#include "core/os/os.h"
#include <sys/sysctl.h>


void iOS::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_rate_url", "app_id"), &iOS::get_rate_url);
	ClassDB::bind_method(D_METHOD("get_interface_orientation"), &iOS::get_interface_orientation);
	ClassDB::bind_method(D_METHOD("share_data"), &iOS::share_data);
	ClassDB::bind_method(D_METHOD("get_app_version"), &iOS::get_app_version);
	
};

void iOS::alert(const char *p_alert, const char *p_title) {
	NSString *title = [NSString stringWithUTF8String:p_title];
	NSString *message = [NSString stringWithUTF8String:p_alert];

	UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
	UIAlertAction *button = [UIAlertAction actionWithTitle:@"OK"
													 style:UIAlertActionStyleCancel
												   handler:^(id){
												   }];

	[alert addAction:button];

	[AppDelegate.viewController presentViewController:alert animated:YES completion:nil];
}

String iOS::get_model() const {
	// [[UIDevice currentDevice] model] only returns "iPad" or "iPhone".
	size_t size;
	sysctlbyname("hw.machine", NULL, &size, NULL, 0);
	char *model = (char *)malloc(size);
	if (model == NULL) {
		return "";
	}
	sysctlbyname("hw.machine", model, &size, NULL, 0);
	NSString *platform = [NSString stringWithCString:model encoding:NSUTF8StringEncoding];
	free(model);
	const char *str = [platform UTF8String];
	return String(str != NULL ? str : "");
}

String iOS::get_rate_url(int p_app_id) const {
	String app_url_path = "itms-apps://itunes.apple.com/app/idAPP_ID";

	String ret = app_url_path.replace("APP_ID", String::num(p_app_id));

	printf("returning rate url %ls\n", ret.c_str());

	return ret;
};

int iOS::get_interface_orientation() const {
	UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
	if (orientation == UIInterfaceOrientationPortrait) {
		return OS::SCREEN_PORTRAIT;
	} else if (orientation == UIInterfaceOrientationPortraitUpsideDown) {
		return OS::SCREEN_REVERSE_PORTRAIT;
	} else if (orientation == UIInterfaceOrientationLandscapeLeft) {
		return OS::SCREEN_LANDSCAPE;
	} else if (orientation == UIInterfaceOrientationLandscapeRight) {
		return OS::SCREEN_REVERSE_LANDSCAPE;
	} else {
		return OS::SCREEN_SENSOR;
	}
}

String iOS::get_app_version()
{
	NSString	*appVersionString = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
	const char	*str = [appVersionString UTF8String];
	return String(str != NULL ? str : "");
}


void iOS::share_data(const String &title, const String &subject, const String &text)
{
    
    UIViewController *root_controller = (UIViewController *)((AppDelegate *)[[UIApplication sharedApplication] delegate]).window.rootViewController;
    
    NSString * message = [NSString stringWithCString:text.utf8().get_data() encoding:NSUTF8StringEncoding];
    
    NSArray * shareItems = @[message];
    
    UIActivityViewController * avc = [[UIActivityViewController alloc] initWithActivityItems:shareItems applicationActivities:nil];
    
    [root_controller presentViewController:avc animated:YES completion:nil];
	//if iPhone
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
  	  [avc presentViewController:avc animated:YES completion:nil];
	}
	//if iPad
	else {
    // Change Rect to position Popover
    UIPopoverController *popup = [[UIPopoverController alloc] initWithContentViewController:avc];
    [popup presentPopoverFromRect:CGRectMake(root_controller.view.frame.size.width * 0.5, root_controller.view.frame.size.height*0.84, 0, 0)inView:root_controller.view permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
}

}

iOS::iOS(){};
