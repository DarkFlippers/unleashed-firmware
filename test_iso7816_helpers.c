#include <stdio.h>
#include <stdlib.h>

#include "lib/nfc/helpers/iso7816.h"

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RESET "\033[0;0m"

#define num_elements(A) (sizeof(A)/sizeof(A[0]))

//TODO: do something with ISO7816-4 Table 9 — Interindustry data objects for tag allocation authority
//0x06 Object identifier (encoding specified in ISO/IEC 8825-1, see examples in annex A)
//0x41 Country code (encoding specified in ISO 3166-1 [1] ) and optional national data
//0x42 Issuer identification number (encoding and registration specified in ISO/IEC 7812-1 [3] ) and optional issuer data
//0x4F Application identifier (AID, encoding specified in 8.2.1.2)

void print_hex(const uint8_t* data, size_t length) {
    for(size_t i=0; i<length; ++i) {
        printf("%02X", data[i]);
    }
}

void print_tlv(char* fmt, TlvInfo tlv) {
    printf("%s Tag: %x, Length: %ld, Value: ", fmt, tlv.tag, tlv.length);
    print_hex(tlv.value, tlv.length);
    printf("\n");
}

void test_iso7816_tlv_parse(const uint8_t* input, size_t input_size, uint16_t exp_tag, size_t exp_length) {
	TlvInfo tlv = iso7816_tlv_parse(input);

	if(tlv.tag != exp_tag) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse Tag for ");
		print_hex(input, input_size);
		printf(" is not %1$d (%1$x), but %2$d (%2$x)\n" COLOR_RESET,
                exp_tag, tlv.tag);
        return;
    }

	if(tlv.length != exp_length) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse Length for ");
		print_hex(input, input_size);
		printf(" is not %ld, but %ld\n" COLOR_RESET,
                exp_length, tlv.length);
        return;
    }

    printf(COLOR_GREEN "SUCCESS - iso7816_tlv_parse for ");
	print_hex(input, input_size);
	printf(" is tag:%d, length:%ld\n" COLOR_RESET, tlv.tag, tlv.length);
}

void test_iso7816_tlv_parse_ber(const uint8_t* input, size_t input_size, uint8_t exp_class, uint8_t exp_constructed, uint16_t exp_tag, size_t exp_length) {
	TlvInfo tlv = iso7816_tlv_parse(input);

	if(tlv.ber.class != exp_class) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse ber.class for ");
		print_hex(input, input_size);
		printf(" is not %d, but %d\n" COLOR_RESET,
                exp_class, tlv.ber.class);
        return;
    }

	if(tlv.ber.constructed != exp_constructed) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse ber.constructed for ");
		print_hex(input, input_size);
		printf(" is not %d, but %d\n" COLOR_RESET,
                exp_constructed, tlv.ber.constructed);
        return;
    }

	if(tlv.ber.tag != exp_tag) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse ber.tag for ");
		print_hex(input, input_size);
		printf(" is not %1$d (%1$x), but %2$d (%2$x)\n" COLOR_RESET,
                exp_tag, tlv.ber.tag);
        return;
    }

	if(tlv.length != exp_length) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_parse length for ");
		print_hex(input, input_size);
		printf(" is not %ld, but %ld\n" COLOR_RESET,
                exp_length, tlv.length);
        return;
    }

    printf(COLOR_GREEN "SUCCESS - iso7816_tlv_parse BER for ");
	print_hex(input, input_size);
	printf(" is class:%d, constructed:%d, tag:%d, length:%ld\n" COLOR_RESET, tlv.ber.class, tlv.ber.constructed, tlv.ber.tag, tlv.length);
}

//TODO: memcmp values above?

