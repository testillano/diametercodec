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
#include <stdexcept>

// Project
#include <ert/diametercodec/codec/Message.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>

namespace ert {
namespace diametercodec {
namespace codec {

// ============================================================================
// Helpers (same encode/decode helpers as Avp.cpp, duplicated to keep files independent)
// ============================================================================

namespace {

inline uint32_t decode4(const uint8_t* b) {
    return (uint32_t(b[0]) << 24) | (uint32_t(b[1]) << 16) | (uint32_t(b[2]) << 8) | uint32_t(b[3]);
}

inline uint32_t decode3(const uint8_t* b) { return (uint32_t(b[0]) << 16) | (uint32_t(b[1]) << 8) | uint32_t(b[2]); }

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

}  // anonymous namespace

// ============================================================================
// Message::decode
// ============================================================================
void Message::decode(const uint8_t* buf, size_t len, const stack::Dictionary& dict) {
    if (len < static_cast<size_t>(core::MessageHeaderLen))
        throw std::runtime_error("Not enough bytes for message header (20 bytes)");

    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |    Version    |                 Message Length                |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | command flags |                  Command-Code                 |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Application-ID                        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                      Hop-by-Hop Identifier                    |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                      End-to-End Identifier                    |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |  AVPs ...
    // +-+-+-+-+-+-+-+-+-+-+-+-+-

    version_ = buf[0];
    uint32_t msgLen = decode3(buf + 1);
    flags_ = buf[4];
    uint32_t code = decode3(buf + 5);
    id_ = core::CommandId(static_cast<core::U24>(code), isRequest());
    applicationId_ = decode4(buf + 8);
    hopByHop_ = decode4(buf + 12);
    endToEnd_ = decode4(buf + 16);

    if (len < msgLen) throw std::runtime_error("Not enough bytes to cover message length");

    // Decode AVPs
    avps_.clear();
    size_t pos = core::MessageHeaderLen;
    while (pos < msgLen) {
        Avp avp;
        size_t consumed = avp.decode(buf + pos, msgLen - pos, dict);
        avps_.push_back(std::move(avp));
        pos += consumed;
    }
}

// ============================================================================
// Message::encode
// ============================================================================
core::Buffer Message::encode(const stack::Dictionary& dict) const {
    core::Buffer out;
    out.reserve(256);  // reasonable initial capacity

    // Version
    out.push_back(version_);

    // Message Length placeholder (3 bytes)
    size_t lenPos = out.size();
    out.push_back(0);
    out.push_back(0);
    out.push_back(0);

    // Command Flags
    out.push_back(flags_);

    // Command-Code (3 bytes)
    encode3(out, id_.first);

    // Application-ID
    encode4(out, applicationId_);

    // Hop-by-Hop
    encode4(out, hopByHop_);

    // End-to-End
    encode4(out, endToEnd_);

    // AVPs
    for (const auto& avp : avps_) avp.encode(out, dict);

    // Write message length
    uint32_t msgLen = static_cast<uint32_t>(out.size());
    out[lenPos] = static_cast<uint8_t>(msgLen >> 16);
    out[lenPos + 1] = static_cast<uint8_t>(msgLen >> 8);
    out[lenPos + 2] = static_cast<uint8_t>(msgLen);

    return out;
}

// ============================================================================
// Message::getLength
// ============================================================================
size_t Message::getLength(const stack::Dictionary& dict) const {
    size_t total = core::MessageHeaderLen;
    for (const auto& avp : avps_) total += 4 * REQUIRED_WORDS(avp.getLength(dict));
    return total;
}

// ============================================================================
// Message::setHeaderToAnswer
// ============================================================================
void Message::setHeaderToAnswer(const Message& request) {
    version_ = request.version_;
    flags_ = request.flags_ & ~core::MsgFlagRequest;  // clear R flag
    id_ = core::CommandId(request.id_.first, false);
    applicationId_ = request.applicationId_;
    hopByHop_ = request.hopByHop_;
    endToEnd_ = request.endToEnd_;
}

// ============================================================================
// Message::getAvp (by id)
// ============================================================================
const Avp* Message::getAvp(const core::AvpId& id) const {
    for (const auto& avp : avps_)
        if (avp.getId() == id) return &avp;
    return nullptr;
}

// ============================================================================
// Message::getAvp (by name)
// ============================================================================
const Avp* Message::getAvp(const std::string& name, const stack::Dictionary& dict) const {
    const stack::Avp* sa = dict.getAvp(name);
    if (!sa) return nullptr;
    return getAvp(sa->getId());
}

// ============================================================================
// Message::toJson
// ============================================================================
nlohmann::json Message::toJson(const stack::Dictionary& dict) const {
    nlohmann::json result = nlohmann::json::object();

    // Header metadata
    result["_header"] = {{"version", version_},
                         {"flags", flags_},
                         {"command-code", id_.first},
                         {"request", isRequest()},
                         {"application-id", applicationId_},
                         {"hop-by-hop-id", hopByHop_},
                         {"end-to-end-id", endToEnd_}};

    // AVPs as flat JSON object (same as Grouped AVP logic)
    for (const auto& avp : avps_) {
        std::string name = avp.getName(dict);
        nlohmann::json val = avp.toJson(dict);
        if (result.contains(name)) {
            if (!result[name].is_array()) {
                nlohmann::json arr = nlohmann::json::array();
                arr.push_back(std::move(result[name]));
                result[name] = std::move(arr);
            }
            result[name].push_back(std::move(val));
        } else {
            result[name] = std::move(val);
        }
    }

    return result;
}

// ============================================================================
// Message::fromJson
// ============================================================================
Message Message::fromJson(const nlohmann::json& j, const stack::Dictionary& dict) {
    Message msg;

    // Parse header if present
    if (j.contains("_header")) {
        const auto& h = j["_header"];
        if (h.contains("version")) msg.version_ = h["version"].get<uint8_t>();
        if (h.contains("flags")) msg.flags_ = h["flags"].get<uint8_t>();
        if (h.contains("command-code")) {
            uint32_t code = h["command-code"].get<uint32_t>();
            bool req = h.value("request", false);
            msg.id_ = core::CommandId(static_cast<core::U24>(code), req);
            // Set R flag accordingly
            if (req)
                msg.flags_ |= core::MsgFlagRequest;
            else
                msg.flags_ &= ~core::MsgFlagRequest;
        }
        if (h.contains("application-id")) msg.applicationId_ = h["application-id"].get<uint32_t>();
        if (h.contains("hop-by-hop-id")) msg.hopByHop_ = h["hop-by-hop-id"].get<uint32_t>();
        if (h.contains("end-to-end-id")) msg.endToEnd_ = h["end-to-end-id"].get<uint32_t>();
    }

    // Parse AVPs (all keys except _header)
    for (auto& [key, val] : j.items()) {
        if (key == "_header") continue;
        if (val.is_array()) {
            for (const auto& elem : val) msg.avps_.push_back(Avp::fromJson(key, elem, dict));
        } else {
            msg.avps_.push_back(Avp::fromJson(key, val, dict));
        }
    }

    return msg;
}

}  // namespace codec
}  // namespace diametercodec
}  // namespace ert
