#include <fstream>
#include <chrono>

#include "client.hh"
#include "helper.hh"
#include "cryptohandlers.hh"
#include "rapidcsv.hh"

ArgType
deductArgtypeFromString(const std::string &str)
{
    assert(!str.empty());
    return str[0] == '\'' && str.back() == '\'' ? ArgType::VARCHAR : ArgType::REAL;
}

Argument::Argument(const std::string &value, bool raw)
    : arg_name(split(value, " = ")[0]),
      arg_type(deductArgtypeFromString(split(value, " = ")[1])),
      value(remove_quote(split(value, " = ")[1], deductArgtypeFromString(split(value, " = ")[1])))
{
}

std::string
Argument::toFieldValuePair()
    const
{
    if (ArgType::VARCHAR == arg_type)
    {
        std::string ans = arg_name;
        ans.append(" = \'");
        ans.append(value + "\'");
        return ans;
    }
    else
    {
        std::string ans = arg_name + " = ";
        ans.append(value);
        return ans;
    }
}

std::string
Argument::toValue()
    const
{
    switch (arg_type)
    {
    case ArgType::INT:
    case ArgType::REAL:
        return value;
    case ArgType::VARCHAR:
    {
        std::string ret = value;
        ret = "\'" + ret;
        ret = ret + "\'";
        return ret;
    }
    default:
        throw std::runtime_error("This is not supported.\n");
    }
}

bool LOG::writeToLog(const std::string &what)
{
    const char *const content = what.c_str();
    std::vector<char> buffer(content, content + strlen(content));

    std::fstream myfile(file_path, std::ios::out | std::ios::binary | std::ios::app);
    myfile.write((char *)&buffer[0], buffer.size());
    myfile.close();

    return true;
}

void Client::writeLog(const std::string &what)
    const
{
    assert(log.get()->writeToLog(what));
}

bool Client::udfInit(const std::string &source_file)
    const
{
    /* Not applicable. Only applicable in shell mode.
    const std::string sql= "source " + source_file;

    return execute(sql, false);
    */
}

bool Client::defaultDatabase(const std::string &db_name)
    const
{
    sql::Statement *statement = this->connection.get()->createStatement();

    try
    {
        return statement->execute("USE " + db_name);
    }
    catch (sql::SQLException e)
    {
        std::cout << e.what() << std::endl;
    }
}

bool Client::directiveHandler(const std::string &sql)
    const
{
    return execute(sql, false);
}

bool Client::execute(const std::string &sql,
                     bool with_transaction)
    const
{
    sql::Connection *const con = this->connection.get();
    sql::Statement *const statement = con->createStatement();

    try
    {
        if (with_transaction)
        {
            std::cout << "LOG: starting trasaction..." << std::endl;
            writeLog("LOG: starting trasaction...");
            statement->execute("START TRANSACTION");
            statement->execute(sql);
            statement->execute("COMMIT");
            std::cout << "LOG: finished!" << std::endl;
            writeLog("LOG: finished!");
        }
        else
        {
            statement->execute(sql);
        }

        return true;
    }
    catch (sql::SQLException e)
    {
        statement->execute("Rollback");
        std::cout << e.what() << std::endl;

        if (e.getSQLState() == "10000")
        {
            std::cout << "LOG: Updated table video\n";
        }
        else
        {
            std::cout << "LOG: this transaction / SQL probably failed. Check again if there is any constraint not met.\n";
        }
    }
}

std::unique_ptr<sql::ResultSet>
Client::executeQuery(const std::string &sql, bool with_view)
    const
{
    sql::Connection *const con = this->connection.get();
    sql::Statement *const statement = con->createStatement();

    try
    {
        if (with_view)
        {
            statement->execute(sql);
            return std::unique_ptr<sql::ResultSet>(statement->executeQuery("SELECT * FROM my_view"));
        }
        else
        {
            return std::unique_ptr<sql::ResultSet>(statement->executeQuery(sql));
        }
    }
    catch (sql::SQLException e)
    {
        std::cout << e.what() << std::endl;
    }
}

