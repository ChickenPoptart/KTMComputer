#include <string.h>
#include <unity.h>
#include "candump_format.h"

void setUp() {}
void tearDown() {}

void test_standard_frame() {
    char line[CANDUMP_LINE_MAX];
    const uint8_t data[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xAB};
    int n = formatCandumpLine(line, sizeof(line), 1234567ULL, 0x120, false, 8, data);
    TEST_ASSERT_EQUAL_STRING("(1.234567) can0 120#00112233445566AB\n", line);
    TEST_ASSERT_EQUAL((int)strlen(line), n);
}

void test_extended_frame_uses_eight_hex_digits() {
    char line[CANDUMP_LINE_MAX];
    const uint8_t data[2] = {0x01, 0x02};
    formatCandumpLine(line, sizeof(line), 5000000ULL, 0x18FEF100, true, 2, data);
    TEST_ASSERT_EQUAL_STRING("(5.000000) can0 18FEF100#0102\n", line);
}

void test_empty_frame() {
    char line[CANDUMP_LINE_MAX];
    formatCandumpLine(line, sizeof(line), 42ULL, 0x7FF, false, 0, nullptr);
    TEST_ASSERT_EQUAL_STRING("(0.000042) can0 7FF#\n", line);
}

void test_timestamp_beyond_32_bit_microseconds() {
    // micros() wraps after ~71 minutes; long rides must not
    char line[CANDUMP_LINE_MAX];
    const uint8_t data[1] = {0xFF};
    formatCandumpLine(line, sizeof(line), 5000000000ULL, 0x100, false, 1, data);
    TEST_ASSERT_EQUAL_STRING("(5000.000000) can0 100#FF\n", line);
}

void test_oversized_length_is_clamped_to_eight() {
    char line[CANDUMP_LINE_MAX];
    const uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    formatCandumpLine(line, sizeof(line), 0, 0x1, false, 15, data);
    TEST_ASSERT_EQUAL_STRING("(0.000000) can0 001#0102030405060708\n", line);
}

void test_marker_line_uses_its_own_channel() {
    char line[CANDUMP_LINE_MAX];
    int n = formatMarkerLine(line, sizeof(line), 12500000ULL, 3);
    TEST_ASSERT_EQUAL_STRING("(12.500000) mark 000#0003\n", line);
    TEST_ASSERT_EQUAL((int)strlen(line), n);
}

void test_nmea_line_keeps_the_sentence_and_drops_its_carriage_return() {
    char line[NMEA_LINE_MAX];
    int n = formatNmeaLine(line, sizeof(line), 3250000ULL, "$GNGGA,123519,4807.038,N*47\r");
    TEST_ASSERT_EQUAL_STRING("(3.250000) nmea $GNGGA,123519,4807.038,N*47\n", line);
    TEST_ASSERT_EQUAL((int)strlen(line), n);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_nmea_line_keeps_the_sentence_and_drops_its_carriage_return);
    RUN_TEST(test_marker_line_uses_its_own_channel);
    RUN_TEST(test_standard_frame);
    RUN_TEST(test_extended_frame_uses_eight_hex_digits);
    RUN_TEST(test_empty_frame);
    RUN_TEST(test_timestamp_beyond_32_bit_microseconds);
    RUN_TEST(test_oversized_length_is_clamped_to_eight);
    return UNITY_END();
}
