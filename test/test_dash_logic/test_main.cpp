#include <unity.h>
#include "dash_logic.h"

void setUp() {}
void tearDown() {}

void test_tach_is_dark_with_engine_off() {
    TEST_ASSERT_EQUAL(0, tachLitSegments(0));
}

void test_idle_lights_the_first_segment() {
    TEST_ASSERT_EQUAL(1, tachLitSegments(1700));  // this bike's warm-up idle
}

void test_green_zone_starts_at_3000() {
    TEST_ASSERT_EQUAL(1, tachLitSegments(2999));
    TEST_ASSERT_EQUAL(2, tachLitSegments(3000));
}

void test_yellow_zone_starts_at_6500() {
    TEST_ASSERT_EQUAL(5, tachLitSegments(6499));
    TEST_ASSERT_EQUAL(6, tachLitSegments(6500));
}

void test_red_segment_lights_with_the_shift_light() {
    TEST_ASSERT_EQUAL(7, tachLitSegments(SHIFT_RPM - 1));
    TEST_ASSERT_EQUAL(TACH_SEGMENTS, tachLitSegments(SHIFT_RPM));
    TEST_ASSERT_EQUAL(TACH_SEGMENTS, tachLitSegments(9500));  // past the limiter
}

void test_rpm_zone_matches_the_tach_bar_colours() {
    TEST_ASSERT_EQUAL(ZONE_OFF, tachZone(0));
    TEST_ASSERT_EQUAL(ZONE_IDLE, tachZone(1700));
    TEST_ASSERT_EQUAL(ZONE_IDLE, tachZone(2999));
    TEST_ASSERT_EQUAL(ZONE_GREEN, tachZone(3000));
    TEST_ASSERT_EQUAL(ZONE_GREEN, tachZone(6499));
    TEST_ASSERT_EQUAL(ZONE_YELLOW, tachZone(6500));
    TEST_ASSERT_EQUAL(ZONE_YELLOW, tachZone(SHIFT_RPM - 1));
    TEST_ASSERT_EQUAL(ZONE_RED, tachZone(SHIFT_RPM));
}

void test_durations_read_like_a_clock() {
    char buf[16];
    formatDuration(buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_STRING("0:00", buf);
    formatDuration(buf, sizeof(buf), (12 * 60 + 34) * 1000UL + 999);
    TEST_ASSERT_EQUAL_STRING("12:34", buf);
    formatDuration(buf, sizeof(buf), (3600 + 5 * 60 + 9) * 1000UL);
    TEST_ASSERT_EQUAL_STRING("1:05:09", buf);
}

void test_shift_triggers_at_threshold() {
    TEST_ASSERT_FALSE(shiftActive(SHIFT_RPM - 1));
    TEST_ASSERT_TRUE(shiftActive(SHIFT_RPM));
}

void test_gear_labels() {
    TEST_ASSERT_EQUAL_STRING("N", gearLabel(0));
    TEST_ASSERT_EQUAL_STRING("1", gearLabel(1));
    TEST_ASSERT_EQUAL_STRING("6", gearLabel(6));
}

void test_unknown_gear_shows_dash() {
    TEST_ASSERT_EQUAL_STRING("-", gearLabel(7));
    TEST_ASSERT_EQUAL_STRING("-", gearLabel(-1));
}

void test_heading_cardinals() {
    TEST_ASSERT_EQUAL_STRING("N", headingCardinal(0));
    TEST_ASSERT_EQUAL_STRING("N", headingCardinal(359));
    TEST_ASSERT_EQUAL_STRING("NE", headingCardinal(45));
    TEST_ASSERT_EQUAL_STRING("E", headingCardinal(90));
    TEST_ASSERT_EQUAL_STRING("S", headingCardinal(180));
    TEST_ASSERT_EQUAL_STRING("NW", headingCardinal(315));
}

void test_heading_rounds_to_nearest_cardinal() {
    TEST_ASSERT_EQUAL_STRING("N", headingCardinal(22));
    TEST_ASSERT_EQUAL_STRING("NE", headingCardinal(23));
    TEST_ASSERT_EQUAL_STRING("N", headingCardinal(338));
}

void test_heading_normalizes_out_of_range() {
    TEST_ASSERT_EQUAL_STRING("E", headingCardinal(450));
    TEST_ASSERT_EQUAL_STRING("W", headingCardinal(-90));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_tach_is_dark_with_engine_off);
    RUN_TEST(test_idle_lights_the_first_segment);
    RUN_TEST(test_green_zone_starts_at_3000);
    RUN_TEST(test_yellow_zone_starts_at_6500);
    RUN_TEST(test_red_segment_lights_with_the_shift_light);
    RUN_TEST(test_rpm_zone_matches_the_tach_bar_colours);
    RUN_TEST(test_durations_read_like_a_clock);
    RUN_TEST(test_shift_triggers_at_threshold);
    RUN_TEST(test_gear_labels);
    RUN_TEST(test_unknown_gear_shows_dash);
    RUN_TEST(test_heading_cardinals);
    RUN_TEST(test_heading_rounds_to_nearest_cardinal);
    RUN_TEST(test_heading_normalizes_out_of_range);
    return UNITY_END();
}
