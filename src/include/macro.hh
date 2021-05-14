#ifndef _MACRO_HH_
#define _MACRO_HH_

#include <string>
#include <map>
#include <random>

#define DEFAULT_MEMORY_SIZE 65535

#define TEST_VARIABLES_DISCREPANCY(var1, var2)           \
    {                                                    \
        assert(0 == strcmp(var1, var2) || var1 == var2); \
    }

static const std::string MYSQL_DUMP_DIR = "/Users/chenhaobin/Documents/mysql-dump/";

static const std::string CREATE_DET_TABLE = "CREATE TABLE IF NOT EXISTS det_test(ciphertext VARCHAR(256))";
static const std::string DROP_DET_TABLE = "DROP TABLE IF EXISTS det_test";

static const std::string CREATE_OPE_TABLE = "CREATE TABLE IF NOT EXISTS ope_test(ciphertext VARCHAR(256), encoding DOUBLE)";
static const std::string DROP_OPE_TABLE = "DROP TABLE IF EXISTS ope_test";

static const std::string CREATE_NO_FH_TABLE = "CREATE TABLE IF NOT EXISTS no_fh(ciphertext VARCHAR(256))";
static const std::string DROP_NO_FH_TABLE = "DROP TABLE IF EXISTS no_fh";

static const std::string CREATE_PLAIN_TABLE = "CREATE TABLE IF NOT EXISTS plain(plaintext VARCHAR(256))";
static const std::string DROP_PLAIN_TABLE = "DROP TABLE IF EXISTS plain";

static const std::string possible_characters = "0123456789";

static const std::map<unsigned int, std::string> proc_map;

static const std::string
getRandomString(const unsigned int &length)
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> dist(0, possible_characters.size() - 1);
    std::string ret = "";
    for (unsigned int i = 0; i < length; i++)
    {
        unsigned int random_index = dist(engine); //get index between 0 and possible_characters.size()-1

        while (i == 0 && '0' == possible_characters[random_index])
        {
            random_index = dist(engine);
        }
        ret += possible_characters[random_index];
    }

    return ret;
}

#endif