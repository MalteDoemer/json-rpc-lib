#include <doctest/doctest.h>

#include <jsonrpc/common.hpp>

using json = nlohmann::json;

TEST_CASE("test error code conversion")
{
    CHECK_EQ(jsonrpc::error(jsonrpc::parse_error, "").type(), jsonrpc::parse_error);
    CHECK_EQ(jsonrpc::error(jsonrpc::invalid_request, "").type(), jsonrpc::invalid_request);
    CHECK_EQ(jsonrpc::error(jsonrpc::method_not_found, "").type(), jsonrpc::method_not_found);
    CHECK_EQ(jsonrpc::error(jsonrpc::invalid_params, "").type(), jsonrpc::invalid_params);
    CHECK_EQ(jsonrpc::error(jsonrpc::internal_error, "").type(), jsonrpc::internal_error);

    CHECK_EQ(jsonrpc::error(jsonrpc::server_error, "").type(), jsonrpc::server_error);

    for (int code = -32000; code >= -32099; code--) {
        CHECK_EQ(jsonrpc::error(code, "").type(), jsonrpc::server_error);
    }

    CHECK_EQ(jsonrpc::error(jsonrpc::invalid_error, "").type(), jsonrpc::invalid_error);
    CHECK_EQ(jsonrpc::error(999, "").type(), jsonrpc::invalid_error);
    CHECK_EQ(jsonrpc::error(-999, "").type(), jsonrpc::invalid_error);
}

TEST_CASE("test deserialization of error object from json")
{

    auto text = R"(
        {
            "code": -32600,
            "message": "invalid request"
        }
    )";

    auto err = json::parse(text).get<jsonrpc::error>();

    CHECK(err.code() == jsonrpc::invalid_request);
    CHECK(err.type() == jsonrpc::invalid_request);
    CHECK(err.message() == "invalid request");
    CHECK(err.data().is_null());

    text = R"(
        {
            "code": -32005,
            "message": "server error",
            "data" : {
                "description": "a server error occured"
            }
        }
    )";

    err = json::parse(text).get<jsonrpc::error>();

    CHECK(err.code() == -32005);
    CHECK(err.type() == jsonrpc::server_error);
    CHECK(err.message() == "server error");
    CHECK(err.data().is_object());
    CHECK(err.data().contains("description"));
    CHECK(err.data()["description"] == "a server error occured");
}

TEST_CASE("test serialization of error object to json")
{
    auto err = jsonrpc::error();
    auto j = json(err);

    CHECK(j.is_object());
    CHECK(j.at("code") == jsonrpc::invalid_error);
    CHECK(j.at("message") == "invalid error");
    CHECK_FALSE(j.contains("data"));

    err = jsonrpc::error(jsonrpc::invalid_params, "invalid params");
    j = json(err);

    CHECK(j.is_object());
    CHECK(j.at("code") == jsonrpc::invalid_params);
    CHECK(j.at("message") == "invalid params");
    CHECK_FALSE(j.contains("data"));

    err = jsonrpc::error(jsonrpc::internal_error, "internal error: unexpected error",
        json { { "exception", "std::out_of_memory_exception" } });
    j = json(err);

    CHECK(j.is_object());
    CHECK(j.at("code") == jsonrpc::internal_error);
    CHECK(j.at("message") == "internal error: unexpected error");
    CHECK(j.at("data").is_object());
    CHECK(j.at("data").at("exception").is_string());
}