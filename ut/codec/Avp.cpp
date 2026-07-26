#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/codec/Avp.hpp>
#include <nlohmann/json.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <arpa/inet.h>

using namespace ert::diametercodec;

class CodecAvp_test : public ::testing::Test {
public:
    stack::Dictionary dict_;
    CodecAvp_test() {
        dict_.load(json::stacks::base);
    }

    // Helper: build a raw AVP buffer without vendor bit
    // code(4) + flags(1) + length(3) + data
    static core::Buffer buildRawAvp(uint32_t code, uint8_t flags,
                                    const std::vector<uint8_t>& data) {
        uint32_t avpLen = 8 + data.size(); // header(8) + data
        core::Buffer buf;
        // Code (4 bytes big-endian)
        buf.push_back(static_cast<uint8_t>(code >> 24));
        buf.push_back(static_cast<uint8_t>(code >> 16));
        buf.push_back(static_cast<uint8_t>(code >> 8));
        buf.push_back(static_cast<uint8_t>(code));
        // Flags (1 byte)
        buf.push_back(flags);
        // Length (3 bytes big-endian)
        buf.push_back(static_cast<uint8_t>(avpLen >> 16));
        buf.push_back(static_cast<uint8_t>(avpLen >> 8));
        buf.push_back(static_cast<uint8_t>(avpLen));
        // Data
        buf.insert(buf.end(), data.begin(), data.end());
        // Padding
        size_t pad = (4 - (avpLen % 4)) % 4;
        for (size_t i = 0; i < pad; ++i) buf.push_back(0);
        return buf;
    }

    // Helper: build a raw AVP buffer with vendor bit
    // code(4) + flags(1) + length(3) + vendor-id(4) + data
    static core::Buffer buildRawAvpWithVendor(uint32_t code, uint8_t flags,
                                              uint32_t vendorId,
                                              const std::vector<uint8_t>& data) {
        uint32_t avpLen = 12 + data.size(); // header(12) + data
        core::Buffer buf;
        // Code
        buf.push_back(static_cast<uint8_t>(code >> 24));
        buf.push_back(static_cast<uint8_t>(code >> 16));
        buf.push_back(static_cast<uint8_t>(code >> 8));
        buf.push_back(static_cast<uint8_t>(code));
        // Flags (must have vendor bit set)
        buf.push_back(flags | core::AvpFlagVendor);
        // Length
        buf.push_back(static_cast<uint8_t>(avpLen >> 16));
        buf.push_back(static_cast<uint8_t>(avpLen >> 8));
        buf.push_back(static_cast<uint8_t>(avpLen));
        // Vendor-ID
        buf.push_back(static_cast<uint8_t>(vendorId >> 24));
        buf.push_back(static_cast<uint8_t>(vendorId >> 16));
        buf.push_back(static_cast<uint8_t>(vendorId >> 8));
        buf.push_back(static_cast<uint8_t>(vendorId));
        // Data
        buf.insert(buf.end(), data.begin(), data.end());
        // Padding
        size_t pad = (4 - (avpLen % 4)) % 4;
        for (size_t i = 0; i < pad; ++i) buf.push_back(0);
        return buf;
    }

    // Helper: encode uint32 to big-endian bytes
    static std::vector<uint8_t> u32bytes(uint32_t v) {
        return {static_cast<uint8_t>(v >> 24), static_cast<uint8_t>(v >> 16),
                static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v)};
    }

    // Helper: encode int32 to big-endian bytes
    static std::vector<uint8_t> s32bytes(int32_t v) {
        uint32_t u;
        std::memcpy(&u, &v, 4);
        return u32bytes(u);
    }

    // Helper: encode uint64 to big-endian bytes
    static std::vector<uint8_t> u64bytes(uint64_t v) {
        std::vector<uint8_t> b(8);
        for (int i = 7; i >= 0; --i) {
            b[i] = static_cast<uint8_t>(v & 0xFF);
            v >>= 8;
        }
        return b;
    }

    // Helper: encode int64 to big-endian bytes
    static std::vector<uint8_t> s64bytes(int64_t v) {
        uint64_t u;
        std::memcpy(&u, &v, 8);
        return u64bytes(u);
    }

    // Helper: encode float32 to big-endian bytes
    static std::vector<uint8_t> f32bytes(float v) {
        uint32_t u;
        std::memcpy(&u, &v, 4);
        return u32bytes(u);
    }

    // Helper: encode float64 to big-endian bytes
    static std::vector<uint8_t> f64bytes(double v) {
        uint64_t u;
        std::memcpy(&u, &v, 8);
        return u64bytes(u);
    }
};

// ============================================================================
// Decode/Encode Roundtrip Tests
// ============================================================================

