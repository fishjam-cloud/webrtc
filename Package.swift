// swift-tools-version:5.7
import PackageDescription

// NOTE: unlike the podspec, SPM cannot interpolate the version. On every release
// bump both the `url` (version in the path) and the `checksum` below.
// The checksum is the `shasum -a 256 FishjamWebRTC.xcframework.zip` from
// RELEASING.md step 1. The zip is the same asset the podspec consumes.
let package = Package(
    name: "FishjamWebRTC",
    platforms: [.iOS(.v12)],
    products: [
        .library(
            name: "WebRTC",
            targets: ["WebRTC"]),
    ],
    dependencies: [],
    targets: [
        .binaryTarget(
            name: "WebRTC",
            url: "https://github.com/fishjam-cloud/webrtc/releases/download/v124.0.2.3/FishjamWebRTC.xcframework.zip",
            checksum: "7df79c094bcba0b27699608f476eb857e9995c04aa892135283dd6ed6db674db"
        ),
    ]
)
