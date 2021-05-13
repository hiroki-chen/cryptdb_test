#pragma once
#ifndef _CLIENT_HH_

#include <memory>
#include <iostream>
#include <vector>
#include <type_traits>
#include <map>
#include <random>

#include "macro.hh"
#include "connect.hh"

enum ArgType
{
    REAL,
    VARCHAR,
    INT
};

typedef class Salt
{
public:
    Salt() {}

    Salt(const unsigned long &salt_length) : salt_name(getRandomString(salt_length)), count(1) {}

    Salt(const unsigned long &_count, const std::string &_salt_name) : salt_name(_salt_name), count(_count) {}

    unsigned long getCount() const { return count; }

    bool incrementCount() { return (count++); }

    bool decrementCount() { return (--count); }

    std::string getSaltName() const { return salt_name; }

private:
    const std::string salt_name;
    unsigned long count; // The number of ciphertexts encrypted by this salt.
} Salt;

typedef class Argument
{
public:
    Argument() = delete;

    Argument(const long long &value)
        : arg_name(""), arg_type(ArgType::INT), value(std::to_string(value)) {}

    Argument(const std::string &value_pair, bool raw);

    Argument(const std::string &value)
        : arg_name(""), arg_type(ArgType::VARCHAR), value(value) {}

    Argument(const ArgType &arg_type, const std::string &value)
        : arg_name(""), arg_type(arg_type), value(value) {}

    Argument(const std::string &arg_name, const ArgType &arg_type, const std::string &value)
        : arg_name(arg_name), arg_type(arg_type), value(value) {}

    /**
     * This interface is used when you want to update the table by
     *      SET <attribute> = <value>
     */
    std::string toFieldValuePair() const;

    std::string toValue() const;

private:
    const std::string arg_name;
    const std::string value;
    const ArgType arg_type;
} Argument;

typedef class LOG
{
public:
    LOG() = delete;
    LOG(const std::string &file_path)
        : file_path(file_path), buffersize(DEFAULT_MEMORY_SIZE) {}

    // TODO: BUFFER is to be implemented...
    LOG(const std::string &file_path, const uint32_t &buffersize)
        : file_path(file_path), buffersize(buffersize) {}

    bool
    writeToLog(const std::string &what);

protected:
    std::vector<uint64_t> GenerateData(const char *const what, const size_t &size);

private:
    const std::string file_path;
    const uint32_t buffersize;

} LOG;

typedef class Client
{
public:
    Client() = delete;
    Client(const std::string &address,
           const std::string &user_name,
           const std::string &password,
           const std::string &port,
           const std::string &log_path = "",
           bool sensitive = false)
        : address(address), user_name(user_name),
          password(password), port(port),
          connection(getConn(this->user_name, this->password, this->address, this->port)),
          sensitive(sensitive), log(std::unique_ptr<LOG>(new LOG(log_path)))
    {
        std::cout << "Connection established: with state " << connection.get()->isValid() << std::endl;
    }

    enum SQLHanlderType
    {
        INSERT,
        DELETE,
        UPDATE,
        SELECT,
        OTHER_TYPE
    };

    virtual void
    writeLog(const std::string &what) const;

    virtual bool
    udfInit(const std::string &source_file) const;

    virtual bool
    defaultDatabase(const std::string &db_name) const;

    virtual bool directiveHandler(const std::string &sql) const;

    virtual std::unique_ptr<sql::ResultSet>
    selectHandler(const std::vector<std::string> &tables,
                  const std::vector<std::string> &columns,
                  const std::string &where,
                  bool with_view = false) const;

    virtual bool
    deleteHandler(const std::string &table,
                  const std::string &where,
                  bool with_transaction = false) const;

    virtual bool
    updateHandler(const std::string &table,
                  const std::vector<std::unique_ptr<Argument>> &field_value_pairs,
                  const std::string &where,
                  bool with_transaction = false,
                  bool with_procedure = false,
                  const unsigned int &proc_id = 1,
                  const std::string &values = "") const;

    virtual bool
    insertHandler(const std::string &table,
                  const std::vector<std::unique_ptr<Argument>> &values,
                  bool with_transaction = false) const;

    virtual bool
    addTrigger(const std::string &name,
               const std::string &table,
               const std::string &what,
               const Client::SQLHanlderType &type,
               bool before) const;

    virtual bool
    mysqlDumpTable(const std::string &table,
                   const std::string &secure_path_dir = MYSQL_DUMP_DIR) const;

    /**
     * By default we do not need out argument for stored procedures.
     * 
     * TODO: support
     *      CREATE PROCEDURE <name> (IN <arg1> <type1>, OUT <arg2> <type2>) BEGIN <PROCEDURE BODY> END
     */
    virtual bool
    addProcedure(const std::string &name,
                 const std::vector<std::pair<std::string, ArgType>> &args,
                 const std::string &procedure_body) const;

    /**
     * This function provides interface for DET encryption scheme.
     * 
     * @param file_path the path of the file.
     * @param column_name the column needed to be encrypted.
     * @param parameters essential parameters for encryption.
     * @param limited the number of records needed to be encrypted (in case it is too large).
     * 
     * @note
     *        parameters[0] = alpha;
     *        parameters[1] = p (the probability of success).
     *        parameters[2] = range_begin;
     *        parameters[3] = range_end
     *        parameters[4] = interval_num (or k);
     *        
     *        Also, if you need to add more, just add some rules here.
     * ----------------------------------------------------------------
     * 
     * @return Whether the encryption succeeded or not.
     */
    virtual bool
    encryptByDET(const std::string &file_path,
                 const std::string &column_name,
                 const std::vector<double> &parameters,
                 const unsigned int &limited) const;

    virtual bool
    encryptByOPE(const std::string &file_path,
                 const std::string &column_name,
                 const unsigned int &limited) const;

    ~Client() { connection.get()->close(); }

protected:
    virtual bool
    execute(const std::string &sql, bool with_transaction) const;

    virtual std::unique_ptr<sql::ResultSet>
    executeQuery(const std::string &sql, bool with_view) const;

private:
    const std::string address;
    const std::string user_name;
    const std::string password;
    const std::string port;

    const std::unique_ptr<sql::Connection> connection;

    const bool sensitive;

    const std::unique_ptr<LOG> log;

} Client;

ArgType
deductArgtypeFromString(const std::string &str);

#endif