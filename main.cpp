#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>

#include "NanoSpark.h"
#include "NanoPrintLib.h"
#include "NanoStackLib.h"
#include "NanoStringLib.h"

#include "NanoSource.h"
#include "NanoRunLib.h"

spark::SparkManager sm;

int main()
{
    try
    {
        spark::SourceLoader loader;
        while (1)
        {
            spark::SparkManager sm;
            PrintLib().Register(sm);
            StringLib().Register(sm);
            StackLib().Register(sm);
            spark::RunLib runLib(sm);
            runLib.Register();
            auto tokens = loader.LoadTokens("../nanospark/main.spark");
            sm.run(tokens);
            std::cin.get();
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cout << "SOURCE_DIR = [" << NANOSPARK_SOURCE_DIR << "]\n";
        std::cerr << e.what() << '\n';
        std::cin.get();
    }
}
