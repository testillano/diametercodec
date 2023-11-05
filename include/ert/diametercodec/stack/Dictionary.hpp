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
#include <map>

// Project
#include <nlohmann/json.hpp>
#include <ert/diametercodec/core/defines.hpp>
#include <ert/diametercodec/stack/Format.hpp>
#include <ert/diametercodec/stack/Vendor.hpp>
#include <ert/diametercodec/stack/Avp.hpp>
#include <ert/diametercodec/stack/Command.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

/**
* Dictionary information
*/
class Dictionary {

public:

    typedef std::map<std::string, Format> format_container;
    typedef std::map<core::S32, Vendor> vendor_container;

    struct lessAvp { // order by vendor id
        bool operator()(core::AvpId id1, core::AvpId id2) const {
            if(id1.second == id2.second) return (id1.first < id2.first);

            return (id1.second < id2.second);
        }
    };
    typedef std::map<core::AvpId, Avp, lessAvp> avp_container;


    struct lessCommand {
        bool operator()(core::CommandId id1, core::CommandId id2) const {
            if(id1.first == id2.first) {
                if(id1.second != id2.second) return (id1.second);  // request -> answer

                return (false);
            }

            return (id1.first < id2.first);
        }
    };
    typedef std::map<core::CommandId, Command, lessCommand> command_container;

private:

    std::string name_;
    format_container formats_;
    vendor_container vendors_;
    avp_container avps_;
    command_container commands_;

    // Name identifiers:
    typedef std::map<std::string, const Vendor *> vendorNames_container;
    typedef std::map<std::string, const Avp *> avpNames_container;
    typedef std::map<std::string, const Command *> commandNames_container;

    vendorNames_container vendor_names_;
    avpNames_container avp_names_;
    commandNames_container command_names_;


    // init
    void initialize();

    // check & addings
    void extractFormats(const nlohmann::json &doc);
    void extractVendors(const nlohmann::json &doc);
    void extractAvps(const nlohmann::json &doc);
    void extractCommands(const nlohmann::json &doc);

public:

    Dictionary() {
        initialize();
    }
    ~Dictionary() {};

    // get
    const std::string & getName() const {
        return name_;
    }
    const Format * getFormat(const std::string & formatName) const;
    const Vendor * getVendor(core::S32 vendorId) const;
    const Vendor * getVendor(const std::string & vendorName) const;
    Avp * getAvp(const core::AvpId & avpId) const;
    Avp * getAvp(const std::string & avpName) const;
    Command * getCommand(const core::CommandId & commandId) const;
    Command * getCommand(const std::string & commandName) const;

    // set
    void addFormat(const Format &, bool reserved = false);
    void addVendor(const Vendor &);
    void addAvp(const Avp &);
    void addCommand(const Command &);

    // helpers
    nlohmann::json asJson() const ;

    // operators

    /**
    * Loads json dictionary document.
    *
    * Successive loadings will imply data accumulation where updates are allowed.
    *
    * @param json json document which represents the diameter dictionary.
    */
    virtual void load(const nlohmann::json &json);

    /**
    * Clears dictionary content
    */
    void clear(void) {
        initialize();    // initialize and prepares the dictionary
    }
};

}
}
}

