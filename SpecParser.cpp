#include "SpecParser.h"

// std::ostream& operator << (std::ostream &os, const SpecParser::lib_data &lib)
// {
//     return os << lib.name << " " << lib.sign << " " << lib.version << " " << lib.type;
// }

std::string prepareSpec(std::string specfile) {
	return specfile.substr(0, specfile.size() - specfile.find("%changelog"));
}

std::set<std::string> SpecParser::getDeprecatedPackages(std::string specfile) {
	specfile = prepareSpec(specfile);
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
			ex = "([^<>=]+) (<=|>=|=|<|>)";
			std::regex expr_str(ex, std::regex::icase);
			int f = 0;
			while (std::regex_search(searchStart, str_buildrpre.cend(), res_str, expr_str)) {
				if (!f) {
					std::cerr << res_str[0].str() << std::endl; 
					result.insert(" " + res_str[1].str() );
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
	specfile = prepareSpec(specfile);

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
			ex = "(.*) (<=|>=|=|<|>)"; // ?
			ex = "([^<>=]+) (<=|>=|=|<|>)";
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




bool operator< (const SpecParser::lib_data& l, const SpecParser::lib_data& r) {
	return l.name + l.sign + l.version + l.type < l.name + l.sign + l.version + l.type;
}


std::set<std::string> SpecParser::getDeprecatedPackages_test(std::string specfile) {
	std::set<std::string> result;
	std::set<lib_data> result_structs;

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
			//std::cout << str_buildrpre << std::endl;
			bool end_name = false;
			bool end_version = false;
			bool read_version = false;
			std::string sign = "";
			std::string name = "";
			std::string version = "";
			std::string struct_lib = "{";
			lib_data test;
			for (int i = 0; i < str_buildrpre.size(); i++) {
				switch (str_buildrpre[i])
				{
				case '>':
					sign += '>';
					end_version = false;
					if (end_name) {
						struct_lib += name + ",";
						name = "";
					}
					end_name = false;
					read_version = true;
					break;
				case '<':
				//	std::cout << name << std::endl;
					sign += '<';
					end_version = false;
					struct_lib += name + ",";
					name = "";
					end_name = false;
					read_version = true;
					break;
				case '=':
					sign += '=';
					end_version = false;
					if (end_name) {
						struct_lib += name + ",";
						name = "";
					}
					end_name = false;
					read_version = true;
					break;
				case ' ':
					if (name != "") {
						end_name = true;
					}
					if (version != "") {
						end_version = true;
						struct_lib += sign + "," + version + "," + keyword + "}";
					//	std::cout << struct_lib << std::endl;

						result.insert(struct_lib);
						struct_lib = "{";
						read_version = false;
						sign = "";
						version = "";
					}
					break;
				case ',':
					end_name = false;
					end_version = false;
					if (read_version) {
						struct_lib += sign + "," + version + "," + keyword + "}";
					} else {
						struct_lib += name + ",,," + keyword + "}";
					}

					result.insert(struct_lib);
					read_version = false;
					name = "";
					version = "";
					sign  = "";
					struct_lib = "{";
					break;
				default:
					if (read_version) {
						version += str_buildrpre[i];
					} else {
						if (end_name) {
							struct_lib += name + ",,," + keyword + "}";
							result.insert(struct_lib);

							name = "";
							version = "";
							sign  = "";
							struct_lib = "{";
							end_name = false;
						}
						name += str_buildrpre[i];
					}
					break;
				}

			}	
			if (read_version) {
				struct_lib += sign + "," + version + "," + keyword + "}";
			} else {
				struct_lib += name + ",,," + keyword + "}";
			}

			result.insert(struct_lib);
			
			
			searchStart = res.suffix().first;
		}
	}
	return result;
}


SpecParser::lib_data* SpecParser::strToStruct_lib_data(std::string data) {
	std::string info = "";
	int index = 0;
	std::string name, version, type, sign;
	bool f = true;
	for(auto elem: data) {
		if (f) {
			f = false;
			continue;
		}
		if (elem == ',') {
			switch (index)
			{
			case 0:
				name = info;
				break;
			case 1:
				sign = info;
				break;
			case 2:
				version = info;
				break;
			default:
				break;
			}
			info = "";
			index++;
		} else {
			info += elem;
		}
	}
	type = info.substr(0, info.size()-2);
	SpecParser::lib_data* s_data = new SpecParser::lib_data();
	s_data->name = name;
	s_data->sign = sign;
	s_data->version = version;
	s_data->type = type;
	return s_data;
}

std::vector<SpecParser::lib_data> SpecParser::strToStructSet_lib_data(std::set<std::string> libs) {
	std::vector<SpecParser::lib_data> out;
	for(auto lib: libs) {
		out.push_back(*strToStruct_lib_data(lib));
	}
	return out;
}