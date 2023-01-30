#include <iostream>

#include <jsonrpc/server.hpp>

using json = nlohmann::json;

class tester {

public:
    int test(int param)
    {
        std::cout << "test\n";
        return param + 1;
    }
};

int main()
{
    tester t;
    jsonrpc::server server;

    // // server.add_method("test", [](const json& params) {
    // //     std::cout << "param 1: " << params.at(0) << "\n";
    // //     return json(5);
    // // });

    server.add_method("test", jsonrpc::get_handle(&tester::test, t), { "param" });

    auto request = R"(
        {
            "jsonrpc": "2.0",
            "method": "test",
            "params": [1],
            "id": 1
        }
    )";

    auto res = server.handle_request(request);

    std::cout << res << "\n";
}