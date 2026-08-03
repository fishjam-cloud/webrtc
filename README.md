[![Ad](https://swm-delivery.com/www/images/zone-gh-fishjam-1?n=2)](https://swm-delivery.com/www/delivery/ck.php?zoneid=zone-gh-fishjam-1&n=2)
[![Ad](https://swm-delivery.com/www/images/zone-gh-fishjam-2?n=2)](https://swm-delivery.com/www/delivery/ck.php?zoneid=zone-gh-fishjam-2&n=2)
[![Ad](https://swm-delivery.com/www/images/zone-gh-fishjam-3?n=2)](https://swm-delivery.com/www/delivery/ck.php?zoneid=zone-gh-fishjam-3&n=2)

# FishjamWebRTC

This repository contains the [WebRTC](https://webrtc.org) source code with Fishjam
patches, used to build the libraries consumed by Fishjam's
[react-native-webrtc](https://github.com/react-native-webrtc/react-native-webrtc) fork.

It is a fork of [jitsi/webrtc](https://github.com/jitsi/webrtc). For a given Chrome
milestone there are two kinds of branches:

- **Source branches** (e.g. `fishjam-m124`): the WebRTC tree with Fishjam patches. Builds are made
  from here.
- **`master`**: the release/meta branch — it only carries the packaging manifests
  (`ios/FishjamWebRTC.podspec`, `Package.swift`, the `android/` JitPack project, `jitpack.yml`),
  not WebRTC source.

## Using the builds

### iOS — CocoaPods

```ruby
pod 'FishjamWebRTC', '~> 124.0.2.0'   # any patch on 124.0.2, or pin exact e.g. '124.0.2.2'
```

### iOS — Swift Package Manager

A `Package.swift` is provided that points at the prebuilt `WebRTC.xcframework.zip` on the matching
GitHub release.

### Android — JitPack

```kotlin
// settings.gradle(.kts) — dependencyResolutionManagement { repositories { ... } }
maven { url = uri("https://jitpack.io") }
```

```kotlin
// module build.gradle(.kts)
implementation("com.github.fishjam-cloud:webrtc:v124.0.2.2")
```

The JitPack coordinate is the **v-prefixed tag**. JitPack serves the prebuilt AAR
(`classes.jar` + the per-ABI `libjingle_peerconnection_so.so`) attached to the GitHub release.

## Versioning

`<upstream-version>.<fishjam-patch-N>`, e.g. `124.0.2.2` — the jitsi upstream version followed by a
Fishjam patch counter. See [`RELEASING.md`](RELEASING.md) for the full scheme.

## Releasing

Builds are produced manually on a depot_tools host and attached to a GitHub release; CocoaPods and
JitPack both pull the prebuilt artifacts from there. The end-to-end runbook lives in
[`RELEASING.md`](RELEASING.md).
