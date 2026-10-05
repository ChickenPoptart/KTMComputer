#include <unity.h>
#include "can_guard.h"

static CanGuard guard;

void setUp() { guard = CanGuard(); }
void tearDown() {}

void test_healthy_bus_keeps_running() {
    // ~600 frames/s, no errors
    for (unsigned long t = 0; t <= 5000; t += 1000) {
        TEST_ASSERT_EQUAL(GUARD_NONE, guard.check(t, t * 0.6, 0));
    }
    TEST_ASSERT_FALSE(guard.faulted());
}

void test_error_flood_without_frames_stops_can() {
    guard.check(0, 0, 0);
    TEST_ASSERT_EQUAL(GUARD_STOP, guard.check(1000, 0, 5000));
    TEST_ASSERT_TRUE(guard.faulted());
}

void test_occasional_errors_on_a_busy_bus_are_tolerated() {
    guard.check(0, 0, 0);
    TEST_ASSERT_EQUAL(GUARD_NONE, guard.check(1000, 600, 20));
}

void test_restarts_after_the_pause_and_stops_again_if_still_bad() {
    guard.check(0, 0, 0);
    guard.check(1000, 0, 5000);  // stopped
    TEST_ASSERT_EQUAL(GUARD_NONE, guard.check(1000 + CAN_GUARD_PAUSE_MS - 1, 0, 5000));
    TEST_ASSERT_EQUAL(GUARD_RESTART, guard.check(1000 + CAN_GUARD_PAUSE_MS, 0, 5000));
    TEST_ASSERT_TRUE(guard.faulted());  // still suspect until proven healthy
    unsigned long t = 1000 + CAN_GUARD_PAUSE_MS;
    TEST_ASSERT_EQUAL(GUARD_STOP, guard.check(t + 1000, 0, 9000));
}

void test_fault_clears_once_frames_flow_after_a_restart() {
    guard.check(0, 0, 0);
    guard.check(1000, 0, 5000);
    unsigned long t = 1000 + CAN_GUARD_PAUSE_MS;
    guard.check(t, 0, 5000);  // restart
    TEST_ASSERT_EQUAL(GUARD_NONE, guard.check(t + 1000, 600, 5000));
    TEST_ASSERT_FALSE(guard.faulted());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_healthy_bus_keeps_running);
    RUN_TEST(test_error_flood_without_frames_stops_can);
    RUN_TEST(test_occasional_errors_on_a_busy_bus_are_tolerated);
    RUN_TEST(test_restarts_after_the_pause_and_stops_again_if_still_bad);
    RUN_TEST(test_fault_clears_once_frames_flow_after_a_restart);
    return UNITY_END();
}
