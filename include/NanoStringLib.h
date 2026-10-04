#pragma once
#include "NanoSpark.h"

#include <string>

namespace spark
{
    class StringLib
    {
    public:
        void Register(SparkManager& sm)
        {
            // int → string，压栈
            sm.commands["int2str"] = [](NanoSpark& spark) {
                auto v = spark.pop();
                if (!std::holds_alternative<int>(v)) {
                    error("int2str: expected int");
                }
                spark.push(std::to_string(std::get<int>(v)));
            };

            // double → string
            sm.commands["dbl2str"] = [](NanoSpark& spark) {
                auto v = spark.pop();
                if (!std::holds_alternative<double>(v)) {
                    error("dbl2str: expected double");
                }
                spark.push(std::to_string(std::get<double>(v)));
            };

            // string → int
            sm.commands["str2int"] = [](NanoSpark& spark) {
                auto v = spark.pop();
                if (!std::holds_alternative<std::string>(v)) {
                    error("str2int: expected string");
                }
                const auto& s = std::get<std::string>(v);
                try {
                    spark.push(std::stoi(s));
                } catch (...) {
                    error("str2int: not a number: " + s);
                }
            };

            // 两个 string 拼接，[a][b] → a+b
            sm.commands["concat"] = [](NanoSpark& spark) {
                auto b = spark.pop();
                auto a = spark.pop();
                if (!std::holds_alternative<std::string>(a) ||
                    !std::holds_alternative<std::string>(b)) {
                    error("concat: expected two strings");
                }
                spark.push(std::get<std::string>(a) + std::get<std::string>(b));
            };

            // 字符串长度
            sm.commands["strlen"] = [](NanoSpark& spark) {
                auto v = spark.pop();
                if (!std::holds_alternative<std::string>(v)) {
                    error("strlen: expected string");
                }
                spark.push((int)std::get<std::string>(v).size());
            };

            // 字符串相等：1 / 0
            sm.commands["streq"] = [](NanoSpark& spark) {
                auto b = spark.pop();
                auto a = spark.pop();
                if (!std::holds_alternative<std::string>(a) ||
                    !std::holds_alternative<std::string>(b)) {
                    error("streq: expected two strings");
                }
                spark.push((int)(std::get<std::string>(a) == std::get<std::string>(b)));
            };

            // 取子串：[str][start][len] → substr
            sm.commands["substr"] = [](NanoSpark& spark) {
                auto len = spark.pop();
                auto start = spark.pop();
                auto str = spark.pop();
                if (!std::holds_alternative<std::string>(str) ||
                    !std::holds_alternative<int>(start) ||
                    !std::holds_alternative<int>(len)) {
                    error("substr: expected [str][start][len]");
                }
                const auto& s = std::get<std::string>(str);
                int st = std::get<int>(start);
                int ln = std::get<int>(len);
                if (st < 0 || ln < 0 || (size_t)(st + ln) > s.size()) {
                    error("substr: index out of range");
                }
                spark.push(s.substr(st, ln));
            };
        }
    };
}