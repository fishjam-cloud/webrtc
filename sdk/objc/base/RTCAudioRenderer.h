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

#import "RTCMacros.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Objective-C version of webrtc::AudioTrackSinkInterface. Receives raw PCM
 * audio from an RTCAudioTrack. This callback is only invoked for remote
 * audio tracks.
 */
RTC_OBJC_EXPORT
@protocol RTC_OBJC_TYPE
(RTCAudioRenderer)<NSObject>

    /**
     * Called when PCM audio data is available. The `audioData` buffer is only
     * valid for the duration of this call; implementations must copy it if they
     * wish to use it afterwards.
     */
    - (void)renderPCMBuffer : (const void *)audioData bitsPerSample : (int)bitsPerSample sampleRate
    : (int)sampleRate numberOfChannels : (size_t)numberOfChannels numberOfFrames : (size_t)
          numberOfFrames;

@end

NS_ASSUME_NONNULL_END
