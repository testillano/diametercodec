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
#include <ert/diametercodec/stack/Format.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

assign_enum(Format::Type) = { "Unknown", "Any", "OctetString", "Integer32", "Integer64", "Unsigned32", "Unsigned64", "Float32", "Float64", "Grouped", "Address", "Time", "UTF8String", "DiameterIdentity", "DiameterURI", "Enumerated", "IPFilterRule", "QoSFilterRule", nullptr /* list end indicator */};

nlohmann::json Format::asJson(void) const {
    nlohmann::json result;

    result["name"] = name_;
    if (!parent_name_.empty()) result["parent-type"] = parent_name_;

    return result;
}

Format::Type::_v Format::getBasicType(void) const {
    if(isDerived()) return dictionary_->getFormat(parent_name_)->getBasicType();
    if(isReserved())
        throw std::runtime_error("Develop error: there is no basic format type for reserved type");

    return (Format::Type::asEnum(name_));
}

void Format::setParentName(const std::string & parentName) {
    const Format * parent = dictionary_->getFormat(parentName);
    if(parent && !parent->isBasic()) // 'Any' should not be allowed
        throw std::runtime_error("Only basic diameter format allowed for parent type");

    parent_name_ = parentName;
}

}
}
}

