/*
 * sst-rackhelpers - a Surge Synth Team product
 *
 * A set of header-only utilities we use when making stuff for VCV Rack
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * sst-rackhelpers is released under the MIT license, found in the file
 * "LICENSE.md" in this repository.
 *
 * All source for sst-rackhelpers is available at
 * https://github.com/surge-synthesizer/sst-rackhelpers
 */
#ifndef INCLUDE_SST_RACKHELPERS_JSON_H
#define INCLUDE_SST_RACKHELPERS_JSON_H

#include <optional>
#include <string>

namespace sst::rackhelpers::json
{
template <typename T> inline std::optional<T> convertFromJson(json_t *o) { return {}; }
template <> inline std::optional<std::string> convertFromJson<std::string>(json_t *o)
{
    if (!o)
        return {};
    auto t = json_typeof(o);
    if (t == JSON_STRING)
    {
        return json_string_value(o);
    }
    return {};
}
template <> inline std::optional<bool> convertFromJson<bool>(json_t *o)
{
    if (!o)
        return {};
    auto t = json_typeof(o);
    if (t == JSON_TRUE || t == JSON_FALSE)
    {
        return json_boolean_value(o);
    }
    return {};
}
template <> inline std::optional<int> convertFromJson<int>(json_t *o)
{
    if (!o)
        return {};
    auto t = json_typeof(o);
    if (t == JSON_INTEGER)
    {
        return json_integer_value(o);
    }
    return {};
}

template <> inline std::optional<float> convertFromJson<float>(json_t *o)
{
    if (!o)
        return {};
    auto t = json_typeof(o);
    if (t == JSON_REAL)
    {
        return json_real_value(o);
    }
    return {};
}

template <> inline std::optional<double> convertFromJson<double>(json_t *o)
{
    if (!o)
        return {};
    auto t = json_typeof(o);
    if (t == JSON_REAL)
    {
        return json_real_value(o);
    }
    return {};
}

template <typename T> inline std::optional<T> jsonSafeGet(json_t *rootJ, const std::string key)
{
    auto val = json_object_get(rootJ, key.c_str());
    if (!val)
    {
        return {};
    }
    return convertFromJson<T>(val);
}
} // namespace sst::rackhelpers::json
#endif
