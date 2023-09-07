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

#include <nlohmann/json.hpp>


namespace ert
{
namespace diametercodec
{
namespace json
{
namespace stacks
{

const nlohmann::json base = R"(
{
    "name": "Base protocol dictionary",
    "vendor": [
        { "name": "IETF", "code": 0 }
    ],
    "avp": [
        {
            "name": "User-Name",
            "code": 1,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "NAS-IP-Address",
            "code": 4,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Framed-IP-Address",
            "code": 8,
            "v-bit": false,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Filter-Id",
            "code": 11,
            "v-bit": false,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Class",
            "code": 25,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Session-Timeout",
            "code": 27,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Called-Station-Id",
            "code": 30,
            "v-bit": false,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Proxy-State",
            "code": 33,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Accounting-Session-Id",
            "code": 44,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Acct-Multi-Session-Id",
            "code": 50,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Event-Timestamp",
            "code": 55,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Time" }
        },
        {
            "name": "Acct-Interim-Interval",
            "code": 85,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Framed-IPv6-Prefix",
            "code": 97,
            "v-bit": false,
            "single": { "format": "OctetString" }
        },
        {
            "name": "Host-IP-Address",
            "code": 257,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Address" }
        },
        {
            "name": "Auth-Application-Id",
            "code": 258,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Acct-Application-Id",
            "code": 259,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "label": [
                    { "data": "19302", "alias": "ERICSSON_SCAP" }
                ],
                "format": "Unsigned32"
            }
        },
        {
            "name": "Vendor-Specific-Application-Id",
            "code": 260,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "qual": "1*", "name": "Vendor-Id" },
                    { "type": "Optional", "qual": "*1", "name": "Auth-Application-Id" },
                    { "type": "Optional", "qual": "*1", "name": "Acct-Application-Id" }
                ]
            }
        },
        {
            "name": "Redirect-Host-Usage",
            "code": 261,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-6",
                "label": [
                    { "data": "0", "alias": "DONT_CACHE" },
                    { "data": "1", "alias": "ALL_SESSION" },
                    { "data": "2", "alias": "ALL_REALM" },
                    { "data": "3", "alias": "REALM_AND_APPLICATION" },
                    { "data": "4", "alias": "ALL_APPLICATION" },
                    { "data": "5", "alias": "ALL_HOST" },
                    { "data": "6", "alias": "ALL_USER" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Redirect-Max-Cache-Time",
            "code": 262,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Session-Id",
            "code": 263,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Origin-Host",
            "code": 264,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Supported-Vendor-Id",
            "code": 265,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Vendor-Id",
            "code": 266,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Firmware-Revision",
            "code": 267,
            "v-bit": false,
            "m-bit": false,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Result-Code",
            "code": 268,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "label": [
                    { "data": "1001", "alias": "DIAMETER_MULTI_ROUND_AUTH" },
                    { "data": "2001", "alias": "DIAMETER_SUCCESS" },
                    { "data": "2002", "alias": "DIAMETER_LIMITED_SUCCESS" },
                    { "data": "3001", "alias": "DIAMETER_COMMAND_UNSUPPORTED" },
                    { "data": "3002", "alias": "DIAMETER_UNABLE_TO_DELIVER" },
                    { "data": "3003", "alias": "DIAMETER_REALM_NOT_SERVED" },
                    { "data": "3004", "alias": "DIAMETER_TOO_BUSY" },
                    { "data": "3005", "alias": "DIAMETER_LOOP_DETECTED" },
                    { "data": "3006", "alias": "DIAMETER_REDIRECT_INDICATION" },
                    { "data": "3007", "alias": "DIAMETER_APPLICATION_UNSUPPORTED" },
                    { "data": "3008", "alias": "DIAMETER_INVALID_HDR_BITS" },
                    { "data": "3009", "alias": "DIAMETER_INVALID_AVP_BITS" },
                    { "data": "3010", "alias": "DIAMETER_UNKNOWN_PEER" },
                    { "data": "4001", "alias": "DIAMETER_AUTHENTICATION_REJECTED" },
                    { "data": "4002", "alias": "DIAMETER_OUT_OF_SPACE" },
                    { "data": "4003", "alias": "ELECTION_LOST" },
                    { "data": "4005", "alias": "DIAMETER_ERROR_MIP_REPLY_FAILURE" },
                    { "data": "4006", "alias": "DIAMETER_ERROR_HA_NOT_AVAILABLE" },
                    { "data": "4007", "alias": "DIAMETER_ERROR_BAD_KEY" },
                    { "data": "4008", "alias": "DIAMETER_ERROR_MIP_FILTER_NOT_SUPPORTED" },
                    { "data": "4010", "alias": "DIAMETER_END_USER_SERVICE_DENIED" },
                    { "data": "4011", "alias": "DIAMETER_CREDIT_CONTROL_NOT_APPLICABLE" },
                    { "data": "4012", "alias": "DIAMETER_CREDIT_LIMIT_REACHED" },
                    { "data": "4241", "alias": "DIAMETER_END_USER_SERVICE_DENIED" },
                    { "data": "5001", "alias": "DIAMETER_AVP_UNSUPPORTED" },
                    { "data": "5002", "alias": "DIAMETER_UNKNOWN_SESSION_ID" },
                    { "data": "5003", "alias": "DIAMETER_AUTHORIZATION_REJECTED" },
                    { "data": "5004", "alias": "DIAMETER_INVALID_AVP_VALUE" },
                    { "data": "5005", "alias": "DIAMETER_MISSING_AVP" },
                    { "data": "5006", "alias": "DIAMETER_RESOURCES_EXCEEDED" },
                    { "data": "5007", "alias": "DIAMETER_CONTRADICTING_AVPS" },
                    { "data": "5008", "alias": "DIAMETER_AVP_NOT_ALLOWED" },
                    { "data": "5009", "alias": "DIAMETER_AVP_OCCURS_TOO_MANY_TIMES" },
                    { "data": "5010", "alias": "DIAMETER_NO_COMMON_APPLICATION" },
                    { "data": "5011", "alias": "DIAMETER_UNSUPPORTED_VERSION" },
                    { "data": "5012", "alias": "DIAMETER_UNABLE_TO_COMPLY" },
                    { "data": "5013", "alias": "DIAMETER_INVALID_BIT_IN_HEADER" },
                    { "data": "5014", "alias": "DIAMETER_INVALID_AVP_LENGTH" },
                    { "data": "5015", "alias": "DIAMETER_INVALID_MESSAGE_LENGTH" },
                    { "data": "5016", "alias": "DIAMETER_INVALID_AVP_BIT_COMBO" },
                    { "data": "5017", "alias": "DIAMETER_NO_COMMON_SECURITY" },
                    { "data": "5024", "alias": "DIAMETER_ERROR_NO_FOREIGN_HA_SERVICE" },
                    { "data": "5025", "alias": "DIAMETER_ERROR_END_TO_END_MIP_KEY_ENCRYPTION" },
                    { "data": "5030", "alias": "DIAMETER_USER_UNKNOWN" },
                    { "data": "5031", "alias": "DIAMETER_RATING_FAILED" },
                    { "data": "5063", "alias": "REQUESTED_SERVICE_NOT_AUTHORIZED" },
                    { "data": "5065", "alias": "IP_CAN_SESSION_NOT_AVAILABLE" },
                    { "data": "5241", "alias": "DIAMETER_END_USER_NOT_FOUND" }
                ],
                "format": "Unsigned32"
            }
        },
        {
            "name": "Product-Name",
            "code": 269,
            "v-bit": false,
            "m-bit": false,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Session-Binding",
            "code": 270,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "label": [
                    { "data": "1", "alias": "RE_AUTH" },
                    { "data": "2", "alias": "STR" },
                    { "data": "4", "alias": "ACCOUNTING" }
                ],
                "format": "Unsigned32"
            }
        },
        {
            "name": "Session-Server-Failover",
            "code": 271,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-3",
                "label": [
                    { "data": "0", "alias": "REFUSE_SERVICE" },
                    { "data": "1", "alias": "TRY_AGAIN" },
                    { "data": "2", "alias": "ALLOW_SERVICE" },
                    { "data": "3", "alias": "TRY_AGAIN_ALLOW_SERVICE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Multi-Round-Time-Out",
            "code": 272,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Disconnect-Cause",
            "code": 273,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-2",
                "label": [
                    { "data": "0", "alias": "REBOOTING" },
                    { "data": "1", "alias": "BUSY" },
                    { "data": "2", "alias": "DO_NOT_WANT_TO_TALK_TO_YOU" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Auth-Request-Type",
            "code": 274,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-3",
                "label": [
                    { "data": "0", "alias": "RESERVED" },
                    { "data": "1", "alias": "AUTHENTICATE_ONLY" },
                    { "data": "2", "alias": "AUTHORIZE_ONLY" },
                    { "data": "3", "alias": "AUTHORIZE_AUTHENTICATE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Auth-Grace-Period",
            "code": 276,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Auth-Session-State",
            "code": 277,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "STATE_MAINTAINED" },
                    { "data": "1", "alias": "NO_STATE_MAINTAINED" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Origin-State-Id",
            "code": 278,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Failed-AVP",
            "code": 279,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "qual": "1*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Proxy-Host",
            "code": 280,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Error-Message",
            "code": 281,
            "v-bit": false,
            "m-bit": false,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Route-Record",
            "code": 282,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Destination-Realm",
            "code": 283,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Proxy-Info",
            "code": 284,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "Proxy-Host" },
                    { "type": "Mandatory", "name": "Proxy-State" },
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Re-Auth-Request-Type",
            "code": 285,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "AUTHORIZE_ONLY" },
                    { "data": "1", "alias": "AUTHORIZE_AUTHENTICATE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Accounting-Sub-Session-Id",
            "code": 287,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "Authorization-Lifetime",
            "code": 291,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Redirect-Host",
            "code": 292,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterURI" }
        },
        {
            "name": "Destination-Host",
            "code": 293,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Error-Reporting-Host",
            "code": 294,
            "v-bit": false,
            "m-bit": false,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Termination-Cause",
            "code": 295,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "1-8",
                "label": [
                    { "data": "1", "alias": "DIAMETER_LOGOUT" },
                    { "data": "2", "alias": "DIAMETER_SERVICE_NOT_PROVIDED" },
                    { "data": "3", "alias": "DIAMETER_BAD_ANSWER" },
                    { "data": "4", "alias": "DIAMETER_ADMINISTRATIVE" },
                    { "data": "5", "alias": "DIAMETER_LINK_BROKEN" },
                    { "data": "6", "alias": "DIAMETER_AUTH_EXPIRED" },
                    { "data": "7", "alias": "DIAMETER_USER_MOVED" },
                    { "data": "8", "alias": "DIAMETER_SESSION_TIMEOUT" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Origin-Realm",
            "code": 296,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "DiameterIdentity" }
        },
        {
            "name": "Experimental-Result",
            "code": 297,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "Vendor-Id" },
                    { "type": "Mandatory", "name": "Experimental-Result-Code" }
                ]
            }
        },
        {
            "name": "Experimental-Result-Code",
            "code": 298,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "label": [
                    { "data": "5061", "alias": "INVALID_SERVICE_INFORMATION" },
                    { "data": "5062", "alias": "FILTER_RESTRICTIONS" },
                    { "data": "5063", "alias": "REQUESTED_SERVICE_NOT_AUTHORIZED" },
                    { "data": "5064", "alias": "DUPLICATED_AF_SESSION" },
                    { "data": "5065", "alias": "IP-CAN_SESSION_NOT_AVAILABLE" }
                ],
                "format": "Unsigned32"
            }
        },
        {
            "name": "Inband-Security-Id",
            "code": 299,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "label": [
                    { "data": "0", "alias": "NO_INBAND_SECURITY" },
                    { "data": "1", "alias": "TLS" }
                ],
                "format": "Unsigned32"
            }
        },
        {
            "name": "E2E-Sequence-AVP",
            "code": 300,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Accounting-Record-Type",
            "code": 480,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "1-4",
                "label": [
                    { "data": "1", "alias": "EVENT_RECORD" },
                    { "data": "2", "alias": "START_RECORD" },
                    { "data": "3", "alias": "INTERIM_RECORD" },
                    { "data": "4", "alias": "STOP_RECORD" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Accounting-Interim-Interval",
            "code": 482,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Integer32" }
        },
        {
            "name": "Accounting-Realtime-Required",
            "code": 483,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "1-3",
                "label": [
                    { "data": "1", "alias": "DELIVER_AND_GRANT" },
                    { "data": "2", "alias": "GRANT_AND_STORE" },
                    { "data": "3", "alias": "GRANT_AND_LOSE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Accounting-Record-Number",
            "code": 485,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "CC-Correlation-Id",
            "code": 411,
            "v-bit": false,
            "m-bit": "may",
            "single": { "format": "OctetString" }
        },
        {
            "name": "CC-Input-Octets",
            "code": 412,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "CC-Money",
            "code": 413,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Fixed", "name": "Unit-Value" },
                    { "type": "Optional", "name": "Currency-Code" }
                ]
            }
        },
        {
            "name": "CC-Output-Octets",
            "code": 414,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "CC-Request-Number",
            "code": 415,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "CC-Request-Type",
            "code": 416,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "1-4",
                "label": [
                    { "data": "1", "alias": "INITIAL_REQUEST" },
                    { "data": "2", "alias": "UPDATE_REQUEST" },
                    { "data": "3", "alias": "TERMINATION_REQUEST" },
                    { "data": "4", "alias": "EVENT_REQUEST" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "CC-Service-Specific-Units",
            "code": 417,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "CC-Session-Failover",
            "code": 418,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "FAILOVER_NOT_SUPPORTED" },
                    { "data": "1", "alias": "FAILOVER_SUPPORTED" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "CC-Sub-Session-Id",
            "code": 419,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "CC-Time",
            "code": 420,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "CC-Total-Octets",
            "code": 421,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned64" }
        },
        {
            "name": "Check-Balance-Result",
            "code": 422,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "ENOUGH_CREDIT" },
                    { "data": "1", "alias": "NO_CREDIT" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Cost-Information",
            "code": 423,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "Unit-Value" },
                    { "type": "Mandatory", "name": "Currency-Code" },
                    { "type": "Optional", "name": "Cost-Unit" }
                ]
            }
        },
        {
            "name": "Cost-Unit",
            "code": 424,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Currency-Code",
            "code": 425,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Credit-Control",
            "code": 426,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "CREDIT_AUTHORIZATION" },
                    { "data": "1", "alias": "RE_AUTHORIZATION" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Credit-Control-Failure-Handling",
            "code": 427,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-2",
                "label": [
                    { "data": "0", "alias": "TERMINATE" },
                    { "data": "1", "alias": "CONTINUE" },
                    { "data": "2", "alias": "RETRY_AND_TERMINATE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Direct-Debiting-Failure-Handling",
            "code": 428,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "TERMINATE_OR_BUFFER" },
                    { "data": "1", "alias": "CONTINUE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Exponent",
            "code": 429,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Integer32" }
        },
        {
            "name": "Final-Unit-Indication",
            "code": 430,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Fixed", "name": "Final-Unit-Action" },
                    { "type": "Optional", "qual": "*", "name": "Restriction-Filter-Rule" },
                    { "type": "Optional", "qual": "*", "name": "Filter-Id" },
                    { "type": "Optional", "name": "Redirect-Server" }
                ]
            }
        },
        {
            "name": "Granted-Service-Unit",
            "code": 431,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "name": "Tariff-Time-Change" },
                    { "type": "Optional", "name": "CC-Time" },
                    { "type": "Optional", "name": "CC-Money" },
                    { "type": "Optional", "name": "CC-Total-Octets" },
                    { "type": "Optional", "name": "CC-Input-Octets" },
                    { "type": "Optional", "name": "CC-Output-Octets" },
                    { "type": "Optional", "name": "CC-Service-Specific-Units" },
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Rating-Group",
            "code": 432,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Redirect-Address-Type",
            "code": 433,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-3",
                "label": [
                    { "data": "0", "alias": "IPv4_Address" },
                    { "data": "1", "alias": "IPv6_Address" },
                    { "data": "2", "alias": "URL" },
                    { "data": "3", "alias": "SIP_URL" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Redirect-Server",
            "code": 434,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Fixed", "name": "Redirect-Address-Type" },
                    { "type": "Fixed", "name": "Redirect-Server-Address" }
                ]
            }
        },
        {
            "name": "Redirect-Server-Address",
            "code": 435,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Requested-Action",
            "code": 436,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-3",
                "label": [
                    { "data": "0", "alias": "DIRECT_DEBITING" },
                    { "data": "1", "alias": "REFUND_ACCOUNT" },
                    { "data": "2", "alias": "CHECK_BALANCE" },
                    { "data": "3", "alias": "PRICE_ENQUIRY" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Requested-Service-Unit",
            "code": 437,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "name": "CC-Time" },
                    { "type": "Optional", "name": "CC-Money" },
                    { "type": "Optional", "name": "CC-Total-Octets" },
                    { "type": "Optional", "name": "CC-Input-Octets" },
                    { "type": "Optional", "name": "CC-Output-Octets" },
                    { "type": "Optional", "name": "CC-Service-Specific-Units" },
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Restriction-Filter-Rule",
            "code": 438,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "IPFilterRule" }
        },
        {
            "name": "Service-Identifier",
            "code": 439,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Service-Parameter-Info",
            "code": 440,
            "v-bit": false,
            "m-bit": "may",
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "Service-Parameter-Type" },
                    { "type": "Mandatory", "name": "Service-Parameter-Value" }
                ]
            }
        },
        {
            "name": "Service-Parameter-Type",
            "code": 441,
            "v-bit": false,
            "m-bit": "may",
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Service-Parameter-Value",
            "code": 442,
            "v-bit": false,
            "m-bit": "may",
            "single": { "format": "OctetString" }
        },
        {
            "name": "Subscription-Id",
            "code": 443,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "Subscription-Id-Type" },
                    { "type": "Mandatory", "name": "Subscription-Id-Data" }
                ]
            }
        },
        {
            "name": "Subscription-Id-Data",
            "code": 444,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        },
        {
            "name": "Unit-Value",
            "code": 445,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Fixed", "name": "Value-Digits" },
                    { "type": "Optional", "name": "Exponent" }
                ]
            }
        },
        {
            "name": "Used-Service-Unit",
            "code": 446,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "name": "Tariff-Change-Usage" },
                    { "type": "Optional", "name": "CC-Time" },
                    { "type": "Optional", "name": "CC-Money" },
                    { "type": "Optional", "name": "CC-Total-Octets" },
                    { "type": "Optional", "name": "CC-Input-Octets" },
                    { "type": "Optional", "name": "CC-Output-Octets" },
                    { "type": "Optional", "name": "CC-Service-Specific-Units" },
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "Value-Digits",
            "code": 447,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Integer64" }
        },
        {
            "name": "Validity-Time",
            "code": 448,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "Final-Unit-Action",
            "code": 449,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-2",
                "label": [
                    { "data": "0", "alias": "TERMINATE" },
                    { "data": "1", "alias": "REDIRECT" },
                    { "data": "2", "alias": "RESTRICT_ACCESS" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Subscription-Id-Type",
            "code": 450,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-4",
                "label": [
                    { "data": "0", "alias": "END_USER_E164" },
                    { "data": "1", "alias": "END_USER_IMSI" },
                    { "data": "2", "alias": "END_USER_SIP_URL" },
                    { "data": "3", "alias": "END_USER_NAI" },
                    { "data": "4", "alias": "END_USER_PRIVATE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Tariff-Time-Change",
            "code": 451,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Time" }
        },
        {
            "name": "Tariff-Change-Usage",
            "code": 452,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-2",
                "label": [
                    { "data": "0", "alias": "UNIT_BEFORE_TARIFF_CHANGE" },
                    { "data": "1", "alias": "UNIT_AFTER_TARIFF_CHANGE" },
                    { "data": "2", "alias": "UNIT_INDETERMINATE" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "G-S-U-Pool-Identifier",
            "code": 453,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "Unsigned32" }
        },
        {
            "name": "CC-Unit-Type",
            "code": 454,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-5",
                "label": [
                    { "data": "0", "alias": "TIME" },
                    { "data": "1", "alias": "MONEY" },
                    { "data": "2", "alias": "TOTAL_OCTETS" },
                    { "data": "3", "alias": "INPUT_OCTETS" },
                    { "data": "4", "alias": "OUTPUT_OCTETS" },
                    { "data": "5", "alias": "SERVICE_SPECIFIC_UNITS" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Multiple-Services-Indicator",
            "code": 455,
            "v-bit": false,
            "m-bit": true,
            "single": {
                "enum": "0-1",
                "label": [
                    { "data": "0", "alias": "MULTIPLE_SERVICES_NOT_SUPPORTED" },
                    { "data": "1", "alias": "MULTIPLE_SERVICES_SUPPORTED" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "Multiple-Services-Credit-Control",
            "code": 456,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Optional", "name": "Granted-Service-Unit" },
                    { "type": "Optional", "name": "Requested-Service-Unit" },
                    { "type": "Optional", "qual": "*", "name": "Used-Service-Unit" },
                    { "type": "Optional", "name": "Tariff-Change-Usage" },
                    { "type": "Optional", "qual": "*", "name": "Service-Identifier" },
                    { "type": "Optional", "name": "Rating-Group" },
                    { "type": "Optional", "qual": "*", "name": "G-S-U-Pool-Reference" },
                    { "type": "Optional", "name": "Validity-Time" },
                    { "type": "Optional", "name": "Result-Code" },
                    { "type": "Optional", "name": "Final-Unit-Indication" },
                    { "type": "Optional", "qual": "*", "name": "AVP" }
                ]
            }
        },
        {
            "name": "G-S-U-Pool-Reference",
            "code": 457,
            "v-bit": false,
            "m-bit": true,
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "G-S-U-Pool-Identifier" },
                    { "type": "Mandatory", "name": "CC-Unit-Type" },
                    { "type": "Mandatory", "name": "Unit-Value" }
                ]
            }
        },
        {
            "name": "User-Equipment-Info",
            "code": 458,
            "v-bit": false,
            "m-bit": "may",
            "grouped": {
                "avprule": [
                    { "type": "Mandatory", "name": "User-Equipment-Info-Type" },
                    { "type": "Mandatory", "name": "User-Equipment-Info-Value" }
                ]
            }
        },
        {
            "name": "User-Equipment-Info-Type",
            "code": 459,
            "v-bit": false,
            "m-bit": "may",
            "single": {
                "enum": "0-3",
                "label": [
                    { "data": "0", "alias": "IMEISV" },
                    { "data": "1", "alias": "MAC" },
                    { "data": "2", "alias": "EUI64" },
                    { "data": "3", "alias": "MODIFIED_EUI64" }
                ],
                "format": "Enumerated"
            }
        },
        {
            "name": "User-Equipment-Info-Value",
            "code": 460,
            "v-bit": false,
            "m-bit": "may",
            "single": { "format": "OctetString" }
        },
        {
            "name": "Service-Context-Id",
            "code": 461,
            "v-bit": false,
            "m-bit": true,
            "single": { "format": "UTF8String" }
        }
    ],
    "command": [
        {
            "name": "Capabilities-Exchange-Request",
            "code": 257,
            "avprule": [
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "qual": "1*", "name": "Host-IP-Address" },
                { "type": "Mandatory", "name": "Vendor-Id" },
                { "type": "Mandatory", "name": "Product-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "qual": "*", "name": "Supported-Vendor-Id" },
                { "type": "Optional", "qual": "*", "name": "Auth-Application-Id" },
                { "type": "Optional", "qual": "*", "name": "Inband-Security-Id" },
                { "type": "Optional", "qual": "*", "name": "Acct-Application-Id" },
                { "type": "Optional", "name": "Vendor-Specific-Application-Id" },
                { "type": "Optional", "name": "Firmware-Revision" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": true
        },
        {
            "name": "Capabilities-Exchange-Answer",
            "code": 257,
            "avprule": [
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "qual": "1*", "name": "Host-IP-Address" },
                { "type": "Mandatory", "name": "Vendor-Id" },
                { "type": "Mandatory", "name": "Product-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" },
                { "type": "Optional", "qual": "*", "name": "Supported-Vendor-Id" },
                { "type": "Optional", "qual": "*", "name": "Auth-Application-Id" },
                { "type": "Optional", "qual": "*", "name": "Inband-Security-Id" },
                { "type": "Optional", "qual": "*", "name": "Acct-Application-Id" },
                { "type": "Optional", "name": "Vendor-Specific-Application-Id" },
                { "type": "Optional", "name": "Firmware-Revision" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": false
        },
        {
            "name": "Re-Auth-Request",
            "code": 258,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Destination-Realm" },
                { "type": "Mandatory", "name": "Destination-Host" },
                { "type": "Mandatory", "name": "Auth-Application-Id" },
                { "type": "Mandatory", "name": "Re-Auth-Request-Type" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "Route-Record" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": true,
            "p-bit": true
        },
        {
            "name": "Re-Auth-Answer",
            "code": 258,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "name": "Error-Reporting-Host" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" },
                { "type": "Optional", "qual": "*", "name": "Redirect-Host" },
                { "type": "Optional", "name": "Redirect-Host-Usage" },
                { "type": "Optional", "name": "Redirect-Max-Cache-Time" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": false,
            "p-bit": true
        },
        {
            "name": "Accounting-Request",
            "code": 271,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Destination-Realm" },
                { "type": "Mandatory", "name": "Accounting-Record-Type" },
                { "type": "Mandatory", "name": "Accounting-Record-Number" },
                { "type": "Optional", "name": "Acct-Application-Id" },
                { "type": "Optional", "name": "Vendor-Specific-Application-Id" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Accounting-Sub-Session-Id" },
                { "type": "Optional", "name": "Accounting-Session-Id" },
                { "type": "Optional", "name": "Acct-Multi-Session-Id" },
                { "type": "Optional", "name": "Acct-Interim-Interval" },
                { "type": "Optional", "name": "Accounting-Realtime-Required" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "name": "Event-Timestamp" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "Route-Record" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": true,
            "p-bit": true
        },
        {
            "name": "Accounting-Answer",
            "code": 271,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Accounting-Record-Type" },
                { "type": "Mandatory", "name": "Accounting-Record-Number" },
                { "type": "Optional", "name": "Acct-Application-Id" },
                { "type": "Optional", "name": "Vendor-Specific-Application-Id" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Accounting-Sub-Session-Id" },
                { "type": "Optional", "name": "Accounting-Session-Id" },
                { "type": "Optional", "name": "Acct-Multi-Session-Id" },
                { "type": "Optional", "name": "Error-Reporting-Host" },
                { "type": "Optional", "name": "Acct-Interim-Interval" },
                { "type": "Optional", "name": "Accounting-Realtime-Required" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "name": "Event-Timestamp" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": false,
            "p-bit": true
        },
        {
            "name": "Abort-Session-Request",
            "code": 274,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Destination-Realm" },
                { "type": "Mandatory", "name": "Destination-Host" },
                { "type": "Mandatory", "name": "Auth-Application-Id" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "Route-Record" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": true
        },
        {
            "name": "Abort-Session-Answer",
            "code": 274,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "name": "Error-Reporting-Host" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" },
                { "type": "Optional", "qual": "*", "name": "Redirect-Host" },
                { "type": "Optional", "name": "Redirect-Host-Usage" },
                { "type": "Optional", "name": "Redirect-Max-Cache-Time" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": false,
            "p-bit": true
        },
        {
            "name": "Session-Termination-Request",
            "code": 275,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Destination-Realm" },
                { "type": "Mandatory", "name": "Auth-Application-Id" },
                { "type": "Mandatory", "name": "Termination-Cause" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "name": "Destination-Host" },
                { "type": "Optional", "qual": "*", "name": "Class" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "Route-Record" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": true,
            "p-bit": true
        },
        {
            "name": "Session-Termination-Answer",
            "code": 275,
            "avprule": [
                { "type": "Fixed", "name": "Session-Id" },
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "User-Name" },
                { "type": "Optional", "qual": "*", "name": "Class" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "name": "Error-Reporting-Host" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" },
                { "type": "Optional", "name": "Origin-State-Id" },
                { "type": "Optional", "qual": "*", "name": "Redirect-Host" },
                { "type": "Optional", "name": "Redirect-Host-Usage" },
                { "type": "Optional", "name": "Redirect-Max-Cache-Time" },
                { "type": "Optional", "qual": "*", "name": "Proxy-Info" },
                { "type": "Optional", "qual": "*", "name": "AVP" }
            ],
            "r-bit": false,
            "p-bit": true
        },
        {
            "name": "Device-Watchdog-Request",
            "code": 280,
            "avprule": [
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "Origin-State-Id" }
            ],
            "r-bit": true
        },
        {
            "name": "Device-Watchdog-Answer",
            "code": 280,
            "avprule": [
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" },
                { "type": "Optional", "name": "Origin-State-Id" }
            ],
            "r-bit": false
        },
        {
            "name": "Disconnect-Peer-Request",
            "code": 282,
            "avprule": [
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Mandatory", "name": "Disconnect-Cause" }
            ],
            "r-bit": true
        },
        {
            "name": "Disconnect-Peer-Answer",
            "code": 282,
            "avprule": [
                { "type": "Mandatory", "name": "Result-Code" },
                { "type": "Mandatory", "name": "Origin-Host" },
                { "type": "Mandatory", "name": "Origin-Realm" },
                { "type": "Optional", "name": "Error-Message" },
                { "type": "Optional", "qual": "*", "name": "Failed-AVP" }
            ],
            "r-bit": false
        }
    ]
}
)"_json;

}
}
}
}