std::unique_ptr<sql::ResultSet>
Client::selectHandler(const std::vector<std::string> &tables,
                      const std::vector<std::string> &columns,
                      const std::string &where,
                      bool with_view)
    const
{
    std::stringstream sql;

    if (true == with_view)
    {
        sql << "CREATE OR REPLACE VIEW my_view AS ";
    }
    sql << "SELECT " << string_join(columns, ", ").str() << " FROM "
        << string_join(tables, " join ").str() << " WHERE "
        << (0 == where.compare("") ? "1" : where);

    // TODO: add a log.
    std::cout << "\nSELECT CLAUSE: " << sql.str() << std::endl;

    return executeQuery(sql.str(), with_view);
}

bool Client::deleteHandler(const std::string &table,
                           const std::string &where,
                           bool with_transaction)
    const
{
    std::cout << "Warning: delete from table is extremely dangerous, are you sure you want to continue? (Y/N)";
    std::string input;
    std::cin >> input;
    if (0 != input.compare("y") && 0 != input.compare("Y"))
    {
        // user does not want to delete from table.
        return true;
    }

    sql::Connection *const con = this->connection.get();
    sql::Statement *const statement = con->createStatement();

    std::string sql = "DELETE FROM ";
    sql.append(table);
    sql.append(" WHERE " + (0 == where.compare("") ? "1" : where));
    std::cout << "DELETE CLAUSE: " << sql << std::endl;

    // Begin transaction.
    return execute(sql, with_transaction);
}

bool Client::insertHandler(const std::string &table,
                           const std::vector<std::unique_ptr<Argument>> &values,
                           bool with_transaction)
    const
{
    std::string sql = "INSERT INTO ";
    sql.append(table);
    sql.append(" VALUES(");

    std::vector<std::string> arr;
    auto lambda = [](const std::unique_ptr<Argument> &arg) {
        return arg.get()->toValue();
    };
    std::transform(values.cbegin(), values.cend(), std::back_inserter(arr), lambda);

    sql.append(string_join(arr, ", ").str());
    sql.append(")");

    std::cout << "\nINSERT STATEMENT: " << sql << std::endl;

    return execute(sql, with_transaction);
}

bool Client::updateHandler(const std::string &table,
                           const std::vector<std::unique_ptr<Argument>> &field_value_pairs,
                           const std::string &where,
                           bool with_transaction,
                           bool with_procedure,
                           const unsigned int &proc_id,
                           const std::string &values)
    const
{

    if (with_procedure)
    {
        std::stringstream sql;
        sql << "CALL update_on_video(" << values << ")";

        return execute(sql.str(), false);
    }

    std::vector<std::string> vec;
    std::transform(field_value_pairs.begin(),
                   field_value_pairs.end(),
                   std::back_inserter(vec),
                   [](const std::unique_ptr<Argument> &item) {
                       return item.get()->toFieldValuePair();
                   });

    std::stringstream sql;
    sql << "UPDATE " << table;
    sql << " SET " << string_join(vec, ", ").str();
    sql << " WHERE " << (0 == where.compare("") ? "1" : where);
    std::cout << "UPDATE CLAUSE: " << sql.str() << std::endl;

    return execute(sql.str(), with_transaction);
}

/**
 * @see Firstly we need to backup any csv file if this file exists, or there would be errors
 */
bool Client::mysqlDumpTable(const std::string &table, const std::string &secure_path_dir)
    const
{
    std::string full_path = secure_path_dir + table;
    std::string command = "mv ";
    full_path.append(".csv");
    command.append(full_path + " ");
    command.append(full_path + ".bak || true");

    system(command.c_str());

    try
    {
        std::unique_ptr<sql::ResultSet> res =
            std::unique_ptr<sql::ResultSet>(executeQuery("SHOW VARIABLES LIKE \'secure_file_priv\'", false));

        assert(res.get()->next());
        std::string server_side_secure_path = res.get()->getString(2);

        TEST_VARIABLES_DISCREPANCY(secure_path_dir.c_str(), server_side_secure_path.c_str());

        std::stringstream sql;
        sql << "TABLE " << table << " INTO OUTFILE \'" << full_path
            << "\' FIELDS TERMINATED BY \',\' ENCLOSED BY \'\"\' ESCAPED BY \'\' LINES TERMINATED BY \'\\n\'";

        std::cout << "LOG: dumping table " << table << " to csv file, using command\n\t" << sql.str() << std::endl;

        return execute(sql.str(), false);
    }
    catch (sql::SQLException e)
    {
        std::cout << e.what() << std::endl;
    }
}

