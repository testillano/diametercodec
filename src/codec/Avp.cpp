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

#include <cctype>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>

// Project
#include <algorithm>
#include <ert/diametercodec/codec/Avp.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <ert/diametercodec/stack/AvpRule.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/Format.hpp>
#include <ert/tracing/Logger.hpp>

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
    if (hex.size() % 2 != 0) throw std::runtime_error("Invalid hex string (odd number of digits): '" + hex + "'");
    core::Buffer buf;
    buf.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        if (!std::isxdigit(static_cast<unsigned char>(hex[i])) ||
            !std::isxdigit(static_cast<unsigned char>(hex[i + 1])))
            throw std::runtime_error("Invalid hex string (non-hex digit): '" + hex + "'");
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

namespace {
// Readable form of an OctetString value (defined below, with the encoders).
nlohmann::json readableOctets(const std::string& bytes, const std::string& form);
}  // anonymous namespace

// Forward declaration (defined below, before Avp::toJson)
static std::string avpValueTrace(const core::AvpId& id, const Avp::Data& data, const stack::Dictionary& dict);

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

    LOGDEBUG(ert::tracing::Logger::debug(
        ert::tracing::Logger::asString("Diameter decode AVP '%s' (%s): 0x%s -> %s", getName(dict).c_str(),
                                       stack::Format::Type::asText(basicType), toHex(buf, dataLen).c_str(),
                                       avpValueTrace(id_, data_, dict).c_str()),
        ERT_FILE_LOCATION));
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
    if (raw_) {
        // Negative testing: verbatim bytes, whatever the dictionary format.
        const auto& s = std::get<std::string>(data_);
        out.insert(out.end(), reinterpret_cast<const uint8_t*>(s.data()),
                   reinterpret_cast<const uint8_t*>(s.data()) + s.size());
        LOGDEBUG(ert::tracing::Logger::debug(
            ert::tracing::Logger::asString("Diameter encode AVP '%s' (raw): 0x%s", getName(dict).c_str(),
                                           toHex(out.data() + start, out.size() - start).c_str()),
            ERT_FILE_LOCATION));
        return out.size() - start;
    }
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

    LOGDEBUG(ert::tracing::Logger::debug(
        ert::tracing::Logger::asString("Diameter encode AVP '%s' (%s): %s -> 0x%s", getName(dict).c_str(),
                                       stack::Format::Type::asText(basicType), avpValueTrace(id_, data_, dict).c_str(),
                                       toHex(out.data() + start, out.size() - start).c_str()),
        ERT_FILE_LOCATION));

    return out.size() - start;
}

// ============================================================================
// Avp::getLength
// ============================================================================
size_t Avp::getLength(const stack::Dictionary& dict) const {
    int headerLen = vendorBit() ? core::AvpHeaderLenWithVendor : core::AvpHeaderLenWithoutVendor;
    if (raw_) return headerLen + std::get<std::string>(data_).size();
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
    const stack::Avp* sa = raw_ ? nullptr : dict.getAvp(id_);
    if (sa) return sa->getName();
    // Unknown AVP: use code-vendor format
    std::ostringstream oss;
    oss << "avp-" << id_.first;
    if (id_.second != 0) oss << "-" << id_.second;
    return oss.str();
}

// ============================================================================
// avpValueTrace: human-readable rendering of an AVP value for debug traces.
// For Enumerated it appends the dictionary alias/literal when available.
// ============================================================================
static std::string avpValueTrace(const core::AvpId& id, const Avp::Data& data, const stack::Dictionary& dict) {
    auto t = resolveFormatType(id, dict);
    switch (t) {
        case stack::Format::Type::Integer32:
            return std::to_string(std::get<int32_t>(data));
        case stack::Format::Type::Enumerated: {
            int32_t v = std::get<int32_t>(data);
            std::string s = std::to_string(v);
            const stack::Avp* sa = dict.getAvp(id);
            const char* alias = sa ? sa->getAlias(std::to_string(v)) : nullptr;
            if (alias) {
                s += " (";
                s += alias;
                s += ")";
            }
            return s;
        }
        case stack::Format::Type::Integer64:
            return std::to_string(std::get<int64_t>(data));
        case stack::Format::Type::Unsigned32:
            return std::to_string(std::get<uint32_t>(data));
        case stack::Format::Type::Unsigned64:
            return std::to_string(std::get<uint64_t>(data));
        case stack::Format::Type::Float32:
            return std::to_string(std::get<float>(data));
        case stack::Format::Type::Float64:
            return std::to_string(std::get<double>(data));
        case stack::Format::Type::Address:
            return std::get<std::string>(data);
        case stack::Format::Type::Time:
            return std::to_string(std::get<uint32_t>(data) - core::NtpEpochOffset) + " (epoch)";
        case stack::Format::Type::Grouped:
            return "<grouped>";
        case stack::Format::Type::UTF8String:
        case stack::Format::Type::DiameterIdentity:
        case stack::Format::Type::DiameterURI:
        case stack::Format::Type::IPFilterRule:
        case stack::Format::Type::QoSFilterRule:
            return "\"" + std::get<std::string>(data) + "\"";
        default: {  // OctetString / Unknown
            const auto& s = std::get<std::string>(data);
            return "0x" + toHex(reinterpret_cast<const uint8_t*>(s.data()), s.size());
        }
    }
}

