#include <unity.h>
#include "trip_stats.h"
#include "dash_logic.h"

static TripStats trip;

void setUp() { trip = TripStats(); }
void tearDown() {}

static TripSample cruising(int mph) {
    TripSample s;
    s.speedLive = true;
    s.speedMph = mph;
    s.rpmLive = true;
    s.rpm = 4000;
    return s;
}

void test_distance_is_speed_times_time() {
    // 30 mph for 2 minutes = 1 mile
    for (int i = 0; i < 120; i++) trip.update(1000, cruising(30));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, trip.miles());
    TEST_ASSERT_EQUAL_UINT32(120000, trip.elapsedMs());
}

void test_moving_time_skips_stops() {
    for (int i = 0; i < 60; i++) trip.update(1000, cruising(25));
    for (int i = 0; i < 30; i++) trip.update(1000, cruising(0));  // stopped, idling
    TEST_ASSERT_EQUAL_UINT32(90000, trip.elapsedMs());
    TEST_ASSERT_EQUAL_UINT32(60000, trip.movingMs());
}

void test_maximums_ignore_values_that_are_not_live() {
    TripSample s = cruising(45);
    s.rpm = 7200;
    s.coolantLive = true;
    s.coolantF = 205;
    trip.update(1000, s);
    TripSample stale = cruising(99);
    stale.speedLive = false;
    stale.rpmLive = false;
    stale.rpm = 9999;
    trip.update(1000, stale);
    TEST_ASSERT_EQUAL(45, trip.maxSpeedMph());
    TEST_ASSERT_EQUAL(7200, trip.maxRpm());
    TEST_ASSERT_EQUAL(205, trip.maxCoolantF());
}

static TripSample atElevation(int ft) {
    TripSample s = cruising(20);
    s.elevationValid = true;
    s.elevationFt = ft;
    return s;
}

void test_elevation_gain_and_loss() {
    const int path[] = {5000, 5100, 5300, 5250, 5100};  // up 300, down 200
    for (int ft : path) trip.update(1000, atElevation(ft));
    TEST_ASSERT_EQUAL(300, trip.gainFt());
    TEST_ASSERT_EQUAL(200, trip.lossFt());
}

void test_gps_jitter_is_not_counted_as_climbing() {
    // Parked, GPS altitude wandering +/- 15 ft
    const int jitter[] = {5000, 5012, 4990, 5015, 4988, 5010, 4995, 5014};
    for (int i = 0; i < 20; i++)
        for (int ft : jitter) trip.update(1000, atElevation(ft));
    TEST_ASSERT_EQUAL(0, trip.gainFt());
    TEST_ASSERT_EQUAL(0, trip.lossFt());
}

void test_elevation_ignored_without_a_fix() {
    trip.update(1000, atElevation(5000));
    TripSample noFix = atElevation(0);
    noFix.elevationValid = false;
    trip.update(1000, noFix);
    trip.update(1000, atElevation(5000));
    TEST_ASSERT_EQUAL(0, trip.lossFt());
}

static TripSample atRpm(int rpm) {
    TripSample s = cruising(20);
    s.rpm = rpm;
    return s;
}

void test_time_in_each_rpm_zone() {
    for (int i = 0; i < 10; i++) trip.update(1000, atRpm(1700));  // idle
    for (int i = 0; i < 30; i++) trip.update(1000, atRpm(4500));  // green
    for (int i = 0; i < 5; i++) trip.update(1000, atRpm(7000));   // yellow
    trip.update(1000, atRpm(8500));                               // red
    TripSample off = atRpm(0);
    off.rpmLive = false;
    trip.update(1000, off);                                       // no data
    TEST_ASSERT_EQUAL_UINT32(10000, trip.zoneMs(ZONE_IDLE));
    TEST_ASSERT_EQUAL_UINT32(30000, trip.zoneMs(ZONE_GREEN));
    TEST_ASSERT_EQUAL_UINT32(5000, trip.zoneMs(ZONE_YELLOW));
    TEST_ASSERT_EQUAL_UINT32(1000, trip.zoneMs(ZONE_RED));
    TEST_ASSERT_EQUAL_UINT32(0, trip.zoneMs(ZONE_OFF));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_time_in_each_rpm_zone);
    RUN_TEST(test_elevation_gain_and_loss);
    RUN_TEST(test_gps_jitter_is_not_counted_as_climbing);
    RUN_TEST(test_elevation_ignored_without_a_fix);
    RUN_TEST(test_moving_time_skips_stops);
    RUN_TEST(test_maximums_ignore_values_that_are_not_live);
    RUN_TEST(test_distance_is_speed_times_time);
    return UNITY_END();
}
