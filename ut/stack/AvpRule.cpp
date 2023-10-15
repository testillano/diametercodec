#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/AvpRule.hpp>
#include <nlohmann/json.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class AvpRule_test : public ::testing::Test
{
public:
    ert::diametercodec::stack::Dictionary dictionary_{};
    ert::diametercodec::stack::AvpRule *avp_rule_{};

    AvpRule_test() {
        //dictionary_.load(ert::diametercodec::json::stacks::base);
        avp_rule_ = new ert::diametercodec::stack::AvpRule(&dictionary_);

        ert::diametercodec::core::AvpId id(258,0);
        avp_rule_->setAvpId(id); // Auth-Application-Id
        avp_rule_->setPresence(ert::diametercodec::stack::AvpRule::Presence::Optional);
        avp_rule_->setQual("1*");
    }
};

//TEST_F(AvpRule_test, getAvpName)
//{
//    EXPECT_EQ(avp_rule_->getAvpName(), "Auth-Application-Id");
//}

TEST_F(AvpRule_test, getPresence)
{
    EXPECT_EQ(avp_rule_->getPresence(), ert::diametercodec::stack::AvpRule::Presence::Optional);
}

TEST_F(AvpRule_test, getQual)
{
    EXPECT_EQ(avp_rule_->getQual(), "1*");
}

//TEST_F(AvpRule_test, isAny)
//{
//    EXPECT_EQ(avp_rule_->isAny(), false);
//}

TEST_F(AvpRule_test, isFixed)
{
    EXPECT_EQ(avp_rule_->isFixed(), false);
}

TEST_F(AvpRule_test, isMandatory)
{
    EXPECT_EQ(avp_rule_->isMandatory(), false);
}

TEST_F(AvpRule_test, isOptional)
{
    EXPECT_EQ(avp_rule_->isOptional(), true);
}

TEST_F(AvpRule_test, getQualMin)
{
    EXPECT_EQ(avp_rule_->getQualMin(), 1);
}

TEST_F(AvpRule_test, getQualMax)
{
    EXPECT_EQ(avp_rule_->getQualMax(), -1);
}

//TEST_F(AvpRule_test, asJson)
//{
//    const nlohmann::json doc = R"({ "type": "Fixed", "name": "Unit-Value"})"_json;
//    EXPECT_EQ(avp_rule_->asJson(), doc);
//}