void test_iso7816_tlv_select(const uint8_t* input, size_t input_size, const uint16_t tags[], size_t num_tags, uint16_t exp_tag, uint8_t* exp_data, size_t exp_data_length) {
    TlvInfo tlv = iso7816_tlv_select(input, input_size, tags, num_tags);

    if(tlv.tag != exp_tag) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_select tag for ");
		print_hex(input, input_size);
		printf(" is not %d, but %d\n" COLOR_RESET,
                exp_tag, tlv.tag);
        return;
    }

    if(tlv.length != exp_data_length) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_select length for ");
		print_hex(input, input_size);
		printf(" is not %ld, but %ld\n" COLOR_RESET,
                exp_data_length, tlv.length);
        return;
    }

    if(memcmp(tlv.value, exp_data, tlv.length)) {
        printf(COLOR_RED "FAILED  - iso7816_tlv_select value for ");
		print_hex(input, input_size);
		printf(" is not\n");
        print_hex(exp_data, exp_data_length);
        printf(", but\n");
        print_hex(tlv.value, tlv.length);
        printf(COLOR_RESET "\n");
        return;
    }

    printf(COLOR_GREEN "SUCCESS - iso7816_tlv_select for ");
	print_hex(input, input_size);
	printf(" is tag:%d, length:%ld\n" COLOR_RESET, tlv.tag, tlv.length);
}

void describe_tlv(const uint8_t* data, size_t length, size_t level) {
    size_t offset = 0;
    char prefix[level+1];
    memset(prefix, ' ', level);
    prefix[level] = '\x00';

    printf("%sDescribe TLV (lvl: %ld), size: %ld\n", prefix, level, length);

    while(offset < length) {
        TlvInfo tlv = iso7816_tlv_parse(data + offset);

        printf("%sTag: %x (%d) (BER - class: %d, constr: %d, tag: %d)\n", prefix, tlv.tag, tlv.tag, tlv.ber.class, tlv.ber.constructed, tlv.ber.tag);
        printf("%sLength: %ld\n", prefix, tlv.length);
        printf("%sValue: ", prefix);
        print_hex(tlv.value, tlv.length);
        printf("\n");

        if(tlv.ber.constructed) {
            describe_tlv(tlv.value, tlv.length, level+1);
        }

        offset = tlv.next - data;
    }
}

