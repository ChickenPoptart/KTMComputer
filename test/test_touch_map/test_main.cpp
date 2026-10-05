#include <unity.h>
#include "touch_map.h"

void setUp() {}
void tearDown() {}

// Corner taps from the 2026-10-03 bench calibration (screen rotation 1)
void test_corner_taps_land_in_their_corners() {
    int x, y;
    touchToScreen(541, 458, x, y);    // top-left
    TEST_ASSERT_INT_WITHIN(8, 0, x);
    TEST_ASSERT_INT_WITHIN(8, 0, y);
    touchToScreen(539, 3586, x, y);   // top-right
    TEST_ASSERT_INT_WITHIN(8, 319, x);
    TEST_ASSERT_INT_WITHIN(8, 0, y);
    touchToScreen(3526, 3648, x, y);  // bottom-right
    TEST_ASSERT_INT_WITHIN(8, 319, x);
    TEST_ASSERT_INT_WITHIN(8, 239, y);
    touchToScreen(3470, 434, x, y);   // bottom-left
    TEST_ASSERT_INT_WITHIN(8, 0, x);
    TEST_ASSERT_INT_WITHIN(8, 239, y);
}

void test_center_tap_lands_near_the_center() {
    int x, y;
    touchToScreen(1839, 2027, x, y);
    TEST_ASSERT_INT_WITHIN(20, 160, x);
    TEST_ASSERT_INT_WITHIN(20, 120, y);
}

void test_readings_past_the_edges_stay_on_screen() {
    int x, y;
    touchToScreen(0, 0, x, y);
    TEST_ASSERT_EQUAL(0, x);
    TEST_ASSERT_EQUAL(0, y);
    touchToScreen(4095, 4095, x, y);
    TEST_ASSERT_EQUAL(319, x);
    TEST_ASSERT_EQUAL(239, y);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_corner_taps_land_in_their_corners);
    RUN_TEST(test_center_tap_lands_near_the_center);
    RUN_TEST(test_readings_past_the_edges_stay_on_screen);
    return UNITY_END();
}
