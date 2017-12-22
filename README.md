# Notes

Runs on Vocore V1 and uses a NESDR Nano 2. Green LED on GPIO22, red LED on GPIO23.

Connection to Vocore: UART 57600 8N1, or ssh via WiFi in STA mode, or WiFi AP SSID "Vocore", "Openwrt" or "PBot".

# Building

Use the OpenWRT SDK. Source must be put in `~/SDK.../package/PBot/src`.
The vocore needs librtlsdr (which needs libusb (which needs libpthread and librt)), also libopenssl and curl.

Warning: opkg might not succeed in resolving dependencies, if that's the case, wget them from `downloads.openwrt.org/chaos_calmer/15.05/ramips/rt288x/packages/` (HTTP not HTTPS !)

For the SDK, do `./scripts/feeds udpate -a` then `/scripts/feeds install xxx` to get the required libs.

To build, in the SDK's root: `make package/PBot/compile`

Testing code: SCP the binary alone from `~/SDK.../build_dir/target.../pbot/pbot`, or the .ipk and `opkg install pbot.ipk` from `~/SDK.../bin/ramips/packages/base/`
