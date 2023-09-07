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
* Format data container
*/
class Format {
    const Dictionary *dictionary_{};
    std::string parent_name_{}; // empty for basic types, Any and Unknown

    std::string name_{};

public:

    /**
     * Diameter RFC3588 Avp Data Format definition
     */
    struct Type {
        enum _v {
            None = -1,        // Initialized

            // Reserved
            Unknown,          // Unregistered at diameter dictionary
            Any,              // used on generic AVP

            // RFC 6733
            OctetString,      // S8*  n (possible zero-padding)
            Integer32,        // S32  4
            Integer64,        // S64  8
            Unsigned32,       // U32  4
            Unsigned64,       // U64  8
            Float32,          // F32  4
            Float64,          // F64  8
            Grouped,          // U32* 4*n
            Address,          // S8*  4 (IPv4) or 16 (IPv6)
            Time,             // S8*  4
            UTF8String,       // S8*  n (possible zero-padding)
            DiameterIdentity, // S8*  n (possible zero-padding)
            DiameterURI,      // S8*  n (possible zero-padding)
            Enumerated,       // S32  4
            IPFilterRule,     // S8*  n (possible zero-padding)
            QoSFilterRule     // S8*  n (possible zero-padding)
        };

        declare_enum(Format);

        /**
         * Format description
         * @param v Format type
         * @return Format name
         */
        static const char* asText(const Type::_v v) {
            return asCString(v);
        }

        static bool isAny(const std::string &formatName) {
            return(formatName == asText(Type::Any));
        }

        static bool isReserved(const std::string &formatName) {
            if(formatName == asText(Type::Unknown)) return true;
            if(formatName == asText(Type::Any)) return true;
            return false;
        }

        static bool isRFC6733(const std::string &formatName) {
            if(isReserved(formatName)) return false;
            return (asEnum(formatName) != None);
        }

        static bool isApplicationspecific(const std::string &formatName) {
            return (!isReserved(formatName) && !isRFC6733(formatName));
        }
    };

    Format(const Dictionary *d = nullptr) {
        dictionary_ = d;
    }

    ~Format() {};


    // getters
    const std::string &getName() const {
        return name_;
    }
    const std::string &getParentName() const {
        return parent_name_;
    }

    // helpers
    bool isAny() const {
        return Type::isAny(name_);
    }
    bool isReserved() const {
        return Type::isReserved(name_);
    }
    bool isRFC6733() const {
        return Type::isRFC6733(name_);
    }
    bool isApplicationspecific() const {
        return Type::isApplicationspecific(name_);
    }

    /** @return The format is 'OctetString' */
    bool isOctetString() const {
        return (name_ == Type::asText(Type::OctetString));
    }
    /** @return The format is 'Integer32' */
    bool isInteger32() const {
        return (name_ == Type::asText(Type::Integer32));
    }
    /** @return The format is 'Integer64' */
    bool isInteger64() const {
        return (name_ == Type::asText(Type::Integer64));
    }
    /** @return The format is 'Unsigned32' */
    bool isUnsigned32() const {
        return (name_ == Type::asText(Type::Unsigned32));
    }
    /** @return The format is 'Unsigned64' */
    bool isUnsigned64() const {
        return (name_ == Type::asText(Type::Unsigned64));
    }
    /** @return The format is 'Float32' */
    bool isFloat32() const {
        return (name_ == Type::asText(Type::Float32));
    }
    /** @return The format is 'Float64' */
    bool isFloat64() const {
        return (name_ == Type::asText(Type::Float64));
    }
    /** @return The format is 'Grouped' */
    bool isGrouped() const {
        return (name_ == Type::asText(Type::Grouped));
    }
    /** @return The format is 'Address' */
    bool isAddress() const {
        return (name_ == Type::asText(Type::Address));
    }
    /** @return The format is 'Time' */
    bool isTime() const {
        return (name_ == Type::asText(Type::Time));
    }
    /** @return The format is 'UTF8String' */
    bool isUTF8String() const {
        return (name_ == Type::asText(Type::UTF8String));
    }
    /** @return The format is 'DiameterIdentity' */
    bool isDiameterIdentity() const {
        return (name_ == Type::asText(Type::DiameterIdentity));
    }
    /** @return The format is 'DiameterURI' */
    bool isDiameterURI() const {
        return (name_ == Type::asText(Type::DiameterURI));
    }
    /** @return The format is 'Enumerated' */
    bool isEnumerated() const {
        return (name_ == Type::asText(Type::Enumerated));
    }
    /** @return The format is 'IPFilterRule' */
    bool isIPFilterRule() const {
        return (name_ == Type::asText(Type::IPFilterRule));
    }
    /** @return The format is 'QoSFilterRule' */
    bool isQoSFilterRule() const {
        return (name_ == Type::asText(Type::QoSFilterRule));
    }

    bool isDerived() const {
        return (!parent_name_.empty());
    }

    // OctetString, Integer32, Integer64, Unsigned32, Unsigned64, Float32, Float64, Grouped
    bool isBasic() const {
        return (!isDerived() && !isReserved());
    }

    /** Gets the diameter basic format type from which a derived type inherit, or basic type itself for non-derived */
    Type::_v getBasicType(void) const;

    const Format *getParent(void) const;

    nlohmann::json asJson() const;

    // operators
    friend bool operator == (const Format &f1, const Format &f2) {
        return ((f1.getName() == f2.getName()));
    }

    // set
    void setName(const std::string & name) {
        if(name.empty()) throw std::runtime_error("Empty format-name not allowed");

        name_ = name;
    }

    void setType(Type::_v type) {
        setName(Type::asText(type));
    }

    void setParentName(const std::string & name);
};

}
}
}

