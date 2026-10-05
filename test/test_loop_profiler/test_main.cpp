#include <string.h>
#include <unity.h>
#include "loop_profiler.h"

static LoopProfiler prof;

void setUp() { prof = LoopProfiler(); }
void tearDown() {}

void test_time_between_marks_goes_to_each_section() {
    prof.startPass(1000);       // microseconds
    prof.mark(PROF_CAN, 1300);  // 300 us of CAN
    prof.mark(PROF_UI, 1800);   // 500 us of drawing
    prof.startPass(2000);       // 200 us outside the marked sections
    prof.mark(PROF_CAN, 2100);
    TEST_ASSERT_EQUAL_UINT32(400, prof.totalUs(PROF_CAN));
    TEST_ASSERT_EQUAL_UINT32(500, prof.totalUs(PROF_UI));
    TEST_ASSERT_EQUAL_UINT32(200, prof.totalUs(PROF_OTHER));
    TEST_ASSERT_EQUAL_UINT32(1, prof.passes());  // the second is still running
}

void test_slowest_pass_is_tracked() {
    prof.startPass(0);
    prof.startPass(1000);
    prof.startPass(251000);  // a 250 ms pass
    prof.startPass(252000);
    TEST_ASSERT_EQUAL_UINT32(250000, prof.slowestPassUs());
}

void test_report_in_milliseconds_then_starts_over() {
    prof.startPass(0);
    prof.mark(PROF_GPS, 7000);
    prof.startPass(10000);
    char buf[160];
    prof.report(buf, sizeof(buf));
    TEST_ASSERT_NOT_NULL(strstr(buf, "passes=1"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "slowest=10ms"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "gps=7ms"));
    TEST_ASSERT_EQUAL_UINT32(0, prof.passes());
    TEST_ASSERT_EQUAL_UINT32(0, prof.totalUs(PROF_GPS));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_time_between_marks_goes_to_each_section);
    RUN_TEST(test_slowest_pass_is_tracked);
    RUN_TEST(test_report_in_milliseconds_then_starts_over);
    return UNITY_END();
}