// --- Unsigned32: Session-Timeout (27, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Unsigned32) {
    uint32_t value = 3600;
    auto raw = buildRawAvp(27, core::AvpFlagMandatory, u32bytes(value));

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, raw.size());
    EXPECT_EQ(avp.getId(), (core::AvpId{27, 0}));
    EXPECT_EQ(avp.getUnsigned32(), value);
    EXPECT_TRUE(avp.mandatoryBit());
    EXPECT_FALSE(avp.vendorBit());

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Integer32: Exponent (429, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Integer32) {
    int32_t value = -3;
    auto raw = buildRawAvp(429, core::AvpFlagMandatory, s32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getInteger32(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Integer32_Positive) {
    int32_t value = 42;
    auto raw = buildRawAvp(429, core::AvpFlagMandatory, s32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getInteger32(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Integer64: Value-Digits (447, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Integer64) {
    int64_t value = -123456789012345LL;
    auto raw = buildRawAvp(447, core::AvpFlagMandatory, s64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getInteger64(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Unsigned64: Accounting-Sub-Session-Id (287, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Unsigned64) {
    uint64_t value = 0xFEDCBA9876543210ULL;
    auto raw = buildRawAvp(287, core::AvpFlagMandatory, u64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getUnsigned64(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- OctetString: NAS-IP-Address (4, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_OctetString) {
    std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF, 0x01};
    auto raw = buildRawAvp(4, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    std::string expected(reinterpret_cast<const char*>(data.data()), data.size());
    EXPECT_EQ(avp.getString(), expected);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- UTF8String: User-Name (1, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_UTF8String) {
    std::string value = "alice@example.com";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- DiameterIdentity: Origin-Host (264, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_DiameterIdentity) {
    std::string value = "node.example.com";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(264, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Address IPv4: Host-IP-Address (257, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_AddressIPv4) {
    // Family(2) + IPv4(4) = 6 bytes
    uint8_t ipv4[4];
    inet_pton(AF_INET, "192.168.1.100", ipv4);
    std::vector<uint8_t> data = {0x00, 0x01}; // family = 1 (IPv4)
    data.insert(data.end(), ipv4, ipv4 + 4);
    auto raw = buildRawAvp(257, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), "192.168.1.100");

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Address IPv6: Host-IP-Address (257, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_AddressIPv6) {
    // Family(2) + IPv6(16) = 18 bytes
    uint8_t ipv6[16];
    inet_pton(AF_INET6, "2001:db8::1", ipv6);
    std::vector<uint8_t> data = {0x00, 0x02}; // family = 2 (IPv6)
    data.insert(data.end(), ipv6, ipv6 + 16);
    auto raw = buildRawAvp(257, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), "2001:db8::1");

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Time: Event-Timestamp (55, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Time) {
    // NTP timestamp: unix epoch 1609459200 (2021-01-01 00:00:00 UTC)
    uint32_t unixTs = 1609459200U;
    uint32_t ntpTs = unixTs + core::NtpEpochOffset;
    auto raw = buildRawAvp(55, core::AvpFlagMandatory, u32bytes(ntpTs));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getUnsigned32(), ntpTs);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Enumerated: Disconnect-Cause (273, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Enumerated) {
    int32_t value = 1; // BUSY
    auto raw = buildRawAvp(273, core::AvpFlagMandatory, s32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getInteger32(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- DiameterURI: Redirect-Host (292, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_DiameterURI) {
    std::string value = "aaa://host.example.com:6666;transport=tcp";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(292, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- IPFilterRule: Restriction-Filter-Rule (438, 0) ---
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_IPFilterRule) {
    std::string value = "permit in ip from any to 10.0.0.0/8";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(438, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getString(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// --- Grouped: Subscription-Id (443, 0) ---
// Children: Subscription-Id-Type (450, Enumerated) + Subscription-Id-Data (444, UTF8String)
TEST_F(CodecAvp_test, DecodeEncodeRoundtrip_Grouped) {
    // Build child 1: Subscription-Id-Type (450) = 0 (END_USER_E164)
    auto child1 = buildRawAvp(450, core::AvpFlagMandatory, s32bytes(0));
    // Build child 2: Subscription-Id-Data (444) = "1234567890"
    std::string subData = "1234567890";
    std::vector<uint8_t> subDataBytes(subData.begin(), subData.end());
    auto child2 = buildRawAvp(444, core::AvpFlagMandatory, subDataBytes);

    // Grouped data = child1 + child2 concatenated
    std::vector<uint8_t> groupedData;
    groupedData.insert(groupedData.end(), child1.begin(), child1.end());
    groupedData.insert(groupedData.end(), child2.begin(), child2.end());

    auto raw = buildRawAvp(443, core::AvpFlagMandatory, groupedData);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{443, 0}));

    const auto& children = avp.getGrouped();
    ASSERT_EQ(children.size(), 2u);
    EXPECT_EQ(children[0].getId(), (core::AvpId{450, 0}));
    EXPECT_EQ(children[0].getInteger32(), 0);
    EXPECT_EQ(children[1].getId(), (core::AvpId{444, 0}));
    EXPECT_EQ(children[1].getString(), "1234567890");

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

// ============================================================================
// toJson Tests
// ============================================================================

TEST_F(CodecAvp_test, ToJson_Unsigned32) {
    uint32_t value = 7200;
    auto raw = buildRawAvp(27, core::AvpFlagMandatory, u32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<uint32_t>(), value);
}

TEST_F(CodecAvp_test, ToJson_Integer32) {
    int32_t value = -7;
    auto raw = buildRawAvp(429, core::AvpFlagMandatory, s32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<int32_t>(), value);
}

TEST_F(CodecAvp_test, ToJson_Integer64) {
    int64_t value = -9876543210LL;
    auto raw = buildRawAvp(447, core::AvpFlagMandatory, s64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<int64_t>(), value);
}

TEST_F(CodecAvp_test, ToJson_Unsigned64) {
    uint64_t value = 12345678901234ULL;
    auto raw = buildRawAvp(287, core::AvpFlagMandatory, u64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<uint64_t>(), value);
}

TEST_F(CodecAvp_test, ToJson_OctetString_HexEncoded) {
    std::vector<uint8_t> data = {0xCA, 0xFE, 0xBA, 0xBE};
    auto raw = buildRawAvp(4, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), "cafebabe");
}

TEST_F(CodecAvp_test, ToJson_UTF8String) {
    std::string value = "bob@realm.org";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), value);
}

TEST_F(CodecAvp_test, ToJson_DiameterIdentity) {
    std::string value = "peer.example.com";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(264, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), value);
}

TEST_F(CodecAvp_test, ToJson_AddressIPv4) {
    uint8_t ipv4[4];
    inet_pton(AF_INET, "10.0.0.1", ipv4);
    std::vector<uint8_t> data = {0x00, 0x01};
    data.insert(data.end(), ipv4, ipv4 + 4);
    auto raw = buildRawAvp(257, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), "10.0.0.1");
}

TEST_F(CodecAvp_test, ToJson_AddressIPv6) {
    uint8_t ipv6[16];
    inet_pton(AF_INET6, "::1", ipv6);
    std::vector<uint8_t> data = {0x00, 0x02};
    data.insert(data.end(), ipv6, ipv6 + 16);
    auto raw = buildRawAvp(257, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), "::1");
}

TEST_F(CodecAvp_test, ToJson_Time_UnixEpoch) {
    uint32_t unixTs = 1700000000U;
    uint32_t ntpTs = unixTs + core::NtpEpochOffset;
    auto raw = buildRawAvp(55, core::AvpFlagMandatory, u32bytes(ntpTs));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<uint32_t>(), unixTs);
}

TEST_F(CodecAvp_test, ToJson_Enumerated) {
    int32_t value = 2; // DO_NOT_WANT_TO_TALK_TO_YOU
    auto raw = buildRawAvp(273, core::AvpFlagMandatory, s32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<int32_t>(), value);
}

TEST_F(CodecAvp_test, ToJson_Grouped) {
    auto child1 = buildRawAvp(450, core::AvpFlagMandatory, s32bytes(1));
    std::string subData = "test_user";
    std::vector<uint8_t> subDataBytes(subData.begin(), subData.end());
    auto child2 = buildRawAvp(444, core::AvpFlagMandatory, subDataBytes);

    std::vector<uint8_t> groupedData;
    groupedData.insert(groupedData.end(), child1.begin(), child1.end());
    groupedData.insert(groupedData.end(), child2.begin(), child2.end());
    auto raw = buildRawAvp(443, core::AvpFlagMandatory, groupedData);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);

    EXPECT_TRUE(j.is_object());
    EXPECT_EQ(j["Subscription-Id-Type"].get<int32_t>(), 1);
    EXPECT_EQ(j["Subscription-Id-Data"].get<std::string>(), "test_user");
}

TEST_F(CodecAvp_test, ToJson_DiameterURI) {
    std::string value = "aaa://host:3868;transport=sctp";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(292, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), value);
}

TEST_F(CodecAvp_test, ToJson_IPFilterRule) {
    std::string value = "deny in ip from 10.0.0.0/8 to any";
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(438, core::AvpFlagMandatory, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_EQ(j.get<std::string>(), value);
}

// ============================================================================
// fromJson Tests
// ============================================================================

TEST_F(CodecAvp_test, FromJson_Unsigned32) {
    nlohmann::json value = 1800;
    codec::Avp avp = codec::Avp::fromJson("Session-Timeout", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{27, 0}));
    EXPECT_EQ(avp.getUnsigned32(), 1800u);
    EXPECT_TRUE(avp.mandatoryBit());
    EXPECT_FALSE(avp.vendorBit());
}

TEST_F(CodecAvp_test, FromJson_Integer32) {
    nlohmann::json value = -5;
    codec::Avp avp = codec::Avp::fromJson("Exponent", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{429, 0}));
    EXPECT_EQ(avp.getInteger32(), -5);
}

TEST_F(CodecAvp_test, FromJson_Integer64) {
    nlohmann::json value = 987654321012345LL;
    codec::Avp avp = codec::Avp::fromJson("Value-Digits", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{447, 0}));
    EXPECT_EQ(avp.getInteger64(), 987654321012345LL);
}

TEST_F(CodecAvp_test, FromJson_Unsigned64) {
    nlohmann::json value = 0xABCDEF0123456789ULL;
    codec::Avp avp = codec::Avp::fromJson("Accounting-Sub-Session-Id", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{287, 0}));
    EXPECT_EQ(avp.getUnsigned64(), 0xABCDEF0123456789ULL);
}

TEST_F(CodecAvp_test, FromJson_OctetString_HexInput) {
    nlohmann::json value = "deadbeef";
    codec::Avp avp = codec::Avp::fromJson("NAS-IP-Address", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{4, 0}));
    // Raw bytes should be 0xDE 0xAD 0xBE 0xEF
    std::string expected = {'\xDE', '\xAD', '\xBE', '\xEF'};
    EXPECT_EQ(avp.getString(), expected);
}

TEST_F(CodecAvp_test, FromJson_UTF8String) {
    nlohmann::json value = "charlie@domain.net";
    codec::Avp avp = codec::Avp::fromJson("User-Name", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{1, 0}));
    EXPECT_EQ(avp.getString(), "charlie@domain.net");
}

TEST_F(CodecAvp_test, FromJson_DiameterIdentity) {
    nlohmann::json value = "origin.example.org";
    codec::Avp avp = codec::Avp::fromJson("Origin-Host", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{264, 0}));
    EXPECT_EQ(avp.getString(), "origin.example.org");
}

TEST_F(CodecAvp_test, FromJson_AddressIPv4) {
    nlohmann::json value = "172.16.0.1";
    codec::Avp avp = codec::Avp::fromJson("Host-IP-Address", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{257, 0}));
    EXPECT_EQ(avp.getString(), "172.16.0.1");
}

TEST_F(CodecAvp_test, FromJson_AddressIPv6) {
    nlohmann::json value = "fe80::1";
    codec::Avp avp = codec::Avp::fromJson("Host-IP-Address", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{257, 0}));
    EXPECT_EQ(avp.getString(), "fe80::1");
}

TEST_F(CodecAvp_test, FromJson_Time) {
    uint32_t unixTs = 1609459200U; // 2021-01-01
    nlohmann::json value = unixTs;
    codec::Avp avp = codec::Avp::fromJson("Event-Timestamp", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{55, 0}));
    // Internal storage is NTP
    EXPECT_EQ(avp.getUnsigned32(), unixTs + core::NtpEpochOffset);
}

TEST_F(CodecAvp_test, FromJson_Enumerated) {
    nlohmann::json value = 0; // REBOOTING
    codec::Avp avp = codec::Avp::fromJson("Disconnect-Cause", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{273, 0}));
    EXPECT_EQ(avp.getInteger32(), 0);
}

TEST_F(CodecAvp_test, FromJson_Grouped) {
    nlohmann::json value = {
        {"Subscription-Id-Type", 2},
        {"Subscription-Id-Data", "sip:user@example.com"}
    };
    codec::Avp avp = codec::Avp::fromJson("Subscription-Id", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{443, 0}));

    const auto& children = avp.getGrouped();
    ASSERT_EQ(children.size(), 2u);

    // Find each child (order may depend on JSON iteration)
    bool foundType = false, foundData = false;
    for (const auto& child : children) {
        if (child.getId() == core::AvpId{450, 0}) {
            EXPECT_EQ(child.getInteger32(), 2);
            foundType = true;
        } else if (child.getId() == core::AvpId{444, 0}) {
            EXPECT_EQ(child.getString(), "sip:user@example.com");
            foundData = true;
        }
    }
    EXPECT_TRUE(foundType);
    EXPECT_TRUE(foundData);
}

TEST_F(CodecAvp_test, FromJson_DiameterURI) {
    nlohmann::json value = "aaa://redirect.example.com:3868";
    codec::Avp avp = codec::Avp::fromJson("Redirect-Host", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{292, 0}));
    EXPECT_EQ(avp.getString(), "aaa://redirect.example.com:3868");
}

