/*************************************************************************/
/*  ios.mm                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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
#include <sys/sysctl.h>
#include "core/os/os.h"

#import <UIKit/UIKit.h>
#import "app_delegate.h"

void iOS::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_rate_url", "app_id"), &iOS::get_rate_url);
	ClassDB::bind_method(D_METHOD("get_interface_orientation"), &iOS::get_interface_orientation);
	ClassDB::bind_method(D_METHOD("share_data"), &iOS::share_data);
	ClassDB::bind_method(D_METHOD("get_app_version"), &iOS::get_app_version);
	
};

void iOS::alert(const char *p_alert, const char *p_title) {
	UIAlertView *alert = [[[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:p_title] message:[NSString stringWithUTF8String:p_alert] delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil, nil] autorelease];
	[alert show];
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
	String templ = "itms-apps://ax.itunes.apple.com/WebObjects/MZStore.woa/wa/viewContentsUserReviews?type=Purple+Software&id=APP_ID";
	String templ_iOS7 = "itms-apps://itunes.apple.com/app/idAPP_ID";
	String templ_iOS8 = "itms-apps://itunes.apple.com/WebObjects/MZStore.woa/wa/viewContentsUserReviews?id=APP_ID&onlyLatestVersion=true&pageNumber=0&sortOrdering=1&type=Purple+Software";

	//ios7 before
	String ret = templ;

	if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0 && [[[UIDevice currentDevice] systemVersion] floatValue] < 7.1) {
		// iOS 7 needs a different templateReviewURL @see https://github.com/arashpayan/appirater/issues/131
		ret = templ_iOS7;
	} else if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0) {
		// iOS 8 needs a different templateReviewURL also @see https://github.com/arashpayan/appirater/issues/182
		ret = templ_iOS8;
	}

	// ios7 for everything?
	ret = templ_iOS7.replace("APP_ID", String::num(p_app_id));

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
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        avc.popoverPresentationController.sourceView = root_controller.view;
        avc.popoverPresentationController.sourceRect = CGRectMake(root_controller.view.bounds.size.width/2, root_controller.view.bounds.size.height/4, 0, 0); 
    }
    [root_controller presentViewController:avc animated:true completion:nil];

}

iOS::iOS(){};
