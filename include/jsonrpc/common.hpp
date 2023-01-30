#pragma once

#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace jsonrpc {
using json = nlohmann::json;
using value_t = json::value_t;

/**
 * Helper function to check if the json value is one of the given types.
 */
template<typename arg_type>
bool has_type(const json& json, arg_type type)
{
    return json.type() == type;
}

/**
 * Helper function to check if the json value is one of the given types.
 */
template<typename arg_type, typename... other_types>
bool has_type(const json& json, arg_type type, other_types... others)
{
    return has_type(json, type) || has_type<other_types...>(json, others...);
}

/**
 * Helper function to check if a given key exists and if that key is one of the given types.
 */
template<typename... arg_types>
bool has_key_and_type(const json& json, const std::string& key, arg_types... args)
{
    return json.contains(key) && has_type<arg_types...>(json.at(key), args...);
}

/**
 * Checks if the json object has a valid jsonrpc id field.
 */
inline bool has_valid_id(const json& obj)
{
    return has_key_and_type(
        obj, "id", value_t::string, value_t::number_integer, value_t::number_unsigned, value_t::null);
}

/**
 * Checks if the json object has a valid jsonrpc field with the value "2.0".
 */
inline bool has_valid_jsonrpc(const json& obj)
{
    return has_key_and_type(obj, "jsonrpc", value_t::string) && obj["jsonrpc"] == "2.0";
}

/**
 * This class represents a jsonrpc id.
 * An instance can either be empty, meaning that no id was present in the request,
 * or it can be a json value which is either an integer, string or null.
 */
class id_type {

public:
    explicit id_type() : m_has_value(false), m_value(nullptr) {}
    explicit id_type(const nlohmann::json& json) : m_has_value(true), m_value(json) {}

    bool is_empty() const { return !m_has_value; }

    const nlohmann::json& value() const { return m_value; }

private:
    bool m_has_value;
    nlohmann::json m_value;
};

/**
 * Error codes from -32768 to -32000 are reserved for predefined errors. The rest are free to use.
 * @note The server_error has a range from -32000 to -32099 which is reserved for use by the server.
 */
enum error_type : int {

    /// Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
    parse_error = -32700,
    /// The JSON sent is not a valid Request object.
    invalid_request = -32600,
    /// The method does not exist / is not available.
    method_not_found = -32601,
    /// Invalid method parameter(s).
    invalid_params = -32602,
    /// Internal JSON-RPC error.
    internal_error = -32603,
    /// Reserved for implementation-defined server-errors.
    server_error = -32000,

    invalid_error = 0,
};

/**
 * This class represents a jsonrpc error object containing a code, a message and optionally json data.
 */
class error {

public:
    explicit error() : m_code(invalid_error), m_message("invalid error"), m_data(nullptr) {}
    explicit error(int code, const std::string& message) : m_code(code), m_message(message), m_data(nullptr) {}
    explicit error(int code, const std::string& message, const json& data) :
        m_code(code), m_message(message), m_data(data)
    {
    }

    int code() const noexcept { return m_code; }

    jsonrpc::error_type type() const noexcept { return parse_error_code(m_code); }

    const std::string& message() const noexcept { return m_message; }

    const json& data() const noexcept { return m_data; }

    bool has_data() const noexcept { return !m_data.is_null(); }

    /**
     * Constructs a human readable error message.
     */
    std::string to_string() const
    {
        if (m_data.is_null()) {
            return std::to_string(m_code) + ": " + m_message;
        } else {
            return std::to_string(m_code) + ": " + m_message + ", data: " + m_data.dump();
        }
    }

private:
    inline static jsonrpc::error_type parse_error_code(int code)
    {
        switch (code) {
        case -32700:
            return parse_error;
        case -32600:
            return invalid_request;
        case -32601:
            return method_not_found;
        case -32602:
            return invalid_params;
        case -32603:
            return internal_error;
        default:
            if (code >= -32099 && code <= -32000)
                return server_error;
            else
                return invalid_error;
        }
    }

private:
    int m_code;
    std::string m_message;
    json m_data;
};

/**
 * This exception class wraps a jsonrpc::error.
 */
class exception : public std::exception {

public:
    /**
     * Construct a new jsonrpc::exception from a jsonrpc::error.
     */
    explicit exception(const jsonrpc::error& error) : m_error(error), m_full_message(m_error.to_string()) {}

    /**
     * Construct a new jsonrpc::exception with an error code and a error message.
     */
    explicit exception(int code, const std::string& message) :
        m_error(code, message), m_full_message(m_error.to_string())
    {
    }

    /**
     * Construct a new jsonrpc::exception with an error code, a message and json data.
     */
    explicit exception(int code, const std::string& message, const json& data) :
        m_error(code, message, data), m_full_message(m_error.to_string())
    {
    }

    const jsonrpc::error& error() const noexcept { return m_error; }

    const std::string& full_message() const noexcept { return m_full_message; }

    const char* what() const noexcept override { return m_full_message.c_str(); }

private:
    jsonrpc::error m_error;
    std::string m_full_message;
};

inline void to_json(json& j, const jsonrpc::error& p)
{
    if (p.has_data()) {
        j = json { { "code", p.code() }, { "message", p.message() }, { "data", p.data() } };
    } else {
        j = json { { "code", p.code() }, { "message", p.message() } };
    }
}

inline void from_json(const json& j, jsonrpc::error& p)
{
    auto code = j.at("code").get<int>();
    auto message = j.at("message").get<std::string>();

    json data = nullptr;

    if (j.contains("data")) {
        data = j.at("data");
    }

    p = jsonrpc::error(code, message, data);
}

}
