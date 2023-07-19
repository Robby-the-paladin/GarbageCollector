#include "PostgreHandler.h"

PostgreHandler::PostgreHandler(): connect(pqxx::connection("user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write")) {
   
    pqxx::work W(connect);
    W.exec("CREATE TABLE IF NOT EXISTS depr (name TEXT PRIMARY KEY, data TEXT[])");
    W.commit();
    connect.prepare("insert", "INSERT INTO depr VALUES ($1, $2)");
}


bool PostgreHandler::addDeprecated(std::string name, std::set<std::string> data) {
    getDeprecated(name, data);
    pqxx::work W(connect);
    W.exec("DELETE FROM depr WHERE name = '" + name + "'");
    std::vector<std::string> vc(data.begin(), data.end());
    W.exec_prepared("insert", name, vc);
    W.commit();
    
    return true;
}

bool PostgreHandler::getDeprecated(std::string name,  std::set<std::string>& data) {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT data FROM depr WHERE name = '" + name + "'"));
    
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