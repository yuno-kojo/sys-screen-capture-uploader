#pragma once

#include <string>
#include <switch.h>
#include <map>

using namespace std;

class Config {
public:
    static Config& get() {
        static Config instance;
        return instance;
    }

    bool refresh();

    string getTelegramBotToken();
    string getTelegramChatId();
    bool uploadAllowed(string &tid, bool isMovie);
    bool keepLogs();

    bool error;

private:
    string m_telegramBotToken;
    string m_telegramChatId;
    bool m_uploadScreenshots;
    bool m_uploadMovies;
    bool m_keepLogs;
    map<string, bool> m_titleScreenshots;
    map<string, bool> m_titleMovies;
};