TEST_F(CodecAvp_test, FromJson_IPFilterRule) {
    nlohmann::json value = "permit out ip from any to 192.168.0.0/16";
    codec::Avp avp = codec::Avp::fromJson("Restriction-Filter-Rule", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{438, 0}));
    EXPECT_EQ(avp.getString(), "permit out ip from any to 192.168.0.0/16");
}

TEST_F(CodecAvp_test, FromJson_UnknownAvp_Throws) {
    nlohmann::json value = 42;
    EXPECT_THROW(codec::Avp::fromJson("NonExistent-AVP", value, dict_),
                 std::runtime_error);
}

// ============================================================================
// JSON Roundtrip: fromJson -> encode -> decode -> toJson
// ============================================================================

TEST_F(CodecAvp_test, JsonRoundtrip_Unsigned32) {
    nlohmann::json input = 9999;
    codec::Avp avp1 = codec::Avp::fromJson("Session-Timeout", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Integer32) {
    nlohmann::json input = -42;
    codec::Avp avp1 = codec::Avp::fromJson("Exponent", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Integer64) {
    nlohmann::json input = -55555555555LL;
    codec::Avp avp1 = codec::Avp::fromJson("Value-Digits", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Unsigned64) {
    nlohmann::json input = 18446744073709551615ULL; // max uint64
    codec::Avp avp1 = codec::Avp::fromJson("Accounting-Sub-Session-Id", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_OctetString) {
    nlohmann::json input = "aabbccdd";
    codec::Avp avp1 = codec::Avp::fromJson("NAS-IP-Address", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_UTF8String) {
    nlohmann::json input = "user@example.com";
    codec::Avp avp1 = codec::Avp::fromJson("User-Name", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_DiameterIdentity) {
    nlohmann::json input = "my.identity.example.com";
    codec::Avp avp1 = codec::Avp::fromJson("Origin-Host", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_AddressIPv4) {
    nlohmann::json input = "10.20.30.40";
    codec::Avp avp1 = codec::Avp::fromJson("Host-IP-Address", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_AddressIPv6) {
    nlohmann::json input = "2001:db8::abcd";
    codec::Avp avp1 = codec::Avp::fromJson("Host-IP-Address", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Time) {
    uint32_t unixTs = 1700000000U;
    nlohmann::json input = unixTs;
    codec::Avp avp1 = codec::Avp::fromJson("Event-Timestamp", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Enumerated) {
    nlohmann::json input = 1; // BUSY
    codec::Avp avp1 = codec::Avp::fromJson("Disconnect-Cause", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_Grouped) {
    nlohmann::json input = {
        {"Subscription-Id-Type", 3},
        {"Subscription-Id-Data", "user@nai.example"}
    };
    codec::Avp avp1 = codec::Avp::fromJson("Subscription-Id", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(output["Subscription-Id-Type"].get<int32_t>(), 3);
    EXPECT_EQ(output["Subscription-Id-Data"].get<std::string>(), "user@nai.example");
}

TEST_F(CodecAvp_test, JsonRoundtrip_DiameterURI) {
    nlohmann::json input = "aaa://server.example.com:6666;transport=tcp";
    codec::Avp avp1 = codec::Avp::fromJson("Redirect-Host", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

TEST_F(CodecAvp_test, JsonRoundtrip_IPFilterRule) {
    nlohmann::json input = "permit in ip from 0.0.0.0/0 to 0.0.0.0/0";
    codec::Avp avp1 = codec::Avp::fromJson("Restriction-Filter-Rule", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_EQ(input, output);
}

// ============================================================================
// Error Cases
// ============================================================================

TEST_F(CodecAvp_test, Error_BufferTooShortForHeader) {
    // Less than 8 bytes (minimum AVP header without vendor)
    std::vector<uint8_t> buf = {0x00, 0x00, 0x00, 0x1B, 0x40, 0x00, 0x00};
    codec::Avp avp;
    EXPECT_THROW(avp.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_BufferTooShortForVendorHeader) {
    // Has vendor bit set but only 8 bytes (needs 12)
    std::vector<uint8_t> buf = {
        0x00, 0x00, 0x00, 0x1B, // code = 27
        0xC0,                   // flags: vendor(0x80) + mandatory(0x40)
        0x00, 0x00, 0x0C,       // length = 12 (with vendor header)
    };
    codec::Avp avp;
    EXPECT_THROW(avp.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_TruncatedData) {
    // AVP header says length=12 (8 header + 4 data) but buffer only has 10 bytes
    std::vector<uint8_t> buf = {
        0x00, 0x00, 0x00, 0x1B, // code = 27
        0x40,                   // flags: mandatory
        0x00, 0x00, 0x0C,       // length = 12
        0x00, 0x00              // only 2 bytes of data (need 4)
    };
    codec::Avp avp;
    EXPECT_THROW(avp.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_AvpLengthSmallerThanHeader) {
    // AVP length field claims 4 which is less than minimum header (8)
    std::vector<uint8_t> buf = {
        0x00, 0x00, 0x00, 0x1B, // code = 27
        0x40,                   // flags: mandatory
        0x00, 0x00, 0x04,       // length = 4 (invalid, less than header)
        0x00, 0x00, 0x00, 0x00  // extra bytes
    };
    codec::Avp avp;
    EXPECT_THROW(avp.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_Integer32_WrongDataLength) {
    // Exponent (429) expects 4 bytes, provide 2
    std::vector<uint8_t> data = {0x00, 0x01};
    auto raw = buildRawAvp(429, core::AvpFlagMandatory, data);

    codec::Avp avp;
    EXPECT_THROW(avp.decode(raw.data(), raw.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_Unsigned64_WrongDataLength) {
    // Accounting-Sub-Session-Id (287) expects 8 bytes, provide 4
    std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01};
    auto raw = buildRawAvp(287, core::AvpFlagMandatory, data);

    codec::Avp avp;
    EXPECT_THROW(avp.decode(raw.data(), raw.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_Time_WrongDataLength) {
    // Event-Timestamp (55) expects 4 bytes, provide 8
    std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02};
    auto raw = buildRawAvp(55, core::AvpFlagMandatory, data);

    codec::Avp avp;
    EXPECT_THROW(avp.decode(raw.data(), raw.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvp_test, Error_EmptyBuffer) {
    codec::Avp avp;
    EXPECT_THROW(avp.decode(nullptr, 0, dict_), std::runtime_error);
}

// ============================================================================
// Flag Handling
// ============================================================================

TEST_F(CodecAvp_test, Flags_MandatoryBitSet) {
    // Session-Timeout (27, 0): M-bit is set in dictionary
    nlohmann::json value = 100;
    codec::Avp avp = codec::Avp::fromJson("Session-Timeout", value, dict_);
    EXPECT_TRUE(avp.mandatoryBit());
    EXPECT_FALSE(avp.vendorBit());
    EXPECT_EQ(avp.getFlags(), core::AvpFlagMandatory);
}

TEST_F(CodecAvp_test, Flags_NoFlagsSet) {
    // Framed-IP-Address (8, 0): no v-bit, no m-bit in dictionary
    nlohmann::json value = "0a000001";
    codec::Avp avp = codec::Avp::fromJson("Framed-IP-Address", value, dict_);
    EXPECT_FALSE(avp.mandatoryBit());
    EXPECT_FALSE(avp.vendorBit());
    EXPECT_EQ(avp.getFlags(), 0);
}

TEST_F(CodecAvp_test, Flags_VendorBitDecoded) {
    // Manually build an AVP with vendor bit set (vendor-id=94, code=9999)
    // This won't be in dictionary so it's "unknown" but vendor bit parsing works
    std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01}; // 4 bytes of data
    auto raw = buildRawAvpWithVendor(9999, core::AvpFlagMandatory, 94, data);

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_TRUE(avp.vendorBit());
    EXPECT_TRUE(avp.mandatoryBit());
    EXPECT_EQ(avp.getId(), (core::AvpId{9999, 94}));
}

TEST_F(CodecAvp_test, Flags_VendorBitEncoded) {
    // Build AVP with vendor bit and verify it encodes correctly
    codec::Avp avp;
    avp.setId(core::AvpId{9999, 94});
    avp.setFlags(core::AvpFlagVendor | core::AvpFlagMandatory);
    avp.setData(std::string("test"));

    core::Buffer buf;
    avp.encode(buf, dict_);

    // Verify vendor bit in encoded buffer
    EXPECT_EQ(buf[4] & core::AvpFlagVendor, core::AvpFlagVendor);
    EXPECT_EQ(buf[4] & core::AvpFlagMandatory, core::AvpFlagMandatory);

    // Verify header length is 12 (with vendor)
    uint32_t encodedLen = (uint32_t(buf[5]) << 16) |
                          (uint32_t(buf[6]) << 8) | uint32_t(buf[7]);
    EXPECT_EQ(encodedLen, 12u + 4u); // header(12) + data("test" = 4 bytes)
}

TEST_F(CodecAvp_test, Flags_PreservedOnDecodeEncode) {
    uint8_t flags = core::AvpFlagMandatory;
    auto raw = buildRawAvp(27, flags, u32bytes(500));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(avp.getFlags(), flags);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded[4], flags);
}

// ============================================================================
// Padding Verification
// ============================================================================

TEST_F(CodecAvp_test, Padding_NoDataNoPadding) {
    // 4-byte data needs no padding (8+4=12, already 4-byte aligned)
    auto raw = buildRawAvp(27, core::AvpFlagMandatory, u32bytes(1));
    // Total should be 12 bytes, no padding
    EXPECT_EQ(raw.size(), 12u);

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 12u);
}

TEST_F(CodecAvp_test, Padding_1ByteData_3BytesPadding) {
    // 1-byte data: header(8) + data(1) = 9 -> padded to 12
    std::vector<uint8_t> data = {0x41}; // 'A'
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);
    EXPECT_EQ(raw.size(), 12u); // 9 padded to 12

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 12u);

    // Verify re-encode produces same padded size
    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded.size(), 12u);
}

TEST_F(CodecAvp_test, Padding_2ByteData_2BytesPadding) {
    // 2-byte data: header(8) + data(2) = 10 -> padded to 12
    std::vector<uint8_t> data = {0x41, 0x42};
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);
    EXPECT_EQ(raw.size(), 12u);

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 12u);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded.size(), 12u);
}

TEST_F(CodecAvp_test, Padding_3ByteData_1BytePadding) {
    // 3-byte data: header(8) + data(3) = 11 -> padded to 12
    std::vector<uint8_t> data = {0x41, 0x42, 0x43};
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);
    EXPECT_EQ(raw.size(), 12u);

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 12u);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded.size(), 12u);
}

TEST_F(CodecAvp_test, Padding_5ByteData_3BytesPadding) {
    // 5-byte data: header(8) + data(5) = 13 -> padded to 16
    std::vector<uint8_t> data = {0x41, 0x42, 0x43, 0x44, 0x45};
    auto raw = buildRawAvp(4, core::AvpFlagMandatory, data);
    EXPECT_EQ(raw.size(), 16u);

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 16u);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded.size(), 16u);
}

TEST_F(CodecAvp_test, Padding_PaddingBytesAreZero) {
    // Verify padding bytes are 0x00
    std::string value = "Hi"; // 2 bytes -> needs 2 bytes padding
    std::vector<uint8_t> data(value.begin(), value.end());
    auto raw = buildRawAvp(1, core::AvpFlagMandatory, data);

    // raw should be 12 bytes: header(8) + data(2) + padding(2)
    ASSERT_EQ(raw.size(), 12u);
    EXPECT_EQ(raw[10], 0x00);
    EXPECT_EQ(raw[11], 0x00);

    // Also verify encode produces zero padding
    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    core::Buffer encoded;
    avp.encode(encoded, dict_);
    ASSERT_EQ(encoded.size(), 12u);
    EXPECT_EQ(encoded[10], 0x00);
    EXPECT_EQ(encoded[11], 0x00);
}

TEST_F(CodecAvp_test, Padding_VendorAvpAlignment) {
    // Vendor AVP with 3-byte data: header(12) + data(3) = 15 -> padded to 16
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto raw = buildRawAvpWithVendor(9999, core::AvpFlagMandatory, 94, data);
    EXPECT_EQ(raw.size(), 16u);

    codec::Avp avp;
    size_t consumed = avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_EQ(consumed, 16u);
}

// ============================================================================
// Additional coverage: getName, getLength
// ============================================================================

TEST_F(CodecAvp_test, GetName_Known) {
    codec::Avp avp = codec::Avp::fromJson("Session-Timeout",
                                           nlohmann::json(100), dict_);
    EXPECT_EQ(avp.getName(dict_), "Session-Timeout");
}

TEST_F(CodecAvp_test, GetName_Unknown) {
    codec::Avp avp;
    avp.setId(core::AvpId{99999, 0});
    avp.setFlags(0);
    avp.setData(std::string("x"));
    EXPECT_EQ(avp.getName(dict_), "avp-99999");
}

TEST_F(CodecAvp_test, GetName_UnknownWithVendor) {
    codec::Avp avp;
    avp.setId(core::AvpId{99999, 12345});
    avp.setFlags(core::AvpFlagVendor);
    avp.setData(std::string("x"));
    EXPECT_EQ(avp.getName(dict_), "avp-99999-12345");
}

TEST_F(CodecAvp_test, GetLength_Unsigned32) {
    codec::Avp avp = codec::Avp::fromJson("Session-Timeout",
                                           nlohmann::json(100), dict_);
    // header(8) + data(4) = 12
    EXPECT_EQ(avp.getLength(dict_), 12u);
}

TEST_F(CodecAvp_test, GetLength_UTF8String) {
    codec::Avp avp = codec::Avp::fromJson("User-Name",
                                           nlohmann::json("hello"), dict_);
    // header(8) + data(5) = 13
    EXPECT_EQ(avp.getLength(dict_), 13u);
}

TEST_F(CodecAvp_test, GetLength_AddressIPv4) {
    codec::Avp avp = codec::Avp::fromJson("Host-IP-Address",
                                           nlohmann::json("1.2.3.4"), dict_);
    // header(8) + family(2) + ipv4(4) = 14
    EXPECT_EQ(avp.getLength(dict_), 14u);
}

TEST_F(CodecAvp_test, GetLength_AddressIPv6) {
    codec::Avp avp = codec::Avp::fromJson("Host-IP-Address",
                                           nlohmann::json("::1"), dict_);
    // header(8) + family(2) + ipv6(16) = 26
    EXPECT_EQ(avp.getLength(dict_), 26u);
}

// ============================================================================
// Float32 / Float64 Tests (custom dictionary AVPs since base lacks them)
// ============================================================================

class CodecAvpFloat_test : public ::testing::Test {
public:
    stack::Dictionary dict_;
    CodecAvpFloat_test() {
        dict_.load(json::stacks::base);
        // Add custom Float32 and Float64 AVPs for testing
        nlohmann::json custom = R"({
            "name": "Custom float dictionary",
            "avp": [
                {
                    "name": "Test-Float32",
                    "code": 60001,
                    "m-bit": true,
                    "single": { "format": "Float32" }
                },
                {
                    "name": "Test-Float64",
                    "code": 60002,
                    "m-bit": true,
                    "single": { "format": "Float64" }
                }
            ]
        })"_json;
        dict_.load(custom);
    }

    // Reuse helpers from CodecAvp_test
    static core::Buffer buildRawAvp(uint32_t code, uint8_t flags,
                                    const std::vector<uint8_t>& data) {
        uint32_t avpLen = 8 + data.size();
        core::Buffer buf;
        buf.push_back(static_cast<uint8_t>(code >> 24));
        buf.push_back(static_cast<uint8_t>(code >> 16));
        buf.push_back(static_cast<uint8_t>(code >> 8));
        buf.push_back(static_cast<uint8_t>(code));
        buf.push_back(flags);
        buf.push_back(static_cast<uint8_t>(avpLen >> 16));
        buf.push_back(static_cast<uint8_t>(avpLen >> 8));
        buf.push_back(static_cast<uint8_t>(avpLen));
        buf.insert(buf.end(), data.begin(), data.end());
        size_t pad = (4 - (avpLen % 4)) % 4;
        for (size_t i = 0; i < pad; ++i) buf.push_back(0);
        return buf;
    }

    static std::vector<uint8_t> f32bytes(float v) {
        uint32_t u;
        std::memcpy(&u, &v, 4);
        return {static_cast<uint8_t>(u >> 24), static_cast<uint8_t>(u >> 16),
                static_cast<uint8_t>(u >> 8), static_cast<uint8_t>(u)};
    }

    static std::vector<uint8_t> f64bytes(double v) {
        uint64_t u;
        std::memcpy(&u, &v, 8);
        std::vector<uint8_t> b(8);
        for (int i = 7; i >= 0; --i) {
            b[i] = static_cast<uint8_t>(u & 0xFF);
            u >>= 8;
        }
        return b;
    }
};

TEST_F(CodecAvpFloat_test, DecodeEncodeRoundtrip_Float32) {
    float value = 3.14f;
    auto raw = buildRawAvp(60001, core::AvpFlagMandatory, f32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_FLOAT_EQ(avp.getFloat32(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

TEST_F(CodecAvpFloat_test, DecodeEncodeRoundtrip_Float32_Negative) {
    float value = -99.5f;
    auto raw = buildRawAvp(60001, core::AvpFlagMandatory, f32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_FLOAT_EQ(avp.getFloat32(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

TEST_F(CodecAvpFloat_test, DecodeEncodeRoundtrip_Float64) {
    double value = 2.718281828459045;
    auto raw = buildRawAvp(60002, core::AvpFlagMandatory, f64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_DOUBLE_EQ(avp.getFloat64(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

TEST_F(CodecAvpFloat_test, DecodeEncodeRoundtrip_Float64_Negative) {
    double value = -1.23456789e+100;
    auto raw = buildRawAvp(60002, core::AvpFlagMandatory, f64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    EXPECT_DOUBLE_EQ(avp.getFloat64(), value);

    core::Buffer encoded;
    avp.encode(encoded, dict_);
    EXPECT_EQ(encoded, raw);
}

TEST_F(CodecAvpFloat_test, ToJson_Float32) {
    float value = 1.5f;
    auto raw = buildRawAvp(60001, core::AvpFlagMandatory, f32bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_FLOAT_EQ(j.get<float>(), value);
}

TEST_F(CodecAvpFloat_test, ToJson_Float64) {
    double value = 9.87654321e+50;
    auto raw = buildRawAvp(60002, core::AvpFlagMandatory, f64bytes(value));

    codec::Avp avp;
    avp.decode(raw.data(), raw.size(), dict_);
    nlohmann::json j = avp.toJson(dict_);
    EXPECT_DOUBLE_EQ(j.get<double>(), value);
}

TEST_F(CodecAvpFloat_test, FromJson_Float32) {
    nlohmann::json value = 2.5f;
    codec::Avp avp = codec::Avp::fromJson("Test-Float32", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{60001, 0}));
    EXPECT_FLOAT_EQ(avp.getFloat32(), 2.5f);
}

TEST_F(CodecAvpFloat_test, FromJson_Float64) {
    nlohmann::json value = 1.234567890123456e+200;
    codec::Avp avp = codec::Avp::fromJson("Test-Float64", value, dict_);
    EXPECT_EQ(avp.getId(), (core::AvpId{60002, 0}));
    EXPECT_DOUBLE_EQ(avp.getFloat64(), 1.234567890123456e+200);
}

TEST_F(CodecAvpFloat_test, JsonRoundtrip_Float32) {
    nlohmann::json input = 42.0f;
    codec::Avp avp1 = codec::Avp::fromJson("Test-Float32", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_FLOAT_EQ(output.get<float>(), input.get<float>());
}

TEST_F(CodecAvpFloat_test, JsonRoundtrip_Float64) {
    nlohmann::json input = 3.141592653589793;
    codec::Avp avp1 = codec::Avp::fromJson("Test-Float64", input, dict_);

    core::Buffer buf;
    avp1.encode(buf, dict_);

    codec::Avp avp2;
    avp2.decode(buf.data(), buf.size(), dict_);
    nlohmann::json output = avp2.toJson(dict_);
    EXPECT_DOUBLE_EQ(output.get<double>(), input.get<double>());
}

TEST_F(CodecAvpFloat_test, Error_Float32_WrongDataLength) {
    std::vector<uint8_t> data = {0x00, 0x01}; // 2 bytes instead of 4
    auto raw = buildRawAvp(60001, core::AvpFlagMandatory, data);

    codec::Avp avp;
    EXPECT_THROW(avp.decode(raw.data(), raw.size(), dict_), std::runtime_error);
}

TEST_F(CodecAvpFloat_test, Error_Float64_WrongDataLength) {
    std::vector<uint8_t> data = {0x00, 0x01, 0x02, 0x03}; // 4 bytes instead of 8
    auto raw = buildRawAvp(60002, core::AvpFlagMandatory, data);

    codec::Avp avp;
    EXPECT_THROW(avp.decode(raw.data(), raw.size(), dict_), std::runtime_error);
}
