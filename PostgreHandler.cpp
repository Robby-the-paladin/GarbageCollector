#include "PostgreHandler.h"

extern std::string postgreConnStr;

PostgreHandler::PostgreHandler(): connect(pqxx::connection(postgreConnStr)) {
   
    pqxx::work W(connect);
    W.exec("CREATE TABLE IF NOT EXISTS packages_what_dep_src (name TEXT PRIMARY KEY, \"what_src\" boolean)");
    W.commit();
    // connect.prepare("insert_data", "INSERT INTO depr VALUES ($1, $2, null, 0)");
    // connect.prepare("insert_depr_data", "INSERT INTO depr VALUES ($1, null, $2, 0)");
    // connect.prepare("update_data", "UPDATE depr SET data = $2 WHERE name = $1");
    // connect.prepare("update_depr_data", "UPDATE depr SET depr_data = $2 WHERE name = $1");
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
    auto res = W.exec("SELECT *  FROM packages_what_dep_src WHERE name = '" + name + "'");
    if (res.begin() == res.end())
        W.exec_prepared("INSERT INTO packages_what_dep_src VALUES ($1, $2)", name, data);
    W.commit();
    
    return true;
}


std::optional<bool> PostgreHandler::getDeprecated(std::string name) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT what_src FROM packages_what_dep_src WHERE name = '" + name + "'"));
    W.commit();

    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
            auto elem = (*it)[0];
            if (elem.is_null()) {
                return std::nullopt;
            } else {
                return elem.as<bool>();
            }
        }
    
    return true;
}