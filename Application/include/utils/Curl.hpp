#ifndef UTILS_CURL_HPP
#define UTILS_CURL_HPP

#include <functional>
#include <string>
#include <vector>

// Wrapper for cURL to make requests and download content from the internet
namespace Utils::Curl {
    // Initialize/tidy up the library
    void init();
    void exit();

    // Returns the latest error message
    std::string error();

    // All requests return true if they succeeded and false otherwise
    // Download a file into the provided buffer
    bool downloadToBuffer(const std::string &, std::vector<unsigned char> &);
    // Download a file from the provided URL to the given destination
    bool downloadToFile(const std::string &, const std::string &, std::function<void(long long, long long)>);
    // Return the response in the provided string
    bool downloadToString(const std::string &, std::string &);

    // Returns the encoded version of the passed string (not changed if an error occurred)
    std::string encodeString(const std::string &);
};

#endif