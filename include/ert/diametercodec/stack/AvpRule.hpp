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


namespace ert
{
namespace diametercodec
{
namespace stack
{

class Dictionary;

/**
* AVP rule information
*/
class AvpRule {

public:

    struct Presence {
        enum _v {
            None = -1,
            Fixed,
            Mandatory,
            Optional
        };

        declare_enum(Presence);

        /**
        * Presence description
        * @param v Presence type
        * @return Presence description
        */
        static const char* asText(const Presence::_v v) {
            return asCString(v);
        }
    };

private:

    const Dictionary *dictionary_;
    core::AvpId avp_id_; // reference
    Presence::_v presence_;
    std::string qual_;

public:


    AvpRule(const Dictionary *d = nullptr) {
        dictionary_ = d;
        presence_ = Presence::None;
        qual_ = "";
    }
    ~AvpRule() {};


    // getters
    std::string getAvpName(void) const;
    const Presence::_v & getPresence(void) const {
        return presence_;
    }
    const std::string & getQual(void) const {
        return qual_;
    }

    // helpers
    core::AvpId getId(void) const {
        return avp_id_;
    }
    bool isAny(void) const; // generic Avp
    bool isFixed(void) const {
        return (presence_ == Presence::Fixed);
    }
    bool isMandatory(void) const {
        return (presence_ == Presence::Mandatory);
    }
    bool isOptional(void) const {
        return (presence_ == Presence::Optional);
    }
    int getQualMin(void) const ;
    int getQualMax(void) const ; // -1 is infinite

    nlohmann::json asJson() const;

    // setters
    void setAvpId(const core::AvpId & ai) {
        avp_id_ = ai;
    }
    void setPresence(const Presence::_v & p) {
        presence_ = p;
    }
    void setQual(const std::string & q);
};

}
}
}

