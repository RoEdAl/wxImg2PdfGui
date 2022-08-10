/*
 * wxJdon.h
*/
#ifndef _WX_JSON_H_
#define _WX_JSON_H_

#include <nlohmann/json.hpp>

typedef nlohmann::basic_json<
    std::map,
    std::vector,
    std::string,
    bool,
    wxInt64,
    wxUint64,
    wxDouble,
    std::allocator,
    nlohmann::adl_serializer,
    std::vector< wxByte >
> wxJson;

#endif
