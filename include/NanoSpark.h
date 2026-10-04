#pragma once

#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <functional>
#include <charconv>

// 留作统一错误处理接口的占位符。
inline void error(const std::string &msg)
{
    throw std::runtime_error(msg);
}

namespace spark
{
    struct Value;
    using value = std::variant<
        int,
        double,
        std::string,
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<std::unordered_map<std::string, Value>>>;

    struct NanoStack
    {
        std::vector<value> stack;
        std::vector<int> IntStack;
        std::unordered_map<std::string, value> MapStack;
        std::vector<std::string> StringCache;
        std::vector<value> Cache;
    };
    struct Value
    {
        value data;
        Value() = default;
        Value(int v) : data(v) {}
        Value(double v) : data(v) {}
        Value(const std::string &v) : data(v) {}
        Value(std::shared_ptr<std::vector<Value>> v) : data(v) {}
        Value(const value &v) : data(v) {}
        Value(std::shared_ptr<std::unordered_map<std::string, Value>> v) : data(v) {}
    };
    /*
    关于目前的想法：
    本身只提供简单的pop，push，clear，top相关操作，其他一律作为外部函数
    NanoStack提供多个容器：
    stack，为主要的栈。
    IntStack，纯整数栈，可以用作计数器，或者存储跳转地址。
    MapStack，存储键值对，作为变量存储。
    StringCache，作为纯文字缓存。宏的定义方式将会用到他。
    Cache，作为通用缓存。用于存储一些临时数据。
    */
    enum class PointStackType
    {
        Stack,
        IStack,
        MStack,
        SCache,
        Cache
    };
    class NanoSpark
    {
    public:
        NanoStack stack;
        PointStackType pointStackType = PointStackType::Stack; // 用作指示的枚举类型变量
    public:
        template <typename T>
        void push(const T &value)
        {
            stack.stack.push_back(value);
        }

        value pop()
        {
            if (stack.stack.empty())
            {
                error("Stack is empty");
            }
            value val = stack.stack.back();
            stack.stack.pop_back();
            return val;
        }

        value top() const
        {
            if (stack.stack.empty())
            {
                error("Stack is empty");
            }
            return stack.stack.back();
        }

        void clear()
        {
            stack.stack.clear();
        }
    };
    using Func = std::function<void(NanoSpark &)>;
    using strings = std::vector<std::string>;
    class SparkManager
    {
    public:
        NanoSpark spark;
        std::unordered_map<std::string, Func> commands; // 存储定义内部的函数
        std::unordered_map<std::string, strings> words; // 存储定义的文字宏
    public:
        value parse_value(const std::string &s)
        {
            int int_val;
            if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), int_val);
                ec == std::errc{} && ptr == s.data() + s.size())
            {
                return value{int_val};
            }
            double double_val;
            if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), double_val);
                ec == std::errc{} && ptr == s.data() + s.size())
            {
                return value{double_val};
            }
            return value{s};
        }
        int runtimes = 0;
        void run(const strings &codes)
        {
            do
            {
                for (const auto &code : codes)
                {
                    if (code == "_RUNEND")
                    {
                        return;
                    }
                    if (code == "_RUNNEXT")
                    {
                        continue;
                    }
                    if (code == "_TOSTACK")
                    {
                        spark.pointStackType = PointStackType::Stack;
                        continue;
                    }
                    if (spark.pointStackType == PointStackType::SCache)
                    {
                        spark.stack.StringCache.push_back(code);
                        continue;
                    }

                    auto it = commands.find(code);
                    if (it != commands.end())
                    {
                        it->second(spark);
                    }
                    else
                    {
                        auto wordIt = words.find(code);
                        if (wordIt != words.end())
                        {
                            run(wordIt->second);
                        }
                        else
                        {
                            spark.push(parse_value(code));
                        }
                    }
                }
            } while (runtimes-- > 0);
        }
        void commandInit();
        SparkManager()
        {
            commandInit();
        }
    };

}
