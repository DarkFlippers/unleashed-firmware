#
# NRF24L01+ Enhanced ShockBurst packets decoder
#
payload_len_default = 4
packets = \
(
  '10101010 11101110 00000011 00001000 00001011 01000111 000100 10 0 10101010 10101010 10101010 10101010 00011101',
  '10101010 11001000 11001000 11000011 110011 10 0 00001011 00000011 00000101 00000000 0010001100100000',
)

def bin2hex(x):
    def bin2hex_helper(r):
        while r:
            yield r[0:2].upper()
            r = r[2:]

    fmt = "{0:0" + str(int(len(x) / 8 * 2)) + "X}"
    hex_data = fmt.format(int(x, 2))
    return list(bin2hex_helper(hex_data))


def split_packet(packet, parts):
    """Split a string of 1s and 0s into multiple substrings as specified by parts.
    Example: "111000011000", (3, 4, 2) -> ["111", "0000", "11", "000"]
    :param packet: String of 1s and 0s
    :param parts: Tuple of length of substrings
    :return: list of substrings
    """
    out = []
    packet = packet.replace(' ', '')
    for part_length in parts:
        out.append(packet[0:part_length])
        packet = packet[part_length:]
    out.append(packet)
    return out


def parse_packet(packet, address_length):
    """Split a packet into its fields and return them as tuple."""
    preamble, address, payload_length, pid, no_ack, rest = split_packet(packet=packet,
                                                                        parts=(8, 8 * address_length, 6, 2, 1))
    payload, crc = split_packet(packet=rest, parts=((payload_len_default if int(payload_length, 2) > 32 else int(payload_length, 2)) * 8,))

    assert preamble in ('10101010', '01010101')
    assert len(crc) in (8, 16)

    return preamble, address, payload_length, pid, no_ack, payload, crc


def crc(bits, size=8):
    """Calculate the crc value for the polynomial initialized with 0xFF/0xFFFF)
    :param size: 8 or 16 bit crc
    :param bits: String of 1s and 0s
    :return:
    :polynomial: 1 byte CRC - standard is 0x107 = 0b100000111 = x^8+x^2+x^1+1, result the same for 0x07
    :polynomial: 2 byte CRC - standard is 0x11021 = X^16+X^12+X^5+1, result the same for 0x1021
    """
    if size == 8:
      polynomial = 0x107
      crc = 0xFF
    else:
      polynomial = 0x11021
      crc = 0xFFFF
    max_crc_value = (1 << size) - 1  # e.g. 0xFF for mode 8bit-crc
    for bit in bits:
        bit = int(bit, 2)
        crc <<= 1
        if (crc >> size) ^ bit:  # top most lfsr bit xor current data bit
            crc ^= polynomial
        crc &= max_crc_value  # trim the crc to reject carry over bits
    return crc


if __name__ == '__main__':
    for packet in packets:
        fld = packet.split(' ');
        address_length = -1
        for f in fld:
            if len(f) == 6: break
            address_length += 1
        preamble, address, payload_length, pid, no_ack, payload, crc_received = \
            parse_packet(packet=packet, address_length=address_length)
        crc_size = len(crc_received)
        crc_received = hex(int(crc_received, 2))
        print(f"Packet: {packet}")
        print('\n'.join((
            f'Preamble: {preamble}',
            f'Address: {address_length} bytes - {bin2hex(address)}',
            f'Payload length in packet: {int(payload_length, 2)}, used: {(payload_len_default if int(payload_length, 2) > 32 else int(payload_length, 2))}',
            f'Pid: {int(pid, 2)}',
            f'No_ack: {int(no_ack, 2) == 1}',
            f'Payload: {bin2hex(payload)}',
            f'CRC{crc_size}: {crc_received}')))
        crc_calculated = hex(crc(address + payload_length + pid + no_ack + payload, size=crc_size))
        if crc_received == crc_calculated:
            print('CRC is valid!')
        else:
            print(f'CRC mismatch! Calculated CRC is f{crc_calculated}.')
        print('-------------')
