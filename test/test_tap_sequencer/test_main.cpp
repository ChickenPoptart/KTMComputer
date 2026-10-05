#include <unity.h>
#include "tap_sequencer.h"

static TapSequencer taps;

void setUp() { taps = TapSequencer(); }
void tearDown() {}

void test_single_tap_reported_after_the_double_tap_window_with_its_position() {
    taps.tap(1000, 30, 220);
    TEST_ASSERT_EQUAL(TAP_NONE, taps.poll(1000 + DOUBLE_TAP_MS - 1).kind);
    TapResult r = taps.poll(1000 + DOUBLE_TAP_MS);
    TEST_ASSERT_EQUAL(TAP_SINGLE, r.kind);
    TEST_ASSERT_EQUAL(30, r.x);
    TEST_ASSERT_EQUAL(220, r.y);
    TEST_ASSERT_EQUAL(TAP_NONE, taps.poll(5000).kind);  // reported once
}

void test_two_quick_taps_are_a_double_tap_and_no_single() {
    taps.tap(1000, 160, 120);
    taps.tap(1000 + DOUBLE_TAP_MS - 1, 165, 125);
    TEST_ASSERT_EQUAL(TAP_DOUBLE, taps.poll(1000 + DOUBLE_TAP_MS - 1).kind);
    TEST_ASSERT_EQUAL(TAP_NONE, taps.poll(5000).kind);
}

void test_slow_taps_are_two_singles() {
    taps.tap(1000, 10, 10);
    TEST_ASSERT_EQUAL(TAP_SINGLE, taps.poll(1000 + DOUBLE_TAP_MS).kind);
    taps.tap(2000, 300, 200);
    TapResult r = taps.poll(2000 + DOUBLE_TAP_MS);
    TEST_ASSERT_EQUAL(TAP_SINGLE, r.kind);
    TEST_ASSERT_EQUAL(300, r.x);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_single_tap_reported_after_the_double_tap_window_with_its_position);
    RUN_TEST(test_two_quick_taps_are_a_double_tap_and_no_single);
    RUN_TEST(test_slow_taps_are_two_singles);
    return UNITY_END();
}
