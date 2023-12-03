#include "Cacher.h"

std::optional<Aux::checked_package> Cacher::getCache(std::string key)
{   

    if (in_memore_cache.find(key) != in_memore_cache.end()) {
        return Aux::checked_package{
            name: key,
            http_code: 200,
            can_delete: in_memore_cache[key],
        };
    }

    ch_mutex.lock();

    auto elem = ph.getDeprecated(key);
    if (!elem.has_value()) {
        ch_mutex.unlock();
        return std::nullopt;
    }

    in_memore_cache[key] = elem.value();
    
    ch_mutex.unlock();
    
    return Aux::checked_package{
        name: key,
        http_code: 200,
        can_delete: elem.value(),
    };
}

void Cacher::setCache(std::string key, Aux::checked_package pack)
{   
    ch_mutex.lock();
    ph.addDeprecated(key, pack.can_delete);
    ch_mutex.unlock();
}

void Cacher::setCache(std::string key, bool can_delete)
{   
    ch_mutex.lock();
    ph.addDeprecated(key, can_delete);
    ch_mutex.unlock();
}
