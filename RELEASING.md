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

> ⚠️ **Toolchain:** m124 does **not** build with Xcode 26 (SDK 26.x) — UIKit's
> `UIUtilities/UIDefines.h` is not found and the `RTCAudioSession*` objc files fail to compile
> within ~30s. Build with Xcode 16.x. If it isn't the active `xcode-select`, prefix the build with
> `DEVELOPER_DIR`, e.g. `export DEVELOPER_DIR=/Applications/Xcode-16.4.0.app/Contents/Developer`.

```bash
python3 tools_webrtc/ios/build_ios_libs.py \
    --build_config release \
    --arch device:arm64 simulator:arm64 simulator:x64 \
    -r 0

# LICENSE.md is generated inside WebRTC.xcframework/, so zipping the framework includes it.
cd out_ios_libs
zip -qr --symlinks FishjamWebRTC.xcframework.zip WebRTC.xcframework
shasum -a 256 FishjamWebRTC.xcframework.zip   # note this for Package.swift
cd ..
```

**Android AAR** (bundles `classes.jar` + the per-ABI `libjingle_peerconnection_so.so`):

> ⚠️ **Linux-only.** WebRTC's build asserts `host_os == "linux"` for Android targets, so this
> **cannot** run on the macOS build host — it fails instantly in `gn gen`. Build it in a Linux
> container. A persistent Docker volume `wrtc-android-vol` holds a `target_os=["android","linux"]`
> gclient checkout (amd64); the container is ephemeral, the volume is reused.

```bash
# from the macOS host (Docker Desktop, amd64 emulation):
docker run --rm --platform linux/amd64 -v wrtc-android-vol:/vol -w /vol/src ubuntu:22.04 bash -c '
  set -e
  export DEBIAN_FRONTEND=noninteractive
  apt-get update -qq && apt-get install -y -qq git python3 python3-setuptools curl xz-utils zip unzip ca-certificates pkg-config >/dev/null
  export PATH=/vol/src/third_party/depot_tools:$PATH DEPOT_TOOLS_UPDATE=0
  git remote get-url fishjam >/dev/null 2>&1 || git remote add fishjam https://github.com/fishjam-cloud/webrtc.git
  git fetch fishjam --quiet && git checkout -f <fishjam-m124-sha>
  python3 tools_webrtc/android/build_aar.py --output /vol/FishjamWebRTC.aar
'
# then copy it out (volume is already android-synced, so no gclient sync needed for same DEPS):
docker run --rm -v wrtc-android-vol:/vol -v "$PWD":/out alpine cp /vol/FishjamWebRTC.aar /out/
```

Output: `FishjamWebRTC.aar` (~45 MB, all four ABIs). The whole build runs under amd64 emulation, so
it is slow (~30–40 min).

### 2. Validate the patches are in the binaries

Update this fingerprint check to match the patch set being released. The current
`fishjam-m124` set is the `defer mic permission` change plus the audio-track sink
(iOS `RTCAudioRenderer`, Android `AudioTrackSink`).

**iOS** — the release framework is **stripped** (only ~190 symbols are exported), so `nm -gU`
won't find the patch symbols. Grep the binary's string table instead — every slice must hit both
patches:

```bash
for slice in out_ios_libs/WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  echo "=== $slice ($(lipo -archs "$slice")) ==="
  for needle in RestartAudioUnit RTCAudioRendererAdapter addRenderer:; do
    echo "  $needle: $(strings -a "$slice" | grep -cF "$needle")"
  done
done
```

Each `needle` count must be non-zero in every slice (`RestartAudioUnit` = defer-mic patch,
`RTCAudioRendererAdapter`/`addRenderer:` = audio-sink patch).

**Android** — the AAR must contain the audio-sink Java API and all four ABIs:

```bash
unzip -l FishjamWebRTC.aar | grep -E "org/webrtc/AudioTrackSink|libjingle_peerconnection_so.so"
```

If anything is missing, do not release.

### 3. Bump the podspec on `master`

```bash
git checkout master
```

- `ios/FishjamWebRTC.podspec`: bump `s.version` (the `:http` URL interpolates `s.version`, so no
  other edit needed).
- `Package.swift` (SPM): bump the version in the binary target `url` **and** set `checksum` to the
  `shasum -a 256 FishjamWebRTC.xcframework.zip` from step 1. SPM can't interpolate the version, so
  both must be edited by hand every release.
- `android/build.gradle` (JitPack): nothing to edit — it reads the version from the tag (`$VERSION`)
  automatically.

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

# Re-run the iOS string check against the binary CocoaPods fetched:
for slice in Pods/FishjamWebRTC/WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  strings -a "$slice" | grep -cF "RTCAudioRendererAdapter"
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
