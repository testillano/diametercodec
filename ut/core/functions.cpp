#include <ert/diametercodec/core/functions.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class Functions_test : public ::testing::Test
{
public:

    Functions_test() {
    }

};

TEST_F(Functions_test, avpIdAsPairString) {
    ert::diametercodec::core::AvpId id = {200, 10};
    EXPECT_EQ(ert::diametercodec::core::functions::avpIdAsPairString(id), "(200,10)");
}

TEST_F(Functions_test, commandIdAsPairString) {
    ert::diametercodec::core::CommandId id = {200, true};
    EXPECT_EQ(ert::diametercodec::core::functions::commandIdAsPairString(id), "(200,request)");
}

