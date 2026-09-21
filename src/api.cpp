#include <curl/curl.h>
#include <iostream>
#include <string>

int main() {
    CURL* curl = curl_easy_init();

    if (!curl) {
        return 1;
    }

    std::string json =
        R"({"name":"John","email":"john@example.com"})";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(
        headers,
        "Content-Type: application/json"
    );

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        "https://example.com/insert.php"
    );

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

    CURLcode result = curl_easy_perform(curl);

    if (result == CURLE_OK) {
        std::cout << "Request sent successfully\n";
    } else {
        std::cerr << curl_easy_strerror(result) << '\n';
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
