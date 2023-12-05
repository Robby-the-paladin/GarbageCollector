#include "PostgreHandler.h"

extern std::string postgreConnStr;

PostgreHandler::PostgreHandler(): connect(pqxx::connection(postgreConnStr)) {
    table_name = "packages_what_dep_src_TEST";
    pqxx::work W(connect);
    W.exec("CREATE TABLE IF NOT EXISTS " + table_name + " (name TEXT PRIMARY KEY, \"what_src\" boolean)");
    W.commit();
}

void PostgreHandler::reconnect() {
    std::cerr << "Connection lost, trying to reconnect..." << std::endl;
    int times = 0;
    try {
        times++;
        if(!connect.is_open()) {
            connect = pqxx::connection(postgreConnStr);
        }
        times = 0;
    }
    catch(const pqxx::broken_connection & e) {
        if(times > 10) {
            times = 0;
            return;
        }
        reconnect();
    }
};


bool PostgreHandler::addDeprecated(std::string name, bool data) {
    // getDeprecated(name, col, data);
    pqxx::work W(connect);
    auto res = W.exec("SELECT *  FROM " + table_name + " WHERE name = '" + name + "'");
    if (res.begin() == res.end())
        auto res = W.exec_params("INSERT INTO " + table_name + " VALUES ($1, $2)", name, data);
    W.commit();
    
    return true;
}


std::optional<bool> PostgreHandler::getDeprecated(std::string name) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT what_src FROM " + table_name + " WHERE name = '" + name + "'"));
    W.commit();

    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
            auto elem = (*it)[0];
            if (elem.is_null()) {
                return std::nullopt;
            } else {
                return elem.as<bool>();
            }
        }
    
    return std::nullopt;
}