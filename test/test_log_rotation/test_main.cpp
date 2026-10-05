#include <unity.h>
#include "log_rotation.h"

void setUp() {}
void tearDown() {}

void test_log_number_from_file_name() {
    TEST_ASSERT_EQUAL(12, logNumberFromName("can_0012.log"));
}

void test_log_number_with_leading_slash_and_past_9999() {
    TEST_ASSERT_EQUAL(7, logNumberFromName("/can_0007.log"));
    TEST_ASSERT_EQUAL(10000, logNumberFromName("can_10000.log"));
}

void test_other_files_are_not_ride_logs() {
    TEST_ASSERT_EQUAL(-1, logNumberFromName("System Volume Information"));
    TEST_ASSERT_EQUAL(-1, logNumberFromName("can_.log"));
    TEST_ASSERT_EQUAL(-1, logNumberFromName("can_12a.log"));
    TEST_ASSERT_EQUAL(-1, logNumberFromName("can_0012.txt"));
    TEST_ASSERT_EQUAL(-1, logNumberFromName("._can_0012.log"));  // macOS metadata
}

void test_first_log_on_an_empty_card_is_1() {
    TEST_ASSERT_EQUAL(1, nextLogNumber(nullptr, 0));
}

void test_next_log_follows_the_highest_even_after_old_ones_are_deleted() {
    const int existing[] = {14, 12, 13};  // 1..11 deleted to make space
    TEST_ASSERT_EQUAL(15, nextLogNumber(existing, 3));
}

static const uint64_t GB = 1024ULL * 1024 * 1024;

void test_plenty_of_space_deletes_nothing() {
    const int existing[] = {3, 4, 5};
    TEST_ASSERT_EQUAL(-1, logToDelete(existing, 3, 6, 20 * GB, 32 * GB));
}

void test_low_space_deletes_the_oldest() {
    const int existing[] = {5, 3, 4};
    TEST_ASSERT_EQUAL(3, logToDelete(existing, 3, 6, 2 * GB, 32 * GB));
}

void test_never_deletes_the_log_being_recorded() {
    const int existing[] = {6};
    TEST_ASSERT_EQUAL(-1, logToDelete(existing, 1, 6, 1 * GB, 32 * GB));
}

void test_threshold_is_ten_percent_free() {
    const int existing[] = {3, 4};
    const uint64_t total = 100 * GB;
    TEST_ASSERT_EQUAL(-1, logToDelete(existing, 2, 4, 10 * GB, total));
    TEST_ASSERT_EQUAL(3, logToDelete(existing, 2, 4, 10 * GB - 1, total));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_never_deletes_the_log_being_recorded);
    RUN_TEST(test_threshold_is_ten_percent_free);
    RUN_TEST(test_plenty_of_space_deletes_nothing);
    RUN_TEST(test_low_space_deletes_the_oldest);
    RUN_TEST(test_first_log_on_an_empty_card_is_1);
    RUN_TEST(test_next_log_follows_the_highest_even_after_old_ones_are_deleted);
    RUN_TEST(test_log_number_with_leading_slash_and_past_9999);
    RUN_TEST(test_other_files_are_not_ride_logs);
    RUN_TEST(test_log_number_from_file_name);
    return UNITY_END();
}
