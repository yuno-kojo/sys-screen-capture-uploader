#include <iostream>
#include <cstdio>
#include <netdb.h>
#include <filesystem>
#include <curl/curl.h>
#include "config.hpp"
#include "logger.hpp"

namespace fs = filesystem;

struct upload_info {
    FILE *f;
    size_t sizeLeft;
};

static size_t _uploadReadFunction(void *ptr, size_t size, size_t nmemb, void *data) {
    auto *ui = (struct upload_info*)data;
    size_t maxBytes = size * nmemb;
    if (maxBytes < 1)
        return 0;

    if (ui->sizeLeft) {
        size_t bytes = min(ui->sizeLeft, maxBytes);
        size_t bytesRead = fread(ptr, (size_t)1, bytes, ui->f);
        ui->sizeLeft -= bytesRead;
        return bytes;
    }

    return 0;
}

bool sendFileToServer(string &path, size_t size) {
    string tid = path.substr(path.length() - 36, 32);
    Logger::get().debug() << "Title ID: " << tid << endl;
    if (!Config::get().uploadAllowed(tid, path.back() == '4')) {
        Logger::get().info() << "Skipping upload for " << path << endl;
        return true;
    }
    
    fs::path filePath = path;
    string contentType, copyName, telegramMethod;
    if (filePath.extension() == ".jpg") {
        contentType = "image/jpeg";
        copyName = "photo";
        telegramMethod = "sendPhoto";
    } else if (filePath.extension() == ".mp4") {
        contentType = "video/mp4";
        copyName = "video";
        telegramMethod = "sendVideo";
    } else {
        Logger::get().error() << "Unknown file extension: " + filePath.extension().string() << endl;
        return false;
    }

    FILE *f = fopen(filePath.c_str(), "rb");

    if (f == nullptr) {
        Logger::get().error() << "fopen() failed" << endl;
        return false;
    }

    struct upload_info ui = { f, size };
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, copyName.c_str(),
                 CURLFORM_FILENAME, filePath.c_str(),
                 CURLFORM_STREAM, &ui,
                 CURLFORM_CONTENTSLENGTH, size,
                 CURLFORM_CONTENTTYPE, contentType.c_str(),
                 CURLFORM_END);

    CURL *curl = curl_easy_init();
    if (curl) {
        const string url = "https://api.telegram.org/bot" + 
            Config::get().getTelegramBotToken() + "/" + telegramMethod +
            "?chat_id=" + Config::get().getTelegramChatId();

        Logger::get().debug() << "URL is " << url << endl;

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, _uploadReadFunction);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 0x2000L);
        curl_easy_setopt(curl, CURLOPT_UPLOAD_BUFFERSIZE, 0x2000L);
        CURLcode res = curl_easy_perform(curl);
        fclose(f);

        if (res == CURLE_OK) {
            long responseCode;
            double requestSize;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &requestSize);
            Logger::get().debug() << requestSize << " bytes sent, response code: " << responseCode << endl;
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
            if (responseCode == 200) {
                 Logger::get().info() << "Successfully uploaded " << path << endl;
                 return true;
            }
            Logger::get().error() << "Error uploading, got response code " << responseCode << endl;
            return false;
        } else {
            Logger::get().error() << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
            return false;
        }
    }

    Logger::get().error() << "curl_easy_init() failed" << endl;
    return false;
}
