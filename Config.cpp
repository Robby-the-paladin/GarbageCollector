#include "Config.h"

Config::Config(std::string file)
{
    std::ifstream F(file, std::ifstream::binary);
    F >> conf;

}

std::string Config::getConnectDB()
{   
    std::string user = "user=" + getDBUser();
    std::string passw = "password=" + getDBPassword();
    std::string host = "host=" + getDBHost();
    std::string port = "port=" + getDBPort();
    std::string dbname = "dbname=" + getDBName();
    std::string target_session_attrs = "target_session_attrs=" + getDBSession();     

    return user + " " + passw + " " + host + " " + port + " " + dbname + " " + target_session_attrs;
}

std::vector<std::string> Config::getPackRegexs()
{   
    std::vector<std::string> out;
    for (auto r: conf["pack_regexs"]) {
        out.push_back(r.asString());
    }
    return out;
}

std::string Config::getDBUser()
{
    return conf["user"].asString();
}

std::string Config::getDBPassword()
{
    return conf["password"].asString();
}

std::string Config::getDBHost()
{
    return conf["host"].asString();
}

std::string Config::getDBPort()
{
    return conf["port"].asString();
}

std::string Config::getDBName()
{
    return conf["dbname"].asString();
}

std::string Config::getDBSession()
{
    return conf["target_session_attrs"].asString();
}