bool Client::testSelectFromPlain(const std::string &file_path,
                                 const std::string column_name,
                                 const unsigned long &limited)
    const
{
        std::unique_ptr<rapidcsv::Document>
        doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));

    std::vector<std::string>
        plaintexts = doc.get()->GetColumn<std::string>(column_name);

    assert(limited <= plaintexts.size());

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        std::string plain = "\'" + plaintexts[i];
        plain.append("\'");
        selectHandler({"plain"}, {"plaintext"}, "plaintext = " + plain, false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    std::stringstream log;
    log << "SELECT with Plaintext Efficiency (total " << ms_double.count() << " ms, average "
        << avg << " ms) : " << limited << " plaintexts.\n";

    writeLog(log.str());

    return true;
}

bool Client::testSelectEfficieny(const std::string &file_path,
                                 const std::string column_name,
                                 const unsigned long &limited,
                                 const std::pair<double, double> &range,
                                 const unsigned int &k,
                                 const std::map<unsigned int,
                                                std::vector<std::unique_ptr<Salt>>> &salt_table)
    const
{
    std::unique_ptr<rapidcsv::Document>
        doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));

    std::vector<std::string>
        plaintexts = doc.get()->GetColumn<std::string>(column_name);

    assert(limited <= plaintexts.size());

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        const std::string plaintext = plaintexts[i];
        const unsigned int index = getInterval(std::stod(plaintext), k, range);

        std::vector<std::string> possible_cipher;
        const std::vector<std::unique_ptr<Salt>> &salts = salt_table.at(index);
        std::transform(salts.cbegin(), salts.cend(),
                       std::back_inserter(possible_cipher), [plaintext, this](const std::unique_ptr<Salt> &salt_item) {
                           std::string cipher = "ciphertext = \'" +
                                                encrypt_DET(plaintext, salt_item.get()->getSaltName(), this->password);
                           cipher.append("\'");
                           return cipher;
                       });

        const std::string where = string_join(possible_cipher, " OR ").str();
        selectHandler({"det_test"}, {"ciphertext"}, where, false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    std::stringstream log;
    log << "SELECT Efficiency (total " << ms_double.count() << " ms, average "
        << avg << " ms) : " << limited << " plaintexts.\n";

    writeLog(log.str());

    return testSelectFromPlain(file_path, column_name, limited);
}

bool Client::encryptByDET(const std::string &file_path,
                          const std::string &column_name,
                          const std::vector<double> &parameters,
                          const unsigned long &limited,
                          bool test_select)
    const
{
    execute(DROP_DET_TABLE, false);
    execute(CREATE_DET_TABLE, false);

    std::unique_ptr<rapidcsv::Document>
        doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));

    std::vector<std::string> plaintexts = doc.get()->GetColumn<std::string>(column_name); // Get all the plaintexts from csv file.

    assert(parameters.size() == 6 && limited <= plaintexts.size());

    // Fetch all the essential paramaters.
    const double alpha = parameters[0];
    const double p = parameters[1];
    const std::pair<double, double> range = std::make_pair(parameters[2], parameters[3]);
    const unsigned int k = (unsigned int)parameters[4];

    unsigned int total_salt_used = 0, ptext_size = 0;

    std::map<unsigned int, std::vector<std::unique_ptr<Salt>>> salt_table;

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        if (0 != plaintexts[i].compare(""))
        {
            const std::string salt =
                getSalt(std::stod(plaintexts[i]), alpha, p, k, range, salt_table, total_salt_used, ptext_size);

            std::string ciphertext = encrypt_DET(plaintexts[i], salt, password);

            // std::cout << ciphertext << std::endl;
            std::stringstream sql;
            sql << "INSERT INTO det_test VALUES(\'" << ciphertext << "\')";
            execute(sql.str(), false);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    std::stringstream log;
    log << "Encrypted DET (" << ms_double.count() << " ms, on average " << avg << " ms) : "
        << limited << " plaintexts, "
        << "with parameters alpha = " << alpha << ", interval_num = " << k
        << ", p = " << p << ", range = [" << range.first << ", " << range.second << "].\n";
    writeLog(log.str());

    if (test_select)
    {
        testSelectEfficieny(file_path, column_name, (unsigned)parameters[5], range, k, salt_table);
    }

    return true;
}

