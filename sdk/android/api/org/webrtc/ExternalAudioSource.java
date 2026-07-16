/*
 *  Copyright 2026 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

package org.webrtc;

import java.nio.ByteBuffer;

/**
 * An AudioSource whose audio is pushed by the application instead of being
 * captured by the audio device module. Create it with
 * PeerConnectionFactory.createExternalAudioSource() and pass it to
 * PeerConnectionFactory.createAudioTrack(). A track backed by this source is
 * excluded from microphone capture: sending it neither engages the microphone
 * nor mixes microphone audio into the stream.
 */
public class ExternalAudioSource extends AudioSource {
  private final int sampleRateHz;
  private final int channelCount;

  ExternalAudioSource(long nativeSource, int sampleRateHz, int channelCount) {
    super(nativeSource);
    this.sampleRateHz = sampleRateHz;
    this.channelCount = channelCount;
  }

  public int getSampleRateHz() {
    return sampleRateHz;
  }

  public int getChannelCount() {
    return channelCount;
  }

  /**
   * Delivers exactly one 10 ms frame of interleaved signed 16-bit PCM in
   * native byte order: numberOfFrames must equal getSampleRateHz() / 100 and
   * directPcmBuffer must be a direct ByteBuffer holding numberOfFrames *
   * getChannelCount() samples starting at position 0. The caller is
   * responsible for real-time pacing and for serializing calls (single
   * producer); calls may come from any thread.
   */
  public void pushAudioFrame(ByteBuffer directPcmBuffer, int numberOfFrames) {
    if (!directPcmBuffer.isDirect()) {
      throw new IllegalArgumentException("directPcmBuffer must be a direct ByteBuffer");
    }
    nativePushAudioFrame(getNativeAudioSource(), directPcmBuffer, numberOfFrames);
  }

  private static native void nativePushAudioFrame(
      long sourcePointer, ByteBuffer directPcmBuffer, int numberOfFrames);
}
