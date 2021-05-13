#include <algorithm>
#include <fstream>
#include <string>

#include "tester.hh"
#include "helper.hh"

/**
 * @note By defualt, be sure that you split every parameters in comma.
 */
void Tester::start() const
{
    client->defaultDatabase("test");

    std::ifstream in(test_case_path, std::ios::in);

    std::string line;
    while (!in.eof())
    {
        getline(in, line);

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> params = split(line, ", ");

        const std::string type = params[0];
        const std::string column_name = params[1];
        const unsigned int limited = std::stoul(params[2]);

        if (0 == type.compare("OPE"))
        {
            client.get()->encryptByOPE(csv_file_path, column_name, limited);
        }
        else if (0 == type.compare("DET"))
        {
            std::vector<double> parameters;

            std::transform(params.cbegin() + 3, params.cend(), std::back_inserter(parameters), [](const std::string &param) {
                return std::stod(param);
            });

            client.get()->encryptByDET(csv_file_path, column_name, parameters, limited);
        }
    }

    in.close();
}