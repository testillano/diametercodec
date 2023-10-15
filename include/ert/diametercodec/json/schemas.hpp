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
namespace schema
{

const nlohmann::json diccionary = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "name": { "type": "string" },
    "format": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "parent-type": { "type": "string", "enum": [ "OctetString", "Integer32", "Integer64", "Unsigned32", "Unsigned64", "Float32", "Float64", "Grouped", "Address", "Time", "UTF8String", "DiameterIdentity", "DiameterURI", "Enumerated", "IPFilterRule" ] }
        },
        "required": [
          "name"
        ]
      }
    },
    "vendor": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "code": { "type": "integer", "minimum": 0, "maximum": 4294967295 }
        },
        "required": [
          "name",
          "code"
        ]
      }
    },
    "avp": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "code": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
          "v-bit": { "type": "boolean" },
          "m-bit": { "type": "boolean" },
          "vendor": { "type": "string" },
          "single": {
            "type": "object",
            "properties": {
              "enum": { "type": "string" },
              "label": {
                "type": "array",
                "items": {
                  "type": "object",
                  "properties": {
                    "data": { "type": "string" },
                    "alias": { "type": "string" }
                  },
                  "required": [
                    "data",
                    "alias"
                  ]
                }
              },
              "format": { "type": "string", "enum": [ "OctetString", "Integer32", "Integer64", "Unsigned32", "Unsigned64", "Float32", "Float64", "Grouped", "Address", "Time", "UTF8String", "DiameterIdentity", "DiameterURI", "Enumerated", "IPFilterRule" ] }
            },
            "required": [
              "format"
            ]
          },
          "grouped": {
            "type": "object",
            "properties": {
              "avprule": {
                "type": "array",
                "items": {
                  "type": "object",
                  "properties": {
                    "name": { "type": "string" },
                    "type": { "type": "string" },
                    "qual": { "type": "string" }
                  },
                  "required": [
                    "name",
                    "type"
                  ]
                }
              }
            }
          }
        },
        "required": [
          "name",
          "code"
        ],
        "oneOf": [
          { "required": [ "single" ] },
          { "required": [ "grouped" ] }
        ]
      }
    },
    "command": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "code": { "type": "integer", "minimum": 0, "maximum": 16777215 },
          "application-id": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
          "avprule": {
            "type": "array",
            "items": {
              "type": "object",
              "properties": {
                "name": { "type": "string" },
                "type": { "type": "string" },
                "qual": { "type": "string" }
              },
              "required": [
                "name",
                "type"
              ]
            }
          },
          "r-bit": { "type": "boolean" },
          "p-bit": { "type": "boolean" }
        },
        "required": [
          "name",
          "code",
          "avprule"
        ]
      }
    }
  },
  "required": [
    "name"
  ]
}
)"_json;

const nlohmann::json message = R"(
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "definitions": {
    "avpObject": {
      "oneOf": [
        {
          "type": "object",
          "required": ["name", "data"],
          "properties": {
            "name": { "type": "string" },
            "data": { "type": "string" },
            "alias": { "type": "string" }
          }
        },
        {
          "type": "object",
          "required": ["name", "hex-data"],
          "properties": {
            "name": { "type": "string" },
            "hex-data": { "type": "string" }
          }
        },
        {
          "type": "object",
          "required": ["name", "avp"],
          "properties": {
            "name": { "type": "string" },
            "avp": {
              "type": "array",
              "items": { "$ref": "#/definitions/avpObject" }
            }
          }
        },
        {
          "type": "object",
          "required": ["code", "vendor", "data"],
          "properties": {
            "code": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "v-bit": { "type": "boolean" },
            "m-bit": { "type": "boolean" },
            "vendor": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "data": { "type": "string" },
            "alias": { "type": "string" }
          }
        },
        {
          "type": "object",
          "required": ["code", "vendor", "hex-data"],
          "properties": {
            "code": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "v-bit": { "type": "boolean" },
            "m-bit": { "type": "boolean" },
            "vendor": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "hex-data": { "type": "string" }
          }
        },
        {
          "type": "object",
          "required": ["code", "vendor", "avp"],
          "properties": {
            "code": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "v-bit": { "type": "boolean" },
            "m-bit": { "type": "boolean" },
            "vendor": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
            "avp": {
              "type": "array",
              "items": { "$ref": "#/definitions/avpObject" }
            }
          }
        }
      ]
    }
  },
  "type": "object",
  "properties": {
    "group_a": {
      "type": "object",
      "properties": {
        "name": { "type": "string" },
        "avp": {
          "type": "array",
          "items": { "$ref": "#/definitions/avpObject" }
        }
      },
      "required": ["name", "avp"]
    },
    "group_b": {
      "type": "object",
      "properties": {
        "version": { "type": "integer", "minimum": 0, "maximum": 255 },
        "r-bit": { "type": "boolean" },
        "p-bit": { "type": "boolean" },
        "e-bit": { "type": "boolean" },
        "t-bit": { "type": "boolean" },
        "code": { "type": "integer", "minimum": 0, "maximum": 16777215 },
        "application-id": { "type": "integer", "minimum": 0, "maximum": 4294967295 },
        "avp": {
          "type": "array",
          "items": { "$ref": "#/definitions/avpObject" }
        }
      },
      "required": ["version", "code", "avp"]
    }
  },
  "oneOf": [
    { "$ref": "#/properties/group_a" },
    { "$ref": "#/properties/group_b" }
  ]
}
)"_json;

}
}
}
}

