#include "PostgreHandler.h"

PostgreHandler::PostgreHandler(): connect(pqxx::connection("user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write")) {
   
    pqxx::work W(connect);
    W.exec("CREATE TABLE IF NOT EXISTS depr (name TEXT PRIMARY KEY, data TEXT[], depr_data TEXT[])");
    W.commit();
    connect.prepare("insert_data", "INSERT INTO depr VALUES ($1, $2, null)");
    connect.prepare("insert_depr_data", "INSERT INTO depr VALUES ($1, null, $2)");
    connect.prepare("update_data", "UPDATE depr SET data = $2 WHERE name = $1");
    connect.prepare("update_depr_data", "UPDATE depr SET depr_data = $2 WHERE name = $1");
}

void PostgreHandler::reconnect() {
    std::cerr << "Connection lost, trying to reconnect..." << std::endl;
    int times = 0;
    try {
        times++;
        if(!connect.is_open()) {
            connect = pqxx::connection("user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write");
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


bool PostgreHandler::addDeprecated(std::string name, std::string col, std::set<std::string> data) {
    getDeprecated(name, col, data);
    pqxx::work W(connect);
    auto res = W.exec("SELECT *  FROM depr WHERE name = '" + name + "'");
    std::vector<std::string> vc(data.begin(), data.end());
    if (res.begin() == res.end())
        W.exec_prepared("insert_" + col, name, vc);
    else
        W.exec_prepared("update_" + col, name, vc);
    W.commit();
    
    return true;
}

bool PostgreHandler::getDeprecated(std::string name, std::string col,  std::set<std::string>& data) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT " + col + " FROM depr WHERE name = '" + name + "'"));
    
    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
       // auto elems = (*it).as<std::vector<std::string>>();
     //   std::cout << (*it)[0] << std::endl;
        auto elems = (*it)[0].as_array();
        auto elem = elems.get_next();     //.as<std::vector<std::string>>();
        while (elem.first != pqxx::array_parser::juncture::done) {
            data.insert(std::string(elem.second));
            elem = elems.get_next();
        }
       // data.insert(elems.begin(), elems.end());
    }
    W.commit();
    return true;
}

std::set<std::string> PostgreHandler::getAllNames() {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT name FROM depr"));

    std::set<std::string> data;
    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
        auto elem = (*it)[0]; 
        data.insert(elem.as<std::string>());
    }
    W.commit();
    return data;
}

bool PostgreHandler::isDeprecatedNull(std::string name) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT depr_data FROM depr WHERE name = '" + name + "'"));
    W.commit();
    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
        auto elem = (*it)[0]; 
        return elem.is_null();
    }
    return true;
}

bool PostgreHandler::replaceDeprecatedWith(std::string name, std::string col, std::set<std::string> data) {
    pqxx::work W(connect);
    auto res = W.exec("SELECT *  FROM depr WHERE name = '" + name + "'");
    std::vector<std::string> vc(data.begin(), data.end());
    if (res.begin() == res.end())
        W.exec_prepared("insert_" + col, name, vc);
    else
        W.exec_prepared("update_" + col, name, vc);
    W.commit();
    
    return true;
}


bool PostgreHandler::getCheckedPackage(std::string name) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT \"check\" FROM deprcheck WHERE name = '" + name + "'"));
    if (R.begin() == R.end()) {
        W.exec("INSERT INTO deprcheck VALUES ('"+ name + "', null)");
    }
    W.commit();

    pqxx::work Update(connect);
    R = Update.exec("SELECT \"check\" FROM deprcheck WHERE name = '" + name + "'");
    for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
        auto elem = (*it)[0];
        if (elem.is_null()) {
            Api::checked_package chk = Api::checkPackage(name);
            if (chk.http_code == 200) {
                Update.exec("UPDATE deprcheck SET \"check\" = false WHERE name = '" + name + "'");
                Update.commit();
                return false;
            } else if   (chk.http_code == 404) {
                Update.exec("UPDATE deprcheck SET \"check\" = true WHERE name = '" + name + "'");
                Update.commit();
                return true;
            }
        } else {
            Update.commit();
            return elem.as<bool>();
        }
    }
    Update.commit();
    return true;
}