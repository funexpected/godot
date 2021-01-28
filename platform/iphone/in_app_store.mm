/*************************************************************************/
/*  in_app_store.mm                                                      */
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

#ifdef STOREKIT_ENABLED

#include "in_app_store.h"
#include "core/project_settings.h"
#include "core/os/file_access.h"

#include "mbedtls/asn1.h"
#include "openssl/bio.h"
#include "openssl/x509v3.h"

extern "C" {
#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
};

bool auto_finish_transactions = true;
NSMutableDictionary *pending_transactions = [NSMutableDictionary dictionary];

@interface SKProduct (LocalizedPrice)
@property(nonatomic, readonly) NSString *localizedPrice;
@end

//----------------------------------//
// SKProduct extension
//----------------------------------//
@implementation SKProduct (LocalizedPrice)
- (NSString *)localizedPrice {
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	[numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
	[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[numberFormatter setLocale:self.priceLocale];
	NSString *formattedString = [numberFormatter stringFromNumber:self.price];
	[numberFormatter release];
	return formattedString;
}
@end

InAppStore *InAppStore::instance = NULL;

void InAppStore::_bind_methods() {
	ClassDB::bind_method(D_METHOD("request_product_info"), &InAppStore::request_product_info);
	ClassDB::bind_method(D_METHOD("restore_purchases"), &InAppStore::restore_purchases);
	ClassDB::bind_method(D_METHOD("purchase"), &InAppStore::purchase);

	ClassDB::bind_method(D_METHOD("get_pending_event_count"), &InAppStore::get_pending_event_count);
	ClassDB::bind_method(D_METHOD("pop_pending_event"), &InAppStore::pop_pending_event);
	ClassDB::bind_method(D_METHOD("finish_transaction"), &InAppStore::finish_transaction);
	ClassDB::bind_method(D_METHOD("finish_all_transactions"), &InAppStore::finish_all_transactions);
	ClassDB::bind_method(D_METHOD("set_auto_finish_transaction"), &InAppStore::set_auto_finish_transaction);
	ClassDB::bind_method(D_METHOD("request_review"), &InAppStore::request_review);
	ClassDB::bind_method(D_METHOD("get_payload"), &InAppStore::get_payload);
};

@interface ProductsDelegate : NSObject <SKProductsRequestDelegate> {
};

@end

@implementation ProductsDelegate

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response {

	NSArray *products = response.products;
	Dictionary ret;
	ret["type"] = "product_info";
	ret["result"] = "ok";
	Array offers;
	PoolStringArray titles;
	PoolStringArray descriptions;
	PoolRealArray prices;
	PoolStringArray ids;
	PoolStringArray localized_prices;
	PoolStringArray currency_codes;
	PoolIntArray subscription_periods;
	PoolStringArray subscription_units;

	for (NSUInteger i = 0; i < [products count]; i++) {

		SKProduct *product = [products objectAtIndex:i];

		const char *str = [product.localizedTitle UTF8String];
		titles.push_back(String::utf8(str != NULL ? str : ""));

		str = [product.localizedDescription UTF8String];
		descriptions.push_back(String::utf8(str != NULL ? str : ""));
		prices.push_back([product.price doubleValue]);
		ids.push_back(String::utf8([product.productIdentifier UTF8String]));
		localized_prices.push_back(String::utf8([product.localizedPrice UTF8String]));
		currency_codes.push_back(String::utf8([[[product priceLocale] objectForKey:NSLocaleCurrencyCode] UTF8String]));
		NSString *ver = [[UIDevice currentDevice] systemVersion];
		float ver_float = [ver floatValue];
		if (ver_float >= 11.2)
		{	
			subscription_periods.push_back(product.subscriptionPeriod.numberOfUnits);
			switch(product.subscriptionPeriod.unit){
				case SKProductPeriodUnitDay: subscription_units.push_back("day"); break;
				case SKProductPeriodUnitWeek: subscription_units.push_back("week"); break;
				case SKProductPeriodUnitMonth: subscription_units.push_back("month"); break;
				case SKProductPeriodUnitYear: subscription_units.push_back("year"); break;
			}
			if (product.introductoryPrice){
				Dictionary discont_offers; 
				//discont_offers["identifier"] = String::utf8([product.introductoryPrice.identifier UTF8String]);
				switch(product.introductoryPrice.paymentMode){
					case SKProductDiscountPaymentModePayAsYouGo: discont_offers["paymentMode"] = Variant("payAsYouGo"); break;
					case SKProductDiscountPaymentModePayUpFront: discont_offers["paymentMode"] = Variant("payUpFront"); break;
					case SKProductDiscountPaymentModeFreeTrial: discont_offers["paymentMode"] = Variant("freeTrial"); break;
				}
				/*
				switch(product.introductoryPrice.type){
					case SKProductDiscountTypeIntroductory: discont_offers["type"] = Variant("introductory"); break;
					case SKProductDiscountTypeSubscription: discont_offers["type"] = Variant("subscription"); break;
				}
				*/
				discont_offers["price"] = Variant([product.introductoryPrice.price doubleValue]);
				discont_offers["priceLocale"] = String::utf8([[[product priceLocale] objectForKey:NSLocaleCurrencyCode] UTF8String]);
				discont_offers["number_of_period"] = Variant(product.introductoryPrice.numberOfPeriods);
				offers.push_back(discont_offers);
			} else
				offers.push_back(String::utf8([@"no introductoryPrice" UTF8String]));
		}
		else
		{
			subscription_periods.push_back(Variant());
			offers.push_back(Variant());
		}
	};
	ret["titles"] = titles;
	ret["descriptions"] = descriptions;
	ret["prices"] = prices;
	ret["ids"] = ids;
	ret["localized_prices"] = localized_prices;
	ret["currency_codes"] = currency_codes;
	ret["subscription_periods"] = subscription_periods;
	ret["subscription_units"] = subscription_units;
	ret["offers"] = offers;

	PoolStringArray invalid_ids;

	for (NSString *ipid in response.invalidProductIdentifiers) {

		invalid_ids.push_back(String::utf8([ipid UTF8String]));
	};
	ret["invalid_ids"] = invalid_ids;

	InAppStore::get_singleton()->_post_event(ret);

	[request release];
};

@end

Error InAppStore::request_product_info(Variant p_params) {

	Dictionary params = p_params;
	ERR_FAIL_COND_V(!params.has("product_ids"), ERR_INVALID_PARAMETER);

	PoolStringArray pids = params["product_ids"];
	printf("************ request product info! %i\n", pids.size());

	NSMutableArray *array = [[[NSMutableArray alloc] initWithCapacity:pids.size()] autorelease];
	for (int i = 0; i < pids.size(); i++) {
		printf("******** adding %ls to product list\n", pids[i].c_str());
		NSString *pid = [[[NSString alloc] initWithUTF8String:pids[i].utf8().get_data()] autorelease];
		[array addObject:pid];
	};

	NSSet *products = [[[NSSet alloc] initWithArray:array] autorelease];
	SKProductsRequest *request = [[SKProductsRequest alloc] initWithProductIdentifiers:products];

	ProductsDelegate *delegate = [[ProductsDelegate alloc] init];

	request.delegate = delegate;
	[request start];

	return OK;
};

@interface ReceiptRestoreDelegate : NSObject <SKRequestDelegate> {
};
@end

@implementation ReceiptRestoreDelegate

- (void)requestDidFinish:(SKRequest *)request {
	Dictionary ret;
	ret["type"] = "receipt_restored";
	ret["result"] = "ok";
	InAppStore::get_singleton()->_post_event(ret);

};

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error {
	Dictionary ret;
	ret["type"] = "receipt_restored";
	ret["result"] = "error";
	ret["error"] = String::utf8([error.localizedDescription UTF8String]);
	InAppStore::get_singleton()->_post_event(ret);
};

@end

Error InAppStore::restore_purchases() {

	printf("restoring purchases!\n");
	ReceiptRestoreDelegate *delegate = [[ReceiptRestoreDelegate alloc] init] ;
	SKReceiptRefreshRequest *request = [[SKReceiptRefreshRequest alloc] init];
	request.delegate = delegate;
	[request start];
	


	[[SKPaymentQueue defaultQueue] restoreCompletedTransactions];

	return OK;
};

Dictionary InAppStore::get_payload(){
	NSData *receipt = nil;
	NSBundle *bundle = [NSBundle mainBundle];
	NSURL *url = [bundle appStoreReceiptURL];
	String url_string = String([[url absoluteString] UTF8String]);
	bool sandbox = url_string.find("sandboxReceipt") >= 0;
	print_line(String("appstorereceipturl: ") + url_string + ", sandbox: " + Variant(sandbox));
	receipt = [NSData dataWithContentsOfURL:url];
	if (receipt != nil) {
		const void *_Nullable rawData = [receipt bytes];
		char *data = (char *)rawData;
		Dictionary res = InAppStore::_validate_payload(data, [receipt length]);
		res["sandbox"] = sandbox;
		return res;
	} else {
		Dictionary res;
		res["error"] = "Payload not available";
		res["sandbox"] = sandbox;
		return res;
	}

}

Dictionary InAppStore::_validate_payload(char* buff, int buff_size){
	print_line("parsing receipt");
	Dictionary res;
	res["error"] = "Invalid receipt";
	BIO *receiptBIO = BIO_new(BIO_s_mem());
	BIO_write(receiptBIO, buff, buff_size);
	PKCS7 *receiptPKCS7 = d2i_PKCS7_bio(receiptBIO, NULL);

	ERR_FAIL_COND_V(!receiptPKCS7, res);
	// Check that the container has a signature
	ERR_FAIL_COND_V(!PKCS7_type_is_signed(receiptPKCS7), res);
	// Check that the signed container has actual data
	ERR_FAIL_COND_V(!PKCS7_type_is_data(receiptPKCS7->d.sign->contents), res);
	print_line("Container  extracted");


	
	// Load the Apple Root CA (downloaded from https://www.apple.com/certificateauthority/)
	res["error"] = "Can't load Apple Root Certificate";
	String cert_path = ProjectSettings::get_singleton()->get("ios/root_certificate");
	FileAccess *f = FileAccess::open(cert_path, FileAccess::READ);
	ERR_FAIL_COND_V(!f, res);
	PoolVector<uint8_t> cert_buff;
	cert_buff.resize(f->get_len());
	f->get_buffer(cert_buff.write().ptr(), f->get_len());


	BIO *appleRootBIO = BIO_new(BIO_s_mem());
	BIO_write(appleRootBIO, (const void *) cert_buff.read().ptr(), cert_buff.size());
	X509 *appleRootX509 = d2i_X509_bio(appleRootBIO, NULL);

	// Create a certificate store
	X509_STORE *store = X509_STORE_new();
	X509_STORE_add_cert(store, appleRootX509);
	
	// Be sure to load the digests before the verification
	OpenSSL_add_all_digests();

	// Check the signature
	res["error"] = "Receipt verification failed";
	int receipt_valid = PKCS7_verify(receiptPKCS7, NULL, store, NULL, NULL, 0);
	ERR_FAIL_COND_V(receipt_valid != 1, res);

	res["error"] = false;

	// Parsing payload

	// Date formatter to handle RFC 3339 dates in GMT time zone
	NSDate *date = nil;
	NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
	[formatter setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"]];
	[formatter setDateFormat:@"yyyy'-'MM'-'dd'T'HH':'mm':'ss'Z'"];
	[formatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
	ASN1_OCTET_STRING *payload_octets = receiptPKCS7->d.sign->contents->d.data;
	unsigned char *ptr = payload_octets->data;
	unsigned char *end = ptr + payload_octets->length;
	size_t len;
	Array iap_receipts;
	res["receipts"] = iap_receipts;
	//ERR_FAIL_COND_V(mbedtls_asn1_get_len(&ptr, &end, &len), res);
	//print_line(String("First tag: ") + itos(*ptr) );
	ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_SET|MBEDTLS_ASN1_CONSTRUCTED), res);
	while (ptr < end) {
		//print_line(String("Set tag: ") + itos(*ptr) );
		ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_SEQUENCE|MBEDTLS_ASN1_CONSTRUCTED), res);
		int attr_type;
		int attr_version;
		ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, end, &attr_type), res);
		ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, end, &attr_version), res);
		ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_OCTET_STRING), res);
		//print_line(String("Checking attr type ") + itos(attr_type) + ", version " + itos(attr_version) + ", value size: " + itos(len)); 
		switch (attr_type) {
			case 2:
				ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_UTF8_STRING), res);
				res["bundle_id"] = String::utf8((const char*)ptr, len);
				ptr += len;
				break;
			case 3:
				ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_UTF8_STRING), res);
				res["bundle_version"] = String::utf8((const char*)ptr, len);
				ptr += len;
				break;
			case 17: {
				//print_line("iap set tag " + itos(*ptr));
				ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, end, &len, MBEDTLS_ASN1_SET|MBEDTLS_ASN1_CONSTRUCTED), res);
				unsigned char *iend = ptr + len;
				Dictionary iap;
				while (ptr < iend) {
					//print_line("iap seq tag " + itos(*ptr));
					ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_SEQUENCE|MBEDTLS_ASN1_CONSTRUCTED), res);
					int iap_type;
					int iap_version;
					ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &iap_type), res);
					ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &iap_version), res);
					ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_OCTET_STRING), res);
					//print_line(String("Checking iap_attr type ") + itos(iap_type) + ", version " + itos(iap_version) + ", value size: " + itos(len)); 
					switch (iap_type){
						case 1701: // quantity
							int quantity;
							ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &quantity), res);
							iap["quantity"] = quantity;
							break;
						case 1702: // product id
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_UTF8_STRING), res);
							iap["product_id"] = String::utf8((const char*)ptr, len);
							ptr += len;
							break;
						case 1703: // transaction id
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_UTF8_STRING), res);
							iap["transaction_id"] = String::utf8((const char*)ptr, len);
							ptr += len;
							break;
						case 1705: // original transaction id
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_UTF8_STRING), res);
							iap["original_transaction_id"] = String::utf8((const char*)ptr, len);
							ptr += len;
							break;
						case 1704: { // purchase date
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_IA5_STRING), res);
							NSString *dateString = [[NSString alloc] initWithBytes:ptr length:len encoding:NSASCIIStringEncoding];
							date = [formatter dateFromString:dateString];
							iap["purchase_date"] = String::utf8((const char*)ptr, len);
							iap["purchase_date_ts"] = (float)[date timeIntervalSince1970];
							ptr += len;
							} break;
						case 1706: { // original purchase date
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_IA5_STRING), res);
							NSString *dateString = [[NSString alloc] initWithBytes:ptr length:len encoding:NSASCIIStringEncoding];
							date = [formatter dateFromString:dateString];
							iap["original_purchase_date"] = String::utf8((const char*)ptr, len);
							iap["original_purchase_date_ts"] = (float)[date timeIntervalSince1970];
							ptr += len;
							} break;
						case 1708: { // subscription expiration_date
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_IA5_STRING), res);
							NSString *dateString = [[NSString alloc] initWithBytes:ptr length:len encoding:NSASCIIStringEncoding];
							date = [formatter dateFromString:dateString];
							iap["subscription_expiration_date"] = String::utf8((const char*)ptr, len);
							iap["subscription_expiration_date_ts"] = (float)[date timeIntervalSince1970];
							ptr += len;
							} break;
						case 1712: { // cancellation date
							ERR_FAIL_COND_V(mbedtls_asn1_get_tag(&ptr, iend, &len, MBEDTLS_ASN1_IA5_STRING), res);
							NSString *dateString = [[NSString alloc] initWithBytes:ptr length:len encoding:NSASCIIStringEncoding];
							date = [formatter dateFromString:dateString];
							iap["subscription_cancellation_date"] = String::utf8((const char*)ptr, len);
							iap["subscription_cancellation_date_ts"] = (float)[date timeIntervalSince1970];
							ptr += len;
							} break;
						//case 1711: // web order id
						//	int web_order_id;
						//	ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &web_order_id), res);
						//	iap["web_order_id"] = web_order_id;
						//	break;
						case 1719: 
							int introductory_price_period;
							ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &introductory_price_period), res);
							iap["introductory_price_period"] = introductory_price_period;
							break;
						case 1713: // web order id
							int trial_period;
							ERR_FAIL_COND_V(mbedtls_asn1_get_int(&ptr, iend, &trial_period), res);
							iap["trial_period"] = trial_period;
							break;

						default:
							//print_line("skip iap attr (type " + itos(iap_type) + ", tag " + itos(*ptr) + ")");
							ptr += len;
							break;
					}
				}
				iap_receipts.push_back(iap);
				} break;
			default:
				//print_line("skip receipt attr");
				ptr += len;
				break;
		}
	}
	
	

	//res["error"] = "Invalid receipt";
	//ERR_FAIL_COND_V(bundleIdString == nil || bundleVersionString == nil || opaqueData == nil || hashData == nil, res);
	
	res["error"] = false;
	//res["bundle_id"] = String::utf8([bundleIdString UTF8String]);
	//res["bundle_version_string"] = String::utf8([bundleVersionString UTF8String]);
	print_line("Receipt verified");





	

	//NSURL *appleRootURL = [[NSBundle mainBundle] URLForResource:@"AppleIncRootCertificate" withExtension:@"cer"];
	//NSData *appleRootData = [NSData dataWithContentsOfURL:appleRootURL];
	// BIO *appleRootBIO = BIO_new(BIO_s_mem());
	// BIO_write(appleRootBIO, cert_buff.read().ptr(), cert_buff.size());
	// X509 *appleRootX509 = d2i_X509_bio(appleRootBIO, NULL);

	// // Create a certificate store
	// X509_STORE *store = X509_STORE_new();
	// X509_STORE_add_cert(store, appleRootX509);

	// // Be sure to load the digests before the verification
	// OpenSSL_add_all_digests();

	// // Check the signature
	// int result = PKCS7_verify(receiptPKCS7, NULL, store, NULL, NULL, 0);
	// if (result != 1) {
	// 	// Validation fails
	// }
	/*
	print_line(String("parsing receipt payload, len:") + Variant((int)length));
	Payload_t *payload = NULL;
	asn_dec_rval_t rval;
	rval = asn_DEF_Payload.ber_decoder(NULL, &asn_DEF_Payload, (void **)&payload, buff, length, 0);
	print_line("payload parced, try to do attributs");
	for (int i=0; i<payload->list.count; i++){
		ReceiptAttribute_t *entry = payload->list.array[i];
		print_line(String("Check Attr type ") + Variant((int)entry->type));
		if (entry->type == 17){
			void *rcp;
			size_t rcp_size;
			InAppReceipt_t *receipt;
			asn_DEF_InAppReceipt.ber_decoder(NULL, &asn_DEF_InAppReceipt, (void**)&receipt, entry->value.buf, entry->value.size, 0);
			for (int j=0; j<receipt->list.count; j++){
				InAppAttribute_t *attr = receipt->list.array[j];
				String attr_value = String((const CharType *)attr->value.buf, attr->value.size);
				print_line(String("atrr type ") + Variant((int)attr->type) + ", attr value: " + attr_value);
				
				switch (attr->type){
					int attr_value;
					asn_DEF_INTEGER.ber_decoder(NULL, &asn_DEF_INTEGER, (void**)&attr_value, attr->value.buf, attr->value.size, 0);
					case 1701: res["quantity"] = attr_value;
				}
				
			}
		}
	}
	*/
	return res;
}

