# pragma once
/* FBX Data Type Serialization Utility
 *
 * This module provides functionality to serialize FBX property data types
 * into a JSON representation using the nlohmann::json library.
 * 
 * Below is an overview of EFbxType to json value mapping:
 * 
 *   eFbxUndefined,      -> null
 *   eFbxChar,           -> number (int)
 *   eFbxUChar,          -> number (int)
 *   eFbxShort,          -> number (int)
 *   eFbxUShort,         -> number (int)
 *   eFbxUInt,           -> number (int)
 *   eFbxLongLong,       -> number (int)
 *   eFbxULongLong,      -> number (int)
 *   eFbxHalfFloat,      -> number (float)
 *   eFbxBool,           -> boolean
 *   eFbxInt,            -> number (int)
 *   eFbxFloat,          -> number (float)
 *   eFbxDouble,         -> number (float)
 *   eFbxDouble2,        -> array<float, 2>
 *   eFbxDouble3,        -> array<float, 3>
 *   eFbxDouble4,        -> array<float, 4>
 *   eFbxDouble4x4,      -> array<<array<float,4>,4>
 *   eFbxEnum            -> number (int)
 *   eFbxEnumM           -> number (int)
 *   eFbxString          -> string
 *   eFbxTime,           -> string
 *   eFbxReference,      -> <unsupported>
 *   eFbxBlob,           -> <unsupported>
 *   eFbxDistance,       -> <unsupported>
 *   eFbxDateTime,       -> string
 *   eFbxTypeCount       -> number (int)
*/

#include <fbxsdk.h>
#include <nlohmann/json.hpp>
#include <string>

// Convert a generic FbxProperty into a simple JSON representation:
// { name: string, type: string, value: supported JSON value }
// Unsupported property types throw (caller decides how to handle).
nlohmann::json SerializeFbxProperty(const FbxProperty& property);