// ============================================================================
// Avp::toJson
// ============================================================================
nlohmann::json Avp::toJson(const stack::Dictionary& dict) const { return dataToJson(dict, nullptr); }

nlohmann::json Avp::toJson(const stack::Dictionary& dict, const DecodeOptions& options) const {
    return dataToJson(dict, options.empty() ? nullptr : &options);
}

nlohmann::json Avp::dataToJson(const stack::Dictionary& dict, const DecodeOptions* options) const {
    if (raw_) {
        const auto& s = std::get<std::string>(data_);
        return toHex(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    }
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
                nlohmann::json val = child.dataToJson(dict, options);
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

        // OctetString or unknown: hex representation, or the configured readable form
        default: {
            const auto& s = std::get<std::string>(data_);
            if (options) {
                auto it = options->forms.find(id_);
                if (it != options->forms.end()) return readableOctets(s, it->second);
            }
            return toHex(reinterpret_cast<const uint8_t*>(s.data()), s.size());
        }
    }
}

// ============================================================================
// Avp::fromJson
// ============================================================================
namespace {

// Parse a decimal component of a raw AVP key (strict: digits only, fits 32 bits).
bool parseRawKeyNumber(const std::string& s, uint32_t& out) {
    if (s.empty() || s.size() > 10) return false;
    uint64_t v = 0;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        v = v * 10 + static_cast<uint64_t>(c - '0');
    }
    if (v > 0xFFFFFFFFull) return false;
    out = static_cast<uint32_t>(v);
    return true;
}

// 'avp-<code>' or 'avp-<code>-<vendor>' -> (code, vendor). False if not that form.
bool parseRawAvpKey(const std::string& key, uint32_t& code, uint32_t& vendor) {
    static const std::string prefix = "avp-";
    if (key.compare(0, prefix.size(), prefix) != 0) return false;
    std::string rest = key.substr(prefix.size());
    vendor = 0;
    auto dash = rest.find('-');
    if (dash == std::string::npos) return parseRawKeyNumber(rest, code);
    return parseRawKeyNumber(rest.substr(0, dash), code) && parseRawKeyNumber(rest.substr(dash + 1), vendor);
}

// ----------------------------------------------------------------------------
// Readable forms of an OctetString (JSON -> bytes). The canonical JSON form of
// an OctetString is the hex of its bytes (the decoder always gives it back so);
// an object with ONE of these keys is another way to write the same bytes, so a
// provision can show "dnn_mbb.com" instead of "646e6e5f6d62622e636f6d". The key
// names how the bytes are built, never the AVP: which form suits an AVP is its
// 3GPP definition (the writer's knowledge, as the hex is today).
//   generic:    hex, text (UTF-8 bytes), ipv4 (4 bytes), ipv6 (16 bytes)
//   structured: ipv6Prefix (RFC 3162: reserved, length, prefix),
//               msTimeZone (TS 29.061 3GPP-MS-TimeZone: TS 24.008 zone + DST),
//               uli (TS 29.061 3GPP-User-Location-Info: TAI, ECGI, TAI+ECGI)
// ----------------------------------------------------------------------------

[[noreturn]] void formError(const std::string& avpName, const std::string& what) {
    throw std::runtime_error("OctetString AVP '" + avpName + "': " + what);
}

std::string formString(const nlohmann::json& v, const std::string& avpName, const std::string& key) {
    if (!v.is_string()) formError(avpName, "'" + key + "' must be a string");
    return v.get<std::string>();
}

