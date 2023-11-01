#include "RpmHandler.h"

std::vector<std::string> RpmHandler::getAllPackagesName(std::string branch)
{   
    std::vector<std::string> out;
    std::string name =  "%{SOURCERPM}";

    /*
    надо скачать classic файлы для branch
    и записать их имена в classicFilesNames
    */

    for (auto pkglist: classicFilesNames) {
        FD_t Fd = getCalssicFileDescriptor(pkglist);
        if (Ferror(Fd)) {
            fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
            rc = 1;
            pkglist = NULL;
            continue;
        }

        Header h;
        while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
            char *str_name = headerFormat(h, name.c_str(), &err);
            if (str_name == NULL) {
                rc = 1;
                fprintf(stderr, "%s: %s: %s: %s:\n", progname, pkglist, err, format.c_str());
            }
            else {
                out.push_back(std::string(str_name));
                free(str_name);
            }

            headerFree(h);
        }
        
        Fclose(Fd);
    }

    return out;
}

std::vector<PackageDependencies> RpmHandler::getDependenciesForPackages(std::vector<std::string> packageList)
{   
    // словарь [имя пакета, структура PackageDependencies те его зависимости в спек файле]
    std::map<std::string, PackageDependencies> packagesDependencies;

    for (auto name: packageList)
    {
        packagesDependencies[name] = PackageDependencies{packagaName: name};
    }
    
    const char *progname = "";
    std::set<std::string> words;
    words.insert("provides");
    words.insert("obsoletes");
    words.insert("conflicts");
    for (auto& elem: words) {
        std::string format =  "[\\{%{" + elem + "},,%{" + elem.substr(0,elem.size()-1) + "version}," + elem + "\\} ]";
        std::string name =  "%{SOURCERPM}";
        std::vector<char*> pkglists = {"pkglist.classic_x86_64", "pkglist.classic_noarch"};
        for (auto pkglist: pkglists) {
            FD_t Fd = getCalssicFileDescriptor(pkglist);
            if (Ferror(Fd)) {
                fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
                pkglist = NULL;
                continue;
            }
            Header h;
            /*HEADER_MAGIC_YES*/
            while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
                const char *err = "unknown error";
                char *str_format = headerFormat(h, format.c_str(), &err);
                char *str_name = headerFormat(h, name.c_str(), &err);
                if (str_name == NULL || str_format == NULL) {
                    fprintf(stderr, "%s: %s: %s: %s:\n", progname, pkglist, err, format.c_str());
                }
                else {
                    auto data_struct = setStrToStructsDependencies(string_to_set(str_format));
                    for (size_t i = 0; i < data_struct.size(); i++)
                    {
                        std::string version = data_struct[i].version;
                        if (version.size() > 4 && version.substr(0,4) == "set:") {
                            data_struct[i].version = "";
                        }

                        packagesDependencies[std::string(str_name)].dependencies.push_back(data_struct);                     
                    }

                    
                    free(str_format);
                    free(str_name);
                }

                headerFree(h);
            }
            
            Fclose(Fd);
        }
    }

    std::vector<PackageDependencies> out;
    for (auto pack: packagesDependencies) {
        out.push_back(pack.second);
    }

    return out;
}

std::set<std::string> RpmHandler::getPackageFromClassicFileName(std::string fileName)
{   
    std::vector<std::set> out;
    std::string name =  "%{SOURCERPM}";
    FD_t Fd = getCalssicFileDescriptor(fileName);
    if (Ferror(Fd)) {
        fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
        rc = 1;
        pkglist = NULL;
        continue;
    }

    Header h;
    while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
        char *str_name = headerFormat(h, name.c_str(), &err);
        if (str_name == NULL) {
            rc = 1;
            fprintf(stderr, "%s: %s: %s: %s:\n", progname, pkglist, err, format.c_str());
        }
        else {
            out.insert(std::string(str_name));
            free(str_name);
        }

        headerFree(h);
    }
    
    Fclose(Fd);
    return out;
}

std::set<std::string> string_to_set(std::string in) {
    std::set<std::string> out;
    std::string s;
    for(auto elem: in) {
        if(elem == ' '){
            out.insert(s);
            s = "";
        } else {
            s += elem;
        }
    }
    return out;
}

FD_t RpmHandler::getCalssicFileDescriptor(std::string fileName)
{   
    FD_t Fd = Fopen(pkglist, "r.ufdio");
    return FD_t();
}


std::vector<Dependency> RpmHandler::setStrToStructsDependencies(std::set<std::string> stringDependencies) {
    std::vector<Dependency> out;
	for(auto dep: stringDependencies) {
		out.push_back(strToStructDependency(dep));
	}
	return out;
}

Dependency RpmHandler::strToStructDependency(std::string data) {
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
	type = info.substr(0, info.size()-1);
    Dependency s_data;
	s_data.dependencyName = name;
	s_data.sign = sign;
	s_data.version = version;
	s_data.type = type;
	return s_data;
}