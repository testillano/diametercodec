#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <nlohmann/json.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class Avp_test : public ::testing::Test
{
public:
    ert::diametercodec::stack::Dictionary dictionary_{};
    ert::diametercodec::stack::Avp *avp_{};

    Avp_test() {
        dictionary_.load(ert::diametercodec::json::stacks::base);
        avp_ = new ert::diametercodec::stack::Avp(&dictionary_);

        avp_->setCode(2000);
        avp_->setVendorId(200);
        avp_->setName("Test-AVP");
        avp_->setVendorName("TEST");
        avp_->setFormatName("Enumerated");
        avp_->setVBit(true);
        avp_->setMBit(true);
        avp_->setEnums("1,2");
        avp_->addEnums("3-5");
        avp_->addLabel("1", "one");
        avp_->addLabel("2", "two");
        avp_->addLabel("3", "three");
        avp_->addLabel("4", "four");
        avp_->addLabel("5", "five");
    }
};

TEST_F(Avp_test, getId) {
    ert::diametercodec::core::AvpId id = {2000,200};
    EXPECT_EQ(avp_->getId(), id);
}

TEST_F(Avp_test, getName) {
    EXPECT_EQ(avp_->getName(), "Test-AVP");
}

TEST_F(Avp_test, getFormatName) {
    EXPECT_EQ(avp_->getFormatName(), "Enumerated");
}

TEST_F(Avp_test, vBit) {
    EXPECT_EQ(avp_->vBit(), true);
}

TEST_F(Avp_test, mBit) {
    EXPECT_EQ(avp_->mBit(), true);
}

TEST_F(Avp_test, getEnums) {
    EXPECT_EQ(std::string(avp_->getEnums()), "1-5");
}

TEST_F(Avp_test, isChild) {
    EXPECT_FALSE(avp_->isChild({100,0}));
}

TEST_F(Avp_test, getFormat) {
    const ert::diametercodec::stack::Format *format = avp_->getFormat();
    EXPECT_EQ(format->getName(), "Enumerated");
}

TEST_F(Avp_test, getAlias) {
    EXPECT_EQ("two", std::string(avp_->getAlias("2")));
}

TEST_F(Avp_test, addLabel) {
    ert::diametercodec::stack::Avp::label_container labels = avp_->labels();
    ert::diametercodec::stack::Avp::label_container expected_labels;

    expected_labels["1"] = "one";
    expected_labels["2"] = "two";
    expected_labels["3"] = "three";
    expected_labels["4"] = "four";
    expected_labels["5"] = "five";
    EXPECT_EQ(expected_labels, labels);
}

TEST_F(Avp_test, addAvpRuleOnGrouped) {
    ert::diametercodec::stack::Avp *groupedAvp = dictionary_.getAvp("Vendor-Specific-Application-Id");

    auto avpRule = new ert::diametercodec::stack::AvpRule(&dictionary_);
    ert::diametercodec::core::AvpId id(258,0); // Auth-Application-Id
    avpRule->setAvpId(id);
    avpRule->setPresence(ert::diametercodec::stack::AvpRule::Presence::Optional);
    avpRule->setQual("1*");
    EXPECT_THROW(groupedAvp->addAvpRule(*avpRule), std::runtime_error); // Cannot add two rules ...
    id = {1,0}; // User-Name
    avpRule->setAvpId(id);
    avpRule->setPresence(ert::diametercodec::stack::AvpRule::Presence::Optional);
    groupedAvp->addAvpRule(*avpRule); // no exception in this case
}

TEST_F(Avp_test, addAvpRuleOnNonGrouped) {
    auto avpRule = new ert::diametercodec::stack::AvpRule(&dictionary_);
    ert::diametercodec::core::AvpId id(258,0); // Auth-Application-Id
    avpRule->setAvpId(id);
    avpRule->setPresence(ert::diametercodec::stack::AvpRule::Presence::Optional);
    avpRule->setQual("1*");
    EXPECT_THROW(avp_->addAvpRule(*avpRule), std::runtime_error);
}

TEST_F(Avp_test, allowEnum) {
    EXPECT_TRUE(avp_->allowEnum(5));
    EXPECT_FALSE(avp_->allowEnum(6));
    avp_->addEnums("6-9");
    EXPECT_TRUE(avp_->allowEnum(6));
}

TEST_F(Avp_test, hasAliases) {
    EXPECT_TRUE(avp_->hasAliases());
}

TEST_F(Avp_test, asJson) {
    const nlohmann::json doc = R"({"code":2000,"m-bit":true,"name":"Test-AVP","single":{"enum":"1-5","format":"Enumerated","label":[{"alias":"one","data":"1"},{"alias":"two","data":"2"},{"alias":"three","data":"3"},{"alias":"four","data":"4"},{"alias":"five","data":"5"}]},"v-bit":true,"vendor-name":"TEST"})"_json;
    EXPECT_EQ(avp_->asJson(), doc);
}
