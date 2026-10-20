#pragma once
#include "Interfaces.h"
#include "LogRecord.h"
#include <vector>
#include <string>

class ProjectionStage : public Stage<LogRecord, LogRecord> {
private:
    std::vector<std::string> fields;

public:
    // Dinamik olarak gelen std::vector türünü kabul eden constructor
    ProjectionStage(std::vector<std::string> fList) : fields(std::move(fList)) {}

    // Süslü parantezli sabit tanımlar için initializer_list desteği
    ProjectionStage(std::initializer_list<std::string> fList) : fields(fList) {}

    void process(const LogRecord& input, Emitter<LogRecord>& out) override {
        LogRecord projected = input;
        // Projeksiyon işlemleri burada gerçekleştirilir
        out.emit(projected);
    }
};