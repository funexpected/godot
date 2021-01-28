/*************************************************************************/
/*  icloud.mm                                                            */
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

#ifdef ICLOUD_ENABLED

#include "icloud.h"

#ifndef __IPHONE_9_0
extern "C" {
#endif

#import "app_delegate.h"

#import <Foundation/Foundation.h>

#ifndef __IPHONE_9_0
};
#endif

ICloud *ICloud::instance = NULL;

void ICloud::_bind_methods() {
	ClassDB::bind_method(D_METHOD("remove_key"), &ICloud::remove_key);
	ClassDB::bind_method(D_METHOD("set_key_values"), &ICloud::set_key_values);
	ClassDB::bind_method(D_METHOD("get_key_value"), &ICloud::get_key_value);
	ClassDB::bind_method(D_METHOD("synchronize_key_values"), &ICloud::synchronize_key_values);
	ClassDB::bind_method(D_METHOD("get_all_key_values"), &ICloud::get_all_key_values);

	ClassDB::bind_method(D_METHOD("get_pending_event_count"), &ICloud::get_pending_event_count);
	ClassDB::bind_method(D_METHOD("pop_pending_event"), &ICloud::pop_pending_event);
};

int ICloud::get_pending_event_count() {

	return pending_events.size();
};

Variant ICloud::pop_pending_event() {

	Variant front = pending_events.front()->get();
	pending_events.pop_front();

	return front;
};

ICloud *ICloud::get_singleton() {
	return instance;
};

Variant nsobject_to_variant(NSObject *object);
NSObject *variant_to_nsobject(Variant v);

Error ICloud::remove_key(Variant p_param) {
	String param = p_param;
	NSString *key = [[[NSString alloc] initWithUTF8String:param.utf8().get_data()] autorelease];

	NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];

	if (![[store dictionaryRepresentation] objectForKey:key]) {
		return ERR_INVALID_PARAMETER;
	}

	[store removeObjectForKey:key];
	return OK;
}

//return an array of the keys that could not be set
Variant ICloud::set_key_values(Variant p_params) {
	Dictionary params = p_params;
	Array keys = params.keys();

	Array error_keys;

	for (unsigned int i = 0; i < keys.size(); ++i) {
		String variant_key = keys[i];
		Variant variant_value = params[variant_key];

		NSString *key = [[[NSString alloc] initWithUTF8String:variant_key.utf8().get_data()] autorelease];
		if (key == NULL) {
			error_keys.push_back(variant_key);
			continue;
		}

		NSObject *value = variant_to_nsobject(variant_value);

		if (value == NULL) {
			error_keys.push_back(variant_key);
			continue;
		}

		NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];
		[store setObject:value forKey:key];
	}

	return error_keys;
}

Variant ICloud::get_key_value(Variant p_param) {
	String param = p_param;

	NSString *key = [[[NSString alloc] initWithUTF8String:param.utf8().get_data()] autorelease];
	NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];

	if (![[store dictionaryRepresentation] objectForKey:key]) {
		return Variant();
	}

	Variant result = nsobject_to_variant([[store dictionaryRepresentation] objectForKey:key]);

	return result;
}

Variant ICloud::get_all_key_values() {
	Dictionary result;

	NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];
	NSDictionary *store_dictionary = [store dictionaryRepresentation];

	NSArray *keys = [store_dictionary allKeys];
	int count = [keys count];
	for (int i = 0; i < count; ++i) {
		NSString *k = [keys objectAtIndex:i];
		NSObject *v = [store_dictionary objectForKey:k];

		const char *str = [k UTF8String];
		if (str != NULL) {
			result[String::utf8(str)] = nsobject_to_variant(v);
		}
	}

	return result;
}

Error ICloud::synchronize_key_values() {
	NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];
	BOOL result = [store synchronize];
	if (result == YES) {
		return OK;
	} else {
		return FAILED;
	}
}
/*
Error ICloud::initial_sync() {
	//you sometimes have to write something to the store to get it to download new data.  go apple!
	NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];
	if ([store boolForKey:@"isb"])
		{
				[store setBool:NO forKey:@"isb"];
		}
		else
		{
				[store setBool:YES forKey:@"isb"];
		}
		return synchronize();
}
*/
ICloud::ICloud() {
	ERR_FAIL_COND(instance != NULL);
	instance = this;
	//connected = false;

	[[NSNotificationCenter defaultCenter]
			addObserverForName:NSUbiquitousKeyValueStoreDidChangeExternallyNotification
						object:[NSUbiquitousKeyValueStore defaultStore]
						 queue:nil
					usingBlock:^(NSNotification *notification) {
						NSDictionary *userInfo = [notification userInfo];
						NSInteger change = [[userInfo objectForKey:NSUbiquitousKeyValueStoreChangeReasonKey] integerValue];

						Dictionary ret;
						ret["type"] = "key_value_changed";

						//PoolStringArray result_keys;
						//Array result_values;
						Dictionary keyValues;
						String reason = "";

						if (change == NSUbiquitousKeyValueStoreServerChange) {
							reason = "server";
						} else if (change == NSUbiquitousKeyValueStoreInitialSyncChange) {
							reason = "initial_sync";
						} else if (change == NSUbiquitousKeyValueStoreQuotaViolationChange) {
							reason = "quota_violation";
						} else if (change == NSUbiquitousKeyValueStoreAccountChange) {
							reason = "account";
						}

						ret["reason"] = reason;

						NSUbiquitousKeyValueStore *store = [NSUbiquitousKeyValueStore defaultStore];

						NSArray *keys = [userInfo objectForKey:NSUbiquitousKeyValueStoreChangedKeysKey];
						for (NSString *key in keys) {
							const char *str = [key UTF8String];
							if (str == NULL) {
								continue;
							}

							NSObject *object = [store objectForKey:key];

							//figure out what kind of object it is
							Variant value = nsobject_to_variant(object);

							keyValues[String::utf8(str)] = value;
						}

						ret["changed_values"] = keyValues;
						pending_events.push_back(ret);
					}];
}

ICloud::~ICloud(){};

#endif
