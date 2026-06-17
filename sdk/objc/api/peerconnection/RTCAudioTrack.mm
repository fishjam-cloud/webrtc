/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "RTCAudioTrack+Private.h"

#import "RTCAudioSource+Private.h"
#import "RTCMediaStreamTrack+Private.h"
#import "RTCPeerConnectionFactory+Private.h"
#import "api/RTCAudioRendererAdapter+Private.h"
#import "base/RTCAudioRenderer.h"
#import "helpers/NSString+StdString.h"

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

@implementation RTC_OBJC_TYPE (RTCAudioTrack) {
  NSMutableArray *_adapters;
}

@synthesize source = _source;

- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory
                         source:(RTC_OBJC_TYPE(RTCAudioSource) *)source
                        trackId:(NSString *)trackId {
  RTC_DCHECK(factory);
  RTC_DCHECK(source);
  RTC_DCHECK(trackId.length);

  std::string nativeId = [NSString stdStringForString:trackId];
  rtc::scoped_refptr<webrtc::AudioTrackInterface> track =
      factory.nativeFactory->CreateAudioTrack(nativeId, source.nativeAudioSource.get());
  if (self = [self initWithFactory:factory nativeTrack:track type:RTCMediaStreamTrackTypeAudio]) {
    _source = source;
  }
  return self;
}

- (instancetype)initWithFactory:(RTC_OBJC_TYPE(RTCPeerConnectionFactory) *)factory
                    nativeTrack:(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>)nativeTrack
                           type:(RTCMediaStreamTrackType)type {
  NSParameterAssert(factory);
  NSParameterAssert(nativeTrack);
  NSParameterAssert(type == RTCMediaStreamTrackTypeAudio);
  return [super initWithFactory:factory nativeTrack:nativeTrack type:type];
}

- (RTC_OBJC_TYPE(RTCAudioSource) *)source {
  if (!_source) {
    rtc::scoped_refptr<webrtc::AudioSourceInterface> source(self.nativeAudioTrack->GetSource());
    if (source) {
      _source = [[RTC_OBJC_TYPE(RTCAudioSource) alloc] initWithFactory:self.factory
                                                     nativeAudioSource:source];
    }
  }
  return _source;
}

- (void)dealloc {
  for (RTCAudioRendererAdapter *adapter in _adapters) {
    self.nativeAudioTrack->RemoveSink(adapter.nativeAudioRenderer);
  }
}

- (void)addRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)renderer {
  if (!_adapters) {
    _adapters = [NSMutableArray array];
  }
  // Make sure we don't have this renderer yet.
  for (RTCAudioRendererAdapter *adapter in _adapters) {
    if (adapter.audioRenderer == renderer) {
      RTC_LOG(LS_INFO) << "|renderer| is already attached to this track";
      return;
    }
  }
  // Create a wrapper that provides a native pointer for us.
  RTCAudioRendererAdapter *adapter =
      [[RTCAudioRendererAdapter alloc] initWithNativeRenderer:renderer];
  [_adapters addObject:adapter];
  self.nativeAudioTrack->AddSink(adapter.nativeAudioRenderer);
}

- (void)removeRenderer:(id<RTC_OBJC_TYPE(RTCAudioRenderer)>)renderer {
  __block NSUInteger indexToRemove = NSNotFound;
  [_adapters enumerateObjectsUsingBlock:^(RTCAudioRendererAdapter *adapter, NSUInteger idx, BOOL *stop) {
    if (adapter.audioRenderer == renderer) {
      indexToRemove = idx;
      *stop = YES;
    }
  }];
  if (indexToRemove == NSNotFound) {
    RTC_LOG(LS_INFO) << "removeRenderer called with a renderer that has not been previously added";
    return;
  }
  RTCAudioRendererAdapter *adapterToRemove = [_adapters objectAtIndex:indexToRemove];
  self.nativeAudioTrack->RemoveSink(adapterToRemove.nativeAudioRenderer);
  [_adapters removeObjectAtIndex:indexToRemove];
}

#pragma mark - Private

- (rtc::scoped_refptr<webrtc::AudioTrackInterface>)nativeAudioTrack {
  return rtc::scoped_refptr<webrtc::AudioTrackInterface>(
      static_cast<webrtc::AudioTrackInterface *>(self.nativeTrack.get()));
}

@end
