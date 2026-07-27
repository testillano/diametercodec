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
 * @file Message.hpp
 * @brief Binary Diameter message encoder/decoder with JSON conversion.
 *
 * Handles the 20-byte message header (RFC 6733 section 3) and the AVP payload.
 * Provides:
 * - decode(): parse binary wire format into structured message
 * - encode(): serialize message to binary wire format
 * - toJson(): convert to JSON with _header metadata and named AVP fields
 * - fromJson(): construct message from JSON representation
 * - setHeaderToAnswer(): create answer header from request (clear R-flag, copy identifiers)
 *
 * JSON format:
 * @code
 * {
 *   "_header": {"version":1, "flags":128, "command-code":272, "request":true,
 *               "application-id":4, "hop-by-hop-id":1234, "end-to-end-id":5678},
 *   "Session-Id": "gx;123;456",
 *   "Origin-Host": "pcrf.example.com",
 *   "Subscription-Id": [{"Subscription-Id-Type":1, "Subscription-Id-Data":"..."}]
 * }
 * @endcode
 *
 * @see Avp for individual AVP encoding/decoding
 * @see stack::Dictionary for command and AVP name resolution
 */

#pragma once

// Standard
#include <cstdint>
#include <string>
#include <vector>

// Project
#include <ert/diametercodec/codec/Avp.hpp>
#include <ert/diametercodec/core/defines.hpp>
#include <nlohmann/json.hpp>

namespace ert {
namespace diametercodec {

namespace stack {
class Dictionary;
}

namespace codec {

/**
 * Binary Diameter message encoder/decoder with JSON conversion.
 *
 * Handles the 20-byte message header (RFC 6733 §3) and the AVP payload.
 */
class Message {
   public:
    Message() = default;

    // --- Decode from binary wire format ---
    void decode(const uint8_t* buf, size_t len, const stack::Dictionary& dict);

    // --- Encode to binary wire format ---
    core::Buffer encode(const stack::Dictionary& dict) const;

    // --- JSON conversion ---
    nlohmann::json toJson(const stack::Dictionary& dict) const;
    static Message fromJson(const nlohmann::json& j, const stack::Dictionary& dict);

    // --- Header accessors ---
    uint8_t getVersion() const { return version_; }
    uint8_t getFlags() const { return flags_; }
    core::CommandId getId() const { return id_; }
    uint32_t getApplicationId() const { return applicationId_; }
    uint32_t getHopByHop() const { return hopByHop_; }
    uint32_t getEndToEnd() const { return endToEnd_; }
    bool isRequest() const { return (flags_ & core::MsgFlagRequest) != 0; }

    // --- Header setters ---
    void setVersion(uint8_t v) { version_ = v; }
    void setFlags(uint8_t f) { flags_ = f; }
    void setId(const core::CommandId& id) { id_ = id; }
    void setApplicationId(uint32_t id) { applicationId_ = id; }
    void setHopByHop(uint32_t hbh) { hopByHop_ = hbh; }
    void setEndToEnd(uint32_t e2e) { endToEnd_ = e2e; }

    // Copy header from request to build answer (clears R flag, copies e2e/hbh)
    void setHeaderToAnswer(const Message& request);

    // --- AVP access ---
    const std::vector<Avp>& avps() const { return avps_; }
    std::vector<Avp>& avps() { return avps_; }

    // Find first AVP by id (nullptr if not found)
    const Avp* getAvp(const core::AvpId& id) const;
    // Find first AVP by name via dictionary (nullptr if not found)
    const Avp* getAvp(const std::string& name, const stack::Dictionary& dict) const;

    // Add AVP
    Avp& addAvp() {
        avps_.emplace_back();
        return avps_.back();
    }

    // Compute total message length
    size_t getLength(const stack::Dictionary& dict) const;

   private:
    uint8_t version_{1};
    uint8_t flags_{0};
    core::CommandId id_{0, false};
    uint32_t applicationId_{0};
    uint32_t hopByHop_{0};
    uint32_t endToEnd_{0};
    std::vector<Avp> avps_;
};

}  // namespace codec
}  // namespace diametercodec
}  // namespace ert
