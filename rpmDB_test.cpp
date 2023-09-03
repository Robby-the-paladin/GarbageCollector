#include "rpmDB_test.h"

std::set<std::string> string_to_set(std::string in) {
    std::set<std::string> out;
    std::string s;
    for(auto elem: in) {
        if(elem == ' '){
            out.insert(s);
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
        std::string name =  "%{name}";
        int rc = 0;
        const char *pkglist;
        int ix = 2;
        pkglist = "pkglist.classic_x86_64";

        while (pkglist != NULL) {
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

                    for (auto elem: string_to_set(str_format)) {
                        out[std::string(str_name)].insert(elem);
                    }
                    
                    free(str_format);
                    free(str_name);
                }
               // std::cout << "-------------------------\n";
                headerFree(h);
            }
            pkglist = NULL;
            Fclose(Fd);
        }
    }
    return out;
}