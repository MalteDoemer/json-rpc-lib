#include <doctest/doctest.h>

#include <jsonrpc/common.hpp>

using json = nlohmann::json;

TEST_CASE("test helper functions")
{
    auto obj = json { { "jsonrpc", "2.0" }, { "id", "test" } };

    CHECK(jsonrpc::has_valid_jsonrpc(obj));
    CHECK(jsonrpc::has_valid_id(obj));

    obj["id"] = 5;
    CHECK(jsonrpc::has_valid_id(obj));

    obj["id"] = nullptr;
    CHECK(jsonrpc::has_valid_id(obj));

    obj["jsonrpc"] = "x";
    CHECK_FALSE(jsonrpc::has_valid_jsonrpc(obj));

    obj["jsonrpc"] = 5;
    CHECK_FALSE(jsonrpc::has_valid_jsonrpc(obj));

    obj["id"] = json { { "test", "test" } };
    CHECK_FALSE(jsonrpc::has_valid_id(obj));

    obj["id"] = json::array();
    CHECK_FALSE(jsonrpc::has_valid_id(obj));

    obj = json::object();
    CHECK_FALSE(jsonrpc::has_valid_jsonrpc(obj));
    CHECK_FALSE(jsonrpc::has_valid_id(obj));

    // TODO: check if fractional numbers should be considered legal.
    // The spec says "The value SHOULD normally not be Null and Numbers SHOULD NOT contain fractional parts"
    obj["id"] = 5.10f;
    CHECK_FALSE(jsonrpc::has_valid_id(obj));
}