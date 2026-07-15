# Integration Tests

> [!NOTE]
> This guide is for Flipper Zero firmware repository maintainers.

Manual integration test cases for Flipper Zero firmware, organized by subsystem and subsystem feature. Each Test Case page lists the steps to verify the corresponding functionality.

## What to test

Testing covers the **subsystem that changed.** You don't need to run every test case for every change.

- **Test what changed, plus related regressions.** When you review changes in a subsystem, test the new behavior and re-run a few existing cases for that subsystem (using the steps in the test cases below) to confirm nothing broke. For example, when adding a new Sub-GHz protocol, test the new protocol and some of the existing ones.
- **Skip unrelated subsystems.** There's no point testing NFC when only Sub-GHz was changed.
- **Test everything for core-wide changes.** If a change affects something global, run all the test cases in order.

## When to test

- Before accepting new pull requests.
- Before a release candidate.

## Test cases

- [General](general_test_cases.md)
- [BadUSB](badusb_test_cases.md)
- [CLI](cli_test_cases.md)
- [GoodFAPs](goodfaps_test_cases.md)
- [GPIO](gpio_test_cases.md)
- [iButton](ibutton_test_cases.md)
- [Infrared](infrared_test_cases.md)
- [NFC](nfc_test_cases.md)
- [RFID](rfid_test_cases.md)
- [Sub-GHz](subghz_test_cases.md)
