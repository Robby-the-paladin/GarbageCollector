#include "SpecParser.h"

std::set<std::string> SpecParser::getDeprecatedPackages(std::string specfile) {
	std::set<std::string> result;

	std::ofstream outspec;
	outspec.open("specfile.spec");
	outspec << specfile;
	outspec.close();

	int resp = system("rpm -bE specfile.spec >specfile.spec");
	if (resp != 0)
		std::cout << "Error using rpm -bE:" << resp << std::endl;

	std::ifstream inspec("parsedspec.spec");
	std::string spec;
	std::string nstr;
	while (getline(inspec, nstr)) {
		spec += nstr + "\n";
	}
	inspec.close();

	std::set<std::string> keywords;
	keywords.insert("Obsoletes:");
	keywords.insert("Provides:");

	for (auto keyword : keywords) {
		std::string ex = "(" + keyword + ") (.*) (=|<|<=|>|>=) (.*)";
		std::regex expr(ex, std::regex::icase);
		std::string::const_iterator searchStart(spec.cbegin());
		std::smatch res;
		while (std::regex_search(searchStart, spec.cend(), res, expr)) {
			//cout << (searchStart == spec.cbegin() ? "" : "\n") << res[0] << endl;
			result.insert(res[2]);
			searchStart = res.suffix().first;
		}
	}

	return result;
}