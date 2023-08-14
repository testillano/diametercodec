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
#include <cstdint>
#include <utility>  // std::pair

// Project


namespace ert
{
namespace diametercodec
{

// Basic types
//      TYPE                    BITS (bytes)       Format
//      ----------------------- ------------------ ---------------------------
//      unsigned long int       32/64 (4/8)        lu
//      long int                32/64 (4/8)        ld
//
//      unsigned long long int  64 (8)             llu
//      long long int           64 (8)             lld
//
//      float                   32 (4)             f
//      double                  64 (8)             lf
//      long double             80 (10)            Lf
//
//      pointer = S.O. word     32/64              p
typedef unsigned char U8;
typedef char S8;
typedef uint16_t U16;
typedef int16_t S16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;
typedef int64_t S64;
typedef float F32;
typedef double F64;
typedef long double F80;
typedef U32 U24;

// pares
typedef std::pair < S32/*code*/, S32/*vendor-id*/ > AvpId;
typedef std::pair < U24/*code*/, bool/*request indicator*/ > CommandId;

//typedef std::pair<_avpId_t, U16> par_idAVP_ocurrencia_t;

typedef unsigned int ApplicationId;

// The Hop-by-Hop Identifier is an unsigned 32-bit integer field (in
// network byte order) and aids in matching requests and replies.
typedef unsigned int HopByHop;

typedef unsigned int EndToEnd;

}
}


