#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <curl/curl.h>

#ifdef WIN32
#include <io.h>
#define READ_3RD_ARG unsigned int
#else
#include <unistd.h>
#define READ_3RD_ARG size_t
#endif

#if LIBCURL_VERSION_NUM < 0x070c03
#error "upgrade your libcurl to no less than 7.12.3"
#endif

#include <iostream>
using namespace std;

const string URL = "https://api.github.com/search/repositories"; // GET /search/repositories

string search_repos(const string & api) {
    CURL *curl;
    CURLcode resp;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Awesome-Octocat-App");
        resp = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (resp != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(resp));
        }
    }
    curl_global_cleanup();
    string res(curl_easy_strerror(resp));
    return res;
}

void program(int num, const char **argv) {
    string api = URL + "?";
    string q(argv[1]);
    api += "q=" + q + "&";
    api += "sort=stars&";
    api += "order=desc";
    cout << "params: " << api << endl;
    const string responseBody = search_repos(api);
    cout << "responseBody" << responseBody << endl;
    return ;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage " << argv[0] << " [OPTION]... PATTERNS [URL]..." << endl;
        cout << "Try '" << argv[0] << " --help' for more information." << endl;
    } else {
        program(argc, (const char**)argv);
    }
    return 0;
}
