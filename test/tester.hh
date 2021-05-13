#pragma once
#ifndef _TESTER_HH_
#define _TESTER_HH_

#define DO_OPTIMIZATION

#include <string>

#include "client.hh"

typedef class Tester
{
public:
    Tester() = delete;

    Tester(const std::string &address, const std::string &user_name,
           const std::string &password, const std::string &port,
           const std::string &test_case_path, const std::string &csv_file_path, const std::string &output_file_path)
        : client(std::unique_ptr<Client>(new Client(address, user_name, password, port, output_file_path))),
          test_case_path(test_case_path), csv_file_path(csv_file_path), output_file_path(output_file_path)
    {
    }

    void start() const;

private:
    const std::string test_case_path;
    const std::string csv_file_path;
    const std::string output_file_path;

    const std::unique_ptr<Client> client;
} Tester;

#endif