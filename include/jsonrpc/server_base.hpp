#pragma once

#include <string>

#include "common.hpp"
#include "request.hpp"
#include "response.hpp"

namespace jsonrpc {

/**
 * Base class for the server. It handles the parsing of the request.
 */
class server_base {

public:
    std::string handle_request(const std::string& body)
    {
        try {
            auto request = json::parse(body);
            auto response = handle_request_impl(request);

            if (response.is_null() || response.empty()) {
                return "";
            } else {
                return response.dump();
            }

        } catch (json::exception& e) {
            auto error = jsonrpc::error(parse_error, std::string("parse error: ") + e.what());
            auto response = jsonrpc::response(error, jsonrpc::id_type(nullptr));
            return json(response).dump();
        }
    }

private:
    json handle_request_impl(const json& request)
    {
        if (request.is_array()) {
            return handle_batch_request(request);
        } else if (request.is_object()) {
            return handle_single_request(request);
        } else {
            auto error = jsonrpc::error(invalid_request, "invalid request: expected array or object");
            auto response = jsonrpc::response(error, jsonrpc::id_type(nullptr));
            return json(response);
        }
    }

    json handle_batch_request(const json& request)
    {
        if (request.empty()) {
            auto error = jsonrpc::error(invalid_request, "invalid request: batch array must not be empty");
            auto response = jsonrpc::response(error, jsonrpc::id_type(nullptr));
            return json(response);
        }

        auto result = json::array();
        for (const auto& req : request) {
            auto res = handle_single_request(req);
            if (!res.is_null()) {
                result.push_back(std::move(res));
            }
        }

        return result;
    }

    json handle_single_request(const json& request)
    {
        auto id = get_id_or_null(request);
        try {

            auto valid_request = validate_request(request);
            auto response = handle_validated_request(valid_request);
            return json(response);
        } catch (jsonrpc::exception& e) {
            auto error = e.error();
            auto response = jsonrpc::response(error, id);
            return json(response);
        } catch (std::exception& e) {
            auto error = jsonrpc::error(internal_error, std::string("internal error: ") + e.what());
            auto response = jsonrpc::response(error, id);
            return json(response);
        } catch (...) {
            auto error = jsonrpc::error(internal_error, std::string("internal error"));
            auto response = jsonrpc::response(error, id);
            return json(response);
        }
    }

    jsonrpc::id_type get_id_or_null(const json& request)
    {
        if (!request.contains("id")) {
            return jsonrpc::id_type();
        } else if (has_valid_id(request)) {
            return jsonrpc::id_type(request["id"]);
        } else {
            return jsonrpc::id_type(nullptr);
        }
    }

    jsonrpc::id_type get_id(const json& request)
    {
        if (!request.contains("id")) {
            return jsonrpc::id_type();
        } else if (has_valid_id(request)) {
            return jsonrpc::id_type(request["id"]);
        } else {
            throw jsonrpc::exception(invalid_request, "invalid request: id field must be a number, string or null");
        }
    }

    std::string get_method(const json& request)
    {
        if (has_key_and_type(request, "method", value_t::string)) {
            return request["method"].get<std::string>();
        } else {
            throw jsonrpc::exception(invalid_request, "invalid request: method field must be a string");
        }
    }

    json get_params(const json& request)
    {
        if (!request.contains("params") || request["params"].is_null()) {
            return json::array();
        } else if (has_key_and_type(request, "params", value_t::array, value_t::object, value_t::null)) {
            return request["params"];
        } else {
            throw jsonrpc::exception(invalid_request, "invalid request: params field must be an array, object or null");
        }
    }

    jsonrpc::request validate_request(const json& request)
    {
        if (!has_valid_jsonrpc(request)) {
            throw jsonrpc::exception(invalid_request, "invalid request: jsonrpc field must be \"2.0\"");
        }

        auto id = get_id(request);
        auto method = get_method(request);
        auto params = get_params(request);

        return jsonrpc::request(method, params, id);
    }

protected:
    virtual jsonrpc::response handle_validated_request(const jsonrpc::request& request) = 0;
};

}
