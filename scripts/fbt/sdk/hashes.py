def gnu_sym_hash(name: str) -> int:
    h = 0x1505
    for c in name:
        h = ((h << 5) + h + ord(c)) & 0xFFFFFFFF
    return h
