/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pc/external_audio_source.h"

#include <algorithm>

#include "absl/types/optional.h"
#include "rtc_base/checks.h"
#include "rtc_base/time_utils.h"

namespace webrtc {

rtc::scoped_refptr<ExternalAudioSource> ExternalAudioSource::Create(
    int sample_rate_hz,
    size_t channel_count) {
  RTC_CHECK_GT(sample_rate_hz, 0);
  // Pushes are whole 10 ms frames, so the rate must divide into them evenly.
  RTC_CHECK_EQ(sample_rate_hz % 100, 0);
  RTC_CHECK(channel_count == 1 || channel_count == 2);
  return rtc::make_ref_counted<ExternalAudioSource>(sample_rate_hz,
                                                    channel_count);
}

ExternalAudioSource::ExternalAudioSource(int sample_rate_hz,
                                         size_t channel_count)
    : sample_rate_hz_(sample_rate_hz), channel_count_(channel_count) {}

ExternalAudioSource::~ExternalAudioSource() = default;

const cricket::AudioOptions ExternalAudioSource::options() const {
  cricket::AudioOptions options;
  options.external_audio_injection = true;
  // The audio device module plays no part in producing this source's audio,
  // so starting to send must not pre-initialize recording.
  options.init_recording_on_send = false;
  return options;
}

void ExternalAudioSource::AddSink(AudioTrackSinkInterface* sink) {
  RTC_DCHECK(sink);
  MutexLock lock(&sinks_mutex_);
  if (std::find(sinks_.begin(), sinks_.end(), sink) == sinks_.end()) {
    sinks_.push_back(sink);
  }
}

void ExternalAudioSource::RemoveSink(AudioTrackSinkInterface* sink) {
  MutexLock lock(&sinks_mutex_);
  sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
}

void ExternalAudioSource::PushAudioFrame(const int16_t* interleaved_samples,
                                         size_t number_of_frames) {
  RTC_DCHECK(interleaved_samples);
  RTC_DCHECK_EQ(number_of_frames, static_cast<size_t>(sample_rate_hz_ / 100));
  const absl::optional<int64_t> absolute_capture_timestamp_ms =
      rtc::TimeMillis();
  MutexLock lock(&sinks_mutex_);
  for (AudioTrackSinkInterface* sink : sinks_) {
    sink->OnData(interleaved_samples, /*bits_per_sample=*/16, sample_rate_hz_,
                 channel_count_, number_of_frames,
                 absolute_capture_timestamp_ms);
  }
}

}  // namespace webrtc
