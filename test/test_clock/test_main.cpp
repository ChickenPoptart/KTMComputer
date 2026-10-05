#include <stdlib.h>
#include <time.h>
#include <unity.h>
#include "ride_clock.h"

void setUp() {
    setenv("TZ", MOUNTAIN_TZ, 1);
    tzset();
}
void tearDown() {}

void test_gps_utc_date_and_time_to_epoch() {
    TEST_ASSERT_EQUAL_INT64(1791008378LL, (long long)utcEpoch(2026, 10, 3, 6, 19, 38));
    TEST_ASSERT_EQUAL_INT64(951782400LL, (long long)utcEpoch(2000, 2, 29, 0, 0, 0));  // leap day
}

void test_summer_time_is_mdt() {
    char buf[16];
    formatClock(buf, sizeof(buf), utcEpoch(2026, 10, 3, 6, 19, 38));  // 00:19 MDT
    TEST_ASSERT_EQUAL_STRING("12:19 AM", buf);
    formatClock(buf, sizeof(buf), utcEpoch(2026, 7, 4, 21, 5, 0));    // 15:05 MDT
    TEST_ASSERT_EQUAL_STRING("3:05 PM", buf);
}

void test_winter_time_is_mst() {
    char buf[16];
    formatClock(buf, sizeof(buf), utcEpoch(2026, 1, 15, 19, 5, 0));   // 12:05 MST
    TEST_ASSERT_EQUAL_STRING("12:05 PM", buf);
}

void test_daylight_saving_starts_on_the_second_sunday_of_march() {
    // 2026-03-08 02:00 MST = 09:00 UTC; clocks jump to 03:00 MDT
    char buf[16];
    formatClock(buf, sizeof(buf), utcEpoch(2026, 3, 8, 8, 59, 0));
    TEST_ASSERT_EQUAL_STRING("1:59 AM", buf);
    formatClock(buf, sizeof(buf), utcEpoch(2026, 3, 8, 9, 0, 0));
    TEST_ASSERT_EQUAL_STRING("3:00 AM", buf);
}

void test_date_for_the_trip_page() {
    char buf[16];
    formatDate(buf, sizeof(buf), utcEpoch(2026, 10, 3, 6, 19, 38));
    TEST_ASSERT_EQUAL_STRING("Sat Oct 3", buf);
    // Still Friday evening in Utah when it's already Saturday in UTC
    formatDate(buf, sizeof(buf), utcEpoch(2026, 10, 3, 2, 0, 0));
    TEST_ASSERT_EQUAL_STRING("Fri Oct 2", buf);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_summer_time_is_mdt);
    RUN_TEST(test_winter_time_is_mst);
    RUN_TEST(test_daylight_saving_starts_on_the_second_sunday_of_march);
    RUN_TEST(test_date_for_the_trip_page);
    RUN_TEST(test_gps_utc_date_and_time_to_epoch);
    return UNITY_END();
}
