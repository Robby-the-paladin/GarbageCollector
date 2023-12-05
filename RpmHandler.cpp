#include "RpmHandler.h"

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

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string RpmHandler::constNameClassic = "pkglist.classic.";
std::vector<std::string> RpmHandler::classicArches = {"x86_64", "noarch"};

std::map<std::string, std::pair<std::string, std::string>> RpmHandler::getAllPackagesName(std::string branch, std::set<std::string> packNames)
{   
    std::map<std::string, std::pair<std::string, std::string>> srcNameToPackName; // ставит в соответствии имени .src.rpm нормальное имя пакета 
    std::vector<std::string> out;
    std::string name =  "%{NAME}";
    std::string src_name = "%{SOURCERPM}";
    std::string release = "%{RELEASE}";
    std::string version = "%{VERSION}";
    // зашрузка classic файлов для branch если файлы отсутствуют
    for (auto arch: classicArches)
    {
        FD_t Fd = getCalssicFileDescriptor(constNameClassic + arch);
        if (Ferror(Fd)) {
            // загрузить новый pkglist для branch
            downloadClassicFile(branch, arch);
        }
        Fclose(Fd);
    }
    

    for (auto arch: classicArches) {
        std::string pkglist = constNameClassic + arch;
        
        FD_t Fd = getCalssicFileDescriptor(pkglist);
        if (Ferror(Fd)) {
            fprintf(stderr, "%s: %s: %s:\n", "ERROR",pkglist, Fstrerror(Fd));
            continue;
        }

        Header h;
        while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
            const char *err = "unknown error";
            char *str_name = headerFormat(h, name.c_str(), &err);
            char *src_str_name = headerFormat(h, src_name.c_str(), &err);
            char *str_release = headerFormat(h, release.c_str(), &err);
            char *str_version = headerFormat(h, version.c_str(), &err);
            if (str_name == NULL) {
                fprintf(stderr, "%s: %s:\n", pkglist, err);
            }
            else {
                auto name = ReplaceAll(std::string(str_name), "+", "%2B");
                auto source_name = ReplaceAll(std::string(src_str_name), std::string("-")+str_version+"-"+str_release+".src.rpm", "");
                // std::cout << name << "  " << source_name << std::endl;
                if (packNames.find(source_name) != packNames.end()){
                    srcNameToPackName[src_str_name] = {source_name, str_release};
                }

                out.push_back(ReplaceAll(std::string(str_name), "+", "%2B"));
                free(str_name);
                free(src_str_name);
                free(str_release);
                free(str_version);
            }

            headerFree(h);
        }
        
        Fclose(Fd);
    }

    return srcNameToPackName;
}

