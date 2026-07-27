/*
 ________________________________________________________________________
|                                                                        |
|       _ _                      _                         _             |
|      | (_)                    | |                       | |            |
|    __| |_  __ _ _ __ ___   ___| |_ ___ _ __ ___ ___   __| | ___  ___   |
|   / _` | |/ _` | '_ ` _ \ / _ \ __/ _ \ '__/ __/ _ \ / _` |/ _ \/ __|  |
|  | (_| | | (_| | | | | | |  __/ ||  __/ | | (_| (_) | (_| |  __/ (__   |
|   \__,_|_|\__,_|_| |_| |_|\___|\__\___|_|  \___\___/ \__,_|\___|\___|  |
|                                                                        |
|________________________________________________________________________|

C++ CODEC FOR DIAMETER PROTOCOL (RFC 6733)
Version 0.0.z
https://github.com/testillano/diametercodec

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2021 Eduardo Ramos

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Standard
#include <arpa/inet.h>

#include <cstring>
#include <iomanip>
#include <sstream>

// Project
#include <ert/diametercodec/codec/Avp.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/Format.hpp>

namespace ert {
namespace diametercodec {
namespace codec {

// ============================================================================
// Helpers: network byte order encode/decode
// ============================================================================

namespace {

inline uint32_t decode4(const uint8_t* b) {
    return (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) | (uint32_t(b[2]) << 8) | uint32_t(b[3]);
}

inline uint32_t decode3(const uint8_t* b) { return (uint32_t(b[0]) << 16) | (uint32_t(b[1]) << 8) | uint32_t(b[2]); }

inline uint16_t decode2(const uint8_t* b) { return (uint16_t(b[0]) << 8) | uint16_t(b[1]); }

inline int32_t decodeSigned4(const uint8_t* b) {
    uint32_t u = decode4(b);
    int32_t v;
    std::memcpy(&v, &u, 4);
    return v;
}

inline int64_t decodeSigned8(const uint8_t* b) {
    uint64_t u = (uint64_t(b[0]) << 56) | (uint64_t(b[1]) << 48) | (uint64_t(b[2]) << 40) | (uint64_t(b[3]) << 32) |
                 (uint64_t(b[4]) << 24) | (uint64_t(b[5]) << 16) | (uint64_t(b[6]) << 8) | uint64_t(b[7]);
    int64_t v;
    std::memcpy(&v, &u, 8);
    return v;
}

inline uint64_t decodeUnsigned8(const uint8_t* b) {
    return (uint64_t(b[0]) << 56) | (uint64_t(b[1]) << 48) | (uint64_t(b[2]) << 40) | (uint64_t(b[3]) << 32) |
           (uint64_t(b[4]) << 24) | (uint64_t(b[5]) << 16) | (uint64_t(b[6]) << 8) | uint64_t(b[7]);
}

inline void encode4(core::Buffer& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>(v >> 24));
    out.push_back(static_cast<uint8_t>(v >> 16));
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v));
}

inline void encode3(core::Buffer& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>(v >> 16));
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v));
}

inline void encode2(core::Buffer& out, uint16_t v) {
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v));
}

inline void encode8(core::Buffer& out, uint64_t v) {
    for (int i = 56; i >= 0; i -= 8) out.push_back(static_cast<uint8_t>(v >> i));
}

inline void encodePadding(core::Buffer& out, size_t dataLen) {
    size_t pad = (4 - (dataLen % 4)) % 4;
    for (size_t i = 0; i < pad; ++i) out.push_back(0);
}

// Resolve the format type for an AVP using the dictionary.
// Returns the direct type (e.g., UTF8String, Address, Time) rather than
// recursing to the parent basic type. This allows the codec to handle
// derived types with specialized encode/decode/json logic.
stack::Format::Type::_v resolveFormatType(const core::AvpId& id, const stack::Dictionary& dict) {
    const stack::Avp* sa = dict.getAvp(id);
    if (!sa) return stack::Format::Type::Unknown;
    const stack::Format* fmt = sa->getFormat();
    if (!fmt || fmt->isReserved()) return stack::Format::Type::Unknown;
    return stack::Format::Type::asEnum(fmt->getName());
}

// Hex string helpers
std::string toHex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) oss << std::setw(2) << static_cast<unsigned>(data[i]);
    return oss.str();
}

core::Buffer fromHex(const std::string& hex) {
    core::Buffer buf;
    buf.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        uint8_t byte = static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16));
        buf.push_back(byte);
    }
    return buf;
}

// Address helpers
std::string decodeAddress(const uint8_t* buf, size_t len) {
    if (len < 2) throw std::runtime_error("Address AVP too short");
    uint16_t family = decode2(buf);
    char str[INET6_ADDRSTRLEN];

    if (family == 1 && len == 6) {  // IPv4
        if (inet_ntop(AF_INET, buf + 2, str, sizeof(str))) return str;
        throw std::runtime_error("Failed to decode IPv4 address");
    }
    if (family == 2 && len == 18) {  // IPv6
        if (inet_ntop(AF_INET6, buf + 2, str, sizeof(str))) return str;
        throw std::runtime_error("Failed to decode IPv6 address");
    }
    // Unknown family: return as hex
    return toHex(buf, len);
}

void encodeAddress(core::Buffer& out, const std::string& addr) {
    uint8_t buf4[4], buf6[16];
    if (inet_pton(AF_INET, addr.c_str(), buf4) == 1) {
        encode2(out, 1);  // IPv4 family
        out.insert(out.end(), buf4, buf4 + 4);
    } else if (inet_pton(AF_INET6, addr.c_str(), buf6) == 1) {
        encode2(out, 2);  // IPv6 family
        out.insert(out.end(), buf6, buf6 + 16);
    } else {
        throw std::runtime_error("Invalid IP address: " + addr);
    }
}

}  // anonymous namespace

// ============================================================================
// Avp::decode
// ============================================================================
size_t Avp::decode(const uint8_t* buf, size_t len, const stack::Dictionary& dict) {
    if (len < static_cast<size_t>(core::AvpHeaderLenWithoutVendor))
        throw std::runtime_error("Not enough bytes for AVP header");

    // AVP Code (4 bytes)
    uint32_t code = decode4(buf);
    // Flags (1 byte)
    flags_ = buf[4];
    // AVP Length (3 bytes) — includes header, excludes padding
    uint32_t avpLen = decode3(buf + 5);

    // Vendor-ID
    uint32_t vendorId = 0;
    int headerLen = core::AvpHeaderLenWithoutVendor;
    if (vendorBit()) {
        headerLen = core::AvpHeaderLenWithVendor;
        if (len < static_cast<size_t>(headerLen))
            throw std::runtime_error("Not enough bytes for AVP header with vendor");
        vendorId = decode4(buf + 8);
    }

    id_ = core::AvpId(static_cast<core::S32>(code), static_cast<core::S32>(vendorId));

    if (avpLen < static_cast<uint32_t>(headerLen)) throw std::runtime_error("AVP length smaller than header");

    uint32_t dataLen = avpLen - headerLen;

    if (len < avpLen) throw std::runtime_error("Not enough bytes for AVP data");

    // Decode data part
    decodeData(buf + headerLen, dataLen, dict);

    // Return total consumed bytes (padded to 4-byte boundary)
    return 4 * REQUIRED_WORDS(avpLen);
}

// ============================================================================
// Avp::decodeData
// ============================================================================
void Avp::decodeData(const uint8_t* buf, size_t dataLen, const stack::Dictionary& dict) {
    auto basicType = resolveFormatType(id_, dict);

    switch (basicType) {
        case stack::Format::Type::Integer32:
        case stack::Format::Type::Enumerated:
            if (dataLen != 4) throw std::runtime_error("Integer32/Enumerated must be 4 bytes");
            data_ = decodeSigned4(buf);
            break;

        case stack::Format::Type::Integer64:
            if (dataLen != 8) throw std::runtime_error("Integer64 must be 8 bytes");
            data_ = decodeSigned8(buf);
            break;

        case stack::Format::Type::Unsigned32:
            if (dataLen != 4) throw std::runtime_error("Unsigned32 must be 4 bytes");
            data_ = decode4(buf);
            break;

        case stack::Format::Type::Unsigned64:
            if (dataLen != 8) throw std::runtime_error("Unsigned64 must be 8 bytes");
            data_ = decodeUnsigned8(buf);
            break;

        case stack::Format::Type::Float32: {
            if (dataLen != 4) throw std::runtime_error("Float32 must be 4 bytes");
            uint32_t u = decode4(buf);
            float f;
            std::memcpy(&f, &u, 4);
            data_ = f;
            break;
        }

        case stack::Format::Type::Float64: {
            if (dataLen != 8) throw std::runtime_error("Float64 must be 8 bytes");
            uint64_t u = decodeUnsigned8(buf);
            double d;
            std::memcpy(&d, &u, 8);
            data_ = d;
            break;
        }

        case stack::Format::Type::Address:
            data_ = decodeAddress(buf, dataLen);
            break;

        case stack::Format::Type::Time:
            if (dataLen != 4) throw std::runtime_error("Time must be 4 bytes");
            data_ = decode4(buf);  // stored as uint32_t (NTP timestamp)
            break;

        case stack::Format::Type::Grouped: {
            std::vector<Avp> children;
            size_t pos = 0;
            while (pos < dataLen) {
                Avp child;
                size_t consumed = child.decode(buf + pos, dataLen - pos, dict);
                children.push_back(std::move(child));
                pos += consumed;
            }
            data_ = std::move(children);
            break;
        }

        // OctetString, UTF8String, DiameterIdentity, DiameterURI, IPFilterRule, QoSFilterRule, Unknown
        case stack::Format::Type::OctetString:
        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
        default:
            data_ = std::string(reinterpret_cast<const char*>(buf), dataLen);
            break;
    }
}

// ============================================================================
// Avp::encode
// ============================================================================
void Avp::encode(core::Buffer& out, const stack::Dictionary& dict) const {
    // AVP Code
    encode4(out, static_cast<uint32_t>(id_.first));

    // Flags
    out.push_back(flags_);

    // Length placeholder (3 bytes) — filled after encoding data
    size_t lenPos = out.size();
    out.push_back(0);
    out.push_back(0);
    out.push_back(0);

    // Vendor-ID
    if (vendorBit()) {
        encode4(out, static_cast<uint32_t>(id_.second));
    }

    // Data part
    size_t dataStart = out.size();
    encodeData(out, dict);
    size_t dataEnd = out.size();

    // Compute and write AVP length (header + data, no padding)
    int headerLen = vendorBit() ? core::AvpHeaderLenWithVendor : core::AvpHeaderLenWithoutVendor;
    uint32_t avpLen = headerLen + static_cast<uint32_t>(dataEnd - dataStart);
    out[lenPos] = static_cast<uint8_t>(avpLen >> 16);
    out[lenPos + 1] = static_cast<uint8_t>(avpLen >> 8);
    out[lenPos + 2] = static_cast<uint8_t>(avpLen);

    // Padding to 4-byte boundary
    encodePadding(out, avpLen);
}

// ============================================================================
// Avp::encodeData
// ============================================================================
size_t Avp::encodeData(core::Buffer& out, const stack::Dictionary& dict) const {
    size_t start = out.size();
    auto basicType = resolveFormatType(id_, dict);

    switch (basicType) {
        case stack::Format::Type::Integer32:
        case stack::Format::Type::Enumerated: {
            int32_t v = std::get<int32_t>(data_);
            uint32_t u;
            std::memcpy(&u, &v, 4);
            encode4(out, u);
            break;
        }

        case stack::Format::Type::Integer64: {
            int64_t v = std::get<int64_t>(data_);
            uint64_t u;
            std::memcpy(&u, &v, 8);
            encode8(out, u);
            break;
        }

        case stack::Format::Type::Unsigned32:
            encode4(out, std::get<uint32_t>(data_));
            break;

        case stack::Format::Type::Unsigned64:
            encode8(out, std::get<uint64_t>(data_));
            break;

        case stack::Format::Type::Float32: {
            float f = std::get<float>(data_);
            uint32_t u;
            std::memcpy(&u, &f, 4);
            encode4(out, u);
            break;
        }

        case stack::Format::Type::Float64: {
            double d = std::get<double>(data_);
            uint64_t u;
            std::memcpy(&u, &d, 8);
            encode8(out, u);
            break;
        }

        case stack::Format::Type::Address:
            encodeAddress(out, std::get<std::string>(data_));
            break;

        case stack::Format::Type::Time:
            encode4(out, std::get<uint32_t>(data_));
            break;

        case stack::Format::Type::Grouped: {
            const auto& children = std::get<std::vector<Avp>>(data_);
            for (const auto& child : children) child.encode(out, dict);
            break;
        }

        // OctetString and all string-based types, including Unknown
        case stack::Format::Type::OctetString:
        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
        default: {
            const auto& s = std::get<std::string>(data_);
            out.insert(out.end(), reinterpret_cast<const uint8_t*>(s.data()),
                       reinterpret_cast<const uint8_t*>(s.data()) + s.size());
            break;
        }
    }

    return out.size() - start;
}

// ============================================================================
// Avp::getLength
// ============================================================================
size_t Avp::getLength(const stack::Dictionary& dict) const {
    int headerLen = vendorBit() ? core::AvpHeaderLenWithVendor : core::AvpHeaderLenWithoutVendor;
    auto basicType = resolveFormatType(id_, dict);

    switch (basicType) {
        case stack::Format::Type::Integer32:
        case stack::Format::Type::Enumerated:
        case stack::Format::Type::Unsigned32:
        case stack::Format::Type::Float32:
        case stack::Format::Type::Time:
            return headerLen + 4;

        case stack::Format::Type::Integer64:
        case stack::Format::Type::Unsigned64:
        case stack::Format::Type::Float64:
            return headerLen + 8;

        case stack::Format::Type::Address: {
            const auto& addr = std::get<std::string>(data_);
            uint8_t buf4[4];
            if (inet_pton(AF_INET, addr.c_str(), buf4) == 1) return headerLen + 6;  // 2 family + 4 addr
            return headerLen + 18;                                                  // 2 family + 16 addr
        }

        case stack::Format::Type::Grouped: {
            size_t total = headerLen;
            const auto& children = std::get<std::vector<Avp>>(data_);
            for (const auto& child : children) total += 4 * REQUIRED_WORDS(child.getLength(dict));
            return total;
        }

        // OctetString and all string-based types
        case stack::Format::Type::OctetString:
        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
        default:
            return headerLen + std::get<std::string>(data_).size();
    }
}

// ============================================================================
// Avp::getName
// ============================================================================
std::string Avp::getName(const stack::Dictionary& dict) const {
    const stack::Avp* sa = dict.getAvp(id_);
    if (sa) return sa->getName();
    // Unknown AVP: use code-vendor format
    std::ostringstream oss;
    oss << "avp-" << id_.first;
    if (id_.second != 0) oss << "-" << id_.second;
    return oss.str();
}

// ============================================================================
// Avp::toJson
// ============================================================================
nlohmann::json Avp::toJson(const stack::Dictionary& dict) const { return dataToJson(dict); }

nlohmann::json Avp::dataToJson(const stack::Dictionary& dict) const {
    auto basicType = resolveFormatType(id_, dict);

    switch (basicType) {
        case stack::Format::Type::Integer32:
        case stack::Format::Type::Enumerated:
            return std::get<int32_t>(data_);

        case stack::Format::Type::Integer64:
            return std::get<int64_t>(data_);

        case stack::Format::Type::Unsigned32:
            return std::get<uint32_t>(data_);

        case stack::Format::Type::Unsigned64:
            return std::get<uint64_t>(data_);

        case stack::Format::Type::Float32:
            return std::get<float>(data_);

        case stack::Format::Type::Float64:
            return std::get<double>(data_);

        case stack::Format::Type::Address:
            return std::get<std::string>(data_);

        case stack::Format::Type::Time:
            // Convert NTP to UNIX epoch
            return std::get<uint32_t>(data_) - core::NtpEpochOffset;

        case stack::Format::Type::Grouped: {
            nlohmann::json obj = nlohmann::json::object();
            const auto& children = std::get<std::vector<Avp>>(data_);
            for (const auto& child : children) {
                std::string name = child.getName(dict);
                nlohmann::json val = child.toJson(dict);
                if (obj.contains(name)) {
                    // Convert to array for repeated AVPs
                    if (!obj[name].is_array()) {
                        nlohmann::json arr = nlohmann::json::array();
                        arr.push_back(std::move(obj[name]));
                        obj[name] = std::move(arr);
                    }
                    obj[name].push_back(std::move(val));
                } else {
                    obj[name] = std::move(val);
                }
            }
            return obj;
        }

        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
            return std::get<std::string>(data_);

        // OctetString or unknown: hex representation
        default: {
            const auto& s = std::get<std::string>(data_);
            return toHex(reinterpret_cast<const uint8_t*>(s.data()), s.size());
        }
    }
}

// ============================================================================
// Avp::fromJson
// ============================================================================
Avp Avp::fromJson(const std::string& avpName, const nlohmann::json& value, const stack::Dictionary& dict) {
    Avp avp;

    // Resolve AVP id from name
    const stack::Avp* sa = dict.getAvp(avpName);
    if (!sa) throw std::runtime_error("Unknown AVP name: " + avpName);

    avp.id_ = sa->getId();

    // Set flags from dictionary
    avp.flags_ = 0;
    if (sa->vBit()) avp.flags_ |= core::AvpFlagVendor;
    if (sa->mBit()) avp.flags_ |= core::AvpFlagMandatory;

    const stack::Format* fmt = sa->getFormat();
    if (!fmt || fmt->isReserved()) throw std::runtime_error("No format for AVP: " + avpName);

    auto basicType = stack::Format::Type::asEnum(fmt->getName());

    switch (basicType) {
        case stack::Format::Type::Integer32:
        case stack::Format::Type::Enumerated:
            avp.data_ = value.get<int32_t>();
            break;

        case stack::Format::Type::Integer64:
            avp.data_ = value.get<int64_t>();
            break;

        case stack::Format::Type::Unsigned32:
            avp.data_ = value.get<uint32_t>();
            break;

        case stack::Format::Type::Unsigned64:
            avp.data_ = value.get<uint64_t>();
            break;

        case stack::Format::Type::Float32:
            avp.data_ = value.get<float>();
            break;

        case stack::Format::Type::Float64:
            avp.data_ = value.get<double>();
            break;

        case stack::Format::Type::Address:
            avp.data_ = value.get<std::string>();
            break;

        case stack::Format::Type::Time:
            // JSON has UNIX epoch, convert to NTP
            avp.data_ = value.get<uint32_t>() + core::NtpEpochOffset;
            break;

        case stack::Format::Type::Grouped: {
            std::vector<Avp> children;
            for (auto& [key, val] : value.items()) {
                if (val.is_array()) {
                    for (const auto& elem : val) children.push_back(Avp::fromJson(key, elem, dict));
                } else {
                    children.push_back(Avp::fromJson(key, val, dict));
                }
            }
            avp.data_ = std::move(children);
            break;
        }

        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
            avp.data_ = value.get<std::string>();
            break;

        // OctetString: hex string → raw bytes
        default: {
            auto hex = value.get<std::string>();
            auto raw = fromHex(hex);
            avp.data_ = std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
            break;
        }
    }

    return avp;
}

}  // namespace codec
}  // namespace diametercodec
}  // namespace ert
