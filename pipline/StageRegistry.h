#pragma once
#include <string>
#include <functional>
#include <map>
#include <stdexcept>
#include <memory>
#include "Interfaces.h"
#include "LogRecord.h"

using StageCreator = std::function<std::shared_ptr<Stage<LogRecord, LogRecord>>(const std::map<std::string, std::string>&)>;

class StageRegistry {
private:
    std::map<std::string, StageCreator> creators;

    StageRegistry() = default;

public:
    static StageRegistry& getInstance() {
        static StageRegistry instance;
        return instance;
    }

    void registerStage(const std::string& typeName, StageCreator creator) {
        creators[typeName] = creator;
    }

    std::shared_ptr<Stage<LogRecord, LogRecord>> createStage(const std::string& typeName, const std::map<std::string, std::string>& params) {
        auto it = creators.find(typeName);
        if (it == creators.end()) {
            throw std::runtime_error("Configuration Error: Unknown stage type: " + typeName);
        }
        return it->second(params);
    }
};