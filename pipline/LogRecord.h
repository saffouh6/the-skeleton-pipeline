#pragma once
#include <string>
#include <map>
#include <chrono>

struct LogRecord {
    std::string timestamp; // Şimdilik string veya zaman damgası
    std::string clientIp;
    std::string method;
    std::string path;
    int status;
    long bytes;
    std::string userAgent;
    std::map<std::string, std::string> attributes; // Sonraki haftalar için genişleme noktası
    std::string raw;

    LogRecord(std::string ts, std::string ip, std::string m, std::string p, int stat, long b, std::string ua, std::string r)
        : timestamp(std::move(ts)), clientIp(std::move(ip)), method(std::move(m)), path(std::move(p)),
          status(stat), bytes(b), userAgent(std::move(ua)), raw(std::move(r)) {}
};