# pragma once
/* ufbx Data Type Serialization Utility
 *
 * This module provides functionality to serialize ufbx property data types
 * into a JSON representation using the nlohmann::json library.
 */

#include <nlohmann/json.hpp>
#include <string>

#include <ufbx.h>

// Convert a ufbx string to std::string (UTF-8).
std::string UfbxStringToString(const ufbx_string& value);

// Convert a generic ufbx property into a simple JSON representation:
// { name: string, type: string, value: supported JSON value }
// Unsupported property types serialize as null values.
nlohmann::json SerializeFbxProperty(const ufbx_prop& property);

// Convert a ufbx element attribute into a JSON representation:
// { name: string, type: string, properties: [] }
nlohmann::json SerializeUfbxAttribute(const ufbx_element& attribute);
