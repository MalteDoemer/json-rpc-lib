#include <doctest/doctest.h>

#include <jsonrpc/request.hpp>

using json = nlohmann::json;

TEST_CASE("test request object construction")
{

    auto req = jsonrpc::request();
    CHECK_FALSE(req.has_id());
    CHECK(req.params().is_array());

    req = jsonrpc::request("test", json::array(), jsonrpc::id_type());
    CHECK_FALSE(req.has_id());
    CHECK(req.method() == "test");
    CHECK(req.params().is_array());

    req = jsonrpc::request("test", json { 5, 7, "test", nullptr }, jsonrpc::id_type(6));
    CHECK(req.has_id());
    CHECK(req.id().value() == 6);
    CHECK(req.method() == "test");
    CHECK(req.params().is_array());
}

TEST_CASE("test serialization of request object")
{

    auto req = jsonrpc::request();
    auto j = json(req);

    CHECK(j.is_object());
    CHECK(j.at("method") == "");
    CHECK(j.at("params") == json::array());
    CHECK_FALSE(j.contains("id"));

    req = jsonrpc::request("test", json { { "a", 5 }, { "b", 9 } }, jsonrpc::id_type());
    j = json(req);

    CHECK(j.is_object());
    CHECK(j.at("method") == "test");
    CHECK(j.at("params").is_object());
    CHECK(j.at("params").at("a") == 5);
    CHECK(j.at("params").at("b") == 9);
    CHECK_FALSE(j.contains("id"));

    req = jsonrpc::request("test", json { 5, 7, "test", nullptr }, jsonrpc::id_type(6));
    j = json(req);
    CHECK(j.is_object());
    CHECK(j.at("method") == "test");
    CHECK(j.at("params").is_array());
    CHECK(j.at("params").size() == 4);
    CHECK(j.at("id") == 6);
}