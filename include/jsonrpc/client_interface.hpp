#pragma once

#include <string>

namespace jsonrpc {

/**
 * The client_interface servers as interface between this library and the code that 
 * will send requests for example via http.
*/
class client_interface {
public:
    virtual ~client_interface() = default;
    virtual std::string send(const std::string& request) = 0;
};

}