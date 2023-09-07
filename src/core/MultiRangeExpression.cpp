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


// Standard
#include <limits.h>
#include <cstdlib>
#include <regex>
#include <vector>

// Project
#include <ert/diametercodec/core/MultiRangeExpression.hpp>


namespace ert
{
namespace diametercodec
{
namespace core
{

void MultiRangeExpression::refresh(void) {
    if(literal_.empty()) return;

    std::string range;
    unsigned int min, max;
    data_.clear();

    static std::regex commaRgx(R"(,)", std::regex::optimize);
    static std::regex dashRgx(R"(-)", std::regex::optimize);

    auto ranges = std::vector<std::string>(
                      std::sregex_token_iterator{begin(literal_), end(literal_), commaRgx, -1},
                      std::sregex_token_iterator{}
                  );

    for(auto ranges_it = ranges.begin(); ranges_it != ranges.end(); ranges_it ++) {
        range = *ranges_it;

        auto borders = std::vector<std::string>(
                           std::sregex_token_iterator{begin(range), end(range), dashRgx, -1},
                           std::sregex_token_iterator{}
                       );

        auto borders_it = borders.begin();

        if(borders_it != borders.end()) {
            min = atoi(borders_it->c_str());
            max = min;
            borders_it++;

            if(borders_it != borders.end()) {
                max = atoi(borders_it->c_str());
            }

            // Update data_:
            for(unsigned int k = min; k <= max; k++) {
                data_[k] = 0;

                if(k == UINT_MAX/* overflow */) break;
            }
        }
    }
}

std::string MultiRangeExpression::getExpandedLiteral(void) const {
    std::string result;
    std::map < unsigned int, int/*dummy*/ >::const_iterator it;
    std::map < unsigned int, int/*dummy*/ >::const_iterator it_min(data_.begin());
    std::map < unsigned int, int/*dummy*/ >::const_iterator it_max(data_.end());

    for(it = it_min; it != it_max; it++) {
        result += std::to_string((*it).first);
        result += ",";
    }

    int pos = result.size();

    if(pos) result.erase(pos - 1);

    return (result);
}

const char *MultiRangeExpression::simplifyLiteral(void) {
    if(data_.size() == 0) return nullptr;

    std::map < unsigned int, int/*dummy*/ >::const_iterator it;
    std::map < unsigned int, int/*dummy*/ >::const_iterator it_min(data_.begin());
    std::map < unsigned int, int/*dummy*/ >::const_iterator it_max(data_.end());
    unsigned int min = UINT_MAX;
    unsigned int max = 0;
    unsigned int value;
    unsigned int prevValue = data_.begin()->first;
    literal_ = "";

    for(it = it_min; it != it_max; it++) {
        value = (*it).first;

        if(value < min) min = value;

        if(value - prevValue > 1) {
            literal_ += std::to_string(min);
            if (max != min) {
                literal_ += "-";
                literal_ += std::to_string(max);
            }
            literal_ += ",";
            min = value;
        }

        if(value > max) max = value;

        prevValue = value;
    }

    literal_ += std::to_string(min);

    if(max != min) {
        literal_ += "-";
        literal_ += std::to_string(max);
    }

    return literal_.c_str();
}

}
}
}

