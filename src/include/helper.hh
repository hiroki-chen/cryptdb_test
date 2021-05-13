#pragma once
#ifndef _STRING_HELPER_HH_
#define _STRING_HELPER_HH_

#include "mysql/jdbc.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <random>
#include <vector>
#include <map>

#include "client.hh"

/**
 * This template function is for the package of multiple undetermined variables.
 * @see How to use parameter expasion.
 * 
 * Template 1 ends the recuresion;
 * 
 * Template 2 does everything.
 * 
 * @see template
 */
template <class T>
std::vector<std::unique_ptr<Argument>>
pack_args(T arg)
{
    std::vector<std::unique_ptr<Argument>> values;
    values.push_back(std::unique_ptr<Argument>(new Argument(arg)));
    return values;
}

template <class T, class... Ts>
std::vector<std::unique_ptr<Argument>>
pack_args(T head, Ts... args)
{
    std::vector<std::unique_ptr<Argument>> vec = std::move(pack_args(args...));
    std::vector<std::unique_ptr<Argument>> values;
    values.push_back(std::unique_ptr<Argument>(new Argument(head)));

    // Concatenate.
    values.insert(values.end(), std::make_move_iterator(vec.begin()), std::make_move_iterator(vec.end()));

    return values;
}

/**
 * This class is for string_join just as what is implemented in python. 
 * 
 * @see ''.join(list(....));
 */
template <class T,
          class charT = char,
          class traits = std::char_traits<charT>>
class infix_ostream_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
    std::basic_ostream<charT, traits> *os;
    charT const *delimiter;
    bool first_elem;

public:
    typedef charT char_type;
    typedef traits traits_type;
    typedef std::basic_ostream<charT, traits> ostream_type;
    infix_ostream_iterator(ostream_type &s)
        : os(&s), delimiter(0), first_elem(true)
    {
    }
    infix_ostream_iterator(ostream_type &s, charT const *d)
        : os(&s), delimiter(d), first_elem(true)
    {
    }
    infix_ostream_iterator<T, charT, traits> &operator=(T const &item)
    {
        // Here's the only real change from ostream_iterator:
        // Normally, the '*os << item;' would come before the 'if'.
        if (!first_elem && delimiter != 0)
        {
            *os << delimiter;
        }

        *os << item;
        first_elem = false;
        return *this;
    }
    infix_ostream_iterator<T, charT, traits> &operator*()
    {
        return *this;
    }
    infix_ostream_iterator<T, charT, traits> &operator++()
    {
        return *this;
    }
    infix_ostream_iterator<T, charT, traits> &operator++(int)
    {
        return *this;
    }
};

std::ostringstream
string_join(const std::vector<std::string> &array, const std::string &delimiter);

std::string
string_to_from(const std::vector<std::string> &colummns, const std::string &delimiter = ", ");

std::string
type_to_string(const ArgType &type);

std::string
remove_quote(const std::string &str, const ArgType &arg_type);

std::string
get_time();

std::vector<std::string>
split(const std::string &s, const std::string &delimiter);

long long
getRandomValue(const long long &floor = LONG_MIN, const long long &ceil = LONG_MAX);

/**
 * @note This function will extract the information stored in .csv file into a local_table for
 *       OPE encryption.
 * @note The domain must not be too large.
 * 
 * @deprecated ...
 */
std::map<double, unsigned int>
getLocalTable(const std::string &path, const std::string &column_name);

/**
 * @return A random position bounded by its value.
 */
unsigned int
getPosition(const double &val, std::map<double, unsigned int> &local_table);

std::string
getSalt(const double &plaintext,
        const double &alpha, const double &p, const unsigned int &k,
        const std::pair<double, double> &range,
        std::map<unsigned int, std::vector<std::unique_ptr<Salt>>> &salt_table,
        unsigned int &total_salt_used,
        unsigned int &ptext_size);

int chooseSalt(std::vector<std::unique_ptr<Salt>> &salts,
               const double &alpha,
               const unsigned int &total_salt_used,
               const unsigned int &ptext_size);
/**
 * @return The index of the interval, for convenience.
 */
unsigned int
getInterval(const double &value,
            const unsigned int &interval_num,
            const std::pair<double, double> &range);

static bool tossACoin(const double &p)
{
    std::bernoulli_distribution dist(p);
    std::random_device rd;
    std::mt19937 engine(rd());
    return dist(engine);
}

/*std::string
value_array_to_string(const std::vector<std::string>& values, const std::string &delimtier = ",");
*/
std::ostream &
operator<<(std::ostream &os, const std::unique_ptr<sql::ResultSet> &res);

#endif