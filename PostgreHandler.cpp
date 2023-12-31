#include "PostgreHandler.h"

extern std::string postgreConnStr;

PostgreHandler::PostgreHandler(): connect(pqxx::connection(postgreConnStr)) {
   
    pqxx::work W(connect);
    W.exec("CREATE TABLE IF NOT EXISTS depr (name TEXT PRIMARY KEY, data TEXT[], depr_data TEXT[], \"count\" INTEGER)");
    W.exec("CREATE TABLE IF NOT EXISTS deprcheck (name TEXT PRIMARY KEY, \"check\" boolean, \"actual_package\" boolean)");
    W.commit();
    connect.prepare("insert_data", "INSERT INTO depr VALUES ($1, $2, null, 0)");
    connect.prepare("insert_depr_data", "INSERT INTO depr VALUES ($1, null, $2, 0)");
    connect.prepare("update_data", "UPDATE depr SET data = $2 WHERE name = $1");
    connect.prepare("update_depr_data", "UPDATE depr SET depr_data = $2 WHERE name = $1");
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

bool PostgreHandler::addCount(std::string name, int count) {
    pqxx::work W(connect);
    W.exec(("UPDATE depr SET \"count\" = " + std::to_string(count) + " WHERE name = '" + name + "'").c_str());
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

std::set<std::string> PostgreHandler::getNamesWithData() {
    pqxx::work W(connect);
    pqxx::result R (W.exec("SELECT name FROM depr WHERE NOT (depr.data is null)"));

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
    ph_lock.lock();
    pqxx::work W(connect);
    auto res = W.exec("SELECT *  FROM depr WHERE name = '" + name + "'");
    std::vector<std::string> vc(data.begin(), data.end());
    if (res.begin() == res.end())
        W.exec_prepared("insert_" + col, name, vc);
    else
        W.exec_prepared("update_" + col, name, vc);
    W.commit();
    ph_lock.unlock();
    
    return true;
}


std::vector<std::string> PostgreHandler::getCheckedPackages(std::vector<std::string> names, std::string branch, std::set<std::string>& unic_list) {
    Api a;
    std::vector<std::string> check, out_true, out_false;
    for (auto name: names) {
        ph_lock.lock();
        pqxx::work W(connect);
        std::string output;

        for (char c : name) {
            if (c == '\'') {
                output += "''";  // Заменяем кавычку на две кавычки
            } else {
                output += c;     // Остальные символы оставляем без изменений
            }
        }
        pqxx::result R (W.exec("SELECT \"check\" FROM deprcheck WHERE name = '" + output + "'"));
        if (R.begin() == R.end()) {
            W.exec("INSERT INTO deprcheck VALUES ('"+ output + "', null, null)");
        }
        W.commit();
      //  ph_lock.unlock();
    
        pqxx::work Update(connect);
      //  pqxx::result R;
        R = Update.exec("SELECT \"check\" FROM deprcheck WHERE name = '" + output + "'");
        Update.commit();
        ph_lock.unlock();

        for (pqxx::result::const_iterator it = R.begin(); it != R.end(); it++ ) {
            auto elem = (*it)[0];
            if (elem.is_null()) {
                bool check_name = true;
                for(int i = 0; i < name.size(); i++) {
                    if (!((name[i] >= 'a' && name[i] <= 'z') ||
                        (name[i] >= 'A' && name[i] <= 'Z') ||
                        (name[i] >= '0' && name[i] <= '9') ||
                         name[i] == '.' || name[i] == '_' || name[i] == '%' || name[i] == '-')) {
                            check_name = false;
                            break;
                        }
                }
                if (!check_name) {
                    continue;
                }
                check.push_back(name);
            } else {
                if (elem.as<bool>()) {
                    out_true.push_back(name);
                } else {
                    out_false.push_back(name);
                }
                //return elem.as<bool>();
            }
        }
    }


    std::vector<Api::checked_package> chks = a.divide_et_impera(check, branch);
    map<string, bool> check;
    for (auto chk: chks) {
        ph_lock.lock();
        pqxx::work Up(connect);
        if (chk.http_code == 200) {
            check[chk.name] = false;
            Up.exec("UPDATE deprcheck SET \"check\" = false WHERE name = '" + chk.name + "'");
           // Up.commit();
            //ph_lock.unlock();
            //return false;
        } else if   (chk.http_code == 404) {
            check[chk.name] = true;
            Up.exec("UPDATE deprcheck SET \"check\" = true WHERE name = '" + chk.name + "'");
          //  Up.commit();
          //  ph_lock.unlock();
            //return true;
        }
        Up.commit();
        ph_lock.unlock();
    }
    
    for(auto chk: chks) {
        ph_lock.lock();
        pqxx::work Up(connect);
        if (unic_list.find(chk.name) == unic_list.end()) {
            if (check[chk.name])
                out_true.push_back(chk.name);
            Up.exec("UPDATE deprcheck SET \"actual_package\" = false WHERE name = '" + chk.name + "'");
        } else {
            Up.exec("UPDATE deprcheck SET \"actual_package\" = true WHERE name = '" + chk.name + "'");
        }
        Up.commit();
        ph_lock.unlock();
    }

    return out_true;
}

bool PostgreHandler::checkDeprDate(std::string pname, std::string branch) {
    ph_lock.lock();
    pqxx::work W(connect);
    std::string output;

        for (char c : pname) {
            if (c == '\'') {
                output += "''";  // Заменяем кавычку на две кавычки
            } else {
                output += c;     // Остальные символы оставляем без изменений
            }
        }
    pqxx::result R (W.exec("SELECT builtime FROM deprcheck WHERE name = '" + output + "'"));
    ph_lock.unlock();
    auto elem = (*R.begin())[0];
    Api api;
    int time;
    if (elem.is_null()) {
        auto result = api.getDate(branch, pname);
        time = result.value().first;
        ph_lock.lock();
        pqxx::work Up(connect);
        Up.exec_params1("UPDATE deprcheck SET \"buildtime\" = $1 WHERE name = '" + output + "'", result.value().first);
        ph_lock.unlock();
    } else {
        time = elem.as<int>();
    }
    auto five_years = std::chrono::seconds(157788000);
    auto unix_timestamp = std::chrono::seconds(std::time(NULL));
    if (five_years < std::chrono::duration_cast<std::chrono::seconds>(unix_timestamp - std::chrono::seconds(time)))
        return true;
    return false;
}

void PostgreHandler::remove_irrelevant_pkgs(std::string pkg_name) {
    pqxx::work W(connect);
    W.exec(("UPDATE depr SET \"depr_data\" =  array_remove(depr_data, \'" + pkg_name + "\')").c_str());
    W.commit();
}

void PostgreHandler::remove_provides(std::string key, std::string remove_name) {
    pqxx::work W(connect);
    W.exec(("UPDATE depr SET \"depr_data\" =  array_remove(depr_data, \'" + remove_name + "\') WHERE name = '" + key + "'").c_str());
    W.commit();
}