#pragma once
#include <memory>
#include "Interfaces.h"
#include "LogRecord.h"

class Pipeline {
private:
    std::shared_ptr<Source<std::string>> source;
    std::shared_ptr<Stage<std::string, LogRecord>> stage;
    std::shared_ptr<Sink<LogRecord>> sink;

public:
    // Kaynak, Ara Aşama (Parser) ve Hedef alan constructor (3 parametreli)
    Pipeline(std::shared_ptr<Source<std::string>> src, 
             std::shared_ptr<Stage<std::string, LogRecord>> stg, 
             std::shared_ptr<Sink<LogRecord>> snk)
        : source(src), stage(stg), sink(snk) {}

    void run() {
        class StageEmitter : public Emitter<LogRecord> {
        private:
            Sink<LogRecord>& targetSink;
        public:
            explicit StageEmitter(Sink<LogRecord>& s) : targetSink(s) {}
            void emit(const LogRecord& item) override {
                targetSink.consume(item);
            }
        };

        class SourceEmitter : public Emitter<std::string> {
        private:
            Stage<std::string, LogRecord>& targetStage;
            Emitter<LogRecord>& targetEmitter;
        public:
            SourceEmitter(Stage<std::string, LogRecord>& stg, Emitter<LogRecord>& em) 
                : targetStage(stg), targetEmitter(em) {}

            void emit(const std::string& line) override {
                targetStage.process(line, targetEmitter);
            }
        };

        StageEmitter stageEmitter(*sink);
        SourceEmitter sourceEmitter(*stage, stageEmitter);

        source->produce(sourceEmitter);
    }
};