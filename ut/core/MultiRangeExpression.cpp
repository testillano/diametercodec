#include <ert/diametercodec/core/MultiRangeExpression.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define LITERAL "1-4,23,45-46"
#define LITERAL_EXPANDED "1,2,3,4,23,45,46"


class MultiRangeExpression_test : public ::testing::Test
{
public:
    ert::diametercodec::core::MultiRangeExpression multirange_{};

    MultiRangeExpression_test() {
        multirange_.setLiteral(LITERAL);
    }
};

TEST_F(MultiRangeExpression_test, getLiteral)
{
    EXPECT_EQ(multirange_.getLiteral(), std::string(LITERAL));
}

TEST_F(MultiRangeExpression_test, getExpandedLiteral)
{
    EXPECT_EQ(multirange_.getExpandedLiteral(), std::string(LITERAL_EXPANDED));
}


TEST_F(MultiRangeExpression_test, simplifyLiteral)
{
    multirange_.setLiteral(LITERAL_EXPANDED);
    EXPECT_EQ(std::string(multirange_.simplifyLiteral()), std::string(LITERAL));
}

TEST_F(MultiRangeExpression_test, contain)
{
    EXPECT_TRUE(multirange_.contain(23));
    EXPECT_FALSE(multirange_.contain(24));
}

TEST_F(MultiRangeExpression_test, addLiteral)
{
    multirange_.addLiteral("24-25");
    EXPECT_TRUE(multirange_.contain(24));
    EXPECT_TRUE(multirange_.contain(25));
}

