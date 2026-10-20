#pragma once
#include "Interfaces.h"
#include <string>
#include <fstream>
#include <stdexcept>

class FileLineSource : public Source<std::string> {
private:
    std::string filename;
public:
    explicit FileLineSource(std::string fname) : filename(std::move(fname)) {}

    void produce(Emitter<std::string>& out) override {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Dosya açılamadı: " + filename);
        }
        std::string line;
        while (std::getline(file, line)) {
            out.emit(line);
        }
    }
};