#include <memory>
#include <string>
#include <assert.h>

#include "connect.hh"

std::unique_ptr<sql::Connection>
getConn(const std::string &user_name,
        const std::string &password,
        const std::string &address,
        const std::string &port)
{
    /**
     * Must be non-empty.
     */
    assert((bool)user_name.size() && (bool)address.size());

    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();

    sql::ConnectOptionsMap com = std::move(getOptions(address, user_name, password, port));

    return std::unique_ptr<sql::Connection>(driver->connect(com));
}

sql::ConnectOptionsMap
getOptions(const std::string &address,
           const std::string &user_name,
           const std::string &password,
           const std::string &port)
{
    sql::ConnectOptionsMap connection_properties;

    connection_properties["hostName"] = address;
    connection_properties["userName"] = user_name;
    connection_properties["password"] = password;
    connection_properties["port"] = std::stoi(port);
    connection_properties["OPT_READ_TIMEOUT"] = 10000;

    return connection_properties;
}
