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
        std::cout << (*it)[1] << std::endl;
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