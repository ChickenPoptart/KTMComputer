#include <unity.h>
#include "reset_prompt.h"

static ResetPrompt prompt;

void setUp() { prompt = ResetPrompt(); }
void tearDown() {}

void test_tap_while_open_confirms_and_closes() {
    prompt.open(1000);
    TEST_ASSERT_TRUE(prompt.isOpen(1000 + RESET_PROMPT_MS - 1));
    TEST_ASSERT_TRUE(prompt.confirm(1000 + RESET_PROMPT_MS - 1));
    TEST_ASSERT_FALSE(prompt.isOpen(1000 + RESET_PROMPT_MS - 1));
}

void test_prompt_times_out_and_a_late_tap_does_not_reset() {
    prompt.open(1000);
    TEST_ASSERT_FALSE(prompt.isOpen(1000 + RESET_PROMPT_MS));
    TEST_ASSERT_FALSE(prompt.confirm(1000 + RESET_PROMPT_MS));
}

void test_tap_without_a_prompt_does_not_reset() {
    TEST_ASSERT_FALSE(prompt.isOpen(5000));
    TEST_ASSERT_FALSE(prompt.confirm(5000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_tap_while_open_confirms_and_closes);
    RUN_TEST(test_prompt_times_out_and_a_late_tap_does_not_reset);
    RUN_TEST(test_tap_without_a_prompt_does_not_reset);
    return UNITY_END();
}