@interface TransObserver : NSObject <SKPaymentTransactionObserver> {
};
@end

@implementation TransObserver

- (BOOL)paymentQueue:(SKPaymentQueue *)queue 
shouldAddStorePayment:(SKPayment *)payment 
          forProduct:(SKProduct *)product {
			  return (true);
		  };


- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions {

	printf("transactions updated!\n");
	for (SKPaymentTransaction *transaction in transactions) {

		switch (transaction.transactionState) {
			case SKPaymentTransactionStatePurchased: {
				printf("status purchased!\n");
				String pid = String::utf8([transaction.payment.productIdentifier UTF8String]);
				String transactionId = String::utf8([transaction.transactionIdentifier UTF8String]);
				InAppStore::get_singleton()->_record_purchase(pid);
				Dictionary ret;
				ret["type"] = "purchase";
				ret["result"] = "ok";
				ret["product_id"] = pid;
				ret["transaction_id"] = transactionId;

				NSData *receipt = nil;
				int sdk_version = 6;

				if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0) {

					NSURL *receiptFileURL = nil;
					NSBundle *bundle = [NSBundle mainBundle];
					if ([bundle respondsToSelector:@selector(appStoreReceiptURL)]) {

						// Get the transaction receipt file path location in the app bundle.
						receiptFileURL = [bundle appStoreReceiptURL];

						// Read in the contents of the transaction file.
						receipt = [NSData dataWithContentsOfURL:receiptFileURL];
						sdk_version = 7;

					} else {
						// Fall back to deprecated transaction receipt,
						// which is still available in iOS 7.

						// Use SKPaymentTransaction's transactionReceipt.
						receipt = transaction.transactionReceipt;
					}

				} else {
					receipt = transaction.transactionReceipt;
				}

				NSString *receipt_to_send = nil;
				if (receipt != nil) {
					receipt_to_send = [receipt base64EncodedStringWithOptions:0];
				}
				Dictionary receipt_ret;
				receipt_ret["receipt"] = String::utf8(receipt_to_send != nil ? [receipt_to_send UTF8String] : "");
				receipt_ret["sdk"] = sdk_version;
				ret["receipt"] = receipt_ret;

				InAppStore::get_singleton()->_post_event(ret);

				if (auto_finish_transactions) {
					[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
				} else {
					[pending_transactions setObject:transaction forKey:transaction.payment.productIdentifier];
				}

			}; break;
			case SKPaymentTransactionStateFailed: {
				printf("status transaction failed!\n");
				String pid = String::utf8([transaction.payment.productIdentifier UTF8String]);
				Dictionary ret;
				ret["type"] = "purchase";
				ret["result"] = "error";
				ret["product_id"] = pid;
				ret["error"] = String::utf8([transaction.error.localizedDescription UTF8String]);
				ret["error_code"] = Variant(transaction.error.code);
				InAppStore::get_singleton()->_post_event(ret);
				[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
			} break;
			case SKPaymentTransactionStateRestored: {
				printf("status transaction restored!\n");
				String pid = String::utf8([transaction.originalTransaction.payment.productIdentifier UTF8String]);
				InAppStore::get_singleton()->_record_purchase(pid);
				Dictionary ret;
				ret["type"] = "restore";
				ret["result"] = "ok";
				ret["product_id"] = pid;
				InAppStore::get_singleton()->_post_event(ret);
				[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
			} break;
			default: {
				printf("status default %i!\n", (int)transaction.transactionState);
			}; break;
		};
	};
};

@end

Error InAppStore::purchase(Variant p_params) {

	ERR_FAIL_COND_V(![SKPaymentQueue canMakePayments], ERR_UNAVAILABLE);
	if (![SKPaymentQueue canMakePayments])
		return ERR_UNAVAILABLE;

	printf("purchasing!\n");
	Dictionary params = p_params;
	ERR_FAIL_COND_V(!params.has("product_id"), ERR_INVALID_PARAMETER);

	NSString *pid = [[[NSString alloc] initWithUTF8String:String(params["product_id"]).utf8().get_data()] autorelease];
	SKPayment *payment = [SKPayment paymentWithProductIdentifier:pid];
	SKPaymentQueue *defq = [SKPaymentQueue defaultQueue];
	[defq addPayment:payment];
	printf("purchase sent!\n");

	return OK;
};

int InAppStore::get_pending_event_count() {
	return pending_events.size();
};

Variant InAppStore::pop_pending_event() {

	Variant front = pending_events.front()->get();
	pending_events.pop_front();

	return front;
};

void InAppStore::_post_event(Variant p_event) {

	pending_events.push_back(p_event);
};

void InAppStore::_record_purchase(String product_id) {

	String skey = "purchased/" + product_id;
	NSString *key = [[[NSString alloc] initWithUTF8String:skey.utf8().get_data()] autorelease];
	[[NSUserDefaults standardUserDefaults] setBool:YES forKey:key];
	[[NSUserDefaults standardUserDefaults] synchronize];
};

InAppStore *InAppStore::get_singleton() {

	return instance;
};

InAppStore::InAppStore() {
	ERR_FAIL_COND(instance != NULL);
	instance = this;
	auto_finish_transactions = false;

	TransObserver *observer = [[TransObserver alloc] init];
	[[SKPaymentQueue defaultQueue] addTransactionObserver:observer];
	//pending_transactions = [NSMutableDictionary dictionary];
};

void InAppStore::finish_transaction(String product_id) {
	NSString *prod_id = [NSString stringWithCString:product_id.utf8().get_data() encoding:NSUTF8StringEncoding];

	if ([pending_transactions objectForKey:prod_id]) {
		[[SKPaymentQueue defaultQueue] finishTransaction:[pending_transactions objectForKey:prod_id]];
		[pending_transactions removeObjectForKey:prod_id];
	}
};

void InAppStore::finish_all_transactions() {
	NSArray *transactions = [[SKPaymentQueue defaultQueue] transactions];
	for(id transaction in transactions){
		[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
	}
}

void InAppStore::set_auto_finish_transaction(bool b) {
	auto_finish_transactions = b;
}

void InAppStore::request_review() {
	 if([SKStoreReviewController class]) {
       [SKStoreReviewController requestReview];
    }
}

InAppStore::~InAppStore(){};

#endif
