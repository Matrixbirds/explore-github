#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <curl/curl.h>

#include <rapidjson/reader.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/error/en.h>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>

#include <fstream>

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#define READ_3RD_ARG unsigned int
#else
#include <unistd.h>
#define READ_3RD_ARG size_t
#endif

#if LIBCURL_VERSION_NUM < 0x070c03
#error "upgrade your libcurl to no less than 7.12.3"
#endif


namespace github {};

#include <iostream>
using namespace std;
using namespace rapidjson;
using namespace github;

namespace github {
    class Repository {
        public:
            Repository(string &name, string &url, unsigned &stars) : name_(name) , url_(url), stars_(stars) {}
            Repository(const Repository& ins) : name_(ins.name_) , url_(ins.url_), stars_(ins.stars_) {}
            virtual ~Repository();

            Repository& operator=(const Repository& _rval) {
                name_ = _rval.name_;
                url_= _rval.url_;
                stars_= _rval.stars_;
                return *this;
            }

        protected:
            template <typename Writer>
            void Serialize(Writer& writer) const {
                writer.String("name");
                writer.String("url");
                writer.String("stars");
            }
        private:
            std::string name_;
            std::string url_;
            unsigned stars_;
    };
}

void show_usage (const char **argv) {
   cout << "Usage " << argv[0] << " [OPTION]... PATTERNS [URL]..." << endl;
   cout << "Try '" << argv[0] << " --help' for more information." << endl;
}

void show_help (const char **argv) {
    show_usage(argv);
    cout << "HELP: [repo-name]" << endl;
    cout << "Example" << endl;
    cout << "./explore.exe explore-github" << endl;
}

const string URL = "https://api.github.com/search/repositories"; // GET /search/repositories

static size_t WriteDataCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size *nmemb);
    return size * nmemb;
}

void search_repos(const string & api, string *readBuffer) {
    CURL *curl;
    CURLcode resp;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, api.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Awesome-Octocat-App");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteDataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);
        resp = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (resp != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(resp));
        }
    }
    curl_global_cleanup();
    return ;
}

int doRequest(const char **argv) {
    string api = URL + "?";
    string q(argv[1]);
    api += "q=" + q + "&";
    api += "sort=stars&";
    api += "order=desc";
    cout << "params: " << api << endl;
    string jsonBody;
    search_repos(api, &jsonBody);
    const char *body = jsonBody.c_str();
    Document doc;
    if (doc.Parse(body).HasParseError()) return 1;
    /* ONLY DEBUG
     * cout << "Parsing to document succeeded." << endl;
     * cout << "Access values." << endl;
     */
    //const Value& items = doc["items"][0];
    OStreamWrapper osw(cout);
    PrettyWriter<OStreamWrapper> writer(osw);
    doc.Accept(writer);
    return 0;
}

void program(int num, const char **argv) {
    const string pattern(argv[1]);
    if (pattern.empty()) {
        show_usage(argv);
    }
    if ("--help" == pattern) {
        show_help(argv);
    } else {
        doRequest(argv);
    }
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        show_usage(argv);
    } else {
        program(argc, (const char**)argv);
    }
    return 0;
}

