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


namespace ert
{
namespace diametercodec
{
namespace core
{

/**
* Class helper to manage multi-range expressions like '1-4,23,45-46' (1,2,3,4,23,45,46)
*/
class MultiRangeExpression {

    std::string literal_;
    std::map < unsigned int, int/*dummy*/ > data_; // expands literal


    void refresh(void) ;  // align 'data_' and 'literal_'


public:

    MultiRangeExpression() {};
    ~MultiRangeExpression() {};


    /**
    * Gets the configured literal by mean #setLiteral or #addLiteral
    *
    * @return Literal
    */
    const std::string &getLiteral(void) const {
        return literal_;
    }

    /**
    * Gets expanded representation for stored literal. E.g.: '1-3,8,10' => '1,2,3,7,8,10'
    *
    * @return Expanded literal
    */
    std::string getExpandedLiteral(void) const;

    /**
    * Simplify stored literal. E.g.: '1,1,1,2,3,7,8,10' => '1-3,8,10' and returns it.
    *
    * @return Simplified literal
    */
    const char *simplifyLiteral(void);

    // helpers

    /**
    * Returns true if the value provided is contained in the multirange expression literal
    *
    * @param value Value to be tested
    * @return True or false
    */
    bool contain(const unsigned int & value) const {
        return (data_.find(value) != data_.end());
    }

    // setters

    /**
    * Configures a new literal
    *
    * @param l Literal to be stored
    */
    void setLiteral(const std::string &l) {
        literal_ = l;
        refresh();
    }

    /**
    * Accumulates the provided literal over the stored literal
    * You could simplify with #simplifyLiteral, because perhaps there is overlapping between current literal and provided one.
    *
    * @param l Literal to be added
    */
    void addLiteral(const std::string &l) {
        if(l.empty()) return;

        literal_ += ",";
        literal_ += l;

        refresh();
    }
};

}
}
}

