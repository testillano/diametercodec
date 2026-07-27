#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ert/diametercodec/codec/Message.hpp>
#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <nlohmann/json.hpp>

class CodecMessage_test : public ::testing::Test {
   public:
    ert::diametercodec::stack::Dictionary dict_;
    CodecMessage_test() { dict_.load(ert::diametercodec::json::stacks::base); }
};

// ============================================================================
// Helper: build a minimal CER JSON for reuse across tests
// ============================================================================
static nlohmann::json buildCerJson() {
    return R"({
        "_header": {
            "version": 1,
            "flags": 192,
            "command-code": 257,
            "request": true,
            "application-id": 0,
            "hop-by-hop-id": 305419896,
            "end-to-end-id": 2271560481
        },
        "Origin-Host": "client.example.com",
        "Origin-Realm": "example.com",
        "Host-IP-Address": "192.168.1.1",
        "Vendor-Id": 0,
        "Product-Name": "TestClient"
    })"_json;
}

// ============================================================================
// 1. Decode/encode roundtrip for a complete Diameter message (CER)
// ============================================================================
TEST_F(CodecMessage_test, decodeEncodeRoundtrip) {
    // Build from JSON, encode, decode, verify fields match
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);
    auto wire = msg.encode(dict_);

    ert::diametercodec::codec::Message decoded;
    decoded.decode(wire.data(), wire.size(), dict_);

    EXPECT_EQ(decoded.getVersion(), msg.getVersion());
    EXPECT_EQ(decoded.getFlags(), msg.getFlags());
    EXPECT_EQ(decoded.getId(), msg.getId());
    EXPECT_EQ(decoded.getApplicationId(), msg.getApplicationId());
    EXPECT_EQ(decoded.getHopByHop(), msg.getHopByHop());
    EXPECT_EQ(decoded.getEndToEnd(), msg.getEndToEnd());
    EXPECT_EQ(decoded.avps().size(), msg.avps().size());
}

// ============================================================================
// 2. Header field accessors
// ============================================================================
TEST_F(CodecMessage_test, headerAccessors) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    EXPECT_EQ(msg.getVersion(), 1);
    // CER: R-bit(0x80) + P-bit(0x40) = 0xC0 = 192
    EXPECT_EQ(msg.getFlags(), 0xC0);
    EXPECT_EQ(msg.getId().first, 257u);
    EXPECT_TRUE(msg.getId().second);
    EXPECT_EQ(msg.getApplicationId(), 0u);
    EXPECT_EQ(msg.getHopByHop(), 305419896u);   // 0x12345678
    EXPECT_EQ(msg.getEndToEnd(), 2271560481u);  // 0x87654321
}

// ============================================================================
// 3. isRequest() flag
// ============================================================================
TEST_F(CodecMessage_test, isRequestTrue) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);
    EXPECT_TRUE(msg.isRequest());
}

TEST_F(CodecMessage_test, isRequestFalse) {
    auto ceaJson = buildCerJson();
    ceaJson["_header"]["request"] = false;
    ceaJson["_header"]["flags"] = 0x40;  // P-bit only, no R-bit
    auto msg = ert::diametercodec::codec::Message::fromJson(ceaJson, dict_);
    EXPECT_FALSE(msg.isRequest());
}

// ============================================================================
// 4. setHeaderToAnswer - clear R flag, copy hbh/e2e
// ============================================================================
TEST_F(CodecMessage_test, setHeaderToAnswer) {
    auto cerJson = buildCerJson();
    auto request = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    ert::diametercodec::codec::Message answer;
    answer.setHeaderToAnswer(request);

    // R flag must be cleared
    EXPECT_FALSE(answer.isRequest());
    EXPECT_EQ(answer.getFlags() & ert::diametercodec::core::MsgFlagRequest, 0);

    // P flag preserved
    EXPECT_NE(answer.getFlags() & ert::diametercodec::core::MsgFlagProxiable, 0);

    // Same command code
    EXPECT_EQ(answer.getId().first, request.getId().first);
    EXPECT_FALSE(answer.getId().second);  // answer

    // hbh and e2e copied
    EXPECT_EQ(answer.getHopByHop(), request.getHopByHop());
    EXPECT_EQ(answer.getEndToEnd(), request.getEndToEnd());

    // Same version and app-id
    EXPECT_EQ(answer.getVersion(), request.getVersion());
    EXPECT_EQ(answer.getApplicationId(), request.getApplicationId());
}