// Decimal JSON integer, or a "0x..." hex string; checked against max.
uint32_t formNumber(const nlohmann::json& obj, const char* key, uint32_t max, const std::string& avpName) {
    auto it = obj.find(key);
    if (it == obj.end()) formError(avpName, std::string("uli needs '") + key + "'");
    uint64_t v = 0;
    if (it->is_number_unsigned() || (it->is_number_integer() && it->get<int64_t>() >= 0)) {
        v = it->get<uint64_t>();
    } else if (it->is_string()) {
        const std::string s = it->get<std::string>();
        if (s.size() < 3 || s.size() > 18 || s.compare(0, 2, "0x") != 0 ||
            s.find_first_not_of("0123456789abcdefABCDEF", 2) != std::string::npos)
            formError(avpName, std::string("uli '") + key + "' must be a decimal number or a \"0x...\" hex string");
        v = std::stoull(s.substr(2), nullptr, 16);
    } else {
        formError(avpName, std::string("uli '") + key + "' must be a decimal number or a \"0x...\" hex string");
    }
    if (v > max) formError(avpName, std::string("uli '") + key + "' out of range");
    return static_cast<uint32_t>(v);
}

// MCC (3 digits) + MNC (2 or 3 digits) -> 3 bytes BCD (TS 24.008 10.5.1.3).
std::string plmnBcd(const nlohmann::json& obj, const std::string& avpName) {
    auto digits = [&](const char* key, size_t minLen, size_t maxLen) {
        auto it = obj.find(key);
        if (it == obj.end() || !it->is_string())
            formError(avpName, std::string("uli needs '") + key + "' (string of digits)");
        std::string d = it->get<std::string>();
        if (d.size() < minLen || d.size() > maxLen || d.find_first_not_of("0123456789") != std::string::npos)
            formError(avpName, std::string("uli '") + key + "' must have " + std::to_string(minLen) +
                                   (minLen == maxLen ? "" : " or " + std::to_string(maxLen)) + " digits");
        return d;
    };
    const std::string mcc = digits("mcc", 3, 3), mnc = digits("mnc", 2, 3);
    auto n = [](char c) { return static_cast<uint8_t>(c - '0'); };
    const uint8_t mnc3 = mnc.size() == 3 ? n(mnc[2]) : 0x0F;
    std::string out;
    out.push_back(static_cast<char>(n(mcc[1]) << 4 | n(mcc[0])));
    out.push_back(static_cast<char>(mnc3 << 4 | n(mcc[2])));
    out.push_back(static_cast<char>(n(mnc[1]) << 4 | n(mnc[0])));
    return out;
}

void pushBigEndian(std::string& out, uint32_t v, int bytes) {
    for (int i = bytes - 1; i >= 0; --i) out.push_back(static_cast<char>((v >> (8 * i)) & 0xFF));
}

std::string uliBytes(const nlohmann::json& uli, const std::string& avpName) {
    if (!uli.is_object()) formError(avpName, "'uli' must be an object");
    auto itType = uli.find("type");
    if (itType == uli.end() || !itType->is_string()) formError(avpName, "uli needs 'type': tai|ecgi|tai-ecgi");
    const std::string type = itType->get<std::string>();
    const std::string plmn = plmnBcd(uli, avpName);
    std::string out;
    if (type == "tai") {
        out.push_back(static_cast<char>(128));
        out += plmn;
        pushBigEndian(out, formNumber(uli, "tac", 0xFFFF, avpName), 2);
    } else if (type == "ecgi") {
        out.push_back(static_cast<char>(129));
        out += plmn;
        pushBigEndian(out, formNumber(uli, "eci", 0x0FFFFFFF, avpName), 4);
    } else if (type == "tai-ecgi") {
        out.push_back(static_cast<char>(130));
        out += plmn;
        pushBigEndian(out, formNumber(uli, "tac", 0xFFFF, avpName), 2);
        out += plmn;
        pushBigEndian(out, formNumber(uli, "eci", 0x0FFFFFFF, avpName), 4);
    } else {
        formError(avpName, "uli 'type' must be tai|ecgi|tai-ecgi");
    }
    return out;
}

