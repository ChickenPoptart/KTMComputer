#include <unity.h>
#include "ride_history.h"
#include "trip_stats.h"

void setUp() {}
void tearDown() {}

void test_ride_file_names() {
    TEST_ASSERT_EQUAL(12, rideNumberFromName("ride_0012.bin"));
    TEST_ASSERT_EQUAL(12, rideNumberFromName("/rides/ride_0012.bin"));
    TEST_ASSERT_EQUAL(-1, rideNumberFromName("can_0012.log"));
    TEST_ASSERT_EQUAL(-1, rideNumberFromName("ride_0012.log"));
}

static const int SAVED[] = {9, 12, 10};  // any order

void test_starts_on_this_ride() {
    RideBrowser b;
    b.setRides(SAVED, 3, 14);
    TEST_ASSERT_TRUE(b.viewingLive());
}

void test_back_steps_through_older_rides_and_stops_at_the_oldest() {
    RideBrowser b;
    b.setRides(SAVED, 3, 14);
    TEST_ASSERT_TRUE(b.older());
    TEST_ASSERT_EQUAL(12, b.viewing());
    TEST_ASSERT_TRUE(b.older());
    TEST_ASSERT_EQUAL(10, b.viewing());
    TEST_ASSERT_TRUE(b.older());
    TEST_ASSERT_EQUAL(9, b.viewing());
    TEST_ASSERT_FALSE(b.older());  // nothing older
    TEST_ASSERT_EQUAL(9, b.viewing());
}

void test_forward_returns_to_this_ride_and_stops_there() {
    RideBrowser b;
    b.setRides(SAVED, 3, 14);
    b.older();
    b.older();  // ride 10
    TEST_ASSERT_TRUE(b.newer());
    TEST_ASSERT_EQUAL(12, b.viewing());
    TEST_ASSERT_TRUE(b.newer());
    TEST_ASSERT_TRUE(b.viewingLive());
    TEST_ASSERT_FALSE(b.newer());
}

void test_this_rides_own_file_is_not_listed_as_a_past_ride() {
    const int withCurrent[] = {12, 14};
    RideBrowser b;
    b.setRides(withCurrent, 2, 14);
    b.older();
    TEST_ASSERT_EQUAL(12, b.viewing());
    TEST_ASSERT_FALSE(b.older());
}

void test_learns_about_rides_saved_later() {
    RideBrowser b;
    b.setRides(nullptr, 0, 14);
    TEST_ASSERT_FALSE(b.older());
    b.addRide(13);
    TEST_ASSERT_TRUE(b.older());
    TEST_ASSERT_EQUAL(13, b.viewing());
}

static TripSample running(int mph) {
    TripSample s;
    s.speedLive = true;
    s.speedMph = mph;
    s.rpmLive = true;
    s.rpm = 1700;
    return s;
}

void test_bench_power_ups_are_not_saved_as_rides() {
    TripStats bench;
    for (int i = 0; i < 600; i++) bench.update(1000, TripSample());  // 10 min, no bike data
    TEST_ASSERT_FALSE(worthSaving(bench));
}

void test_a_brief_key_on_is_not_a_ride() {
    TripStats brief;
    for (int i = 0; i < 30; i++) brief.update(1000, running(0));  // 30 s idle
    TEST_ASSERT_FALSE(worthSaving(brief));
}

void test_running_for_a_minute_is_a_ride() {
    TripStats warmUp;
    for (int i = 0; i < RIDE_MIN_SECONDS; i++) warmUp.update(1000, running(0));
    TEST_ASSERT_TRUE(worthSaving(warmUp));
}

void test_tells_whether_the_buttons_can_go_anywhere() {
    RideBrowser b;
    b.setRides(SAVED, 3, 14);
    TEST_ASSERT_TRUE(b.hasOlder());
    TEST_ASSERT_FALSE(b.hasNewer());
    b.older(); b.older(); b.older();  // oldest
    TEST_ASSERT_FALSE(b.hasOlder());
    TEST_ASSERT_TRUE(b.hasNewer());
    RideBrowser empty;
    empty.setRides(nullptr, 0, 1);
    TEST_ASSERT_FALSE(empty.hasOlder());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_tells_whether_the_buttons_can_go_anywhere);
    RUN_TEST(test_bench_power_ups_are_not_saved_as_rides);
    RUN_TEST(test_a_brief_key_on_is_not_a_ride);
    RUN_TEST(test_running_for_a_minute_is_a_ride);
    RUN_TEST(test_ride_file_names);
    RUN_TEST(test_starts_on_this_ride);
    RUN_TEST(test_back_steps_through_older_rides_and_stops_at_the_oldest);
    RUN_TEST(test_forward_returns_to_this_ride_and_stops_there);
    RUN_TEST(test_this_rides_own_file_is_not_listed_as_a_past_ride);
    RUN_TEST(test_learns_about_rides_saved_later);
    return UNITY_END();
}
