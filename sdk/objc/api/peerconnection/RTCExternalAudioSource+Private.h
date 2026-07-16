/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCExternalAudioSource.h"

#import "RTCAudioSource+Private.h"

#include "pc/external_audio_source.h"

NS_ASSUME_NONNULL_BEGIN

@interface RTC_OBJC_TYPE (RTCExternalAudioSource)
()

    /** The native ExternalAudioSource passed to this RTCExternalAudioSource
     *  during construction.
     */
    @property(nonatomic, readonly)
        rtc::scoped_refptr<webrtc::ExternalAudioSource> nativeExternalAudioSource;

/** Initialize an RTCExternalAudioSource from a native ExternalAudioSource. */
- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory
      nativeExternalAudioSource:
          (rtc::scoped_refptr<webrtc::ExternalAudioSource>)nativeExternalAudioSource;

@end

NS_ASSUME_NONNULL_END
