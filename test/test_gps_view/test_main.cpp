#include <unity.h>
#include "gps_view.h"

void setUp() {}
void tearDown() {}

static GpsReading moving() {
    GpsReading r;
    r.locationValid = true;
    r.ageMs = 200;
    r.satellites = 9;
    r.courseDeg = 87.6;
    r.speedMph = 24.6;
    r.altitudeValid = true;
    r.altitudeFt = 5280.4;
    return r;
}

void test_no_location_is_no_fix() {
    GpsView v;
    GpsReading r;  // nothing valid yet
    gpsUpdateView(v, r);
    TEST_ASSERT_FALSE(v.fix);
}

void test_fresh_location_is_a_fix_with_rounded_values() {
    GpsView v;
    gpsUpdateView(v, moving());
    TEST_ASSERT_TRUE(v.fix);
    TEST_ASSERT_EQUAL(9, v.satellites);
    TEST_ASSERT_EQUAL(25, v.speedMph);
    TEST_ASSERT_EQUAL(5280, v.elevationFt);
}

void test_stale_location_is_no_fix() {
    GpsView v;
    GpsReading r = moving();
    r.ageMs = GPS_STALE_MS;
    gpsUpdateView(v, r);
    TEST_ASSERT_TRUE(v.fix);
    r.ageMs = GPS_STALE_MS + 1;
    gpsUpdateView(v, r);
    TEST_ASSERT_FALSE(v.fix);
}

void test_heading_follows_course_while_moving() {
    GpsView v;
    gpsUpdateView(v, moving());
    TEST_ASSERT_EQUAL(88, v.headingDeg);
}

void test_heading_holds_when_stopped() {
    // GPS course is noise at a standstill
    GpsView v;
    gpsUpdateView(v, moving());
    GpsReading stopped = moving();
    stopped.speedMph = GPS_HEADING_MIN_MPH - 0.1;
    stopped.courseDeg = 213;
    gpsUpdateView(v, stopped);
    TEST_ASSERT_EQUAL(88, v.headingDeg);
}

void test_heading_just_below_north_rounds_to_zero() {
    GpsView v;
    GpsReading r = moving();
    r.courseDeg = 359.7;
    gpsUpdateView(v, r);
    TEST_ASSERT_EQUAL(0, v.headingDeg);
}

void test_elevation_waits_for_valid_altitude() {
    GpsView v;
    gpsUpdateView(v, moving());
    GpsReading noAlt = moving();
    noAlt.altitudeValid = false;
    noAlt.altitudeFt = 0;
    gpsUpdateView(v, noAlt);
    TEST_ASSERT_EQUAL(5280, v.elevationFt);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_heading_just_below_north_rounds_to_zero);
    RUN_TEST(test_elevation_waits_for_valid_altitude);
    RUN_TEST(test_heading_follows_course_while_moving);
    RUN_TEST(test_heading_holds_when_stopped);
    RUN_TEST(test_stale_location_is_no_fix);
    RUN_TEST(test_fresh_location_is_a_fix_with_rounded_values);
    RUN_TEST(test_no_location_is_no_fix);
    return UNITY_END();
}
