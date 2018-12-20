#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>

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
#include <vector>
#include <unordered_map>
#include <regex>

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
#include "color.hxx"
using namespace std;
using namespace rapidjson;
using namespace github;

namespace github {
    namespace util {
        enum queryType {
            INT,
            STRING,
            ARRAY,
            OBJECT
        };
    }

    void log(const Value& it, const unordered_map<string, util::queryType>& attrs) {
        for (auto& _: attrs) {
            const char *key = _.first.c_str();
            switch (_.second) {
                cout << _.first << ": ";
                case util::queryType::INT:
                    cout << key << ": " << it[key].GetInt() << endl;
                break;
                case util::queryType::STRING:
                    cout << key << ": " << it[key].GetString() << endl;
                break;
                // ## NEED TEST
                case util::queryType::ARRAY:
                    for (const auto& item : it[key].GetArray())
                        github::log(item, attrs);
                break;
                // ## NEED TEST
                case util::queryType::OBJECT:
                    for (const auto& item : it[key].GetObject())
                        cout << item.name.GetString() << ": " << item.value.GetString() << endl;
                break;
            }
        }
    }

    class Repository {
        public:
            Repository(string& name, string& url,
                unsigned& stars, string& description) :
                name_(name) , url_(url), stars_(stars),
                description_(description)
                {}
            Repository(const Repository& ins) :
                name_(ins.name_) , url_(ins.url_), stars_(ins.stars_),
                description_(ins.description_)
                {}
            ~Repository() {}

            Repository& operator=(const Repository& _rval) {
                name_ = _rval.name_;
                url_= _rval.url_;
                stars_= _rval.stars_;
                description_= _rval.description_;
                return *this;
            }

            template <typename Writer>
            void Serialize(Writer& writer) const {
                writer.StartObject();
                writer.String("name");
#if RAPID_JSON_HAS_STDSTRING
                writer.String(name_);
#else
                writer.String(name_.c_str(), static_cast<SizeType>(name_.length()));
#endif
                writer.String("url");
#if RAPID_JSON_HAS_STDSTRING
                writer.String(url_);
#else
                writer.String(url_.c_str(), static_cast<SizeType>(url_.length()));
#endif
                writer.String("stars");
                writer.Uint(stars_);
                writer.String("description");
#if RAPID_JSON_HAS_STDSTRING
                writer.String(description_);
#else
                writer.String(description_.c_str(), static_cast<SizeType>(description_.length()));
#endif
                writer.EndObject();
            }
        private:
            std::string name_;
            std::string url_;
            std::string description_;
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

void search_repos(const string & api, string *readBuffer, long* http_code) {
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
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
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
    string q(argv[1]), sort(argv[2]), order(argv[3]);
    api += "q=" + q + "&";
    api += "sort=" + sort + "&";
    api += "order=" + order;
    cout << "params: " << api << endl;
    string jsonBody;
    long http_status;
    search_repos(api, &jsonBody, &http_status);
    const char *body = jsonBody.c_str();
    Document doc;
    cout << "STATUS: " << http_status << endl;
    if (doc.Parse(body).HasParseError()){
        fprintf(stderr, "Body Format Error.");
        return 1;
    }
    /**
     * ONLY DEBUG
     * cout << "Parsing to document succeeded." << endl;
     * cout << "Access values." << endl;
    **/
    const Value& items = doc["items"];
    assert(items.IsArray());
    vector <Repository> repos;
    const unordered_map<string, util::queryType> attrs({
            {"name", util::queryType::STRING},
            {"url", util::queryType::STRING},
            {"stargazers_count", util::queryType::INT},
            {"description", util::queryType::STRING}
    });
    const auto& n = min(items.Size(), (SizeType){3});
    for (SizeType i = 0; i < n; i++) {
        const auto& item = items[i];
        string name = item["name"].GetString();
        string url = item["url"].GetString();
        unsigned int stars = item["stargazers_count"].GetInt();
        string description = item["description"].GetString();
        const auto& repo = Repository(name, url, stars, description);
        repos.push_back(repo);
        // HIDDEN github::log(item, attrs);
    }

    //OStreamWrapper osw(cout);
    //PrettyWriter<OStreamWrapper> writer(osw);
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);

    writer.StartArray();
    for (const auto& repo: repos) {
        repo.Serialize(writer);
    }
    writer.EndArray();
    cout << sb.GetString() << endl;
    //doc["items"].Accept(writer);
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
        (num == 2) ? (argv[2] = "stars") : argv[2];
        (num == 3) ? (argv[3] = "desc") : argv[3];
        doRequest(argv);
    }
}

int main(int argc, const char *argv[]) {
    ColorStream s;
    s << "Hello ColorStream" << endl;
    if (argc < 2) {
        show_usage(argv);
    } else {
        program(argc, (const char**)argv);
    }
    return 0;
}

