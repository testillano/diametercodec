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

#pragma once

// Standard
#include <string>
#include <stdexcept>

// Project
#include <nlohmann/json.hpp>
#include <ert/diametercodec/core/defines.hpp>
#include <ert/diametercodec/core/MultiRangeExpression.hpp>
#include <ert/diametercodec/stack/AvpRule.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

class Format;
class Dictionary;

typedef std::map < int /*position*/, AvpRule > avprule_container;

/**
* Avp Reference information
*/
class Avp {

public:

    struct lessLabel {
        bool operator()(const std::string &d1, const std::string &d2) const {
            return (atoi(d1.c_str()) < atoi(d2.c_str()));
        }
    };
    typedef std::map < std::string /* data */, std::string /* alias */, lessLabel > label_container;

private:

    const Dictionary *dictionary_;
    core::AvpId id_;
    std::string name_;
    std::string format_name_;
    std::string vendor_name_;

    // Flag Rules:
    bool v_bit_;
    bool m_bit_;

    core::MultiRangeExpression enums_;
    label_container labels_;

    // Grouped:
    std::map <int /*position*/, AvpRule > avprules_;
    bool allow_fixed_rule_;
    int avprule_position_;

    void _initializeEnumsLabelsAndRules() {
        enums_.setLiteral("");
        labels_.clear();
        avprules_.clear();
        allow_fixed_rule_ = true;
        avprule_position_ = 0;
    }

public:

    Avp(const Dictionary *d = nullptr) {
        dictionary_ = d;
        name_ = "";
        format_name_ = "";
        vendor_name_ = "";
        v_bit_ = false;
        m_bit_ = true;
        _initializeEnumsLabelsAndRules();
    }
    ~Avp() {};


    // get
    const core::AvpId & getId(void) const {
        return id_;
    }
    const std::string & getName(void) const {
        return name_;
    }
    const std::string & getFormatName(void) const {
        return format_name_;
    }

    bool vBit(void) const {
        return v_bit_;
    }
    bool mBit(void) const {
        return m_bit_;
    }

    const char * getEnums(void) const {
        return enums_.getLiteral().c_str();
    }
    const char * getAlias(const std::string data) const {
        auto it = labels_.find(data);
        return ((it != labels_.end()) ? ((*it).second.c_str()) : nullptr);
    }

    const avprule_container &avprules() const {
        return avprules_;
    }

    const label_container &labels() const {
        return labels_;
    }

    // helpers
    bool allowEnum(int value) const {
        return enums_.contain(value);
    }
    bool hasAliases(void) const {
        return (labels_.size() != 0);
    }
    bool isChild(const core::AvpId & avp) const;
    const Format * getFormat() const;
    nlohmann::json asJson() const;

    // set
    void setCode(const core::S32 & c) {
        if(c < 0) throw std::runtime_error("Negative avp-code not allowed");

        id_.first = c;
    }

    void setVendorId(const core::S32 & v) {
        if(v < 0) throw std::runtime_error("Negative vendor-id not allowed");

        id_.second = v;
    }

    void setName(const std::string & n) {
        if(n == "") throw std::runtime_error("Empty avp-name string not allowed");

        name_ = n;
    }

    void setVendorName(const std::string & vn) {
        vendor_name_ = vn;
    }
    void setFormatName(const std::string & fn) {
        format_name_ = fn;
    }
    void setVbit(bool b = true) {
        v_bit_ = b;
    }
    void setMbit(bool b = true) {
        m_bit_ = b;
    }

    void setEnums(const char * e) {
        enums_.setLiteral(e);
    }
    void addEnums(const char * e) {
        enums_.addLiteral(e);
        enums_.simplifyLiteral();
    }

    // After format configuration:
    void addLabel(const std::string & data,  const std::string & alias);
    void addAvpRule(const AvpRule & avpRule);
};

}
}
}

