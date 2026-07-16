/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

#import "RTCAudioSource.h"
#import "RTCMacros.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * An audio source whose audio is pushed by the application instead of being
 * captured by the audio device module. Create it with
 * -[RTCPeerConnectionFactory externalAudioSourceWithSampleRateHz:channelCount:]
 * and pass it to -[RTCPeerConnectionFactory audioTrackWithSource:trackId:].
 * A track backed by this source is excluded from microphone capture: sending
 * it neither engages the microphone nor mixes microphone audio into the
 * stream.
 */
RTC_OBJC_EXPORT
@interface RTC_OBJC_TYPE (RTCExternalAudioSource) : RTC_OBJC_TYPE(RTCAudioSource)

- (instancetype)init NS_UNAVAILABLE;

@property(nonatomic, readonly) int sampleRateHz;
@property(nonatomic, readonly) NSInteger channelCount;

/**
 * Delivers exactly one 10 ms frame of interleaved signed 16-bit PCM:
 * numberOfFrames must equal sampleRateHz / 100. The caller is responsible
 * for real-time pacing and for serializing calls (single producer); calls
 * may come from any thread.
 */
- (void)pushAudioFrameWithSamples:(const int16_t *)samples
                   numberOfFrames:(size_t)numberOfFrames;

@end

NS_ASSUME_NONNULL_END
