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


// C
#include <libgen.h> // basename
#include <signal.h>

// Standard
#include <iostream>
#include <string>

// Project
#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/tracing/Logger.hpp>


const char* progname;


void sighndl(int signal)
{
    std::cout << '\n';
    LOGWARNING(ert::tracing::Logger::warning(ert::tracing::Logger::asString("Signal received: %d", signal), ERT_FILE_LOCATION));
    switch (signal) {
    case SIGTERM:
    case SIGINT:
        exit(1);
    }
}

int main(int argc, char* argv[]) {

    progname = basename(argv[0]);
    ert::tracing::Logger::initialize(progname);
    ert::tracing::Logger::verbose();
    ert::tracing::Logger::setLevel("Debug");

    // Capture TERM/INT signals for graceful exit:
    signal(SIGTERM, sighndl);
    signal(SIGINT, sighndl);

    ert::diametercodec::stack::Dictionary dictionary;
    dictionary.load(ert::diametercodec::json::stacks::base);
    std::cout << dictionary.asJson().dump(2) << std::endl;

    return 0;
}

