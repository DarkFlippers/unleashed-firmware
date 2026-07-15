let tests = require("tests");
let flipper = require("flipper");

tests.assert_eq(1337, 1337);
tests.assert_eq("hello", "hello");

tests.assert_eq("compatible", sdkCompatibilityStatus(1, 0));
tests.assert_eq("firmwareTooOld", sdkCompatibilityStatus(100500, 0));
tests.assert_eq("firmwareTooNew", sdkCompatibilityStatus(-100500, 0));
tests.assert_eq(true, doesSdkSupport(["baseline"]));
tests.assert_eq(false, doesSdkSupport(["abobus", "other-nonexistent-feature"]));

tests.assert_eq("flipperdevices", flipper.firmwareVendor);
tests.assert_eq(1, flipper.jsSdkVersion[0]);
tests.assert_eq(0, flipper.jsSdkVersion[1]);
