#include "SpecParser.h"

std::set<std::string> SpecParser::getDeprecatedPackages(std::string specfile) {
	std::set<std::string> result;

	std::ofstream outspec;
	outspec.open("specfile.spec");
	outspec << specfile;
	outspec.close();

	int resp = system("runuser -l builder -c 'rpm -bE ../../workspaces/GarbageCollector/build/specfile.spec >../../../home/builder/parsedspec.spec'");
	if (resp != 0) {
        error++;
		std::cout << "Error using rpm -bE:" << resp << std::endl;
    }
    resp = system("cp ../../../home/builder/parsedspec.spec ./");
    if (resp != 0) {
		std::cout << "Error using cp:" << resp << std::endl;
    }

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
	
		std::string ex = "(" + keyword + ")" + "(.*)";
		std::regex expr(ex, std::regex::icase);
		std::string::const_iterator searchStart(spec.cbegin());
		std::smatch res;
		while (std::regex_search(searchStart, spec.cend(), res, expr)) {
			
			std::string str_buildrpre = res[2].str(); 
			searchStart = str_buildrpre.cbegin();
			std::smatch res_str;
			ex = "(.*) (<=|>=|=|<|>)";
			std::regex expr_str(ex, std::regex::icase);
			int f = 0;
			while (std::regex_search(searchStart, str_buildrpre.cend(), res_str, expr_str)) {
				if (!f) {
					result.insert(res_str[1]);
				} else {
					bool start = 0;
					for (int i = 0; i < res_str[1].str().size(); i++) {
						if (res_str[1].str()[i] != ' ')	{
							start = 1;						
						}
						if (res_str[1].str()[i] == ' ' && start) {
							result.insert(" " + res_str[1].str().substr(i));
							break;						
						}
					}
				}
				f = 1;
				searchStart = res_str.suffix().first;
			}		
			if (!f) {
				result.insert(std::string(searchStart, str_buildrpre.cend()));
			} else {
				bool start = 0;
				for (auto i = searchStart;  i != str_buildrpre.cend(); i++) {
					if (*i != ' ')	{
						start = 1;						
					}
					if (*i == ' ' && start) {
						result.insert(" " + std::string(i, str_buildrpre.cend()));						
						break;						
					}
				}
			}
			
			searchStart = res.suffix().first;
		}
	}

	return result;
}

std::set<std::string> SpecParser::getBuildRequiresPrePackages(std::string specfile) {
	std::set<std::string> result;

	std::string spec = specfile;	

	std::set<std::string> keywords;
	keywords.insert("BuildRequires\\(pre\\):");
	for (auto keyword : keywords) {
	
		std::string ex = "(" + keyword + ")" + "(.*)";
		std::regex expr(ex, std::regex::icase);
		std::string::const_iterator searchStart(spec.cbegin());
		std::smatch res;
		while (std::regex_search(searchStart, spec.cend(), res, expr)) {
			
			std::string str_buildrpre = res[2].str(); 
			searchStart = str_buildrpre.cbegin();
			std::smatch res_str;
			ex = "(.*) (<=|>=|=|<|>)";
			std::regex expr_str(ex, std::regex::icase);
			int f = 0;
			while (std::regex_search(searchStart, str_buildrpre.cend(), res_str, expr_str)) {
				if (!f) {
					result.insert(res_str[1]);
				} else {
					bool start = 0;
					for (int i = 0; i < res_str[1].str().size(); i++) {
						if (res_str[1].str()[i] != ' ')	{
							start = 1;						
						}
						if (res_str[1].str()[i] == ' ' && start) {
							result.insert(" " + res_str[1].str().substr(i));
							break;						
						}
					}
				}
				f = 1;
				searchStart = res_str.suffix().first;
			}		
			if (!f) {
				if (str_buildrpre.back() == '}') { //NetworkManager }
					std::string r = std::string(searchStart, str_buildrpre.cend());
					r.pop_back();
					result.insert(r);
				} else {
					result.insert(std::string(searchStart, str_buildrpre.cend()));
				}
			} else {
				bool start = 0;
				for (auto i = searchStart;  i != str_buildrpre.cend(); i++) {
					if (*i != ' ')	{
						start = 1;						
					}
					if (*i == ' ' && start) {
						if (str_buildrpre.back() == '}') { //NetworkManager }
							std::string r = " " + std::string(i, str_buildrpre.cend());
							r.pop_back();
							result.insert(r);
						} else {
							result.insert(" " + std::string(i, str_buildrpre.cend()));
						}
						
						break;						
					}
				}
			}
			
			searchStart = res.suffix().first;
		}
	}

	return result;
}