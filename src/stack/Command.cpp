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
#include <ert/tracing/Logger.hpp>
#include <ert/diametercodec/stack/Command.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

void Command::addAvpRule(const AvpRule & avpRule) {
    if(avpRule.isFixed()) {
        if(!allow_fixed_rule_) {
            std::string s_ex = ert::tracing::Logger::asString("Incorrect position for fixed avp rule '<%s>' within command '%s'", avpRule.getAvpName().c_str(), getName().c_str());
            s_ex += ". Fixed avp rules must be located at the beginning";
            throw std::runtime_error(s_ex);
        }
    } else allow_fixed_rule_ = false;

    // Restriction for redefinition (at this same level) of two rules for the same avp:
    if(isChild(avpRule.getId())) {
        std::string s_ex = ert::tracing::Logger::asString("Cannot add two rules for avp '%s', at the same level within command '%s'", avpRule.getAvpName().c_str(), getName().c_str());
        throw std::runtime_error(s_ex);
    }

    avprules_[avprule_position_++] = avpRule;
}

bool Command::isChild(const core::AvpId & avpId) const {
    for(const_avprule_iterator it = avprule_begin(); it != avprule_end(); it++)
        if(avpId == ((*it).second.getId()))
            return true;

    return false;
}

nlohmann::json Command::asJson(void) const {
    nlohmann::json result;

    result["name"] = name_;
    result["code"] = id_.first;
    result["r-bit"] = id_.second;

    // Build avprule array
    nlohmann::json aux;

    for(const_avprule_iterator it = avprule_begin(); it != avprule_end(); it++)
        aux.push_back((*it).second.asJson());

    result["avprule"] = aux;

    return result;
}

}
}
}