// "+-hh:mm[+dst]" -> 2 bytes: TS 24.008 10.5.3.8 time zone (quarters of an
// hour, BCD with swapped semi-octets, sign bit 0x08) + 10.5.3.12 DST (0..2).
std::string msTimeZoneBytes(const std::string& tz, const std::string& avpName) {
    auto bad = [&]() { formError(avpName, "msTimeZone must be +-hh:mm[+dst] (e.g. -08:00+1): " + tz); };
    if (tz.size() != 6 && tz.size() != 8) bad();
    if ((tz[0] != '+' && tz[0] != '-') || tz[3] != ':') bad();
    for (size_t i : {1u, 2u, 4u, 5u})
        if (!std::isdigit(static_cast<unsigned char>(tz[i]))) bad();
    int dst = 0;
    if (tz.size() == 8) {
        if (tz[6] != '+' || tz[7] < '0' || tz[7] > '2') bad();
        dst = tz[7] - '0';
    }
    const int minutes = ((tz[1] - '0') * 10 + (tz[2] - '0')) * 60 + (tz[4] - '0') * 10 + (tz[5] - '0');
    if (minutes % 15 != 0) formError(avpName, "msTimeZone goes in quarters of an hour: " + tz);
    const int q = minutes / 15;
    if (q > 79) formError(avpName, "msTimeZone out of range: " + tz);
    uint8_t zone = static_cast<uint8_t>((q % 10) << 4 | (q / 10));
    if (tz[0] == '-') zone |= 0x08;
    std::string out;
    out.push_back(static_cast<char>(zone));
    out.push_back(static_cast<char>(dst));
    return out;
}

// "addr/len" -> reserved (0) + prefix length + the first ceil(len/8) bytes of
// the address (bits beyond the length cleared), RFC 3162 Framed-IPv6-Prefix.
std::string ipv6PrefixBytes(const std::string& prefix, const std::string& avpName) {
    auto slash = prefix.find('/');
    if (slash == std::string::npos) formError(avpName, "ipv6Prefix must be <ipv6>/<length>: " + prefix);
    const std::string addr = prefix.substr(0, slash), lenText = prefix.substr(slash + 1);
    if (lenText.empty() || lenText.size() > 3 || lenText.find_first_not_of("0123456789") != std::string::npos)
        formError(avpName, "ipv6Prefix length must be 0..128: " + prefix);
    const int len = std::stoi(lenText);
    if (len > 128) formError(avpName, "ipv6Prefix length must be 0..128: " + prefix);
    uint8_t buf[16];
    if (inet_pton(AF_INET6, addr.c_str(), buf) != 1) formError(avpName, "ipv6Prefix address is not IPv6: " + prefix);
    std::string out;
    out.push_back(0);
    out.push_back(static_cast<char>(len));
    const int bytes = (len + 7) / 8;
    for (int i = 0; i < bytes; ++i) {
        uint8_t b = buf[i];
        const int bits = len - 8 * i;
        if (bits < 8) b &= static_cast<uint8_t>(0xFF << (8 - bits));
        out.push_back(static_cast<char>(b));
    }
    return out;
}

std::string octetsFromForm(const nlohmann::json& form, const std::string& avpName) {
    if (form.size() != 1)
        formError(avpName,
                  "a readable form is an object with exactly one key "
                  "(hex|text|ipv4|ipv6|ipv6Prefix|msTimeZone|uli)");
    const auto& [key, v] = *form.items().begin();
    if (key == "hex") {
        auto raw = fromHex(formString(v, avpName, key));
        return std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
    }
    if (key == "text") return formString(v, avpName, key);
    if (key == "ipv4" || key == "ipv6") {
        const std::string addr = formString(v, avpName, key);
        uint8_t buf[16];
        const int family = key == "ipv4" ? AF_INET : AF_INET6;
        if (inet_pton(family, addr.c_str(), buf) != 1) formError(avpName, "'" + key + "' is not valid: " + addr);
        return std::string(reinterpret_cast<const char*>(buf), key == "ipv4" ? 4 : 16);
    }
    if (key == "ipv6Prefix") return ipv6PrefixBytes(formString(v, avpName, key), avpName);
    if (key == "msTimeZone") return msTimeZoneBytes(formString(v, avpName, key), avpName);
    if (key == "uli") return uliBytes(v, avpName);
    formError(avpName, "unknown readable form '" + key + "' (hex|text|ipv4|ipv6|ipv6Prefix|msTimeZone|uli)");
}

// ----------------------------------------------------------------------------
// Readable forms of an OctetString (bytes -> JSON): the inverse of the
// encoders above, for the AVPs configured in DecodeOptions. Each reader only
// proposes a value; readableOctets keeps it if encoding it back gives the same
// bytes (rejects what the encoders cannot express: spare bits, unsupported
// types, non canonical values) and otherwise falls back to {"hex": ...}.
// ----------------------------------------------------------------------------

