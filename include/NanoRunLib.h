#pragma once
#include "NanoSpark.h"
#include "NanoSource.h"

namespace spark
{
    class RunLib
    {
    private:
        SparkManager& manager;

    public:
        explicit RunLib(SparkManager& sm) : manager(sm) {}

        void Register()
        {
            manager.commands["run_file"] = [this](NanoSpark& spark) {
                auto val = spark.pop();
                if (!std::holds_alternative<std::string>(val))
                {
                    error("run_file: expected string path");
                }
                
                const std::string& path = std::get<std::string>(val);
                SourceLoader loader;
                auto tokens = loader.LoadTokens(path);
                if (tokens.empty())
                {
                    return;
                }
                manager.run(tokens);
            };

            manager.commands["run_string"] = [this](NanoSpark& spark) {
                auto val = spark.pop();
                if (!std::holds_alternative<std::string>(val))
                {
                    error("run_string: expected string");
                }
                const std::string& code = std::get<std::string>(val);

                SourceLoader loader;
                auto tokens = loader.LoadTokensFromString(code);
                if (tokens.empty())
                {
                    return;
                }
                manager.run(tokens);
            };
        }
    };
}