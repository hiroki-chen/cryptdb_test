#pragma once
#ifndef _MONITOR_HH_
#define _MONITOR_HH_

#include "client.hh"

/**
 * This is just for fun and I do not want to implement it with Cryptohandlers :)
 */
class Monitor
{
public:
    Monitor() = delete;

    Monitor(const std::string &address,
            const std::string &user_name,
            const std::string &password,
            const std::string &port)
        : client(new Client(address, user_name, password, port))
    {
    }

    bool start(void) const;

protected:
    bool handleInput(const char &input) const;

    bool beforeQuery(const std::string &dn_name = "test") const { return client->defaultDatabase("test"); }

    std::string getWhere(void) const;

private:
    const Client *const client;
};

#endif