#pragma once
#include "NanoSpark.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

namespace spark
{
    class SourceLoader
    {
    public:
        // 把 .spark 文件解析成 token 列表
        // 支持：
        //   - 双引号字符串（"..."）
        //   - 行注释（//）
        //   - 空白分割
        std::vector<std::string> LoadTokens(const std::string& filename)
        {
            std::vector<std::string> tokens;
            std::ifstream file(filename);

            if (!file.is_open())
            {
                error("Failed to open file: " + filename);
                return tokens;
            }

            std::string line;
            while (std::getline(file, line))
            {
                std::string token;
                bool in_quotes = false;

                for (size_t i = 0; i < line.size(); ++i)
                {
                    char ch = line[i];

                    // 引号内：原样累积
                    if (in_quotes)
                    {
                        if (ch == '"')
                        {
                            in_quotes = false;
                            tokens.push_back(token);
                            token.clear();
                        }
                        else
                        {
                            token += ch;
                        }
                        continue;
                    }

                    // 引号外：进入引号
                    if (ch == '"')
                    {
                        in_quotes = true;
                        continue;
                    }

                    // 引号外：// 注释，跳过本行剩余
                    if (ch == '/' && i + 1 < line.size() && line[i + 1] == '/')
                    {
                        break;
                    }

                    // 引号外：空白分割
                    if (std::isspace(static_cast<unsigned char>(ch)))
                    {
                        if (!token.empty())
                        {
                            tokens.push_back(token);
                            token.clear();
                        }
                    }
                    else
                    {
                        token += ch;
                    }
                }

                // 行末的 token
                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }

            return tokens;
        }

        // 可选：从字符串解析（方便测试）
        std::vector<std::string> LoadTokensFromString(const std::string& content)
        {
            std::vector<std::string> tokens;
            std::istringstream stream(content);
            std::string line;

            while (std::getline(stream, line))
            {
                std::string token;
                bool in_quotes = false;

                for (size_t i = 0; i < line.size(); ++i)
                {
                    char ch = line[i];

                    if (in_quotes)
                    {
                        if (ch == '"')
                        {
                            in_quotes = false;
                            tokens.push_back(token);
                            token.clear();
                        }
                        else
                        {
                            token += ch;
                        }
                        continue;
                    }

                    if (ch == '"')
                    {
                        in_quotes = true;
                        continue;
                    }

                    if (ch == '/' && i + 1 < line.size() && line[i + 1] == '/')
                    {
                        break;
                    }

                    if (std::isspace(static_cast<unsigned char>(ch)))
                    {
                        if (!token.empty())
                        {
                            tokens.push_back(token);
                            token.clear();
                        }
                    }
                    else
                    {
                        token += ch;
                    }
                }

                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }

            return tokens;
        }
    };
}