#include <unity.h>
#include "chart_series.h"

void setUp() {}
void tearDown() {}

void test_samples_become_points_and_gaps_stay_gaps() {
    ChartSeries<8> c;
    c.add(5000);
    c.add(CHART_NO_DATA);
    c.add(5100);
    TEST_ASSERT_EQUAL(3, c.size());
    TEST_ASSERT_EQUAL(5000, c.at(0));
    TEST_ASSERT_EQUAL(CHART_NO_DATA, c.at(1));
    TEST_ASSERT_EQUAL(5100, c.at(2));
}

void test_full_chart_merges_pairs_and_keeps_the_whole_ride() {
    ChartSeries<4> c;
    c.add(10); c.add(20); c.add(30); c.add(40);  // full
    TEST_ASSERT_EQUAL(1, c.samplesPerPoint());
    c.add(50);
    // Merged to 15, 35; then the new sample starts a 2-sample point
    TEST_ASSERT_EQUAL(2, c.samplesPerPoint());
    TEST_ASSERT_EQUAL(2, c.size());
    TEST_ASSERT_EQUAL(15, c.at(0));
    TEST_ASSERT_EQUAL(35, c.at(1));
    c.add(70);  // completes the point: average of 50, 70
    TEST_ASSERT_EQUAL(3, c.size());
    TEST_ASSERT_EQUAL(60, c.at(2));
}

void test_merging_a_gap_with_data_keeps_the_data() {
    ChartSeries<2> c;
    c.add(CHART_NO_DATA); c.add(100);
    c.add(CHART_NO_DATA);  // forces a merge
    TEST_ASSERT_EQUAL(100, c.at(0));
    c.add(CHART_NO_DATA);  // a point made only of gaps stays a gap
    TEST_ASSERT_EQUAL(CHART_NO_DATA, c.at(1));
}

void test_long_ride_never_overflows_and_keeps_a_steady_value() {
    ChartSeries<8> c;
    for (long i = 0; i < 100000; i++) {
        c.add(4200);
        TEST_ASSERT_TRUE(c.size() <= 8);
    }
    TEST_ASSERT_TRUE(c.size() >= 4);
    for (int i = 0; i < c.size(); i++) TEST_ASSERT_EQUAL(4200, c.at(i));
}

void test_range_ignores_gaps() {
    ChartSeries<8> c;
    int16_t lo, hi;
    TEST_ASSERT_FALSE(c.range(lo, hi));  // empty
    c.add(5200); c.add(CHART_NO_DATA); c.add(4900); c.add(5050);
    TEST_ASSERT_TRUE(c.range(lo, hi));
    TEST_ASSERT_EQUAL(4900, lo);
    TEST_ASSERT_EQUAL(5200, hi);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_long_ride_never_overflows_and_keeps_a_steady_value);
    RUN_TEST(test_range_ignores_gaps);
    RUN_TEST(test_full_chart_merges_pairs_and_keeps_the_whole_ride);
    RUN_TEST(test_merging_a_gap_with_data_keeps_the_data);
    RUN_TEST(test_samples_become_points_and_gaps_stay_gaps);
    return UNITY_END();
}
