#!/usr/bin/env python
# pylint: disable=missing-module-docstring, too-many-arguments, consider-using-f-string, missing-function-docstring
from smartcard.System import readers


def test_apdu(connection, test_name, apdu, expected_sw1, expected_sw2, expected_data):
    print("Running test: [%s]" % test_name)
    data, sw1, sw2 = connection.transmit(apdu)

    failed = []

    if sw1 != expected_sw1:
        failed.append("SW1: Expected %x, actual %x" % (expected_sw1, sw1))

    if sw2 != expected_sw2:
        failed.append("SW2: Expected %x, actual %x" % (expected_sw2, sw2))

    if len(data) != len(expected_data):
        failed.append(
            "Data: Sizes differ: Expected %x, actual %x"
            % (len(expected_data), len(data))
        )
        print(data)
    elif len(data) > 0:
        data_matches = True
        for i, _ in enumerate(data):
            if data[i] != expected_data[i]:
                data_matches = False

        if not data_matches:
            failed.append("Data: Expected %s, actual %s" % (expected_data, data))

    if len(failed) > 0:
        print("Test failed: ")
        for failure in failed:
            print("- %s" % failure)
    else:
        print("Test passed!")


def main():
    r = readers()
    print("Found following smartcard readers: ")

    for i, sc in enumerate(r):
        print("[%d] %s" % (i, sc))

    print("Select the smartcard reader you want to run tests against:")

    reader_index = int(input())

    if reader_index < len(r):
        connection = r[reader_index].createConnection()

        connection.connect()

        test_apdu(
            connection,
            "INS 0x01: No Lc, no Data, No Le. Expect no data in return",
            [0x01, 0x01, 0x00, 0x00],
            0x90,
            0x00,
            [],
        )

        test_apdu(
            connection,
            "INS 0x02: No Lc, no Data, Le=2. Expect 2 byte data in return",
            [0x01, 0x02, 0x00, 0x00, 0x02],
            0x90,
            0x00,
            [0x62, 0x63],
        )

        test_apdu(
            connection,
            "INS 0x03: Lc=2, data=[0xCA, 0xFE], No Le. Expect no data in return",
            [0x01, 0x03, 0x00, 0x00, 0x02, 0xCA, 0xFE],
            0x90,
            0x00,
            [],
        )

        test_apdu(
            connection,
            "INS 0x04: Lc=2, data=[0xCA, 0xFE], Le=2. Expect 1 byte data in return",
            [0x01, 0x04, 0x00, 0x00, 0x02, 0xCA, 0xFE, 0x02],
            0x90,
            0x00,
            [0xCA, 0xFE],
        )

        small_apdu = list(range(0, 0x0F))

        test_apdu(
            connection,
            "INS 0x04: Lc=0x0F, data=small_apdu, Le=0x0F. Expect 14 bytes data in return",
            [0x01, 0x04, 0x00, 0x00, 0x0F] + small_apdu + [0x0F],
            0x90,
            0x00,
            small_apdu,
        )

        upper_bound = 0xF0
        max_apdu = list(range(0, upper_bound))

        test_apdu(
            connection,
            "INS 0x04: Lc=0x%x, data=max_apdu, Le=0x%x. Expect 0x%x bytes data in return"
            % (upper_bound, upper_bound, upper_bound),
            [0x01, 0x04, 0x00, 0x00, upper_bound] + max_apdu + [upper_bound],
            0x90,
            0x00,
            max_apdu,
        )


if __name__ == "__main__":
    main()
