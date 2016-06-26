/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <WebKit/WKFoundation.h>

#if WK_API_ENABLED

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/*! @constant WKWebsiteDataTypeDiskCache On-disk caches. */
WK_EXTERN NSString * const WKWebsiteDataTypeDiskCache NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeMemoryCache In-memory caches. */
WK_EXTERN NSString * const WKWebsiteDataTypeMemoryCache NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeOfflineWebApplicationCache HTML offline web application caches. */
WK_EXTERN NSString * const WKWebsiteDataTypeOfflineWebApplicationCache NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeCookies Cookies. */
WK_EXTERN NSString * const WKWebsiteDataTypeCookies NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeSessionStorage HTML session storage. */
WK_EXTERN NSString * const WKWebsiteDataTypeSessionStorage NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeLocalStorage HTML local storage. */
WK_EXTERN NSString * const WKWebsiteDataTypeLocalStorage NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeWebSQLDatabases WebSQL databases. */
WK_EXTERN NSString * const WKWebsiteDataTypeWebSQLDatabases NS_AVAILABLE(10_11, 9_0);

/*! @constant WKWebsiteDataTypeIndexedDBDatabases IndexedDB databases. */
WK_EXTERN NSString * const WKWebsiteDataTypeIndexedDBDatabases NS_AVAILABLE(10_11, 9_0);

/*! A WKWebsiteDataRecord represents website data, grouped by domain name using the public suffix list. */
NS_CLASS_AVAILABLE(10_11, 9_0)
@interface WKWebsiteDataRecord : NSObject

/*! @abstract The display name for the data record. This is usually the domain name. */
@property (nonatomic, readonly, copy) NSString *displayName;

/*! @abstract The various types of website data that exist for this data record. */
@property (nonatomic, readonly, copy) NSSet<NSString *> *dataTypes;

@end

NS_ASSUME_NONNULL_END

#endif
