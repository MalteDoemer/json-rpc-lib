#pragma once

#include "common.hpp"

namespace jsonrpc {

/**
 * This class represents a jsonrpc request object.
 */
class request {

public:
    explicit request() : m_method(), m_params(json::array()), m_id() {}
    explicit request(std::string method, json params, jsonrpc::id_type id) :
        m_method(method), m_params(params), m_id(id)
    {
    }

    const std::string& method() const { return m_method; }

    const json& params() const { return m_params; }

    const jsonrpc::id_type& id() const { return m_id; }

    bool has_id() const { return !m_id.is_empty(); }

private:
    std::string m_method;
    json m_params;
    jsonrpc::id_type m_id;
};

inline void to_json(json& j, const request& req)
{
    if (req.has_id()) {
        j = {
            { "jsonrpc", "2.0" },
            { "method", req.method() },
            { "params", req.params() },
            { "id", req.id().value() },
        };
    } else {
        j = {
            { "jsonrpc", "2.0" },
            { "method", req.method() },
            { "params", req.params() },
        };
    }
}
}