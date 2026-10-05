#include <unity.h>
#include "ktm_decode.h"

// Frame layouts from github.com/blalor/ktm-can (2020 KTM 690 Enduro R)

void setUp() {}
void tearDown() {}

void test_rpm_from_0x120_bytes_0_1_big_endian() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0x10, 0x10, 0x00, 0x00, 0x00, 0x30};  // 1500
    ktmDecodeFrame(s, 100, 0x120, false, 8, d);
    TEST_ASSERT_EQUAL(1500, s.rpm);
}

void test_rpm_near_redline() {
    KtmState s;
    const uint8_t d[8] = {0x23, 0x28, 0xFF, 0x10, 0x00, 0x00, 0x00, 0x50};  // 9000
    ktmDecodeFrame(s, 100, 0x120, false, 8, d);
    TEST_ASSERT_EQUAL(9000, s.rpm);
}

void test_other_ids_do_not_change_rpm() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 100, 0x12B, false, 8, d);  // wheel speeds
    TEST_ASSERT_EQUAL(0, s.rpm);
}

void test_extended_frame_with_same_id_is_ignored() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 100, 0x120, true, 8, d);
    TEST_ASSERT_EQUAL(0, s.rpm);
}

void test_frame_too_short_for_rpm_is_ignored() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 100, 0x120, false, 1, d);
    TEST_ASSERT_EQUAL(0, s.rpm);
}

void test_gear_from_0x129_high_nibble_of_byte_0() {
    KtmState s;
    const uint8_t third[8] = {0x30, 0, 0, 0, 0, 0, 0, 0x20};
    ktmDecodeFrame(s, 100, 0x129, false, 8, third);
    TEST_ASSERT_EQUAL(3, s.gear);
}

void test_coolant_from_0x540_bytes_6_7_tenths_celsius_as_fahrenheit() {
    KtmState s;
    const uint8_t d[8] = {0x02, 0x05, 0xDC, 0x00, 0x01, 0x00, 0x03, 0x52};  // 85.0 C
    ktmDecodeFrame(s, 100, 0x540, false, 8, d);
    TEST_ASSERT_EQUAL(185, s.coolantF);
}

void test_coolant_rounds_to_nearest_degree() {
    KtmState s;
    const uint8_t d[8] = {0x02, 0, 0, 0, 0, 0, 0x03, 0x52 - 3};  // 84.7 C = 184.46 F
    ktmDecodeFrame(s, 100, 0x540, false, 8, d);
    TEST_ASSERT_EQUAL(184, s.coolantF);
    const uint8_t up[8] = {0x02, 0, 0, 0, 0, 0, 0x03, 0x52 - 2};  // 84.8 C = 184.64 F
    ktmDecodeFrame(s, 200, 0x540, false, 8, up);
    TEST_ASSERT_EQUAL(185, s.coolantF);
}

void test_frame_too_short_for_coolant_is_ignored() {
    KtmState s;
    const uint8_t d[8] = {0x02, 0, 0, 0, 0, 0, 0x03, 0x52};
    ktmDecodeFrame(s, 100, 0x540, false, 4, d);
    TEST_ASSERT_EQUAL(0, s.coolantF);
}

void test_rpm_is_not_live_before_any_frame() {
    KtmState s;
    TEST_ASSERT_FALSE(ktmRpmLive(s, 0));
}

void test_rpm_is_live_just_after_a_frame() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 5000, 0x120, false, 8, d);
    TEST_ASSERT_TRUE(ktmRpmLive(s, 5000));
}

void test_rpm_goes_stale_when_frames_stop() {
    KtmState s;
    const uint8_t d[8] = {0x05, 0xDC, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 5000, 0x120, false, 8, d);
    TEST_ASSERT_TRUE(ktmRpmLive(s, 5000 + KTM_STALE_MS));
    TEST_ASSERT_FALSE(ktmRpmLive(s, 5000 + KTM_STALE_MS + 1));
}

