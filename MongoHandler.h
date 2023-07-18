#pragma once
#include<string>
#include<cstdint>
#include<fstream>
#include<bsoncxx/builder/stream/document.hpp>
#include<bsoncxx/json.hpp>
#include<bsoncxx/oid.hpp>
#include<mongocxx/client.hpp>
#include<mongocxx/database.hpp>
#include<mongocxx/uri.hpp>
#include<mongocxx/gridfs/bucket.hpp>
#include<mongocxx/gridfs/uploader.hpp>
#include<mongocxx/gridfs/downloader.hpp>
#include<mongocxx/change_stream.hpp>
#include<sstream>

class MongoHandler {
public:
	MongoHandler() :
		uri(mongocxx::uri(bsoncxx::string::view_or_value("mongodb://localhost:27017"))),
		client(mongocxx::client(uri)),
		database(client[bsoncxx::string::view_or_value("GarbageCollector")]) {}

	bool addDeprecated(const std::string& package_name, const std::set<std::string>& deprs);

	bool getDeprecated(const std::string& package_name, std::set<std::string>& deprs);

	bool addFile(const std::string& filename, std::stringstream& input_stream);

private:
	mongocxx::uri uri;
	mongocxx::client client;
	mongocxx::database database;
};