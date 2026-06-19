/*
 *  Copyright 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCAudioRendererAdapter+Private.h"

#include <memory>
#include <optional>

namespace webrtc {

class AudioRendererAdapter : public AudioTrackSinkInterface {
 public:
  AudioRendererAdapter(RTCAudioRendererAdapter* adapter) { adapter_ = adapter; }

  void OnData(const void* audio_data,
              int bits_per_sample,
              int sample_rate,
              size_t number_of_channels,
              size_t number_of_frames) override {
    [adapter_.audioRenderer renderPCMBuffer:audio_data
                              bitsPerSample:bits_per_sample
                                 sampleRate:sample_rate
                           numberOfChannels:number_of_channels
                             numberOfFrames:number_of_frames];
  }

 private:
  __weak RTCAudioRendererAdapter* adapter_;
};

}  // namespace webrtc

@implementation RTCAudioRendererAdapter {
  std::unique_ptr<webrtc::AudioRendererAdapter> _adapter;
}

@synthesize audioRenderer = _audioRenderer;

- (instancetype)initWithNativeRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)audioRenderer {
  NSParameterAssert(audioRenderer);
  if (self = [super init]) {
    _audioRenderer = audioRenderer;
    _adapter.reset(new webrtc::AudioRendererAdapter(self));
  }
  return self;
}

- (webrtc::AudioTrackSinkInterface *)nativeAudioRenderer {
  return _adapter.get();
}

@end