// ============================================================================
// 5. getAvp by id and by name
// ============================================================================
TEST_F(CodecMessage_test, getAvpById) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    // Origin-Host has code 264, vendor 0
    ert::diametercodec::core::AvpId originHostId{264, 0};
    const auto* avp = msg.getAvp(originHostId);
    ASSERT_NE(avp, nullptr);
    EXPECT_EQ(avp->getId(), originHostId);
    EXPECT_EQ(avp->getString(), "client.example.com");
}

TEST_F(CodecMessage_test, getAvpByIdNotFound) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    // Non-existent AVP code
    ert::diametercodec::core::AvpId bogusId{99999, 0};
    const auto* avp = msg.getAvp(bogusId);
    EXPECT_EQ(avp, nullptr);
}

TEST_F(CodecMessage_test, getAvpByName) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    const auto* avp = msg.getAvp("Origin-Realm", dict_);
    ASSERT_NE(avp, nullptr);
    EXPECT_EQ(avp->getString(), "example.com");
}

TEST_F(CodecMessage_test, getAvpByNameNotFound) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    const auto* avp = msg.getAvp("Nonexistent-AVP", dict_);
    EXPECT_EQ(avp, nullptr);
}

// ============================================================================
// 6. addAvp()
// ============================================================================
TEST_F(CodecMessage_test, addAvp) {
    ert::diametercodec::codec::Message msg;
    msg.setVersion(1);
    msg.setFlags(ert::diametercodec::core::MsgFlagRequest);
    msg.setId(ert::diametercodec::core::CommandId(257, true));

    EXPECT_EQ(msg.avps().size(), 0u);

    auto& avp = msg.addAvp();
    avp.setId(ert::diametercodec::core::AvpId{264, 0});
    avp.setFlags(ert::diametercodec::core::AvpFlagMandatory);
    avp.setData(std::string("host.example.com"));

    EXPECT_EQ(msg.avps().size(), 1u);
    EXPECT_EQ(msg.avps()[0].getString(), "host.example.com");

    // Add another
    auto& avp2 = msg.addAvp();
    avp2.setId(ert::diametercodec::core::AvpId{296, 0});
    avp2.setFlags(ert::diametercodec::core::AvpFlagMandatory);
    avp2.setData(std::string("example.com"));

    EXPECT_EQ(msg.avps().size(), 2u);
}

// ============================================================================
// 7. toJson - verify _header and AVP fields
// ============================================================================
TEST_F(CodecMessage_test, toJson) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    auto json = msg.toJson(dict_);

    // Verify _header
    ASSERT_TRUE(json.contains("_header"));
    const auto& hdr = json["_header"];
    EXPECT_EQ(hdr["version"].get<int>(), 1);
    EXPECT_EQ(hdr["command-code"].get<uint32_t>(), 257u);
    EXPECT_EQ(hdr["request"].get<bool>(), true);
    EXPECT_EQ(hdr["application-id"].get<uint32_t>(), 0u);
    EXPECT_EQ(hdr["hop-by-hop-id"].get<uint32_t>(), 305419896u);
    EXPECT_EQ(hdr["end-to-end-id"].get<uint32_t>(), 2271560481u);

    // Verify AVP fields present
    EXPECT_TRUE(json.contains("Origin-Host"));
    EXPECT_TRUE(json.contains("Origin-Realm"));
    EXPECT_TRUE(json.contains("Host-IP-Address"));
    EXPECT_TRUE(json.contains("Vendor-Id"));
    EXPECT_TRUE(json.contains("Product-Name"));
}

