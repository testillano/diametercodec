#include <ert/diametercodec/core/defines.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


class Defines_test : public ::testing::Test
{
public:

    struct Transport {
        enum _v {
            None = -1,
            TCP,
            SCTP,
            UDP
        };

        declare_enum(Transport);

        static const char* asText(const Transport::_v v) {
            return asCString(v);
        }
    };

    Defines_test() {
    }

};

assign_enum(Defines_test::Transport) = { "TCP", "SCTP", "UDP", nullptr};

TEST_F(Defines_test, asEnum) {
    EXPECT_EQ(Transport::asEnum("TCP"), Transport::TCP);
    std::string tcp = "TCP";
    EXPECT_EQ(Transport::asEnum(tcp), Transport::TCP);
}

TEST_F(Defines_test, asEnumNone) {
    EXPECT_EQ(Transport::asEnum("None"), Transport::None);
}

TEST_F(Defines_test, asCString) {
    EXPECT_EQ(Transport::asCString(Transport::SCTP), "SCTP");
}

TEST_F(Defines_test, asStringNull) {
    EXPECT_EQ(Transport::asCString(Transport::None), nullptr);
}
