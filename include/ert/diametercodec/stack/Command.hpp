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
#include <ert/diametercodec/stack/AvpRule.hpp>
#include <ert/diametercodec/stack/Format.hpp>


namespace ert
{
namespace diametercodec
{
namespace stack
{

class Dictionary;

/**
* Command information
*/
class Command {

public:

    typedef std::map < int /*position*/, AvpRule > avprule_container;
    typedef avprule_container::iterator avprule_iterator;
    typedef avprule_container::const_iterator const_avprule_iterator;


private:

    const Dictionary *dictionary_;
    core::CommandId id_;
    std::string name_;

    // Children:
    avprule_container avprules_;
    bool allow_fixed_rule_;
    int avprule_position_;

public:

    Command(const Dictionary *d = nullptr) {
        dictionary_ = d;
        avprules_.clear();
        allow_fixed_rule_ = true;
        avprule_position_ = 0;
    }
    ~Command() {};


    // get
    const core::CommandId & getId(void) const {
        return id_;
    }
    const std::string & getName(void) const {
        return name_;
    }


    // containers
    const_avprule_iterator avprule_begin() const {
        return avprules_.begin();
    }
    const_avprule_iterator avprule_end() const {
        return avprules_.end();
    }
    int avprule_size() const {
        return avprules_.size();
    }


    // helpers
    bool isEmpty(void) const {
        return (!avprules_.size());
    }
    bool isRequest(void) const {
        return (id_.second);
    }
    bool isAnswer(void) const {
        return (!id_.second);
    }
    bool isChild(const core::AvpId & avp) const ;


    nlohmann::json asJson(void) const ;

    // set
    void setCode(const core::U24 & c) {

        id_.first = c;
    }
    void setRequest(bool r = true) {
        id_.second = r;
    }
    void setAnswer(bool a = true) {
        id_.second = a;
    }
    void setName(const std::string & n) {
        if(n.empty()) throw std::runtime_error("Empty command-name string not allowed");

        name_ = n;
    }
    void addAvpRule(const AvpRule & avpRule);
};

}
}
}