// ============================================================================
// 8. fromJson - construct message from JSON
// ============================================================================
TEST_F(CodecMessage_test, fromJson) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    EXPECT_EQ(msg.getVersion(), 1);
    EXPECT_TRUE(msg.isRequest());
    EXPECT_EQ(msg.getId().first, 257u);
    EXPECT_EQ(msg.getApplicationId(), 0u);
    EXPECT_EQ(msg.getHopByHop(), 305419896u);
    EXPECT_EQ(msg.getEndToEnd(), 2271560481u);

    // Must have AVPs
    EXPECT_GE(msg.avps().size(), 5u);

    // Verify Origin-Host AVP content
    const auto* oh = msg.getAvp("Origin-Host", dict_);
    ASSERT_NE(oh, nullptr);
    EXPECT_EQ(oh->getString(), "client.example.com");
}

// ============================================================================
// 9. JSON roundtrip: fromJson -> encode -> decode -> toJson should match
// ============================================================================
TEST_F(CodecMessage_test, jsonRoundtrip) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    // Encode to binary
    auto wire = msg.encode(dict_);

    // Decode from binary
    ert::diametercodec::codec::Message decoded;
    decoded.decode(wire.data(), wire.size(), dict_);

    // Convert both to JSON
    auto jsonOrig = msg.toJson(dict_);
    auto jsonDecoded = decoded.toJson(dict_);

    // Headers must match
    EXPECT_EQ(jsonOrig["_header"], jsonDecoded["_header"]);

    // AVP names must all be present
    EXPECT_EQ(jsonOrig.contains("Origin-Host"), jsonDecoded.contains("Origin-Host"));
    EXPECT_EQ(jsonOrig.contains("Origin-Realm"), jsonDecoded.contains("Origin-Realm"));
    EXPECT_EQ(jsonOrig.contains("Host-IP-Address"), jsonDecoded.contains("Host-IP-Address"));
    EXPECT_EQ(jsonOrig.contains("Vendor-Id"), jsonDecoded.contains("Vendor-Id"));
    EXPECT_EQ(jsonOrig.contains("Product-Name"), jsonDecoded.contains("Product-Name"));

    // Verify string AVPs roundtrip exactly
    EXPECT_EQ(jsonOrig["Origin-Host"], jsonDecoded["Origin-Host"]);
    EXPECT_EQ(jsonOrig["Origin-Realm"], jsonDecoded["Origin-Realm"]);
    EXPECT_EQ(jsonOrig["Product-Name"], jsonDecoded["Product-Name"]);
}

// ============================================================================
// 10. Error cases: buffer too short for header
// ============================================================================
TEST_F(CodecMessage_test, decodeTooShortForHeader) {
    // Less than 20 bytes
    std::vector<uint8_t> shortBuf(10, 0);
    ert::diametercodec::codec::Message msg;
    EXPECT_THROW(msg.decode(shortBuf.data(), shortBuf.size(), dict_), std::runtime_error);
}

