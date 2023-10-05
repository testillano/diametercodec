#include <ert/diametercodec/stack/Vendor.hpp>
#include <nlohmann/json.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class Vendor_test : public ::testing::Test
{
public:
    ert::diametercodec::stack::Vendor vendor_ericsson_{};

    Vendor_test() {
        vendor_ericsson_.setCode(193);
        vendor_ericsson_.setName("Ericsson");
    }
};

TEST_F(Vendor_test, getCode)
{
    EXPECT_EQ(vendor_ericsson_.getCode(), 193);
}

TEST_F(Vendor_test, getName)
{
    EXPECT_EQ(vendor_ericsson_.getName(), "Ericsson");
}

TEST_F(Vendor_test, isVendorSpecific)
{
    EXPECT_TRUE(vendor_ericsson_.isVendorSpecific());
}

TEST_F(Vendor_test, copyAndCompare)
{
    ert::diametercodec::stack::Vendor other = vendor_ericsson_;
    EXPECT_TRUE(vendor_ericsson_ == other);
}

TEST_F(Vendor_test, asJson)
{
    const nlohmann::json doc = R"({"name": "Ericsson", "code": 193})"_json;
    EXPECT_EQ(vendor_ericsson_.asJson(), doc);
}

