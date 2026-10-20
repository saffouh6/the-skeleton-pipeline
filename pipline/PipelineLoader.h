#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory>
#include "StageRegistry.h"
#include "PredicateFilterStage.h"
#include "ProjectionStage.h"
#include "ParserStage.h"

class PipelineLoader {
public:
    static void loadAndValidate(const std::string& configPath) {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw std::runtime_error("Configuration Error: Config file could not be opened: " + configPath);
        }

        std::string line;
        bool hasSource = false;
        bool hasSink = false;

        while (std::getline(file, line)) {
            if (line.find("type: file") != std::string::npos) {
                if (!hasSource) hasSource = true;
                else hasSink = true;
            }
        }

        if (!hasSource) {
            throw std::runtime_error("Configuration Error: Mandatory source parameter is missing!");
        }
        if (!hasSink) {
            throw std::runtime_error("Configuration Error: Mandatory sink parameter is missing!");
        }
    }
};