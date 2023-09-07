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
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <ert/diametercodec/stack/Format.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

bool Avp::isChild(const core::AvpId & avpId) const {
    const Format * format = getFormat();

    if(!format->isGrouped()) return false;

    for(const_avprule_iterator it = avprule_begin(); it != avprule_end(); it++)
        if(avpId == ((*it).second.getId()))
            return true;

    return false;
}

const Format * Avp::getFormat() const {
    return dictionary_->getFormat(format_name_);
}


void Avp::addLabel(const std::string & data,  const std::string & alias) {
    const Format * format = getFormat();

    if(format->isGrouped())
        throw std::runtime_error("Cannot add 'data-alias' entry on a grouped Avp");
    else
        labels_[data] = alias;

    LOGWARNING(

    if(format->isEnumerated() && !allowEnum(atoi(data.c_str()))) {
    ert::tracing::Logger::warning(ert::tracing::Logger::asString("Makes no sense adding alias '%s' for enum value '%s' out of range ('%s'), Avp '%s'",
                                  alias.c_str(), data.c_str(), getEnums(), name_.c_str()), ERT_FILE_LOCATION);
    }
    );
}

void Avp::addAvpRule(const AvpRule & avpRule) {
    const Format * format = getFormat();

    if(format->isGrouped()) {
        if(avpRule.isFixed()) {
            if(!allow_fixed_rule_) {
                std::string s_ex = ert::tracing::Logger::asString("Incorrect position for fixed avp rule '<%s>' within grouped avp '%s'", avpRule.getAvpName().c_str(), getName().c_str());
                s_ex += ". Fixed avp rules must be located at the beginning";
                throw std::runtime_error(s_ex);
            }
        } else allow_fixed_rule_ = false;

        // Restriction for redefinition (at this same level) of two rules for the same avp:
        if(isChild(avpRule.getId())) {
            std::string s_ex = ert::tracing::Logger::asString("Cannot add two rules for avp '%s', at the same level within grouped avp '%s'", avpRule.getAvpName().c_str(), getName().c_str());
            throw std::runtime_error(s_ex);
        }

        avprules_[avprule_position_++] = avpRule;
    } else
        throw std::runtime_error("Cannot add Avp rule on non-grouped Avp");
}

nlohmann::json Avp::asJson(void) const {
    nlohmann::json result;

    result["name"] = name_;
    result["code"] = id_.first;
    if(id_.second != 0) result["vendor-name"] = vendor_name_;
    result["v-bit"] = v_bit_;
    result["m-bit"] = m_bit_;

    const Format * format = getFormat();

    if(format->isGrouped()) {

        // Build rules array:
        nlohmann::json aux;
        for(const_avprule_iterator it = avprule_begin(); it != avprule_end(); it++)
            aux.push_back((*it).second.asJson());

        result["grouped"]["avprule"] = aux;

    } else {
        result["single"]["format"] = format_name_;

        if(format->isEnumerated()) {
            std::string enums = getEnums();
            if(enums != "") result["single"]["enum"] = enums;
        }

        if(hasAliases()) {
            // Build labels array:
            nlohmann::json aux, labelJson;
            for(const_label_iterator it = label_begin(); it != label_end(); it++) {
                labelJson["data"] = (*it).first;
                labelJson["alias"] = (*it).second;
                aux.push_back(labelJson);
            }
            result["single"]["label"] = aux;
        }
    }

    return result;
}

}
}
}

