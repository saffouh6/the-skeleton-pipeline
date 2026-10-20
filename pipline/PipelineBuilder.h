#pragma once
#include <memory>
#include <functional>
#include "Interfaces.h"
#include "LogRecord.h"
#include "Pipeline.h"

class PipelineBuilder {
public:
    // Başlangıç kaynağını alır (.from)
    template <typename SourceType>
    static auto from(std::shared_ptr<SourceType> src) {
        return BuilderStage<std::string, std::string>(src);
    }

private:
    template <typename InT, typename OutT>
    class BuilderStage {
    // Bu sınıf, .via() ve .to() metodları ile zincirlemeyi yönetir
    };
};