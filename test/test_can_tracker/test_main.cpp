#include <unity.h>
#include "can_tracker.h"

static CanTracker tracker;

void setUp() { tracker = CanTracker(); }
void tearDown() {}

static const uint8_t ZEROS[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void test_first_frame_creates_entry() {
    TEST_ASSERT_TRUE(tracker.record(0x120, false, 8, ZEROS, 100));
    TEST_ASSERT_EQUAL(1, tracker.size());
    TEST_ASSERT_EQUAL_HEX32(0x120, tracker.at(0).id);
    TEST_ASSERT_EQUAL(1, tracker.at(0).count);
}

void test_repeated_identical_frame_is_not_a_change() {
    tracker.record(0x120, false, 8, ZEROS, 100);
    TEST_ASSERT_FALSE(tracker.record(0x120, false, 8, ZEROS, 110));
    TEST_ASSERT_EQUAL(2, tracker.at(0).count);
    TEST_ASSERT_EQUAL_HEX8(0x00, tracker.at(0).everChangedMask);
}

void test_changed_byte_is_flagged() {
    uint8_t data[8] = {0, 0, 0x42, 0, 0, 0, 0, 0};
    tracker.record(0x120, false, 8, ZEROS, 100);
    TEST_ASSERT_TRUE(tracker.record(0x120, false, 8, data, 150));
    const CanEntry& e = tracker.at(0);
    TEST_ASSERT_EQUAL_HEX8(0x04, e.everChangedMask);
    TEST_ASSERT_EQUAL_HEX8(0x42, e.data[2]);
    TEST_ASSERT_TRUE(e.byteRecentlyChanged(2, 150 + 500, 1000));
    TEST_ASSERT_FALSE(e.byteRecentlyChanged(2, 150 + 1500, 1000));
    TEST_ASSERT_FALSE(e.byteRecentlyChanged(0, 150, 1000));
}

void test_length_change_counts_as_change() {
    tracker.record(0x120, false, 8, ZEROS, 100);
    TEST_ASSERT_TRUE(tracker.record(0x120, false, 4, ZEROS, 110));
    TEST_ASSERT_EQUAL(4, tracker.at(0).len);
}

void test_entries_are_sorted_by_id() {
    tracker.record(0x300, false, 8, ZEROS, 0);
    tracker.record(0x100, false, 8, ZEROS, 0);
    tracker.record(0x200, false, 8, ZEROS, 0);
    TEST_ASSERT_EQUAL_HEX32(0x100, tracker.at(0).id);
    TEST_ASSERT_EQUAL_HEX32(0x200, tracker.at(1).id);
    TEST_ASSERT_EQUAL_HEX32(0x300, tracker.at(2).id);
}

void test_standard_and_extended_ids_are_distinct() {
    tracker.record(0x100, false, 8, ZEROS, 0);
    tracker.record(0x100, true, 8, ZEROS, 0);
    TEST_ASSERT_EQUAL(2, tracker.size());
}

void test_ids_beyond_capacity_are_counted_as_dropped() {
    for (int i = 0; i < CanTracker::CAPACITY; i++) tracker.record(i, false, 8, ZEROS, 0);
    TEST_ASSERT_FALSE(tracker.record(0x7FF, false, 8, ZEROS, 0));
    TEST_ASSERT_EQUAL(CanTracker::CAPACITY, tracker.size());
    TEST_ASSERT_EQUAL(1, tracker.droppedIds());
}

void test_rates_are_frames_per_second() {
    tracker.updateRates(0);
    for (int i = 0; i < 50; i++) tracker.record(0x120, false, 8, ZEROS, i * 20);
    tracker.updateRates(1000);
    TEST_ASSERT_EQUAL(50, tracker.at(0).hz);
    TEST_ASSERT_EQUAL(50, tracker.totalHz());
}

void test_rates_wait_for_a_full_second() {
    tracker.updateRates(0);
    tracker.record(0x120, false, 8, ZEROS, 10);
    tracker.updateRates(500);
    TEST_ASSERT_EQUAL(0, tracker.at(0).hz);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_first_frame_creates_entry);
    RUN_TEST(test_repeated_identical_frame_is_not_a_change);
    RUN_TEST(test_changed_byte_is_flagged);
    RUN_TEST(test_length_change_counts_as_change);
    RUN_TEST(test_entries_are_sorted_by_id);
    RUN_TEST(test_standard_and_extended_ids_are_distinct);
    RUN_TEST(test_ids_beyond_capacity_are_counted_as_dropped);
    RUN_TEST(test_rates_are_frames_per_second);
    RUN_TEST(test_rates_wait_for_a_full_second);
    return UNITY_END();
}