// Strict UTF-8 (RFC 3629): no overlongs, no surrogates, <= U+10FFFF.
bool isUtf8(const std::string& s) {
    const auto* p = reinterpret_cast<const unsigned char*>(s.data());
    const size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        const unsigned char c = p[i];
        if (c < 0x80) {
            ++i;
            continue;
        }
        size_t len = 0;
        uint32_t cp = 0, min = 0;
        if ((c & 0xE0) == 0xC0) {
            len = 2, cp = c & 0x1F, min = 0x80;
        } else if ((c & 0xF0) == 0xE0) {
            len = 3, cp = c & 0x0F, min = 0x800;
        } else if ((c & 0xF8) == 0xF0) {
            len = 4, cp = c & 0x07, min = 0x10000;
        } else {
            return false;
        }
        if (i + len > n) return false;
        for (size_t k = 1; k < len; ++k) {
            if ((p[i + k] & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (p[i + k] & 0x3F);
        }
        if (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
        i += len;
    }
    return true;
}

std::string ipText(const uint8_t* buf, int family) {
    char text[INET6_ADDRSTRLEN];
    return inet_ntop(family, buf, text, sizeof(text)) ? std::string(text) : std::string();
}

uint32_t bigEndian(const uint8_t* p, int bytes) {
    uint32_t v = 0;
    for (int i = 0; i < bytes; ++i) v = (v << 8) | p[i];
    return v;
}

// 3 bytes BCD (TS 24.008 10.5.1.3) -> mcc/mnc into 'uli'. False on a non digit.
bool readPlmn(const uint8_t* p, nlohmann::json& uli) {
    auto digit = [](uint8_t d, std::string& out) {
        if (d > 9) return false;
        out.push_back(static_cast<char>('0' + d));
        return true;
    };
    std::string mcc, mnc;
    if (!digit(p[0] & 0x0F, mcc) || !digit(p[0] >> 4, mcc) || !digit(p[1] & 0x0F, mcc)) return false;
    if (!digit(p[2] & 0x0F, mnc) || !digit(p[2] >> 4, mnc)) return false;
    if ((p[1] >> 4) != 0x0F && !digit(p[1] >> 4, mnc)) return false;  // F: 2-digit MNC
    uli["mcc"] = mcc;
    uli["mnc"] = mnc;
    return true;
}

// Candidate readable value of 'bytes' for 'form'; null when it does not fit.
nlohmann::json readForm(const std::string& bytes, const std::string& form) {
    const auto* p = reinterpret_cast<const uint8_t*>(bytes.data());
    const size_t n = bytes.size();
    if (form == "text") {
        if (bytes.find('\0') != std::string::npos || !isUtf8(bytes)) return nullptr;
        return bytes;
    }
    if (form == "ipv4" || form == "ipv6") {
        if (n != (form == "ipv4" ? 4u : 16u)) return nullptr;
        std::string text = ipText(p, form == "ipv4" ? AF_INET : AF_INET6);
        return text.empty() ? nlohmann::json() : nlohmann::json(text);
    }
    if (form == "ipv6Prefix") {
        if (n < 2 || p[0] != 0 || p[1] > 128 || n != 2u + (p[1] + 7u) / 8u) return nullptr;
        uint8_t addr[16] = {0};
        std::memcpy(addr, p + 2, n - 2);
        std::string text = ipText(addr, AF_INET6);
        return text.empty() ? nlohmann::json() : nlohmann::json(text + "/" + std::to_string(p[1]));
    }
    if (form == "msTimeZone") {
        if (n != 2 || (p[0] >> 4) > 9 || p[1] > 2) return nullptr;
        const int minutes = ((p[0] & 0x07) * 10 + (p[0] >> 4)) * 15;
        char text[16];
        std::snprintf(text, sizeof(text), "%c%02d:%02d", (p[0] & 0x08) ? '-' : '+', minutes / 60, minutes % 60);
        std::string tz(text);
        if (p[1] != 0) tz += "+" + std::to_string(p[1]);
        return tz;
    }
    if (form == "uli") {
        nlohmann::json uli = nlohmann::json::object();
        if (n == 6 && p[0] == 128) {
            uli["type"] = "tai";
            if (!readPlmn(p + 1, uli)) return nullptr;
            uli["tac"] = bigEndian(p + 4, 2);
        } else if (n == 8 && p[0] == 129) {
            uli["type"] = "ecgi";
            if (!readPlmn(p + 1, uli)) return nullptr;
            uli["eci"] = bigEndian(p + 4, 4);
        } else if (n == 13 && p[0] == 130) {
            uli["type"] = "tai-ecgi";
            if (!readPlmn(p + 1, uli)) return nullptr;  // the ECGI PLMN must be the same one (round trip)
            uli["tac"] = bigEndian(p + 4, 2);
            uli["eci"] = bigEndian(p + 9, 4);
        } else {
            return nullptr;  // other types (CGI, SAI, RAI, 5GS...): not supported by the encoder
        }
        return uli;
    }
    return nullptr;
}

nlohmann::json readableOctets(const std::string& bytes, const std::string& form) {
    nlohmann::json value = readForm(bytes, form);
    if (!value.is_null()) {
        nlohmann::json candidate = {{form, value}};
        try {
            if (octetsFromForm(candidate, "") == bytes) return candidate;
        } catch (const std::exception&) {
            // the encoder cannot express it: fall back to hex
        }
    }
    LOGDEBUG(ert::tracing::Logger::debug(
        ert::tracing::Logger::asString("OctetString does not fit readable form '%s': decoded as hex", form.c_str()),
        ERT_FILE_LOCATION));
    return {{"hex", toHex(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size())}};
}

}  // anonymous namespace

// ============================================================================
// DecodeOptions
// ============================================================================
bool DecodeOptions::isForm(const std::string& form) {
    return form == "text" || form == "ipv4" || form == "ipv6" || form == "ipv6Prefix" || form == "msTimeZone" ||
           form == "uli";
}

// ============================================================================
// Avp::sortByRules
// ============================================================================
void Avp::sortByRules(std::vector<Avp>& avps, const std::map<int, stack::AvpRule>& rules) {
    if (rules.empty() || avps.size() < 2) return;
    auto rank = [&rules](const core::AvpId& id) {
        for (const auto& kv : rules)
            if (kv.second.getId() == id) return kv.first;
        return std::numeric_limits<int>::max();  // not in the rules: last
    };
    std::vector<std::pair<int, Avp>> keyed;
    keyed.reserve(avps.size());
    for (auto& a : avps) {
        int r = a.raw_ ? std::numeric_limits<int>::max() : rank(a.id_);
        keyed.emplace_back(r, std::move(a));
    }
    std::stable_sort(keyed.begin(), keyed.end(), [](const auto& x, const auto& y) { return x.first < y.first; });
    for (size_t i = 0; i < keyed.size(); ++i) avps[i] = std::move(keyed[i].second);
}

// ============================================================================
// Avp::fromJson
// ============================================================================
Avp Avp::fromJson(const std::string& avpName, const nlohmann::json& value, const stack::Dictionary& dict) {
    Avp avp;

    // Resolve AVP id from name
    const stack::Avp* sa = dict.getAvp(avpName);
    if (!sa) {
        uint32_t code = 0, vendor = 0;
        if (!parseRawAvpKey(avpName, code, vendor)) throw std::runtime_error("Unknown AVP name: " + avpName);

        // Raw AVP (negative testing): verbatim data, explicit or default flags.
        avp.raw_ = true;
        avp.id_ = core::AvpId(static_cast<core::S32>(code), static_cast<core::S32>(vendor));
        avp.flags_ = vendor != 0 ? core::AvpFlagVendor : 0;
        std::string hex;
        if (value.is_string()) {
            hex = value.get<std::string>();
        } else if (value.is_object()) {
            auto itHex = value.find("hex");
            if (itHex == value.end() || !itHex->is_string())
                throw std::runtime_error("Raw AVP '" + avpName + "' object requires a 'hex' string");
            hex = itHex->get<std::string>();
            auto itFlags = value.find("flags");
            if (itFlags != value.end()) {
                // Signed or unsigned JSON integer (parsed literals are unsigned,
                // programmatic ones signed); reject floats and out of range.
                if (!itFlags->is_number_integer() || itFlags->get<int64_t>() < 0 || itFlags->get<int64_t>() > 0xFF)
                    throw std::runtime_error("Raw AVP '" + avpName + "' flags must be 0..255");
                avp.flags_ = static_cast<uint8_t>(itFlags->get<int64_t>());
            }
        } else {
            throw std::runtime_error("Raw AVP '" + avpName + "' value must be a hex string or {hex, flags}");
        }
        auto raw = fromHex(hex);
        avp.data_ = std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
        return avp;
    }

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
            sortByRules(children, sa->avprules());
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

        // OctetString: hex string -> raw bytes, or a readable form (object)
        default: {
            if (value.is_object()) {
                avp.data_ = octetsFromForm(value, avpName);
                break;
            }
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
