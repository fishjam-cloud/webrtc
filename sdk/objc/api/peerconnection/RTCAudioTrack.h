/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCMacros.h"
#import "RTCMediaStreamTrack.h"

NS_ASSUME_NONNULL_BEGIN

@class RTC_OBJC_TYPE(RTCAudioSource);
@protocol RTC_OBJC_TYPE
(RTCAudioRenderer);

RTC_OBJC_EXPORT
@interface RTC_OBJC_TYPE (RTCAudioTrack) : RTC_OBJC_TYPE(RTCMediaStreamTrack)

- (instancetype)init NS_UNAVAILABLE;

/** The audio source for this audio track. */
@property(nonatomic, readonly) RTC_OBJC_TYPE(RTCAudioSource) * source;

/** Register a renderer that will receive all decoded PCM audio from this
 *  track. Only meaningful for remote audio tracks. Adding the same renderer
 *  twice is a no-op. */
- (void)addRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)renderer;

/** Deregister a renderer previously added with addRenderer:. */
- (void)removeRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)renderer;

@end

NS_ASSUME_NONNULL_END