std::vector<PackageDependencies> RpmHandler::getDependenciesForPackages(std::map<std::string, std::pair<std::string, std::string>> packageList)
{   
    // словарь [имя пакета, структура PackageDependencies те его зависимости в спек файле]
    std::map<std::string, PackageDependencies> packagesDependencies;

    for (auto name: packageList)
    {   
        packagesDependencies[std::string(name.first)] = PackageDependencies{packageName: name.second.first, release: name.second.second};
    }
    
    const char *progname = "";
    std::set<std::string> words;
    words.insert("provides");
    words.insert("obsoletes");
    words.insert("conflicts");
    for (auto& elem: words) {
        std::string format =  "[\\{%{" + elem + "},,%{" + elem.substr(0,elem.size()-1) + "version}," + elem + "\\} ]";
        std::string name =  "%{SOURCERPM}";
        std::vector<char*> pkglists = {"pkglist.classic.x86_64", "pkglist.classic.noarch"};
        for (auto arch: classicArches) {
            auto pkglist = constNameClassic + arch;
            FD_t Fd = getCalssicFileDescriptor(pkglist);
            if (Ferror(Fd)) {
                fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
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

                        if (packagesDependencies.find(std::string(str_name)) != packagesDependencies.end() && 
                            !(elem == "provides" && data_struct[i].dependencyName == packagesDependencies[std::string(str_name)].packageName) ) { 
                            
                            packagesDependencies[std::string(str_name)].dependencies.push_back(data_struct[i]);  
                        }
                        

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

std::set<std::string> RpmHandler::getPackageFromClassicFileName(std::string folder, std::string branch, 
                                                               std::string classicName, std::string arch)
{   
    std::set<std::string> out;
    std::string name =  "%{NAME}";
    std::string release = "%{RELEASE}";
    std::string version = "%{VERSION}";

    std::string fileName = folder + "/" + branch + "/" + classicName + arch;
    FD_t Fd = getCalssicFileDescriptor(fileName);
    if (Ferror(Fd)) {
        downloadClassicFile(branch, arch, folder);
    }
    Fclose(Fd);

    Fd = getCalssicFileDescriptor(fileName);
    if (Ferror(Fd)) {
        fprintf(stderr, "%s: %s\n", fileName, Fstrerror(Fd));
        return {};
    }

    Header h;
    while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
        const char *err = "unknown error";
        char *str_name = headerFormat(h, name.c_str(), &err);
        char *str_release = headerFormat(h, release.c_str(), &err);
        char *str_version = headerFormat(h, version.c_str(), &err);
        if (str_name == NULL) {
            fprintf(stderr, "%s: %s:\n", fileName, err);
        }
        else {
            // auto source_name = ReplaceAll(std::string(str_name), std::string("-")+str_version+"-"+str_release+".src.rpm", "");
            std::cout << str_name << std::endl;
            out.insert(ReplaceAll(std::string(str_name), "+", "%2B"));
            free(str_name);
            free(str_release);
            free(str_version);
        }

        headerFree(h);
    }
    
    Fclose(Fd);
    return out;
}



FD_t RpmHandler::getCalssicFileDescriptor(std::string fileName)
{   
    FD_t Fd = Fopen(("./" + fileName).c_str(), "r.ufdio");
    return Fd;
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
	s_data.dependencyName = ReplaceAll(name, "+", "%2B");
	s_data.sign = sign;
	s_data.version = version;
	s_data.type = type;
	return s_data;
}

void RpmHandler::downloadClassicFile(std::string branch, std::string arch, std::string path)
{
    CURL* curl;
    CURLcode res;

    // Установка URL FTP сервера и пути к файлу
    std::string url = "http://ftp.altlinux.org/pub/distributions/ALTLinux/" + branch + "/" + arch + "/base/pkglist.classic.xz";
    std::string archiveFile = "pkglist.classic.xz";

    // Инициализация библиотеки libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Указание функции обратного вызова для записи данных в локальный файл
        FILE* fp = fopen(archiveFile.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        // Выполнение операции загрузки
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Ошибка при загрузке файла: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            std::cout << "Файл успешно загружен как " << archiveFile << std::endl;
        }

        // Закрытие файла и освобождение ресурсов
        fclose(fp);
        curl_easy_cleanup(curl);
    }

    // Освобождение ресурсов libcurl
    curl_global_cleanup();

    auto resp = system((std::string("xz -d ") + "./" + "pkglist.classic.xz").c_str());

    if (resp != 0) {
        url = "http://ftp.altlinux.org/pub/distributions/ALTLinux/" + branch + "/branch/" + arch + "/base/pkglist.classic.xz";
         // Инициализация библиотеки libcurl
        curl_global_init(CURL_GLOBAL_DEFAULT);

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            // Указание функции обратного вызова для записи данных в локальный файл
            FILE* fp = fopen(archiveFile.c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            // Выполнение операции загрузки
            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "Ошибка при загрузке файла: " << curl_easy_strerror(res) << std::endl;
            }
            else {
                std::cout << "Файл успешно загружен как " << archiveFile << std::endl;
            }

            // Закрытие файла и освобождение ресурсов
            fclose(fp);
            curl_easy_cleanup(curl);
        }

        // Освобождение ресурсов libcurl
        curl_global_cleanup();
        system((std::string("xz -d ") + "./" + "pkglist.classic.xz").c_str());
    }

    if (path == "") {
        resp = system((std::string("mv ./pkglist.classic ") + "pkglist.classic."+arch).c_str());
    } else {
        resp = system(("mkdir -p " + path + "/" + branch).c_str());
        resp = system((std::string("mv ./pkglist.classic ") + "./" + path + "/" + branch + "/pkglist.classic." + arch).c_str());
    }
    
}

std::set<std::string> RpmHandler::getAllProvides(std::string folder, std::string branch, 
                                                               std::string classicName, std::string arch)
{   
    std::set<std::string> out;
    std::string name =  "%{NAME}";
    std::string prov = "%{provides}";
    std::string format =  std::string("[\\{%{") + "provides" + "},,%{" + "provide" + "version}," + "provides" + "\\} ]";

    std::string fileName = folder + "/" + branch + "/" + classicName + arch;
    FD_t Fd = getCalssicFileDescriptor(fileName);
    if (Ferror(Fd)) {
        downloadClassicFile(branch, arch, folder);
    }
    Fclose(Fd);

    Fd = getCalssicFileDescriptor(fileName);
    if (Ferror(Fd)) {
        fprintf(stderr, "%s: %s\n", fileName, Fstrerror(Fd));
        return {};
    }

    Header h;
    while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
        const char *err = "unknown error";
        char *str_format = headerFormat(h, format.c_str(), &err);
        if (str_format == NULL) {
            fprintf(stderr, "%s: %s: %s:\n", fileName, err, format.c_str());
        }
        else {
            auto data_struct = setStrToStructsDependencies(string_to_set(str_format));
            
            for(auto pack: data_struct) {
                out.insert(pack.dependencyName);
            }

            free(str_format);
        }

        headerFree(h);
    }
    
    Fclose(Fd);
    return out;
}

std::map<std::string, std::set<std::string>> RpmHandler::packagesProvides()
{   
    // словарь [имя пакета, структура PackageDependencies те его зависимости в спек файле]
    std::map<std::string, std::set<std::string>> packagesDependencies;
    
    const char *progname = "";
    std::set<std::string> words;
    words.insert("provides");
    for (auto& elem: words) {
        std::string format =  "[\\{%{" + elem + "},,%{" + elem.substr(0,elem.size()-1) + "version}," + elem + "\\} ]";
        std::string name =  "%{NAME}";
        std::vector<char*> pkglists = {"pkglist.classic.x86_64", "pkglist.classic.noarch"};
        for (auto arch: classicArches) {
            auto pkglist = constNameClassic + arch;
            FD_t Fd = getCalssicFileDescriptor(pkglist);
            if (Ferror(Fd)) {
                fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
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
                        if (packagesDependencies.find(std::string(str_name)) != packagesDependencies.end() && 
                            !(elem == "provides" && str_name == data_struct[i].dependencyName) ) { 
                            
                            packagesDependencies[std::string(str_name)].insert(data_struct[i].dependencyName);  
                        }
                    }

                    
                    free(str_format);
                    free(str_name);
                }

                headerFree(h);
            }
            
            Fclose(Fd);
        }
    }

    return packagesDependencies;
}