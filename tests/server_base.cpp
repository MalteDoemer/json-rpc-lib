#include <doctest/doctest.h>

#include <jsonrpc/server_base.hpp>

using json = nlohmann::json;

class server_base_tester : public jsonrpc::server_base {

    virtual jsonrpc::response handle_validated_request(const jsonrpc::request& request) override
    {
        // return an empty response for notification requests.
        // Note: this statement is not neccessary because the response constructor
        // would handle empty id's but its good for clarity.
        if (!request.has_id()) {
            return jsonrpc::response();
        }

        // our only method named test
        if (request.method() == "test") {
            auto result = json("ok");
            return jsonrpc::response(result, request.id());
        } else {
            auto error = jsonrpc::error(jsonrpc::method_not_found, "method not found");
            return jsonrpc::response(error, request.id());
        }
    }
};

#define CHECK_JSON_EQ(json1, json2) CHECK(json::parse(json1) == json::parse(json2))

std::string get_parse_error_message(std::string json)
{
    std::string error_msg;

    try {
        auto unused = json::parse(json);
    } catch (json::exception& e) {
        error_msg = e.what();
    }

    return error_msg;
}

TEST_CASE("test correct examples")
{
    server_base_tester server;

    // rpc call with positional parameters and integer id
    auto req = R"({"jsonrpc": "2.0", "method": "test", "params": [42, 23], "id": 1})";
    auto res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "result": "ok", "id": 1})");

    // rpc call with positional parameters and string id
    req = R"({"jsonrpc": "2.0", "method": "test", "params": [42, 23], "id": "some_id"})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "result": "ok", "id": "some_id"})");

    // rpc call with named parameters
    req = R"({"jsonrpc": "2.0", "method": "test", "params": { "a": 5, "b": "some_thing" }, "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "result": "ok", "id": 1})");

    // rpc call without parameters
    req = R"({"jsonrpc": "2.0", "method": "test", "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "result": "ok", "id": 1})");

    // rpc notification call
    req = R"({"jsonrpc": "2.0", "method": "test", "params": [42, 23]})";
    res = server.handle_request(req);
    CHECK(res.empty());

    // rpc notification call without parameters
    req = R"({"jsonrpc": "2.0", "method": "test"})";
    res = server.handle_request(req);
    CHECK(res.empty());

    // rpc call with batch
    req = R"([
        {"jsonrpc": "2.0", "method": "test", "params": [1,2,4], "id": "1"},
        {"jsonrpc": "2.0", "method": "test", "params": [7]},
        {"jsonrpc": "2.0", "method": "test", "params": [42,23], "id": "2"},
        {"jsonrpc": "2.0", "method": "test" },
        {"jsonrpc": "2.0", "method": "test", "id": "9"} 
    ])";

    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"([
        {"jsonrpc": "2.0", "result": "ok", "id": "1"},
        {"jsonrpc": "2.0", "result": "ok", "id": "2"},
        {"jsonrpc": "2.0", "result": "ok", "id": "9"}
    ])");


    // rpc call with all notification batch
    req = R"([
        {"jsonrpc": "2.0", "method": "test", "params": [1,2,4]},
        {"jsonrpc": "2.0", "method": "test", "params": [7]},
        {"jsonrpc": "2.0", "method": "test", "params": [42,23]},
        {"jsonrpc": "2.0", "method": "test" },
        {"jsonrpc": "2.0", "method": "test"} 
    ])";

    res = server.handle_request(req);
    CHECK(res.empty());
}

/*
TEST_CASE("") {
    // rpc call with non-existant method
    auto req = R"({"jsonrpc": "2.0", "method": "non_existant_method", "id": 1})";
    auto res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "error": {"code": -32601, "message": "method not found"}, "id": 1})");

    // rpc call with invalid json
    req = R"({"jsonrpc": "2.0", "method": "foobar, "params": "bar", "baz])";
    res = server.handle_request(req);

    auto error_msg = get_parse_error_message(req);
    auto error = jsonrpc::error(jsonrpc::parse_error, "parse error: " + error_msg);
    auto response = jsonrpc::response(error, jsonrpc::id_type(nullptr));
    auto result = json(response).dump();
    CHECK_JSON_EQ(res, result);

    // rpc call with invalid request method
    req = R"({"jsonrpc": "2.0", "method": 1, "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "error": {"code": -32600, "message": "invalid request: method field must be
a string"}, "id": 1})");

    req = R"({"jsonrpc": "2.0", "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "error": {"code": -32600, "message": "invalid request: method field must be
a string"}, "id": 1})");

    // rpc call with invalid jsonrpc field
    req = R"({"jsonrpc": "1.0", "method": "test", "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "error": {"code": -32600, "message": "invalid request: jsonrpc field must
be \"2.0\""}, "id": 1})");

    req = R"({"method": "test", "id": 1})";
    res = server.handle_request(req);
    CHECK_JSON_EQ(res, R"({"jsonrpc": "2.0", "error": {"code": -32600, "message": "invalid request: jsonrpc field must
be \"2.0\""}, "id": 1})");
} */