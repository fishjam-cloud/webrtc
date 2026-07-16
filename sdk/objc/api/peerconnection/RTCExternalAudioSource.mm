/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCExternalAudioSource+Private.h"

#include "rtc_base/checks.h"

@implementation RTC_OBJC_TYPE (RTCExternalAudioSource)

@synthesize nativeExternalAudioSource = _nativeExternalAudioSource;

- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory
      nativeExternalAudioSource:
          (rtc::scoped_refptr<webrtc::ExternalAudioSource>)nativeExternalAudioSource {
  RTC_DCHECK(nativeExternalAudioSource);

  if (self = [super initWithFactory:factory nativeAudioSource:nativeExternalAudioSource]) {
    _nativeExternalAudioSource = nativeExternalAudioSource;
  }
  return self;
}

- (int)sampleRateHz {
  return _nativeExternalAudioSource->sample_rate_hz();
}

- (NSInteger)channelCount {
  return static_cast<NSInteger>(_nativeExternalAudioSource->channel_count());
}

- (void)pushAudioFrameWithSamples:(const int16_t *)samples
                   numberOfFrames:(size_t)numberOfFrames {
  _nativeExternalAudioSource->PushAudioFrame(samples, numberOfFrames);
}

@end
