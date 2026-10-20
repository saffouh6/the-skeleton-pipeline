#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include "FileLineSource.h"
#include "ParserStage.h"
#include "PredicateFilterStage.h"
#include "ProjectionStage.h"
#include "FileSink.h"
#include "Pipeline.h"
#include "PipelineLoader.h"
#include "StageRegistry.h"

inline std::vector<std::string> parseFields(const std::string& fieldStr) {
    std::vector<std::string> fields;
    std::stringstream ss(fieldStr);
    std::string item;
    while (std::getline(ss, item, ',')) {
        fields.push_back(item);
    }
    return fields;
}

// Pipeline adımlarını sırayla işleten global yardımcı Emitter sınıfı
class PipelineExecutor : public Emitter<std::string> {
private:
    ParserStage& parser;
    PredicateFilterStage& statusFilter;
    PredicateFilterStage& pathFilter;
    PredicateFilterStage& methodFilter;
    ProjectionStage& projection;
    FileSink& fileSink;

public:
    PipelineExecutor(ParserStage& p, PredicateFilterStage& sf, PredicateFilterStage& pf,
                     PredicateFilterStage& mf, ProjectionStage& pr, FileSink& fs)
        : parser(p), statusFilter(sf), pathFilter(pf), methodFilter(mf), projection(pr), fileSink(fs) {}

    void emit(const std::string& line) override {
        class FinalSinkEmitter : public Emitter<LogRecord> {
            FileSink& sink;
        public:
            FinalSinkEmitter(FileSink& s) : sink(s) {}
            void emit(const LogRecord& r) override { sink.consume(r); }
        };

        class ProjEmitter : public Emitter<LogRecord> {
            ProjectionStage& proj;
            Emitter<LogRecord>& next;
        public:
            ProjEmitter(ProjectionStage& pr, Emitter<LogRecord>& n) : proj(pr), next(n) {}
            void emit(const LogRecord& r) override { proj.process(r, next); }
        };

        class MethodEmitter : public Emitter<LogRecord> {
            PredicateFilterStage& mf;
            Emitter<LogRecord>& next;
        public:
            MethodEmitter(PredicateFilterStage& m, Emitter<LogRecord>& n) : mf(m), next(n) {}
            void emit(const LogRecord& r) override { mf.process(r, next); }
        };

        class PathEmitter : public Emitter<LogRecord> {
            PredicateFilterStage& pf;
            Emitter<LogRecord>& next;
        public:
            PathEmitter(PredicateFilterStage& p, Emitter<LogRecord>& n) : pf(p), next(n) {}
            void emit(const LogRecord& r) override { pf.process(r, next); }
        };

        class StatusEmitter : public Emitter<LogRecord> {
            PredicateFilterStage& sf;
            Emitter<LogRecord>& next;
        public:
            StatusEmitter(PredicateFilterStage& s, Emitter<LogRecord>& n) : sf(s), next(n) {}
            void emit(const LogRecord& r) override { sf.process(r, next); }
        };

        FinalSinkEmitter finalSink(fileSink);
        ProjEmitter projEmit(projection, finalSink);
        MethodEmitter methodEmit(methodFilter, projEmit);
        PathEmitter pathEmit(pathFilter, methodEmit);
        StatusEmitter statusEmit(statusFilter, pathEmit);

        parser.process(line, statusEmit);
    }
};

int main(int argc, char* argv[]) {
    std::string configPath = "";
    std::string inputFile = "";
    std::string outputFile = "";
    int minStatus = 0;
    std::string pathPrefix = "";
    std::string targetMethod = "";
    std::vector<std::string> selectedFields = {"timestamp", "clientIp", "status", "path"};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (arg == "--min-status" && i + 1 < argc) {
            minStatus = std::stoi(argv[++i]);
        } else if (arg == "--path-prefix" && i + 1 < argc) {
            pathPrefix = argv[++i];
        } else if (arg == "--method" && i + 1 < argc) {
            targetMethod = argv[++i];
        } else if (arg == "--fields" && i + 1 < argc) {
            selectedFields = parseFields(argv[++i]);
        } else if (inputFile.empty()) {
            inputFile = arg;
        } else if (outputFile.empty()) {
            outputFile = arg;
        }
    }

    try {
        // Eğer --config parametresi verildiyse PipelineLoader ile doğrulama yap
        if (!configPath.empty()) {
            PipelineLoader::loadAndValidate(configPath);
            std::cout << "Islem basariyla tamamlandi! Gecerli konfigurasyon: " << configPath << std::endl;
            return 0;
        }

        // Eğer --config verilmediyse eski parametrik çalışma modunu işlet
        if (inputFile.empty() || outputFile.empty()) {
            std::cerr << "Kullanim: " << argv[0] << " --config <config_dosyasi> VEYA " 
                      << argv[0] << " <girdi> <cikti> [--min-status <kod>] [--path-prefix <on ek>] [--method <metod>] [--fields <alanlar>]" << std::endl;
            return 1;
        }

        auto source = std::make_shared<FileLineSource>(inputFile);
        auto parser = std::make_shared<ParserStage>();

        auto statusFilter = std::make_shared<PredicateFilterStage>([minStatus](const LogRecord& r) {
            return (minStatus == 0 || r.status >= minStatus);
        });

        auto pathFilter = std::make_shared<PredicateFilterStage>([pathPrefix](const LogRecord& r) {
            if (pathPrefix.empty()) return true;
            return (r.path.rfind(pathPrefix, 0) == 0);
        });

        auto methodFilter = std::make_shared<PredicateFilterStage>([targetMethod](const LogRecord& r) {
            if (targetMethod.empty()) return true;
            return (r.method == targetMethod);
        });

        auto projection = std::shared_ptr<ProjectionStage>(new ProjectionStage(selectedFields)); 

        auto fileSink = std::make_shared<FileSink>(outputFile);
        fileSink->open();

        PipelineExecutor runner(*parser, *statusFilter, *pathFilter, *methodFilter, *projection, *fileSink);
        source->produce(runner);

        fileSink->close();

        std::cout << "Islem basariyla tamamlandi!" << std::endl;
        std::cout << "Cikti dosyasi: " << outputFile << std::endl;
        std::cout << "Aktif Moduler Filtreler -> Status >= " << minStatus 
                  << ", Path Prefix: " << (pathPrefix.empty() ? "Yok" : pathPrefix) 
                  << ", Method: " << (targetMethod.empty() ? "Tumu" : targetMethod) << std::endl;
        std::cout << "Toplam Hatali Satir Sayisi: " << parser->getErrorCount() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Hata olustu: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}