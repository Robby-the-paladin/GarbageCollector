#pragma once

#include "PostgreHandler.h"
#include "Aux.h"

#include <map>

class Cacher
{
    public:
        Cacher(){
            PostgreHandler ph();
        };

        // пытается получить значение из in_momore если нет то из БД
        std::optional<Aux::checked_package> getCache(std::string key);
        // задает значение кеша по key
        void setCache(std::string key, Aux::checked_package pack);
        void setCache(std::string key, bool can_delete);
    private:
        // работа с БД
        PostgreHandler ph;
        std::mutex ch_mutex;

        // мапа для хранения кеша 
        std::map<std::string, bool> in_memore_cache;
};

