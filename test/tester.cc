#include <algorithm>
#include <fstream>
#include <string>

#include "tester.hh"
#include "helper.hh"
#include "macro.hh"

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

        if (line.empty() || line[0] == '\'')
        {
            continue;
        }

        std::vector<std::string> params = split(line, ", ");

        const std::string type = params[0];
        const std::string column_name = params[1];
        const unsigned long limited = std::stoul(params[2]);

        std::string split_line = "--------------------" + get_time();
        split_line.append("---------------------------------------\n");

        client.get()->writeLog(split_line);
        if (0 == type.compare("OPE"))
        {
            client.get()->noEncyption(csv_file_path, column_name, limited);
            client.get()->encryptByOPE(csv_file_path, column_name, limited);
            //client.get()->noFrequencyHiding(csv_file_path, column_name, limited);

            /**
             * TODO: implement other OPE methods like Kerschbaum's OPE as well as mOPE.
             */
        }
        else if (0 == type.compare("DET"))
        {
            std::vector<double> parameters;

            std::transform(params.cbegin() + 3, params.cend(), std::back_inserter(parameters), [](const std::string &param) {
                return std::stod(param);
            });

            client.get()->noEncyption(csv_file_path, column_name, limited);
            client.get()->noFrequencyHiding(csv_file_path, column_name, limited);
            client.get()->encryptByDET(csv_file_path, column_name, parameters, limited, true);
        }
    }

    in.close();
}

Tester::~Tester()
{
    /**
     * Do some clean-ups.
     */
    client.get()->directiveHandler(DROP_DET_TABLE);
    client.get()->directiveHandler(DROP_OPE_TABLE);
    client.get()->directiveHandler(DROP_NO_FH_TABLE);
}