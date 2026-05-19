# Releasing FishjamWebRTC

This document covers cutting a new `FishjamWebRTC` CocoaPods release from a patched WebRTC source branch (e.g. `fishjam-m124`).

## Versioning

`<upstream-version>.<fishjam-patch-N>`, e.g. `124.0.2.1`.

- First three parts = the jitsi upstream version the build is patched on top of.
- Fourth part = Fishjam patch counter against that base, starts at `1`, increments per release.
- On upstream rebase (e.g. jitsi ships `124.0.3`), reset counter: next release is `124.0.3.1`.
- Consumers: `pod 'FishjamWebRTC', '~> 124.0.2.0'` for any patch on `124.0.2`, or pin exact.

## Branch layout

- **Source branches** (e.g. `fishjam-m124`): the WebRTC tree with Fishjam patches. Used for building.
- **`master`**: release/meta branch — only contains `ios/FishjamWebRTC.podspec`, `Package.swift`, `README.md`, `.github`, `tools`. Never merged into source branches and vice versa.

## Release steps

### 1. Build the xcframework

On the build host (depot_tools / gclient checkout):

```bash
cd <gclient-src>
git fetch <fishjam-remote> fishjam-m124
git checkout fishjam-m124
gclient sync

python3 tools_webrtc/ios/build_ios_libs.py \
    --build_config release \
    --arch device:arm64 simulator:arm64 simulator:x64 \
    -r 0
```

Output: `out_ios_libs/WebRTC.xcframework`.

Zip + checksum:

```bash
cd out_ios_libs
zip -r --symlinks FishjamWebRTC.xcframework.zip WebRTC.xcframework LICENSE.md
shasum -a 256 FishjamWebRTC.xcframework.zip
```

### 2. Validate the patch is in the binary

Per the current patch set (`fishjam-m124` HEAD), the `defer mic permission` change adds `AudioDeviceIOS::RestartAudioUnit` and gives `CreateAudioUnit` / `InitPlayOrRecord` a `bool enable_input` parameter. These symbols survive stripping.

```bash
for slice in WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  echo "=== $slice ==="
  nm -gU "$slice" | c++filt | grep -E "AudioDeviceIOS::(RestartAudioUnit|CreateAudioUnit\(bool\)|InitPlayOrRecord\(bool\))"
done
```

Every slice must print all three symbols. If any is missing, do not release.

(For future patches, update this check to match the new patch's fingerprint.)

### 3. Tag and publish GitHub release

From this repo (any working copy of the source branch):

```bash
git tag v124.0.2.1 fishjam-m124
git push origin v124.0.2.1

gh release create v124.0.2.1 \
    --repo fishjam-cloud/webrtc \
    --title "124.0.2.1" \
    --notes "Fishjam custom build: defer mic permission patch on top of jitsi v124.0.2." \
    <path>/FishjamWebRTC.xcframework.zip
```

### 4. Update the podspec on `master`

```bash
git checkout master
```

Bump `s.version` in `ios/FishjamWebRTC.podspec` to the new version. The `:http` URL interpolates `s.version` automatically, so no other edit needed.

Commit and push.

### 5. Publish to CocoaPods trunk

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

### 6. Smoke-test the published pod

CDN propagation takes a few minutes.

```bash
pod trunk info FishjamWebRTC
pod repo update
pod search FishjamWebRTC

mkdir /tmp/fwrtc-smoke && cd /tmp/fwrtc-smoke
cat > Podfile <<'EOF'
platform :ios, '13.4'
use_frameworks!
target 'Smoke' do
  pod 'FishjamWebRTC', '124.0.2.1'
end
EOF
pod install --verbose

# Re-run the symbol check against the binary CocoaPods fetched:
for slice in Pods/FishjamWebRTC/WebRTC.xcframework/*/WebRTC.framework/WebRTC; do
  nm -gU "$slice" | c++filt | grep "AudioDeviceIOS::RestartAudioUnit"
done
```

## Rollback

If a release is broken, **roll forward** — bump the patch counter and release again. Don't delete the GitHub release or yank the trunk version: CocoaPods caches by version and yanking creates harder-to-debug failures.
