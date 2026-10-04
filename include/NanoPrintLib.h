#pragma once
#include <iostream>
#include "NanoSpark.h"
using namespace spark;

namespace spark
{
    class PrintLib
    {
    public:
        void Register(SparkManager &sm)
        {
            // 打印栈顶并弹出，带换行
            sm.commands["print"] = [](NanoSpark &spark)
            {
                auto val = spark.pop();
                std::visit([](auto &&arg)
                           {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int> ||
                                  std::is_same_v<T, double> ||
                                  std::is_same_v<T, std::string>) {
                        std::cout << arg << "\n";
                    } else {
                        std::cout << "[unsupported]\n";
                    } }, val);
            };

            // 打印栈顶但不弹出
            sm.commands["peek"] = [](NanoSpark &spark)
            {
                if (spark.stack.stack.empty())
                {
                    error("peek: stack is empty");
                }
                std::visit([](auto &&arg)
                           {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int> ||
                                  std::is_same_v<T, double> ||
                                  std::is_same_v<T, std::string>) {
                        std::cout << arg << "\n";
                    } else {
                        std::cout << "[unsupported]\n";
                    } }, spark.top());
            };

            // 只打印字符串，不弹不换行
            sm.commands["putstr"] = [](NanoSpark &spark)
            {
                auto val = spark.pop();
                if (!std::holds_alternative<std::string>(val))
                {
                    error("putstr: expected string");
                }
                std::cout << std::get<std::string>(val);
            };

            // 换行
            sm.commands["newline"] = [](NanoSpark &)
            {
                std::cout << "\n";
            };

            // 打印整个主栈（调试用），不弹
            sm.commands["dump"] = [](NanoSpark &spark)
            {
                std::cout << "[STACK] ";
                for (const auto &v : spark.stack.stack)
                {
                    std::visit([](auto &&arg)
                               {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, int> ||
                                      std::is_same_v<T, double> ||
                                      std::is_same_v<T, std::string>) {
                            std::cout << arg << " ";
                        } else {
                            std::cout << "[?] ";
                        } }, v);
                }
                std::cout << "\n";
            };
        }
    };
}
