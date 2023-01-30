#include <doctest/doctest.h>

#include <jsonrpc/common.hpp>

using json = nlohmann::json;

TEST_CASE("test id behaviour")
{
    CHECK(jsonrpc::id_type().is_empty());

    json j = nullptr;

    CHECK_FALSE(jsonrpc::id_type(j).is_empty());
    CHECK(jsonrpc::id_type(j).value().is_null());

    j = "x";

    CHECK_FALSE(jsonrpc::id_type(j).is_empty());
    CHECK(jsonrpc::id_type(j).value().is_string());

    j = 5;

    CHECK_FALSE(jsonrpc::id_type(j).is_empty());
    CHECK(jsonrpc::id_type(j).value().is_number_integer());
}