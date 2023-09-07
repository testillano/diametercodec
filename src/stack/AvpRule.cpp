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


// Project
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <ert/diametercodec/stack/AvpRule.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

assign_enum(AvpRule::Presence) = { "Fixed", "Mandatory", "Optional", nullptr /* list end indicator */};

void AvpRule::setQual(const std::string & q) {
    const char *asterisk = strstr(q.c_str(), "*");

    if((q != "") && (asterisk == nullptr))
        throw std::runtime_error("Non-empty qualifier must contain '*'");

    qual_ = q;
}

std::string AvpRule::getAvpName(void) const {
    const Avp * avp = dictionary_->getAvp(avp_id_);
    return avp->getName();
}

bool AvpRule::isAny(void) const {
    const Avp * avp = dictionary_->getAvp(avp_id_);
    const Format * format = dictionary_->getFormat(avp->getFormatName());
    return format->isAny();
}

int AvpRule::getQualMin(void) const {
    if(qual_ == "") {
        if(isFixed() || isMandatory()) return 1;

        if(isOptional()) return 0;
    }

    // Asterisk location
    const char * c_qual = qual_.c_str();
    int asterisk_pos = strstr(c_qual, "*") - c_qual;

    // '*', '*y'
    if(asterisk_pos == 0) return 0;

    // 'x*', 'x*y'
    std::string min = qual_.substr(0, asterisk_pos);  // 'x'
    return (atoi(min.c_str()));
}

int AvpRule::getQualMax(void) const {
    if(qual_ == "") return 1;

    // Asterisk location
    const char * c_qual = qual_.c_str();
    int asterisk_pos = strstr(c_qual, "*") - c_qual;

    // '*', 'x*'
    if(asterisk_pos == (qual_.size() - 1)) return -1;  // inf

    // '*y', 'x*y'
    std::string max = qual_.substr(asterisk_pos + 1, qual_.size() - asterisk_pos - 1);  // 'y'
    return (atoi(max.c_str()));
}

nlohmann::json AvpRule::asJson(void) const {
    nlohmann::json result;

    if (dictionary_->getAvp(avp_id_)) {
        result["type"] = Presence::asText(presence_);
        result["qual"] = qual_;
        result["name"] = getAvpName();
    }

    return result;
}

}
}
}