int main(int argc, char** argv) {
	test_iso7816_tlv_parse("\x0F\x05\x48\x65\x6C\x6C\x6F", 7, 15, 5);
	test_iso7816_tlv_parse_ber("\x5F\x0F\x05\x48\x65\x6C\x6C\x6F", 8, BER_CLASS_APPLICATION, 0, 15, 5);
	test_iso7816_tlv_parse_ber("\x5F\x1F\x05\x48\x65\x6C\x6C\x6F", 8, BER_CLASS_APPLICATION, 0, 31, 5);
	test_iso7816_tlv_parse_ber("\x5F\x7F\x05\x48\x65\x6C\x6C\x6F", 8, BER_CLASS_APPLICATION, 0, 127, 5);
	test_iso7816_tlv_parse_ber("\x5F\x81\x00\x05\x48\x65\x6C\x6C\x6F", 9, BER_CLASS_APPLICATION, 0, 128, 5);
	test_iso7816_tlv_parse_ber("\x5F\xFF\x7F\x05\x48\x65\x6C\x6C\x6F", 9, BER_CLASS_APPLICATION, 0, 16383, 5);
	test_iso7816_tlv_parse("\x0F\xff\x00\x05\x48\x65\x6C\x6C\x6F", 9, 15, 5);
	test_iso7816_tlv_parse("\x04\xff\x01\x00\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65\x65", 260, 4, 256);
	test_iso7816_tlv_parse("\x4F\x81\x05\x48\x65\x6C\x6C\x6F", 8, 0x4f, 5);
	test_iso7816_tlv_parse("\x4F\x82\x00\x05\x48\x65\x6C\x6C\x6F", 9, 0x4f, 5);
	test_iso7816_tlv_parse("\x4F\x83\x00\x00\x05\x48\x65\x6C\x6C\x6F", 10, 0x4f, 5);
	test_iso7816_tlv_parse("\x4F\x84\x00\x00\x00\x05\x48\x65\x6C\x6C\x6F", 11, 0x4f, 5);
	test_iso7816_tlv_parse("\x4F\x85\x00\x00\x00\x00\x05\x48\x65\x6C\x6C\x6F", 12, 0, 0);
    test_iso7816_tlv_parse("\x61\x09\x4F\x07\xA0\x00\x00\x02\x47\x10\x01\x90\x00", 13, 97, 9);

    test_iso7816_tlv_parse_ber("\x8A\x02Hi", 4, BER_CLASS_CONTEXT, 0, 10, 2);
    test_iso7816_tlv_parse_ber("\x6A\x04\x8A\x02Hi", 6, BER_CLASS_APPLICATION, 1, 10, 4);
    test_iso7816_tlv_parse_ber("\xDF\x8A\x7F\x02Hi", 4, BER_CLASS_PRIVATE, 0, 0x57f, 2);

    printf("=====\nEF.DIR\n");
    const uint8_t *ef_dir_data = "\x61\x09\x4F\x07\xA0\x00\x00\x02\x47\x10\x01\x61\x09\x4F\x07\xA0\x00\x00\x02\x47\x20\x01";
    size_t ef_dir_data_len = 22;
    describe_tlv(ef_dir_data, ef_dir_data_len, 0);

    printf("=====\nEF.CardAccess\n");
    const uint8_t *ef_cardaccess_data = "\x31\x14\x30\x12\x06\x0A\x04\x00\x7F\x00\x07\x02\x02\x04\x02\x04\x02\x01\x02\x02\x01\x0E\x90\x00";
    size_t ef_cardaccess_data_len = 24;
    describe_tlv(ef_cardaccess_data, ef_cardaccess_data_len, 0);

    printf("=====\nEF.Com\n");
    const uint8_t *ef_com_data = "\x60\x16\x5F\x01\x04\x30\x31\x30\x37\x5F\x36\x06\x30\x34\x30\x30\x30\x30\x5C\x04\x61\x75\x6F\x6E";
    size_t ef_com_data_len = 24;
    describe_tlv(ef_com_data, ef_com_data_len, 0);

    uint16_t lds_tag_path[] = {0x60, 0x5f01};
    uint16_t unicode_tag_path[] = {0x60, 0x5f36};
    uint16_t tags_tag_path[] = {0x60, 0x5c};

    TlvInfo tlv_lds_version = iso7816_tlv_select(ef_com_data, ef_com_data_len, lds_tag_path, num_elements(lds_tag_path));
    if(tlv_lds_version.tag) {
        int tlv_version = tlv_number(tlv_lds_version);
        printf("LDS Version: %d.%d (%.4s)\n", tlv_version/100, tlv_version%100, tlv_lds_version.value);
    } else {
        printf("Error, LDS info not found!\n");
    }

    TlvInfo tlv_unicode_version = iso7816_tlv_select(ef_com_data, ef_com_data_len, unicode_tag_path, num_elements(unicode_tag_path));
    if(tlv_unicode_version.tag) {
        int unicode_version = tlv_number(tlv_unicode_version);
        printf("Unicode Version: %d.%d.%d (%.6s)\n", unicode_version/10000, unicode_version/100%100, unicode_version%100, tlv_unicode_version.value);
    } else {
        printf("Error, Unicode info not found!\n");
    }

    TlvInfo tlv_tag_list = iso7816_tlv_select(ef_com_data, ef_com_data_len, tags_tag_path, num_elements(tags_tag_path));
    if(tlv_tag_list.tag) {
        printf("Tag List:\n");
        for(size_t i=0; i<tlv_tag_list.length; ++i) {
            printf("- %02x\n", tlv_tag_list.value[i]);
        }
    } else {
        printf("Error, Tag List not found!\n");
    }

    printf("====\n");

    test_iso7816_tlv_select(ef_com_data, ef_com_data_len, (uint16_t[]){0x60, 0x5f36}, 2, 0x5f36, "\x30\x34\x30\x30\x30\x30", 6);


    //TlvInfo tlv = iso7816_tlv_select(ef_dir_data, ef_dir_data_len, (uint16_t[]){0x61, 0x4f}, 2);
    //print_tlv("4F-0:", tlv);
    //tlv = iso7816_tlv_select(tlv.next, tlv.next - ef_dir_data + ef_dir_data_len, (uint16_t[]){0x61, 0x4f}, 2);
    //print_tlv("4F-1:", tlv);

    TlvInfo tlv;
    const uint8_t* data = ef_dir_data;
    size_t len = ef_dir_data_len;
    for(uint8_t i=0;;++i) {
        tlv = iso7816_tlv_select(data, ef_dir_data - data + len, (uint16_t[]){0x61, 0x4f}, 2);
        if(!tlv.tag) break;
        printf("4F-%d", i);
        print_tlv(":", tlv);
        data = tlv.next;
    }

	return 0;
}