bool Client::encryptByOPE(const std::string &file_path,
                          const std::string &column_name,
                          const unsigned long &limited)
    const
{
    // std::map<double, unsigned int> local_table = getLocalTable(file_path, column_name);
    execute(DROP_OPE_TABLE, false);
    execute(CREATE_OPE_TABLE, false);
    std::map<double, unsigned int> local_table;

    std::unique_ptr<rapidcsv::Document>
        doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));
    // Get the plaintexts.
    std::vector<std::string> plaintexts = doc.get()->GetColumn<std::string>(column_name);

    assert(limited < plaintexts.size());

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        const unsigned int pos = getPosition(std::stod(plaintexts[i]), local_table);
        const std::string salt = std::to_string(local_table[std::stod(plaintexts[i])]);
        const std::string ciphertext = encrypt_DET(plaintexts[i], salt, password);

        const std::unique_ptr<sql::PreparedStatement>
            prep(connection->prepareStatement("CALL pro_insert(?, ?, ?)"));
        prep.get()->setString(1, ciphertext);
        prep.get()->setInt(2, pos);
        prep.get()->setString(3, "ope_test");
        prep.get()->execute();

        //std::cout << "plain: " << plaintexts[i] << ", " << ciphertext << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    uint32_t storage = sizeof(local_table) +
                       local_table.size() * (sizeof(decltype(local_table)::key_type) +
                                             sizeof(decltype(local_table)::mapped_type));

    std::stringstream log;
    log << "Encrypted OPE (" << ms_double.count() << " ms, on average " << avg << " ms) : "
        << limited << " plaintexts, with client storage " << storage
        << " bytes\n";
    writeLog(log.str());

    return true;
}

bool Client::noFrequencyHiding(const std::string &file_path,
                               const std::string &column_name,
                               const unsigned long &limited)
    const
{
    execute(DROP_NO_FH_TABLE, false);
    execute(CREATE_NO_FH_TABLE, false);

    std::unique_ptr<rapidcsv::Document>
        doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));
    // Get the plaintexts.
    std::vector<std::string> plaintexts = doc.get()->GetColumn<std::string>(column_name);

    assert(limited < plaintexts.size());

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        const std::string ciphertext = encrypt_DET(plaintexts[i], "", password);

        const std::unique_ptr<sql::PreparedStatement>
            prep(connection->prepareStatement("INSERT INTO no_fh values(?)"));
        prep.get()->setString(1, ciphertext);
        prep.get()->execute();

        //std::cout << "plain: " << plaintexts[i] << ", " << ciphertext << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    std::stringstream log;
    log << "Conventional (" << ms_double.count() << " ms, on average " << avg << " ms) : "
        << limited << " plaintexts.\n";
    writeLog(log.str());

    return true;
}

bool Client::noEncyption(const std::string &file_path,
                         const std::string &column_name,
                         const unsigned long &limited)
    const
{
    execute(DROP_PLAIN_TABLE, false);
    execute(CREATE_PLAIN_TABLE, false);

    std::unique_ptr<rapidcsv::Document> doc(new rapidcsv::Document(file_path, rapidcsv::LabelParams(0, 0)));
    // Get the plaintexts.
    std::vector<std::string> plaintexts = doc.get()->GetColumn<std::string>(column_name);

    assert(limited < plaintexts.size());

    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < limited; i++)
    {
        const std::unique_ptr<sql::PreparedStatement>
            prep(connection->prepareStatement("INSERT INTO plain values(?)"));
        prep.get()->setString(1, plaintexts[i]);
        prep.get()->execute();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = end - begin;
    double avg = ms_double.count() / plaintexts.size();

    std::stringstream log;
    log << "Plaintext (" << ms_double.count() << " ms, on average " << avg << " ms) : "
        << limited << " plaintexts.\n";
    writeLog(log.str());

    return true;
}

bool Client::addTrigger(const std::string &name,
                        const std::string &table,
                        const std::string &what,
                        const Client::SQLHanlderType &type,
                        bool before)
    const
{
    throw std::runtime_error("Unimplemented");
}

bool Client::addProcedure(const std::string &name,
                          const std::vector<std::pair<std::string, ArgType>> &args,
                          const std::string &procedure_body)
    const
{
    throw std::runtime_error("Unimplemented");
}