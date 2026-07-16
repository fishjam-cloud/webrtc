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
#include "sdk/android/generated_peerconnection_jni/ExternalAudioSource_jni.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtc {
namespace jni {

static void JNI_ExternalAudioSource_PushAudioFrame(
    JNIEnv* jni,
    jlong j_native_source,
    const JavaParamRef<jobject>& j_direct_pcm_buffer,
    jint j_number_of_frames) {
  ExternalAudioSource* source =
      reinterpret_cast<ExternalAudioSource*>(j_native_source);
  const int16_t* interleaved_samples = static_cast<const int16_t*>(
      jni->GetDirectBufferAddress(j_direct_pcm_buffer.obj()));
  source->PushAudioFrame(interleaved_samples,
                         static_cast<size_t>(j_number_of_frames));
}

}  // namespace jni
}  // namespace webrtc
