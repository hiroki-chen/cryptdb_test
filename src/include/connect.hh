#pragma once

#ifndef _CONNECT_HH_
#define _CONNECT_HH_

#include <memory>

#include "mysql/jdbc.h"

std::unique_ptr<sql::Connection>
getConn(const std::string &user_name,
        const std::string &password,
        const std::string &address,
        const std::string &port);

sql::ConnectOptionsMap
getOptions(const std::string &address,
           const std::string &user_name,
           const std::string &password,
           const std::string &port);

#endif