#ifndef __SST_RACKHELPERS_JSON
#define __SST_RACKHELPERS_JSON

#include <optional>
#include <string>

namespace sst::rackhelpers::json
{
template<typename T> std::optional<T> convertFromJson(json_t *o) {
    return {};
}
template<> std::optional<std::string> convertFromJson<std::string>(json_t *o)
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
template<> std::optional<bool> convertFromJson<bool>(json_t *o)
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
template<> std::optional<int> convertFromJson<int>(json_t *o)
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

template<> std::optional<float> convertFromJson<float>(json_t *o)
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


template<> std::optional<double> convertFromJson<double>(json_t *o)
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


template<typename T> std::optional<T> jsonSafeGet(json_t *rootJ, const std::string key)
{
    auto val = json_object_get(rootJ, key.c_str());
    if (!val)
    {
        return {};
    }
    return convertFromJson<T>(val);
}
}
#endif
