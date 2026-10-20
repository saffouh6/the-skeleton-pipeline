#pragma once
#include "Interfaces.h"
#include "LogRecord.h"
#include <sstream>
#include <regex>
#include <string>

class ParserStage : public Stage<std::string, LogRecord> {
private:
    int errorCount = 0;

public:
    void process(const std::string& input, Emitter<LogRecord>& out) override {
        if (input.empty()) {
            errorCount++;
            return;
        }

        // Örnek log formatı: 127.0.0.1- [10/Oct/2026:13:55:36 +0300] "GET /index.html HTTP/1.1" 200 2326
        // Esnek bir regex kalıbı ile IP, tarih, HTTP metodu, path, status kodu ve byte değerini yakalıyoruz.
        std::regex logRegex(R"(^([^\s\[]+).*?\[([^\]]+)\]\s+\"([A-Z]+)\s+([^\s\?\#\"]+)[^\"]*\"\s+(\d+)\s+(\d+|-))");
        std::smatch match;

        if (std::regex_search(input, match, logRegex) && match.size() >= 7) {
            try {
                std::string ip = match[1].str();
                std::string timestamp = match[2].str();
                std::string method = match[3].str();
                std::string path = match[4].str();
                int status = std::stoi(match[5].str());
                
                std::string bytesStr = match[6].str();
                long bytes = (bytesStr == "-") ? 0 : std::stol(bytesStr);

                // Gerçek ayrıştırılan verilerle LogRecord oluşturuluyor
                LogRecord record(timestamp, ip, method, path, status, bytes, "DefaultAgent", input);
                
                out.emit(record);
            } catch (...) {
                errorCount++;
            }
        } else {
            errorCount++; // Formatı uymayan satırlar hata olarak sayılır
        }
    }

    int getErrorCount() const {
        return errorCount;
    }
};