#pragma once

#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#include <unordered_map>

#include "server_base.hpp"

namespace jsonrpc {

using method_handle = std::function<json(const json&)>;
using parameter_mapping = std::vector<std::string>;
static parameter_mapping empty_parameter_mapping;

class server : public jsonrpc::server_base {

public:
    bool add_method(const std::string& name, method_handle handle, parameter_mapping mapping = empty_parameter_mapping)
    {
        if (contains(name)) {
            return false;
        }

        methods[name] = handle;
        if (!mapping.empty()) {
            mappings[name] = mapping;
        }

        return true;
    }

    bool contains(const std::string& name) const { return methods.find(name) != methods.end(); }

protected:
    virtual jsonrpc::response handle_validated_request(const jsonrpc::request& request) override
    {
        if (!contains(request.method())) {
            throw jsonrpc::exception(method_not_found, "method not found: " + request.method());
        }

        auto params = process_params(request);

        try {
            auto result = methods[request.method()](params);
            return jsonrpc::response(result, request.id());
        } catch (json::type_error& e) {
            throw jsonrpc::exception(invalid_params, "invalid parameter: " + std::string(e.what()));
        }
    }

    json process_params(const jsonrpc::request& request)
    {
        auto name = request.method();
        auto params = request.params();

        if (params.is_array()) {
            return params;
        } else /* is_object() */ {

            if (mappings.find(name) == mappings.end()) {
                throw jsonrpc::exception(invalid_params,
                    "invalid params: procedure \"" + request.method() + "\" does not support named parameters");
            }

            auto result = json::array();
            for (const auto& param_name : mappings[name]) {
                if (!params.contains(param_name)) {
                    throw jsonrpc::exception(
                        invalid_params, "invalid params: missing named parameter \"" + param_name + "\"");
                }
                result.push_back(params[param_name]);
            }

            return result;
        }
    }

private:
    std::unordered_map<std::string, method_handle> methods;
    std::unordered_map<std::string, parameter_mapping> mappings;
};

template<typename return_type, typename... param_types, std::size_t... index>
method_handle create_method_handle_impl(
    std::function<return_type(param_types...)> method, std::index_sequence<index...>)
{
    method_handle handle = [method](const json& params) -> json {
        auto actual_size = params.size();
        auto expected_size = sizeof...(param_types);

        if (actual_size != expected_size) {
            auto msg = "invalid params: expected " + std::to_string(expected_size) + " argument(s), but found "
                + std::to_string(actual_size);

            throw jsonrpc::exception(invalid_params, msg);
        }

        return method(params[index].get<std::decay_t<param_types>>()...);
    };

    return handle;
}

template<typename return_type, typename... param_types>
method_handle get_handle(std::function<return_type(param_types...)> method)
{
    return create_method_handle_impl(method, std::index_sequence_for<param_types...> {});
}

template<typename return_type, typename... param_types>
method_handle get_handle(return_type (*method)(param_types...))
{
    return get_handle(std::function<return_type(param_types...)>(method));
}

template<typename class_type, typename return_type, typename... param_types>
method_handle get_handle(return_type (class_type::*method)(param_types...), class_type& instance)
{
    std::function<return_type(param_types...)> func = [&instance, method](param_types&&... params) -> return_type {
        return (instance.*method)(std::forward<param_types>(params)...);
    };

    return get_handle(func);
}

}
