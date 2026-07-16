/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_EXTERNAL_AUDIO_SOURCE_H_
#define PC_EXTERNAL_AUDIO_SOURCE_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "api/audio_options.h"
#include "api/media_stream_interface.h"
#include "api/notifier.h"
#include "api/scoped_refptr.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/thread_annotations.h"

// ExternalAudioSource implements AudioSourceInterface for audio produced by
// the application instead of captured by the audio device module. Unlike
// LocalAudioSource it retains the sinks attached by the audio sender and
// delivers application-pushed PCM to them, and it marks its send stream (via
// AudioOptions::external_audio_injection) so the stream is excluded from the
// audio device module recording fan-out and never engages audio capture.

namespace webrtc {

class ExternalAudioSource : public Notifier<AudioSourceInterface> {
 public:
  static rtc::scoped_refptr<ExternalAudioSource> Create(int sample_rate_hz,
                                                        size_t channel_count);

  SourceState state() const override { return kLive; }
  bool remote() const override { return false; }

  const cricket::AudioOptions options() const override;

  void AddSink(AudioTrackSinkInterface* sink) override;
  void RemoveSink(AudioTrackSinkInterface* sink) override;

  int sample_rate_hz() const { return sample_rate_hz_; }
  size_t channel_count() const { return channel_count_; }

  // Delivers exactly one 10 ms frame of interleaved 16-bit PCM to the
  // attached sinks: number_of_frames must equal sample_rate_hz() / 100.
  // The caller is responsible for real-time pacing and for serializing
  // calls (single producer); calls may come from any thread.
  void PushAudioFrame(const int16_t* interleaved_samples,
                      size_t number_of_frames);

 protected:
  ExternalAudioSource(int sample_rate_hz, size_t channel_count);
  ~ExternalAudioSource() override;

 private:
  const int sample_rate_hz_;
  const size_t channel_count_;

  mutable Mutex sinks_mutex_;
  std::vector<AudioTrackSinkInterface*> sinks_ RTC_GUARDED_BY(sinks_mutex_);
};

}  // namespace webrtc

#endif  // PC_EXTERNAL_AUDIO_SOURCE_H_
