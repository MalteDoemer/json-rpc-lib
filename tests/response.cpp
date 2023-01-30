#include <doctest/doctest.h>

#include <jsonrpc/response.hpp>

using json = nlohmann::json;

TEST_CASE("test response object construction")
{
    auto res = jsonrpc::response();
    CHECK(res.is_empty());

    res = jsonrpc::response(json::object(), jsonrpc::id_type());
    CHECK(res.is_empty());

    res = jsonrpc::response(jsonrpc::error(), jsonrpc::id_type());
    CHECK(res.is_empty());

    res = jsonrpc::response(json::object(), jsonrpc::id_type(6));
    CHECK(res.is_ok());

    res = jsonrpc::response(jsonrpc::error(), jsonrpc::id_type(6));
    CHECK(res.is_error());
}

TEST_CASE("test serialization of response object")
{
    auto res = jsonrpc::response();
    auto j = json(res);
    CHECK(j.is_null());

    res = jsonrpc::response(json::object(), jsonrpc::id_type());
    j = json(res);
    CHECK(j.is_null());

    res = jsonrpc::response(jsonrpc::error(), jsonrpc::id_type());
    j = json(res);
    CHECK(j.is_null());

    res = jsonrpc::response(json::object(), jsonrpc::id_type(6));
    j = json(res);
    CHECK(j.is_object());
    CHECK(j.at("result").is_object());
    CHECK_FALSE(j.contains("error"));
    CHECK(j.at("id") == 6);

    res = jsonrpc::response(jsonrpc::error(), jsonrpc::id_type(6));
    j = json(res);
    CHECK(j.is_object());
    CHECK(j.at("error").is_object());
    CHECK_FALSE(j.contains("result"));
    CHECK(j.at("id") == 6);
}