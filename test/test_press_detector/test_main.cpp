#include <unity.h>
#include "press_detector.h"

static PressDetector press;

void setUp() { press = PressDetector(); }
void tearDown() {}

void test_no_touch_is_nothing() {
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 0));
}

void test_short_press_is_a_tap_once_released() {
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 1000));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 1200));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 1250));
    TEST_ASSERT_EQUAL(PRESS_TAP, press.update(false, 1250 + PressDetector::RELEASE_MS));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 2000));
}

void test_brief_dropout_while_pressed_is_not_a_tap() {
    press.update(true, 1000);
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 1040));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 1080));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 1100));
}

void test_holding_past_long_press_time_is_a_long_press_while_held() {
    press.update(true, 1000);
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 1000 + PressDetector::LONG_MS - 1));
    TEST_ASSERT_EQUAL(PRESS_LONG, press.update(true, 1000 + PressDetector::LONG_MS));
}

void test_long_press_reports_once_and_its_release_is_not_a_tap() {
    press.update(true, 1000);
    TEST_ASSERT_EQUAL(PRESS_LONG, press.update(true, 1000 + PressDetector::LONG_MS));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(true, 3000));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 3100));
    TEST_ASSERT_EQUAL(PRESS_NONE, press.update(false, 3100 + PressDetector::RELEASE_MS));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_long_press_reports_once_and_its_release_is_not_a_tap);
    RUN_TEST(test_holding_past_long_press_time_is_a_long_press_while_held);
    RUN_TEST(test_brief_dropout_while_pressed_is_not_a_tap);
    RUN_TEST(test_short_press_is_a_tap_once_released);
    RUN_TEST(test_no_touch_is_nothing);
    return UNITY_END();
}
