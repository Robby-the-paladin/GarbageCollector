#include "rpmDB_test.h"

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

std::map<std::string,std::set<std::string>> rpmDB_test::test() {
    std::map<std::string,std::set<std::string>> out;
    rpmdb dbp;
    char* argv;

    const char *progname = "";
    std::set<std::string> words;
    words.insert("provides");
    words.insert("obsoletes");
    words.insert("conflicts");
    for (auto& elem: words) {
        std::string format =  "[\\{%{" + elem + "},,%{" + elem.substr(0,elem.size()-1) + "version}," + elem + "\\} ]";
        std::string name =  "%{SOURCERPM}";
        int rc = 0;
        const char *pkglist;
        int ix = 2;
        std::vector<char*> pkglists = {"pkglist.classic_x86_64", "pkglist.classic_noarch"};

        for (auto pkglist: pkglists) {
            FD_t Fd = Fopen(pkglist, "r.ufdio");
            if (Ferror(Fd)) {
                fprintf(stderr, "%s: %s: %s\n", progname, pkglist, Fstrerror(Fd));
                rc = 1;
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
                    rc = 1;
                    
                    fprintf(stderr, "%s: %s: %s: %s:\n", progname, pkglist, err, format.c_str());
                }
                else {
                    //fputs(str_format, stdout);
                    // std::cout<< str_name << std::endl;
                    // std::cout << str_format << std::endl;
                    
                    if (out[std::string(str_name)].empty()) {
                        out[std::string(str_name)] = {};
                    }

                    

                    auto data_struct = SpecParser::strToStructSet_lib_data(string_to_set(str_format));
                    std::set<std::string> lists;
                    
                    for (size_t i = 0; i < data_struct.size(); i++)
                    {
                        std::string version = data_struct[i].version;
                        if (version.size() > 4 && version.substr(0,4) == "set:") {
                            data_struct[i].version = "";
                        }
     
                        out[std::string(str_name)].insert(SpecParser::structTostr_lib_data(data_struct[i]));
                    }
                    
                    free(str_format);
                    free(str_name);
                }
               // std::cout << "-------------------------\n";
                headerFree(h);
                // if (out.size() > 400) {
                //         break;
                // }
            }
            Fclose(Fd);
            // if (out.size() > 400) {
            //     break;
            // }
        }
    }
    return out;
}

std::set<std::string> rpmDB_test::get_unic_last_name() {
    std::vector<std::string> branchs = {"4.0", "4.1", "p5", "p6", "p7"};
    std::vector<std::string> lists = {"pkglist.classic", "pkglist.classic (1)"};
    std::set<std::string> unic;
    std::string name = "%{name}";
    for (auto branch: branchs) {
        for (auto list: lists) {
            FD_t Fd = Fopen((branch + "/" + list).c_str(), "r.ufdio");
            Header h;
            while ((h = headerRead(Fd, HEADER_MAGIC_YES)) != NULL) {
                const char *err = "unknown error";
                char *str_name = headerFormat(h, name.c_str(), &err);
                if (str_name == NULL) {
                    fprintf(stderr, "%s: %s: %s:\n", list, err, str_name);
                }
                else {
                    unic.insert(str_name);
                    free(str_name);
                }
                headerFree(h);
            }
            Fclose(Fd);
        }
    }

    auto actual = test();
    for (auto elem: actual) {
        unic.erase(elem.first);
    }

    return unic;
}