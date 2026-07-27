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

/**
 * @file Avp.hpp
 * @brief Binary Diameter AVP encoder/decoder with JSON conversion.
 *
 * Handles the binary wire format of Diameter AVPs (RFC 6733 section 4.1)
 * and provides bidirectional JSON conversion for use in h2diagent.
 *
 * Supported AVP data types:
 * - Integer32, Integer64, Unsigned32, Unsigned64, Float32, Float64
 * - OctetString (hex-encoded in JSON), UTF8String, DiameterIdentity, DiameterURI
 * - Address (IPv4/IPv6 string representation in JSON)
 * - Time (UNIX epoch in JSON, NTP on wire)
 * - Enumerated (integer value)
 * - Grouped (recursive AVP container, nested JSON object)
 * - IPFilterRule, QoSFilterRule
 *
 * Not to be confused with stack::Avp which holds dictionary metadata.
 *
 * @see Message for the message-level encoder/decoder
 * @see stack::Dictionary for AVP type resolution
 */

#pragma once

// Standard
#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// Project
#include <ert/diametercodec/core/defines.hpp>
#include <nlohmann/json.hpp>

namespace ert {
namespace diametercodec {

// Forward declarations
namespace stack {
class Dictionary;
}

namespace codec {

/**
 * Binary AVP encoder/decoder with JSON conversion.
 *
 * This class handles the binary wire format of Diameter AVPs (RFC 6733 §4.1)
 * and provides bidirectional JSON conversion for use in h2diagent.
 *
 * Not to be confused with stack::Avp which holds dictionary metadata.
 */
class Avp {
   public:
    // Data variant: covers all Diameter base types
    using Data = std::variant<std::monostate,   // empty / unset
                              int32_t,          // Integer32, Enumerated
                              int64_t,          // Integer64
                              uint32_t,         // Unsigned32, Time
                              uint64_t,         // Unsigned64
                              float,            // Float32
                              double,           // Float64
                              std::string,      // OctetString, UTF8String, DiameterIdentity, DiameterURI, IPFilterRule,
                                                // QoSFilterRule, Address (raw)
                              std::vector<Avp>  // Grouped
                              >;

    Avp() = default;

    // --- Decode from binary wire format ---
    // Returns number of bytes consumed (including padding)
    size_t decode(const uint8_t* buf, size_t len, const stack::Dictionary& dict);

    // --- Encode to binary wire format ---
    void encode(core::Buffer& out, const stack::Dictionary& dict) const;

    // --- JSON conversion ---
    nlohmann::json toJson(const stack::Dictionary& dict) const;
    static Avp fromJson(const std::string& avpName, const nlohmann::json& value, const stack::Dictionary& dict);

    // --- Accessors ---
    const core::AvpId& getId() const { return id_; }
    uint8_t getFlags() const { return flags_; }
    const Data& getData() const { return data_; }

    // Typed getters (throw std::bad_variant_access on mismatch)
    int32_t getInteger32() const { return std::get<int32_t>(data_); }
    int64_t getInteger64() const { return std::get<int64_t>(data_); }
    uint32_t getUnsigned32() const { return std::get<uint32_t>(data_); }
    uint64_t getUnsigned64() const { return std::get<uint64_t>(data_); }
    float getFloat32() const { return std::get<float>(data_); }
    double getFloat64() const { return std::get<double>(data_); }
    const std::string& getString() const { return std::get<std::string>(data_); }
    const std::vector<Avp>& getGrouped() const { return std::get<std::vector<Avp>>(data_); }

    // Setters
    void setId(const core::AvpId& id) { id_ = id; }
    void setFlags(uint8_t f) { flags_ = f; }
    void setData(Data d) { data_ = std::move(d); }

    // Flag helpers
    bool vendorBit() const { return (flags_ & core::AvpFlagVendor) != 0; }
    bool mandatoryBit() const { return (flags_ & core::AvpFlagMandatory) != 0; }

    // Name from dictionary (empty if unknown)
    std::string getName(const stack::Dictionary& dict) const;

    // Compute AVP length (header + data, without padding)
    size_t getLength(const stack::Dictionary& dict) const;

   private:
    core::AvpId id_{0, 0};
    uint8_t flags_{0};
    Data data_;

    // Encode data part into buffer, returns data bytes written
    size_t encodeData(core::Buffer& out, const stack::Dictionary& dict) const;

    // Decode data part based on format from dictionary
    void decodeData(const uint8_t* buf, size_t dataLen, const stack::Dictionary& dict);

    // Convert data to JSON value based on format
    nlohmann::json dataToJson(const stack::Dictionary& dict) const;
};

}  // namespace codec
}  // namespace diametercodec
}  // namespace ert
