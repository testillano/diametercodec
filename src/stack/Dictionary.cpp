#include <iostream>
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


namespace ert
{
namespace diametercodec
{
namespace stack
{

void Dictionary::initialize() {
    formats_.clear();
    vendors_.clear();
    avps_.clear();
    commands_.clear();
    vendor_names_.clear();
    avp_names_.clear();
    command_names_.clear();

    // RFC6733 Diameter Formats harcoding:
    // Basic diameter types
    Format OctetString(this), Integer32(this), Integer64(this), Unsigned32(this), Unsigned64(this), Float32(this), Float64(this), Grouped(this);
    OctetString.setType(Format::Type::OctetString);
    Integer32.setType(Format::Type::Integer32);
    Integer64.setType(Format::Type::Integer64);
    Unsigned32.setType(Format::Type::Unsigned32);
    Unsigned64.setType(Format::Type::Unsigned64);
    Float32.setType(Format::Type::Float32);
    Float64.setType(Format::Type::Float64);
    Grouped.setType(Format::Type::Grouped);

    addFormat(OctetString);
    addFormat(Integer32);
    addFormat(Integer64);
    addFormat(Unsigned32);
    addFormat(Unsigned64);
    addFormat(Float32);
    addFormat(Float64);
    addFormat(Grouped);

    // Derived diameter types
    Format Address(this), Time(this), UTF8String(this), DiameterIdentity(this), DiameterURI(this), Enumerated(this), IPFilterRule(this), QoSFilterRule(this);
    Address.setType(Format::Type::Address);
    Address.setParentName(OctetString.getName());
    Time.setType(Format::Type::Time);
    Time.setParentName(OctetString.getName());
    UTF8String.setType(Format::Type::UTF8String);
    UTF8String.setParentName(OctetString.getName());
    DiameterIdentity.setType(Format::Type::DiameterIdentity);
    DiameterIdentity.setParentName(OctetString.getName());
    DiameterURI.setType(Format::Type::DiameterURI);
    DiameterURI.setParentName(OctetString.getName());
    Enumerated.setType(Format::Type::Enumerated);
    Enumerated.setParentName(Integer32.getName());
    IPFilterRule.setType(Format::Type::IPFilterRule);
    IPFilterRule.setParentName(OctetString.getName());
    QoSFilterRule.setType(Format::Type::QoSFilterRule);
    QoSFilterRule.setParentName(OctetString.getName());

    addFormat(Address);
    addFormat(Time);
    addFormat(UTF8String);
    addFormat(DiameterIdentity);
    addFormat(DiameterURI);
    addFormat(Enumerated);
    addFormat(IPFilterRule);
    addFormat(QoSFilterRule);

    // Generic AVP format:
    Format Any(this);
    Any.setType(Format::Type::Any);
    addFormat(Any, true /*reserved*/);

    // Generic AVP:
    Avp genericAvp(this);
    genericAvp.setCode(0);
    genericAvp.setVendorId(0/*Vendor::Code::Ietf*/);
    genericAvp.setName("AVP");
    genericAvp.setFormatName(Any.getName());
    genericAvp.setVbit(false);
    genericAvp.setMbit(false);
    addAvp(genericAvp);
}

void Dictionary::addFormat(const Format & format, bool reserved) {
    if(!reserved && format.isReserved()) {
        std::string s_ex = ert::tracing::Logger::asString("Format type '%s' is reserved for internal use", format.getName().c_str());
        throw std::runtime_error(s_ex);
    }

    const Format * found = getFormat(format.getName());
    if(found) {
        // Update:
        LOGINFORMATIONAL(
            std::string trace = "Updated format '";
            trace += format.getName();
            trace += "'";
            ert::tracing::Logger::informational(trace, ERT_FILE_LOCATION);
        );
    }

    formats_[format.getName()] = format;
}

void Dictionary::addVendor(const Vendor & vendor) {

    vendors_[vendor.getCode()] = vendor;
    vendor_names_[vendor.getName()] = getVendor(vendor.getCode());
}

void Dictionary::addAvp(const Avp & avp) {

    avps_[avp.getId()] = avp;
    avp_names_[avp.getName()] = getAvp(avp.getId());
}

void Dictionary::addCommand(const Command & command) {

    commands_[command.getId()] = command;
    command_names_[command.getName()] = getCommand(command.getId());
}

const Format * Dictionary::getFormat(const std::string & formatName) const {
    auto it = formats_.find(formatName);

    if(it != formats_.end()) return &(it->second);

    return (nullptr);
}

const Vendor * Dictionary::getVendor(core::S32 vendorId) const {
    auto it = vendors_.find(vendorId);

    if(it != vendors_.end()) return &(it->second);

    return (nullptr);
}

const Vendor * Dictionary::getVendor(const std::string & vendorName) const {
    auto it = vendor_names_.find(vendorName);

    if(it != vendor_names_.end()) return (it->second);

    return (nullptr);
}

const Avp * Dictionary::getAvp(const core::AvpId & avpId) const {
    auto it = avps_.find(avpId);

    if(it != avps_.end()) return &(it->second);

    return (nullptr);
}

const Avp * Dictionary::getAvp(const std::string & avpName) const {
    auto it = avp_names_.find(avpName);

    if(it != avp_names_.end()) return (it->second);

    return (nullptr);
}

const Command * Dictionary::getCommand(const core::CommandId & commandId) const {
    auto it = commands_.find(commandId);

    if(it != commands_.end()) return &(it->second);

    return (nullptr);
}

const Command * Dictionary::getCommand(const std::string & commandName) const {
    auto it = command_names_.find(commandName);

    if(it != command_names_.end()) return (it->second);

    return (nullptr);
}

nlohmann::json Dictionary::asJson(void) const {
    nlohmann::json result;

    result["name"] = name_;

    // Formats
    for(auto it: formats_) {
        if(it.second.isReserved()) continue;
        if(it.second.isRFC6733()) continue; // only user-defined formats are shown
        result["format"].push_back(it.second.asJson());
    }

    // Vendors
    for(auto it: vendors_)
        result["vendor"].push_back(it.second.asJson());

    // Avps
    for(auto it: avps_) {
        if(it.second.getFormat()->isAny()) continue;  // Generic AVP not shown

        result["avp"].push_back(it.second.asJson());
    }

    // Commands
    for(auto it: commands_)
        result["command"].push_back(it.second.asJson());

    return result;
}

void Dictionary::extractFormats(const nlohmann::json &doc) {
    for(auto it: doc) {
        Format aux(this); // set everything below (even empty, zeroed, etc.) to avoid reset() function

        // Mandatory
        auto name_it = it.find("name");
        auto ptype_it = it.find("parent-type");

        // Assignments:
        aux.setName(*name_it);
        aux.setParentName(*ptype_it);

        // New entry:
        addFormat(aux);
    }
}

void Dictionary::extractVendors(const nlohmann::json &doc) {
    for(auto it: doc) {
        Vendor aux; // set everything below (even empty, zeroed, etc.) to avoid reset() function

        // Mandatory
        auto name_it = it.find("name");
        auto code_it = it.find("code");

        // Assignments:
        aux.setCode(*code_it);
        aux.setName(*name_it);

        // New entry:
        addVendor(aux);
    }
}

void Dictionary::extractAvps(const nlohmann::json &doc) {
    for(auto it: doc) {
        Avp aux(this); // set everything below (even empty, zeroed, etc.) to avoid reset() function

        // Mandatory
        auto name_it = it.find("name");
        std::string name = *name_it;
        auto code_it = it.find("code");
        auto vendor_name_it = it.find("vendor-name");
        core::S32 vendorCode = 0; /* IETF by default */
        auto single_it = it.find("single");

        // Optionals
        auto vbit_it = it.find("v-bit");
        auto mbit_it = it.find("m-bit");

        // Vendor ?
        if (vendor_name_it != it.end()) {
            std::string c_name = *vendor_name_it;
            auto v_it = vendor_names_.find(c_name);

            if(v_it == vendor_names_.end()) {
                std::string s_ex = ert::tracing::Logger::asString("Vendor '%s', referenced at '%s' avp definition, not found at xml", c_name, name.c_str());
                throw std::runtime_error(s_ex);
            }

            aux.setVendorName(c_name);
            vendorCode = ((*v_it).second)->getCode();
        }

        // Assignments:
        aux.setCode(*code_it);
        aux.setVendorId(vendorCode);
        aux.setName(*name_it);
        aux.setVbit((vbit_it!=it.end()) ? bool(*vbit_it) : false);
        aux.setMbit((mbit_it!=it.end()) ? bool(*mbit_it) : false);

        // Check vendor specific bit:
        if(vendorCode && !aux.vBit()) {
            std::string s_ex = ert::tracing::Logger::asString("Flag rules for vendor specific bit (mustnot) at '%s' avp definicion, are incompatible with non-zeroed vendor id %d", name.c_str(), vendorCode);
            throw std::runtime_error(s_ex);
        }

        if(!vendorCode && aux.vBit()) {
            std::string s_ex = ert::tracing::Logger::asString("Flag rules for vendor specific bit (must) at '%s' avp definicion, are incompatible with zeroed vendor id %d", name.c_str(), vendorCode);
            throw std::runtime_error(s_ex);
        }

        if(single_it != it.end()) {
            auto f_it = (*single_it).find("format"); // mandatory
            std::string formatName = *f_it;
            auto enum_it = (*single_it).find("enum");
            auto label_it = (*single_it).find("label");

            // Assignments:
            const Format *format = getFormat(formatName);

            if(!format) {
                std::string s_ex = ert::tracing::Logger::asString("Format '%s', referenced at '%s' avp definition, not found at dictionary (neither xml nor RFC6733 diameter format types)", formatName.c_str(), name.c_str());
                throw std::runtime_error(s_ex);
            }

            aux.setFormatName(formatName);

            if(enum_it != (*single_it).end()) {

                std::string s_enum = *enum_it;

                if(!format->isEnumerated()) {
                    std::string s_ex = ert::tracing::Logger::asString("Enumerated literal '%s' is not allowed for '%s' avp format", s_enum.c_str(), formatName.c_str());
                    throw std::runtime_error(s_ex);
                }

                aux.setEnums(s_enum.c_str());
            }

            if(label_it != (*single_it).end()) {
                for(auto l_it: *label_it) {
                    std::string data = *(l_it.find("data"));
                    std::string alias = *(l_it.find("alias"));
                    // Assignment:
                    aux.addLabel(data, alias);
                }
            }
        } else { // grouped
            // Assignments:
            aux.setFormatName(Format::Type::asText(Format::Type::Grouped));
            // Wait for avprule insertion, because we need complete avp reference pool (*)
        }

        // New entry:
        addAvp(aux);
    }

    // Now process grouped ones:
    for(auto it: doc) {
        auto name_it = it.find("name");
        auto grouped_it = it.find("grouped");

        auto a_it = avp_names_.find(*name_it);
        Avp * gavp = (Avp *)((*a_it).second);

        if(!gavp) continue;  // it could be mising (a redefinition could have removed it)

        const Format *format = gavp->getFormat();

        // Avprule updating:
        if(format->isGrouped()) { // double check
            auto avprule_it = grouped_it->find("avprule");
            AvpRule auxAvpRule(this); // set everything below (even empty, zeroed, etc.) to avoid reset() function
            for(auto it: *avprule_it) {
                std::string name = *(it.find("name"));
                std::string type = *(it.find("type"));
                auto qual_it = it.find("qual"); // optional

                const Avp * avp = getAvp(name);
                if(avp == nullptr) {
                    std::string s_ex = ert::tracing::Logger::asString("Avp '%s', referenced at avp rule definition, not found at xml", name.c_str());
                    throw std::runtime_error(s_ex);
                }

                auxAvpRule.setAvpId(avp->getId());
                auxAvpRule.setPresence(AvpRule::Presence::asEnum(type));
                auxAvpRule.setQual((qual_it != it.end()) ? *qual_it:"");
                gavp->addAvpRule(auxAvpRule);
            }
        }
    }

    // Check avp loops between grouped avps:

    // In order to avoid loops, we could force to define grouped avps which children
    //  had been previously defined at xml file. In this way, is imposible to get a loop:
    //   C = ...
    //   D = ...
    //   A = grouped of B,C,D -> error, B unknown
    //   B = grouped of A,F -> with former definition, would become a loop
    //
    // But this supposes a restriction at json configuration (specific order).
    // The other way is an internal check: a grouped AVP won't have descendants within
    //  its ascendants. Then we will check all grouped avps in this way:
    //
    // 1. Searching for another grouped avps which are parents for this avp.
    // 2. If these are children (even this avp(*)) at avp definition, then a loop is detected.
    //
    // Example 1: (1) Analyzing 'A', found parent 'B' / (2) 'B' is already children of 'A'
    //       A -> B
    //            C
    //            D
    //       ...
    //       B -> A -> loop !!
    //            F
    //
    // (*) Example 2: (1) Analyzing 'A', found parent 'A' / (2) 'A' is already children of 'A'
    //       A -> B
    //            C
    //            D
    //            A -> loop !!
    //
    for(auto it = avps_.begin(); it != avps_.end(); it++) {
        const Avp & avp = (*it).second;

        if(!((avp.getFormat())->isGrouped())) continue;

        for(auto it_p = avps_.begin(); it_p != avps_.end(); it_p++) {
            const Avp & avp_p = (*it_p).second;

            if(!((avp_p.getFormat())->isGrouped())) continue;

            if(avp_p.isChild(avp.getId())) {
                if(avp.isChild(avp_p.getId())) {
                    std::string s_ex;


                    if(it != it_p)
                        s_ex = ert::tracing::Logger::asString("Loop detected between grouped avps '%s' and '%s'", avp.getName().c_str(), avp_p.getName().c_str());
                    else
                        s_ex = ert::tracing::Logger::asString("Loop within grouped avp '%s': cannot contain itself !!", avp.getName().c_str());

                    throw std::runtime_error(s_ex);
                } // parent is children of (ref): loop !
            } // parent found
        } // search parents
    } // search grouped avps (ref)
}

void Dictionary::extractCommands(const nlohmann::json &doc) {
    for(auto it: doc) {
        Command aux; // set everything below (even empty, zeroed, etc.) to avoid reset() function

        // Mandatory
        auto name_it = it.find("name");
        auto code_it = it.find("code");
        auto avprule_it = it.find("avprule");

        // Optionals
        auto appid_it = it.find("application-id");
        auto rbit_it = it.find("r-bit");
        auto pbit_it = it.find("p-bit");

        // Assignments:
        aux.setName(*name_it);
        aux.setCode(*code_it);
        aux.setApplicationId((appid_it!=it.end()) ? core::U32(*appid_it) : 0);
        aux.setRequest((rbit_it!=it.end()) ? bool(*rbit_it) : false);
        aux.setPbit((pbit_it!=it.end()) ? bool(*pbit_it) : false);

        AvpRule auxAvpRule(this); // set everything below (even empty, zeroed, etc.) to avoid reset() function
        for(auto it: *avprule_it) {
            std::string name = *(it.find("name"));
            std::string type = *(it.find("type"));
            auto qual_it = it.find("qual"); // optional

            const Avp * avp = getAvp(name);
            if(avp == nullptr) {
                std::string s_ex = ert::tracing::Logger::asString("Avp '%s', referenced at avp rule definition within command '%s', not found at xml", name.c_str(), std::string(*name_it).c_str());
                throw std::runtime_error(s_ex);
            }

            auxAvpRule.setAvpId(avp->getId());
            auxAvpRule.setPresence(AvpRule::Presence::asEnum(type));
            auxAvpRule.setQual((qual_it != it.end()) ? *qual_it:"");
            aux.addAvpRule(auxAvpRule);
        }

        // New entry:
        addCommand(aux);
    }
}

void Dictionary::load(const nlohmann::json &json) {

    // Mandatory
    auto name_it = json.find("name");
    name_ = *name_it;

    // Optional
    auto formats_it = json.find("format");
    if (formats_it != json.end() && formats_it->is_array()) {
        extractFormats(*formats_it);
    }

    auto vendors_it = json.find("vendor");
    if (vendors_it != json.end() && vendors_it->is_array()) {
        extractVendors(*vendors_it);
    }

    auto avps_it = json.find("avp");
    if (avps_it != json.end() && avps_it->is_array()) {
        extractAvps(*avps_it);
    }

    auto commands_it = json.find("command");
    if (commands_it != json.end() && commands_it->is_array()) {
        extractCommands(*commands_it);
    }
}

}
}
}

