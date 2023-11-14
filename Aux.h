#pragma once

#include <string>


namespace Aux {
     struct checked_package {
        // название пакета
        std::string name;
        // код ответа севрера
        long http_code;
        // true если никем не используется, иначе false
        bool can_delete;
    };
}