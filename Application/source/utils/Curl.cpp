#include <curl/curl.h>
#include <fstream>
#include "Log.hpp"
#include <sstream>
#include "utils/Curl.hpp"

// Location of certificates file (extracted from Firefox; up to date as of 14-7-20)
#define CERT_FILE "romfs:/cacert.pem"
// Timeout for connecting (3 seconds)
#define TIMEOUT 3000L
// User agent (concatenation of name, version and github link)
#define USER_AGENT "TriPlayer/" VER_STRING " (http://github.com/tallbl0nde/TriPlayer)"

// Stores the last error that occurred
static std::string error_ = "";
// Has the library inited correctly?
static bool ready = false;

namespace Utils::Curl {
    // Static functions
    static CURL * initCurlHandle(const std::string & url) {
        CURL * c = curl_easy_init();
        CURLcode rc = curl_easy_setopt(c, CURLOPT_CAINFO, CERT_FILE);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_CAINFO: " + std::to_string(rc));
        }
        rc = curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT_MS, TIMEOUT);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_CONNECTTIMEOUT_MS: " + std::to_string(rc));
        }
        curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_FOLLOWLOCATION: " + std::to_string(rc));
        }
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 1L);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_SSL_VERIFYPEER: " + std::to_string(rc));
        }
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 1L);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_SSL_VERIFYHOST: " + std::to_string(rc));
        }
        curl_easy_setopt(c, CURLOPT_TCP_FASTOPEN, 1L);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_TCP_FASTOPEN: " + std::to_string(rc));
        }
        rc = curl_easy_setopt(c, CURLOPT_URL, url.c_str());
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_URL: " + std::to_string(rc));
        }
        curl_easy_setopt(c, CURLOPT_USERAGENT, USER_AGENT);
        if (rc != CURLE_OK) {
            Log::writeWarning("[CURL] Failed to setopt CURLOPT_USERAGENT: " + std::to_string(rc));
        }
        return c;
    }

    static void setErrorMsg(const std::string & msg) {
        Log::writeError("[CURL] " + msg);
        error_ = msg;
    }

    static size_t writeDataBuffer(char * ptr, size_t size, size_t nmemb, void * data){
        std::vector<unsigned char> * buf = (std::vector<unsigned char> *)data;
        size_t bytes = size * nmemb;
        for (size_t b = 0; b < bytes; b++) {
            buf->push_back(*(ptr + b));
        }
        return bytes;
    }

    static size_t writeDataStream(char * ptr, size_t size, size_t nmemb, void * data){
        std::ostringstream * ss = (std::ostringstream *)data;
        size_t bytes = size * nmemb;
        ss->write(ptr, bytes);
        return bytes;
    }

    static size_t writeFile(char * ptr, size_t size, size_t nmemb, void * stream) {
        std::ofstream * file = (std::ofstream *)stream;
        auto before = file->tellp();
        file->write(ptr, nmemb);
        return file->tellp() - before;
    }

    // Public functions
    void init() {
        CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
        if (rc != CURLE_OK) {
            ready = false;
            setErrorMsg("An error occurred initializing the library: " + std::to_string(rc));
        } else {
            ready = true;
            Log::writeSuccess("[CURL] Initialized!");
        }
    }

    void exit() {
        if (ready) {
            curl_global_cleanup();
            ready = false;
        }
    }

    std::string error() {
        return error_;
    }

    bool downloadToBuffer(const std::string & url, std::vector<unsigned char> & buf) {
        if (!ready) {
            setErrorMsg("[downloadToBuffer] curl has not been initialized!");
            return false;
        }

        // Setup request
        CURL * c = initCurlHandle(url);
        CURLcode rc = curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writeDataBuffer);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToBuffer] Failed to setopt CURLOPT_WRITEFUNCTION: " + std::to_string(rc));
        }
        rc = curl_easy_setopt(c, CURLOPT_WRITEDATA, &buf);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToBuffer] Failed to setopt CURLOPT_WRITEDATA: " + std::to_string(rc));
        }

        // Perform request
        rc = curl_easy_perform(c);
        if (rc != CURLE_OK) {
            setErrorMsg("[downloadToBuffer] An error occurred while performing the request: " + std::to_string(rc));
        }

        // Get HTTP result code to determine if the response was ok
        long code;
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
        if (rc == CURLE_OK && code != 200) {
            setErrorMsg("[downloadToBuffer] The request was successful, but the response has HTTP code: " + std::to_string(code));
        }

        // Cleanup
        curl_easy_cleanup(c);

        return (rc == CURLE_OK && code == 200);
    }

    bool downloadToFile(const std::string & url, const std::string & path) {
        if (!ready) {
            setErrorMsg("[downloadToFile] curl has not been initialized!");
            return false;
        }

        // Open file to write to (assumes path exists)
        std::ofstream file(path);
        if (!file) {
            setErrorMsg("[downloadToFile] Unable to open '" + path + "' for writing");
            return false;
        }

        // Setup request
        CURL * c = initCurlHandle(url);
        CURLcode rc = curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writeFile);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToFile] Failed to setopt CURLOPT_WRITEFUNCTION: " + std::to_string(rc));
        }
        rc = curl_easy_setopt(c, CURLOPT_WRITEDATA, &file);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToFile] Failed to setopt CURLOPT_WRITEDATA: " + std::to_string(rc));
        }

        // Perform request
        rc = curl_easy_perform(c);
        if (rc != CURLE_OK) {
            setErrorMsg("[downloadToFile] An error occurred while performing the request: " + std::to_string(rc));
        }

        // Get HTTP result code to determine if the response was ok
        long code;
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
        if (rc == CURLE_OK && code != 200) {
            setErrorMsg("[downloadToFile] The request was successful, but the response has HTTP code: " + std::to_string(code));
        }

        // Cleanup
        curl_easy_cleanup(c);

        return (rc == CURLE_OK && code == 200);
    }

    bool downloadToString(const std::string & url, std::string & buf) {
        if (!ready) {
            setErrorMsg("[downloadToString] curl has not been initialized!");
            return false;
        }

        // Stringstream is used as a buffer
        std::ostringstream ss;

        // Setup request
        CURL * c = initCurlHandle(url);
        CURLcode rc = curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writeDataStream);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToString] Failed to setopt CURLOPT_WRITEFUNCTION: " + std::to_string(rc));
        }
        rc = curl_easy_setopt(c, CURLOPT_WRITEDATA, &ss);
        if (rc != CURLE_OK) {
            Log::writeWarning("[downloadToString] Failed to setopt CURLOPT_WRITEDATA: " + std::to_string(rc));
        }

        // Perform request
        rc = curl_easy_perform(c);
        if (rc != CURLE_OK) {
            setErrorMsg("[downloadToString] An error occurred while performing the request: " + std::to_string(rc));
        }

        // Get HTTP result code to determine if the response was ok
        long code;
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
        if (rc == CURLE_OK && code != 200) {
            setErrorMsg("[downloadToString] The request was successful, but the response has HTTP code: " + std::to_string(code));
        }

        // Cleanup
        curl_easy_cleanup(c);

        buf = ss.str();
        return (rc == CURLE_OK && code == 200);
    }

    std::string encodeString(const std::string & str) {
        if (!ready) {
            setErrorMsg("[encodeString] curl has not been initialized!");
            return str;
        }
        if (str.empty()) {
            return str;
        }

        // Use curl to escape
        std::string ret;
        CURL * c = curl_easy_init();
        char * encStr = curl_easy_escape(c, str.c_str(), str.length());
        if (encStr) {
            ret = std::string(encStr);
            curl_free(encStr);
        } else {
            ret = str;
        }
        curl_easy_cleanup(c);

        return ret;
    }
}