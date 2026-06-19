/*
 *  Copyright 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCAudioRendererAdapter.h"

#import "base/RTCAudioRenderer.h"

#include "api/media_stream_interface.h"

NS_ASSUME_NONNULL_BEGIN

@interface RTCAudioRendererAdapter ()

/**
 * The Objective-C audio renderer passed to this adapter during construction.
 * Calls made to the webrtc::AudioTrackSinkInterface will be adapted and passed
 * to this audio renderer.
 */
@property(nonatomic, readonly) id<RTC_OBJC_TYPE(RTCAudioRenderer)> audioRenderer;

/**
 * The native AudioTrackSinkInterface surface exposed by this adapter. Calls
 * made to this interface will be adapted and passed to the RTCAudioRenderer
 * supplied during construction. This pointer is unsafe and owned by this class.
 */
@property(nonatomic, readonly) webrtc::AudioTrackSinkInterface *nativeAudioRenderer;

/** Initialize an RTCAudioRendererAdapter with an RTCAudioRenderer. */
- (instancetype)initWithNativeRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)audioRenderer
    NS_DESIGNATED_INITIALIZER;

@end

NS_ASSUME_NONNULL_END