TEST_F(CodecMessage_test, decodeTooShortForHeaderExactly19) {
    std::vector<uint8_t> buf(19, 0);
    ert::diametercodec::codec::Message msg;
    EXPECT_THROW(msg.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

TEST_F(CodecMessage_test, decodeMessageLengthExceedingBuffer) {
    // Build a valid 20-byte header but message length says 100 bytes
    std::vector<uint8_t> buf(20, 0);
    buf[0] = 1;  // version
    // message length = 100 in bytes [1..3]
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 100;
    // flags + code
    buf[4] = 0x80;  // R-bit
    buf[5] = 0;
    buf[6] = 1;
    buf[7] = 1;  // command code = 257
    // rest is zeros (app-id, hbh, e2e)

    ert::diametercodec::codec::Message msg;
    EXPECT_THROW(msg.decode(buf.data(), buf.size(), dict_), std::runtime_error);
}

// ============================================================================
// 11. Multiple AVPs with same code (should become array in JSON)
// ============================================================================
TEST_F(CodecMessage_test, multipleAvpsSameCodeBecomeArray) {
    // Build a message with two Route-Record AVPs (code 282)
    nlohmann::json j = R"({
        "_header": {
            "version": 1,
            "flags": 128,
            "command-code": 257,
            "request": true,
            "application-id": 0,
            "hop-by-hop-id": 1,
            "end-to-end-id": 2
        },
        "Origin-Host": "host.example.com",
        "Origin-Realm": "example.com",
        "Host-IP-Address": "10.0.0.1",
        "Vendor-Id": 0,
        "Product-Name": "test",
        "Route-Record": [
            "peer1.example.com",
            "peer2.example.com"
        ]
    })"_json;

    auto msg = ert::diametercodec::codec::Message::fromJson(j, dict_);

    // Encode and decode
    auto wire = msg.encode(dict_);
    ert::diametercodec::codec::Message decoded;
    decoded.decode(wire.data(), wire.size(), dict_);

    auto json = decoded.toJson(dict_);

    // Route-Record should be an array
    ASSERT_TRUE(json.contains("Route-Record"));
    EXPECT_TRUE(json["Route-Record"].is_array());
    EXPECT_EQ(json["Route-Record"].size(), 2u);
    EXPECT_EQ(json["Route-Record"][0].get<std::string>(), "peer1.example.com");
    EXPECT_EQ(json["Route-Record"][1].get<std::string>(), "peer2.example.com");
}

// ============================================================================
// 12. getLength() computation
// ============================================================================
TEST_F(CodecMessage_test, getLength) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);

    size_t computedLen = msg.getLength(dict_);

    // Encode and compare with actual wire size
    auto wire = msg.encode(dict_);
    EXPECT_EQ(computedLen, wire.size());
}

TEST_F(CodecMessage_test, getLengthEmptyMessage) {
    ert::diametercodec::codec::Message msg;
    msg.setVersion(1);
    msg.setFlags(0);
    msg.setId(ert::diametercodec::core::CommandId(280, true));

    // Empty message: just the header (20 bytes)
    EXPECT_EQ(msg.getLength(dict_), 20u);
}

// ============================================================================
// Additional: header setters
// ============================================================================
TEST_F(CodecMessage_test, headerSetters) {
    ert::diametercodec::codec::Message msg;

    msg.setVersion(1);
    msg.setFlags(0xC0);
    msg.setId(ert::diametercodec::core::CommandId(257, true));
    msg.setApplicationId(42);
    msg.setHopByHop(0xAABBCCDD);
    msg.setEndToEnd(0x11223344);

    EXPECT_EQ(msg.getVersion(), 1);
    EXPECT_EQ(msg.getFlags(), 0xC0);
    EXPECT_EQ(msg.getId().first, 257u);
    EXPECT_TRUE(msg.getId().second);
    EXPECT_EQ(msg.getApplicationId(), 42u);
    EXPECT_EQ(msg.getHopByHop(), 0xAABBCCDD);
    EXPECT_EQ(msg.getEndToEnd(), 0x11223344u);
}

