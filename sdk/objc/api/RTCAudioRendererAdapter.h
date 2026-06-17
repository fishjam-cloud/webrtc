/*
 *  Copyright 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/*
 * Creates a webrtc::AudioTrackSinkInterface surface for an RTCAudioRenderer.
 * The webrtc::AudioTrackSinkInterface is used by WebRTC to deliver decoded
 * audio - this adapter adapts calls made to that interface to the
 * RTCAudioRenderer supplied during construction.
 */
@interface RTCAudioRendererAdapter : NSObject

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
