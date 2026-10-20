#pragma once
#include "Interfaces.h"
#include "LogRecord.h"
#include <string>
#include <fstream>
#include <stdexcept>

class FileSink : public Sink<LogRecord> {
private:
    std::string filename;
    std::ofstream outFile;

public:
    explicit FileSink(std::string fname) : filename(std::move(fname)) {}

    void open() {
        outFile.open(filename);
        if (!outFile.is_open()) {
            throw std::runtime_error("Çıktı dosyası açılamadı: " + filename);
        }
    }

    void consume(const LogRecord& item) override {
        if (outFile.is_open()) {
            outFile << item.raw << std::endl;
        }
    }

    void close() {
        if (outFile.is_open()) {
            outFile.close(); // Yaşam döngüsü kancası ile güvenli kapatma
        }
    }

    ~FileSink() {
        close();
    }
};