// ============================================================================
// Additional: decode valid minimal message (header only, no AVPs)
// ============================================================================
TEST_F(CodecMessage_test, decodeMinimalHeaderOnly) {
    // 20-byte message with length=20 (no AVPs)
    std::vector<uint8_t> buf(20, 0);
    buf[0] = 1;     // version
    buf[1] = 0;     // length high byte
    buf[2] = 0;     // length mid byte
    buf[3] = 20;    // length low byte = 20
    buf[4] = 0x80;  // R-bit set
    buf[5] = 0;
    buf[6] = 1;
    buf[7] = 24;  // command code = 280 (DWR) -> 0x000118
    // Actually 280 = 0x118 -> buf[5]=0, buf[6]=1, buf[7]=0x18
    buf[5] = 0;
    buf[6] = 1;
    buf[7] = 0x18;
    // app-id = 0
    // hbh = 0x12345678
    buf[12] = 0x12;
    buf[13] = 0x34;
    buf[14] = 0x56;
    buf[15] = 0x78;
    // e2e = 0xDEADBEEF
    buf[16] = 0xDE;
    buf[17] = 0xAD;
    buf[18] = 0xBE;
    buf[19] = 0xEF;

    ert::diametercodec::codec::Message msg;
    EXPECT_NO_THROW(msg.decode(buf.data(), buf.size(), dict_));

    EXPECT_EQ(msg.getVersion(), 1);
    EXPECT_TRUE(msg.isRequest());
    EXPECT_EQ(msg.getId().first, 280u);
    EXPECT_EQ(msg.getApplicationId(), 0u);
    EXPECT_EQ(msg.getHopByHop(), 0x12345678u);
    EXPECT_EQ(msg.getEndToEnd(), 0xDEADBEEFu);
    EXPECT_EQ(msg.avps().size(), 0u);
}

// ============================================================================
// Additional: encode preserves header fields in binary
// ============================================================================
TEST_F(CodecMessage_test, encodePreservesHeaderFields) {
    ert::diametercodec::codec::Message msg;
    msg.setVersion(1);
    msg.setFlags(0xC0);
    msg.setId(ert::diametercodec::core::CommandId(257, true));
    msg.setApplicationId(0);
    msg.setHopByHop(0x12345678);
    msg.setEndToEnd(0x87654321);

    auto wire = msg.encode(dict_);
    ASSERT_GE(wire.size(), 20u);

    // version
    EXPECT_EQ(wire[0], 1);
    // flags
    EXPECT_EQ(wire[4], 0xC0);
    // command code 257 = 0x000101
    EXPECT_EQ(wire[5], 0x00);
    EXPECT_EQ(wire[6], 0x01);
    EXPECT_EQ(wire[7], 0x01);
    // app-id = 0
    EXPECT_EQ(wire[8], 0);
    EXPECT_EQ(wire[9], 0);
    EXPECT_EQ(wire[10], 0);
    EXPECT_EQ(wire[11], 0);
    // hbh = 0x12345678
    EXPECT_EQ(wire[12], 0x12);
    EXPECT_EQ(wire[13], 0x34);
    EXPECT_EQ(wire[14], 0x56);
    EXPECT_EQ(wire[15], 0x78);
    // e2e = 0x87654321
    EXPECT_EQ(wire[16], 0x87);
    EXPECT_EQ(wire[17], 0x65);
    EXPECT_EQ(wire[18], 0x43);
    EXPECT_EQ(wire[19], 0x21);
}

// ============================================================================
// Additional: message length field in wire format matches total size
// ============================================================================
TEST_F(CodecMessage_test, wireLengthFieldMatchesBufferSize) {
    auto cerJson = buildCerJson();
    auto msg = ert::diametercodec::codec::Message::fromJson(cerJson, dict_);
    auto wire = msg.encode(dict_);

    // Extract length from wire bytes [1..3]
    uint32_t wireLen = (uint32_t(wire[1]) << 16) | (uint32_t(wire[2]) << 8) | uint32_t(wire[3]);
    EXPECT_EQ(wireLen, wire.size());
}

// ============================================================================
// Additional: default-constructed message
// ============================================================================
TEST_F(CodecMessage_test, defaultConstruction) {
    ert::diametercodec::codec::Message msg;
    EXPECT_EQ(msg.getVersion(), 1);
    EXPECT_EQ(msg.getFlags(), 0);
    EXPECT_FALSE(msg.isRequest());
    EXPECT_EQ(msg.getApplicationId(), 0u);
    EXPECT_EQ(msg.getHopByHop(), 0u);
    EXPECT_EQ(msg.getEndToEnd(), 0u);
    EXPECT_EQ(msg.avps().size(), 0u);
}
