#pragma once

#include <variant>

#include "common.hpp"

namespace jsonrpc {

/**
 * This class represents a jsonrpc response object.
 * A response can have 3 states:
 * - It is empty, meaning no response at all (for notifications)
 * - It is successful and it contains a result object.
 * - It is erroneous and it contains an error object.
 *
 * @note A response is considerd empty when the given id is empty.
 */
class response {

public:
    /**
     * Constructs an empty response.
     */
    explicit response() : m_value(json()), m_id() {}

    /**
     * Constructs a successful response.
     * @note If the id is empty this will construct an empty response.
     */
    explicit response(const json& result, const jsonrpc::id_type& id) : m_value(result), m_id(id) {}

    /**
     * Constructs an error response.
     * @note If the id is empty this will construct an empty response.
     */
    explicit response(const jsonrpc::error& error, const jsonrpc::id_type& id) : m_value(error), m_id(id) {}

    bool is_empty() const { return m_id.is_empty(); }

    bool is_ok() const { return !is_empty() && m_value.index() == 0; }

    bool is_error() const { return !is_empty() && m_value.index() == 1; }

    /**
     * Get the result object of a successful response.
     */
    const json& result() const { return std::get<json>(m_value); }

    /**
     * Get the error object of an error response.
     */
    const jsonrpc::error& error() const { return std::get<jsonrpc::error>(m_value); }

    const jsonrpc::id_type& id() const { return m_id; }

private:
    std::variant<json, jsonrpc::error> m_value;
    jsonrpc::id_type m_id;
};

inline void to_json(json& j, const response& res)
{
    if (res.is_ok()) {
        j = {
            { "jsonrpc", "2.0" },
            { "result", res.result() },
            { "id", res.id().value() },
        };
    } else if (res.is_error()) {
        j = {
            { "jsonrpc", "2.0" },
            { "error", res.error() },
            { "id", res.id().value() },
        };
    } else {
        j = nullptr;
    }
}

}