void test_gear_is_live_only_while_0x129_frames_arrive() {
    KtmState s;
    TEST_ASSERT_FALSE(ktmGearLive(s, 0));
    const uint8_t d[8] = {0x20, 0, 0, 0, 0, 0, 0, 0};
    ktmDecodeFrame(s, 5000, 0x129, false, 8, d);
    TEST_ASSERT_TRUE(ktmGearLive(s, 5000 + KTM_STALE_MS));
    TEST_ASSERT_FALSE(ktmGearLive(s, 5000 + KTM_STALE_MS + 1));
}

void test_coolant_is_live_only_while_0x540_frames_arrive() {
    KtmState s;
    TEST_ASSERT_FALSE(ktmCoolantLive(s, 0));
    const uint8_t d[8] = {0x02, 0, 0, 0, 0, 0, 0x03, 0x52};
    ktmDecodeFrame(s, 5000, 0x540, false, 8, d);
    TEST_ASSERT_TRUE(ktmCoolantLive(s, 5000 + KTM_STALE_MS));
    TEST_ASSERT_FALSE(ktmCoolantLive(s, 5000 + KTM_STALE_MS + 1));
}

void test_speed_from_0x12B_front_wheel() {
    // 2026-10-01 ride: front read 1,095 at about 40 mph on the bike's speedo
    KtmState s;
    const uint8_t d[8] = {0x04, 0x47, 0x04, 0x62, 0, 0, 0, 0};  // front 1095, rear 1122
    ktmDecodeFrame(s, 100, 0x12B, false, 8, d);
    TEST_ASSERT_EQUAL(40, s.speedMph);
}

void test_speed_at_22_mph_run() {
    KtmState s;
    const uint8_t d[8] = {0x02, 0x5A, 0x02, 0x67, 0, 0, 0, 0};  // front 602, rear 615
    ktmDecodeFrame(s, 100, 0x12B, false, 8, d);
    TEST_ASSERT_EQUAL(22, s.speedMph);
}

void test_speed_is_live_only_while_0x12B_frames_arrive() {
    KtmState s;
    TEST_ASSERT_FALSE(ktmSpeedLive(s, 0));
    const uint8_t d[8] = {0x02, 0x5A, 0x02, 0x67, 0, 0, 0, 0};
    ktmDecodeFrame(s, 5000, 0x12B, false, 8, d);
    TEST_ASSERT_TRUE(ktmSpeedLive(s, 5000 + KTM_STALE_MS));
    TEST_ASSERT_FALSE(ktmSpeedLive(s, 5000 + KTM_STALE_MS + 1));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_speed_at_22_mph_run);
    RUN_TEST(test_speed_is_live_only_while_0x12B_frames_arrive);
    RUN_TEST(test_speed_from_0x12B_front_wheel);
    RUN_TEST(test_coolant_is_live_only_while_0x540_frames_arrive);
    RUN_TEST(test_gear_is_live_only_while_0x129_frames_arrive);
    RUN_TEST(test_rpm_goes_stale_when_frames_stop);
    RUN_TEST(test_rpm_is_live_just_after_a_frame);
    RUN_TEST(test_rpm_is_not_live_before_any_frame);
    RUN_TEST(test_frame_too_short_for_coolant_is_ignored);
    RUN_TEST(test_coolant_rounds_to_nearest_degree);
    RUN_TEST(test_coolant_from_0x540_bytes_6_7_tenths_celsius_as_fahrenheit);
    RUN_TEST(test_gear_from_0x129_high_nibble_of_byte_0);
    RUN_TEST(test_frame_too_short_for_rpm_is_ignored);
    RUN_TEST(test_extended_frame_with_same_id_is_ignored);
    RUN_TEST(test_other_ids_do_not_change_rpm);
    RUN_TEST(test_rpm_from_0x120_bytes_0_1_big_endian);
    RUN_TEST(test_rpm_near_redline);
    return UNITY_END();
}
