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
            url: "https://github.com/fishjam-cloud/webrtc/releases/download/v124.0.2.4/FishjamWebRTC.xcframework.zip",
            checksum: "9a220a26650409f3a3d751a4b4178fde1ce864ab8136313fd858b03abcf70000"
        ),
    ]
)
