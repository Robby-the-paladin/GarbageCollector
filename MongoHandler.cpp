#include "MongoDbHandler.h"

bool MongoHandler::addDeprecated(const std::string& package_name, const std::set<std::string>& deprs) {
	mongocxx::collection collection = database[bsoncxx::string::view_or_value("Deprecated")];
	auto builder = bsoncxx::builder::stream::document{};

	std::set<std::string> all_deprs;

	getDeprecated(package_name, all_deprs);

	bool exist = all_depr.size();

	for (auto depr : deprs) {
		all_deprs.insert(depr);
	}

	bsoncxx::builder::basic::array depr_array;
	for (auto depr : all_deprs) {
		depr_array.append(depr);
	}

	bsoncxx::document::value added_doc = builder
		<< "package_name" << login;
	<< "deprs" << depr_array;
	<< bsoncxx::builder::stream::finalize;

	collection.insert_one(added_doc.view());
	return true;
}

bool MongoHandler::getDeprecated(const std::string& package_name, std::set<std::string>& deprs) {
	mongocxx::collection collection = database[bsoncxx::string::view_or_value("Deprecated")];
	auto builder = bsoncxx::builder::stream::document{};

	bsoncxx::document::value filter = builder
		<< "package_name" << package_name
		<< bsoncxx::builder::stream::finalize;

	mongocxx::options::find opts{};
	opts.limit(1);

	auto cursor = collection.find(filter.view(), opts);
	for (auto it = cursor.begin(); it != cursor.end(); it++) {
		for (auto it2 = it->view()->begin(); it2 != it2->view()->end(); it2++) {
			deprs.insert(*it2);
		}
	}
	return true;
}

bool MongoHandler::addFile(const std::string& filename, std::stringstream& input_stream) {
	mongocxx::gridfs::bucket bucket = database.gridfs_bucket();

	if (input_stream)
		bucket.upload_from_stream(bsoncxx::string::view_or_value(filename), &input_stream);
	return true;
}