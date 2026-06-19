# Releasing FishjamWebRTC

This document covers cutting a new `FishjamWebRTC` release from a patched WebRTC
source branch (e.g. `fishjam-m124`). One release ships both platforms from a
single tag and GitHub release:

- **iOS** → CocoaPods (`FishjamWebRTC`) + Swift Package Manager.
- **Android** → JitPack (`com.github.fishjam-cloud:webrtc`).

JitPack and CocoaPods both pull **prebuilt** artifacts attached to the GitHub
release — neither service compiles WebRTC. The native builds happen on a
depot_tools host (see steps 1–2); everything else is packaging.

## Versioning

`<upstream-version>.<fishjam-patch-N>`, e.g. `124.0.2.2`.

- First three parts = the jitsi upstream version the build is patched on top of.
- Fourth part = Fishjam patch counter against that base, starts at `1`, increments per release.
- On upstream rebase (e.g. jitsi ships `124.0.3`), reset counter: next release is `124.0.3.1`.
- Consumers (iOS): `pod 'FishjamWebRTC', '~> 124.0.2.0'` for any patch on `124.0.2`, or pin exact.
- Consumers (Android): `implementation 'com.github.fishjam-cloud:webrtc:v124.0.2.2'` (the JitPack
  coordinate is the **v-prefixed tag**, exact-pinned).

## Branch layout

- **Source branches** (e.g. `fishjam-m124`): the WebRTC tree with Fishjam patches. Used for building.
- **`master`**: release/meta branch — only contains `ios/FishjamWebRTC.podspec`, `Package.swift`,
  `android/` (the JitPack publish project), `jitpack.yml`, `README.md`, `tools`. Never merged into
  source branches and vice versa.

The release **tag points at `master`** (the commit carrying the bumped podspec + JitPack project),
not the source branch. JitPack checks out that tag and needs `jitpack.yml` + `android/` present.
Record the source-branch commit in the release notes for build provenance.

## Release steps

### 1. Build the artifacts

On the build host (depot_tools / gclient checkout):

```bash
cd <gclient-src>
git fetch <fishjam-remote> fishjam-m124
git checkout fishjam-m124
gclient sync
SRC_SHA=$(git rev-parse --short HEAD)   # record for the release notes
```

**iOS xcframework:**

```bash
python3 tools_webrtc/ios/build_ios_libs.py \
    --build_config release \
    --arch device:arm64 simulator:arm64 simulator:x64 \
    -r 0

cd out_ios_libs
zip -r --symlinks FishjamWebRTC.xcframework.zip WebRTC.xcframework LICENSE.md
shasum -a 256 FishjamWebRTC.xcframework.zip   # note this for Package.swift
cd ..
```

**Android AAR** (bundles `classes.jar` + the per-ABI `libjingle_peerconnection_so.so`):

```bash
python3 tools_webrtc/android/build_aar.py --output FishjamWebRTC.aar
```

Output: `FishjamWebRTC.aar` in the gclient src root.

### 2. Validate the patches are in the binaries

Update this fingerprint check to match the patch set being released. The current
`fishjam-m124` set is the `defer mic permission` change plus the audio-track sink
(iOS `RTCAudioRenderer`, Android `AudioTrackSink`). These symbols survive stripping.

**iOS** — every slice must print the symbol:

```bash
for slice in out_ios_libs/WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  echo "=== $slice ==="
  nm -gU "$slice" | c++filt | grep -E "AudioDeviceIOS::(RestartAudioUnit|CreateAudioUnit\(bool\)|InitPlayOrRecord\(bool\))"
done
```

**Android** — the AAR must contain the audio-sink Java API and all four ABIs:

```bash
unzip -l FishjamWebRTC.aar | grep -E "org/webrtc/AudioTrackSink|libjingle_peerconnection_so.so"
```

If anything is missing, do not release.

### 3. Bump the podspec on `master`

```bash
git checkout master
```

Bump `s.version` in `ios/FishjamWebRTC.podspec` to the new version (the `:http` URL interpolates
`s.version`, so no other edit needed). The JitPack `android/build.gradle` reads the version from the
tag automatically — no edit there.

> Optional: if SPM is in use, also update `url` + `checksum` in `Package.swift` with the new
> release URL and the `shasum` from step 1.

Commit and push `master`.

### 4. Tag and publish the GitHub release

The tag points at the `master` commit from step 3 and carries **both** artifacts.

```bash
# from a master checkout
git tag v124.0.2.2
git push origin v124.0.2.2

gh release create v124.0.2.2 \
    --repo fishjam-cloud/webrtc \
    --title "124.0.2.2" \
    --notes "Fishjam custom build on top of jitsi v124.0.2 (source: fishjam-m124 @ ${SRC_SHA}). Patches: defer mic permission, audio-track sink (iOS + Android)." \
    <path>/FishjamWebRTC.xcframework.zip \
    <path>/FishjamWebRTC.aar
```

Asset names must be exactly `FishjamWebRTC.xcframework.zip` and `FishjamWebRTC.aar` — the podspec
URL and `android/build.gradle` download URL depend on them.

### 5. Publish iOS to CocoaPods trunk

One-time, per identity:

```bash
pod trunk register milosz.filimowski@swmansion.com 'Milosz Filimowski'
# click the email confirmation link
```

After the GitHub release asset is live (so lint can fetch it):

```bash
git checkout master
pod spec lint ios/FishjamWebRTC.podspec --allow-warnings
pod trunk push ios/FishjamWebRTC.podspec --allow-warnings
```

### 6. Trigger the JitPack (Android) build

JitPack builds lazily on first request. Force it now to surface errors and warm the cache:

```bash
# Triggers a build and streams the log; non-zero exit on build failure.
curl -sS "https://jitpack.io/com/github/fishjam-cloud/webrtc/v124.0.2.2/webrtc-v124.0.2.2.pom" -o /dev/null -w "%{http_code}\n"
```

Or open `https://jitpack.io/#fishjam-cloud/webrtc/v124.0.2.2` and click **Get it** to watch the log.
A green build means the prebuilt AAR was downloaded and published. (No JitPack account or
configuration is needed — the repo's `jitpack.yml` drives it.)

### 7. Smoke-test the published packages

**iOS** (CDN propagation takes a few minutes):

```bash
pod trunk info FishjamWebRTC
pod repo update

mkdir /tmp/fwrtc-smoke && cd /tmp/fwrtc-smoke
cat > Podfile <<'EOF'
platform :ios, '13.4'
use_frameworks!
target 'Smoke' do
  pod 'FishjamWebRTC', '124.0.2.2'
end
EOF
pod install --verbose

# Re-run the iOS symbol check against the binary CocoaPods fetched:
for slice in Pods/FishjamWebRTC/WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  nm -gU "$slice" | c++filt | grep "AudioDeviceIOS::RestartAudioUnit"
done
```

**Android** — resolve the JitPack artifact and confirm the ABIs/API are inside:

```bash
cd /tmp && curl -sSL \
  "https://jitpack.io/com/github/fishjam-cloud/webrtc/v124.0.2.2/webrtc-v124.0.2.2.aar" \
  -o fwrtc.aar
unzip -l fwrtc.aar | grep -E "org/webrtc/AudioTrackSink|libjingle_peerconnection_so.so"
```

## Rollback

If a release is broken, **roll forward** — bump the patch counter and release again. Don't delete the
GitHub release or yank the published versions: CocoaPods and JitPack both cache by version, and
yanking creates harder-to-debug failures.
