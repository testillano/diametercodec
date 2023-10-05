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

/**
* Vendor data container
*/
class Vendor {
    ert::diametercodec::core::S32 code_;
    std::string name_;

public:

    Vendor() {};
    ~Vendor() {};


    // getters
    const ert::diametercodec::core::S32& getCode() const {
        return code_;
    }
    const std::string & getName() const {
        return name_;
    }

    // helpers
    bool isVendorSpecific() const {
        return (code_ > 0);
    }

    nlohmann::json asJson() const;

    // operators
    friend bool operator == (const Vendor & v1, const Vendor & v2) {
        return ((v1.getCode() == v2.getCode()));
    }

    // set
    void setCode(const ert::diametercodec::core::S32 & code) {
        if(code < 0) throw std::runtime_error("Negative vendor-id not allowed");

        code_ = code;
    }

    void setName(const std::string & name) {
        if(name.empty()) throw std::runtime_error("Empty vendor-name not allowed");

        name_ = name;
    }
};

}
}
}

