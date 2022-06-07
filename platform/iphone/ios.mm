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
#include "core/os/os.h"
#import "view_controller.h"

#include <sys/sysctl.h>

#import <UIKit/UIKit.h>
#import <UserNotifications/UserNotifications.h>

void iOS::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_rate_url", "app_id"), &iOS::get_rate_url);
	ClassDB::bind_method(D_METHOD("get_interface_orientation"), &iOS::get_interface_orientation);
	ClassDB::bind_method(D_METHOD("share_data"), &iOS::share_data);
	ClassDB::bind_method(D_METHOD("get_app_version"), &iOS::get_app_version);
	ClassDB::bind_method(D_METHOD("send_notification", "indifiter", "title", "body", "time_offset"), &iOS::send_notification);
	ClassDB::bind_method(D_METHOD("cancel_notifications", "identifier_arr"), &iOS::cancel_notifications);
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


void iOS::send_notification(const String &identifier_s, const String &title_s, const String &body_s, int time_offset){
	UNMutableNotificationContent* content = [[UNMutableNotificationContent alloc] init];
	content.title = [NSString stringWithCString:title_s.utf8().get_data() encoding:NSUTF8StringEncoding];
	content.body = [NSString stringWithCString:body_s.utf8().get_data() encoding:NSUTF8StringEncoding];
	NSString *identifier = [NSString stringWithCString:identifier_s.utf8().get_data() encoding:NSUTF8StringEncoding];

	UNTimeIntervalNotificationTrigger* trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:time_offset repeats:NO];
	UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:trigger];
	UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
	// print_line("Add notification with title: " + title_s + " and body: " + body_s);
	[center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
		if (error) {
			const char *error_str = [error.description UTF8String];
			ERR_PRINTS(String::utf8(error_str != NULL ? error_str : ""));
		}
	}];
}


void iOS::cancel_notifications(const Array &identifier_arr)
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	for (unsigned int i = 0; i < identifier_arr.size(); ++i) {
		String indifiter = identifier_arr[i];
		NSString *identifier = [NSString stringWithCString:indifiter.utf8().get_data() encoding:NSUTF8StringEncoding];
		[result addObject:identifier];
	}
	UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
	[center removePendingNotificationRequestsWithIdentifiers:result];
	// print_line(String("Cancel notifications ") + (Variant)identifier_arr);
}



void iOS::share_data(const String &text, const String &image_path)
{
UIViewController *root_controller = AppDelegate.viewController;
    
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
