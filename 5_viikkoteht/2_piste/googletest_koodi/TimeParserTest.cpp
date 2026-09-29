#include <gtest/gtest.h>
#include "../TimeParser.h"



// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {
    // Test with correct time string
    char time_test[] = "000005";
    ASSERT_EQ(time_parse(time_test) , 5);

    char time_test2[] = "000015";
    ASSERT_EQ(time_parse(time_test2) , 15);

}


TEST(TimeParserTest, TestCaseIncorrectTime) {
    char time_test[] = "000067";
    ASSERT_EQ(time_parse(time_test) , TIME_LEN_ERROR);

    char time_test2[] = "006700";
    ASSERT_EQ(time_parse(time_test2) , TIME_LEN_ERROR);

}

TEST(TimeParserTest, TestCaseNegativeTime) {
    char time_test[] = "-00067";
    ASSERT_EQ(time_parse(time_test) , TIME_VALUE_ERROR);

    char time_test2[] = "0-6700";
    ASSERT_EQ(time_parse(time_test2) , TIME_VALUE_ERROR);
}

TEST(TimeParserTest, TestCaseNullTime) {
    ASSERT_EQ(time_parse(nullptr), TIME_VALUE_ERROR);

}

TEST(TimeParserTest, TestCaseZeroTime) {
    char time_test[] = "000000";
    ASSERT_EQ(time_parse(time_test) , TIME_VALUE_ERROR);
    
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
