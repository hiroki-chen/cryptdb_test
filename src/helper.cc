#include <algorithm>
#include <string>
#include <random>
#include <iomanip>
#include <fstream>
#include <map>

#include "helper.hh"
#include "cryptohandlers.hh"
#include "rapidcsv.hh"

std::map<double, unsigned int>
getLocalTable(const std::string &path, const std::string &column_name)
{
    // All items will be sorted by default.
    std::map<double, unsigned int> local_table;
    rapidcsv::Document doc(path);

    std::vector<std::string> row = doc.GetColumn<std::string>(column_name);

    std::for_each(row.begin(), row.end(), [&](const std::string &cell) {
        if (0 != cell.compare(""))
        {
            local_table[std::stod(cell)] += 1;
        }
    });

    return local_table;
}

unsigned int
getPosition(const double &val, std::map<double, unsigned int> &local_table)
{
    //assert(local_table.count(val));
    auto lambda = [val](const std::pair<double, unsigned int> &item) {
        return item.first == val;
    };

    auto accu = [](const unsigned int &lhs, const std::pair<double, unsigned int> &rhs) {
        return lhs + rhs.second;
    };

    std::map<double, unsigned int>::const_iterator iter =
        std::find_if(local_table.cbegin(), local_table.cend(), lambda);

    local_table[val] += 1;

    if (local_table.end() == iter)
    {
        iter = std::find_if(local_table.begin(), local_table.end(), lambda);
    }

    unsigned int distance = std::distance(local_table.cbegin(), iter);

    const unsigned int total = std::accumulate(local_table.begin(), std::next(local_table.begin(), distance), 0, accu);
    //std::cout << "pos before this ciphertext: " << total << std::endl;
    /**
     * Generate a random position.
     *
     * Be aware of the range.
     */
    return getRandomValue(total + 1, total + iter->second) - 1;
}

unsigned int
getInterval(const double &value,
            const unsigned int &interval_num,
            const std::pair<double, double> &range)
{
    double begin = range.first, end = range.second;
    double length = std::ceil((end - begin) * 1.0 / interval_num);

    unsigned int interval_index = std::floor((value - begin) * 1.0 / length);
    return interval_index;
}

int chooseSalt(std::vector<std::unique_ptr<Salt>> &salts,
               const double &alpha,
               const unsigned int &total_salt_used,
               const unsigned int &ptext_size)
{
    if (salts.empty())
    {
        return -1;
    }

    std::random_device rd;
    std::mt19937 engine(rd());
    std::shuffle(salts.begin(), salts.end(), engine);

    for (unsigned int i = 0; i < salts.size(); i++)
    {
        if ((double)(salts[i].get()->getCount() * 1.0 / salts.size()) <=
            (double)(alpha * (ptext_size + 1) /*Because new item added*/ / total_salt_used))
        {
            return int(i);
        }
    }

    return -1; // Not fount.
}

std::string
getSalt(const double &value,
        const double &alpha, const double &p, const unsigned int &k,
        const std::pair<double, double> &range,
        std::map<unsigned int, std::vector<std::unique_ptr<Salt>>> &salt_table,
        unsigned int &total_salt_used,
        unsigned int &ptext_size)
{
    const unsigned int interval = getInterval(value, k, range); // This is the interval's index, not itself.

    std::vector<std::unique_ptr<Salt>> &this_salt_table = salt_table[interval];

    const int salt = chooseSalt(this_salt_table, alpha, total_salt_used, ptext_size);

    if (true == tossACoin(p) || -1 == salt)
    {
        this_salt_table.push_back(std::move(std::unique_ptr<Salt>(new Salt(DEFAULT_SALT_LENGTH))));

        total_salt_used += 1;
        ptext_size += 1;

        return this_salt_table.back().get()->getSaltName();
    }
    else
    {
        this_salt_table[salt].get()->incrementCount();
        ptext_size += 1;

        return this_salt_table[salt].get()->getSaltName();
    }
}

std::string
remove_quote(const std::string &str, const ArgType &arg_type)
{
    assert(str.size());
    std::string ans = str.substr(1, str.size() - 2);
    return ArgType::VARCHAR == arg_type ? ans : str;
}

std::string
string_to_from(const std::vector<std::string> &columns, const std::string &delimiter)
{
    // If the size of columns is 0, then it means select all.
    if (0 == columns.size())
    {
        return "*";
    }
    else
    {
        ;
        return string_join(columns, delimiter).str();
    }
}

std::string
type_to_string(const ArgType &type)
{
    switch (type)
    {
    case ArgType::REAL:
        return "REAL";
    case ArgType::INT:
        return "INTEGER";
    case ArgType::VARCHAR:
        return "VARCHAR(128)";
    default:
        throw std::runtime_error("ERROR: the current type is not supported.\n");
    }
}

std::string
get_time()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::ostringstream
string_join(const std::vector<std::string> &array, const std::string &delimiter)
{
    std::ostringstream out;
    std::copy(array.begin(), array.end(), infix_ostream_iterator<std::string>(out, delimiter.c_str()));
    return out;
}

/**
 * @param os the output.
 * @param res the result set to be printed.
 * 
 * @return std::ostream
 */
std::ostream &
operator<<(std::ostream &os, const std::unique_ptr<sql::ResultSet> &res)
{
    sql::ResultSet *const res_type = res.get();

    if (0 == res_type->rowsCount())
    {
        os << "\nEmpty Set." << std::endl;
        return os;
    }

    sql::ResultSetMetaData *const metadata = res_type->getMetaData();
    unsigned int column_size = metadata->getColumnCount();

    unsigned int cnt = 1;
    os << "\n";

    for (unsigned int i = 1; i <= column_size; i++)
    {
        os << "\t" << metadata->getColumnName(i) << "     ";
    }
    os << "\n";

    while (res_type->next() && cnt)
    {
        os << "row " << cnt++ << ":   ";
        std::vector<std::string> line;

        for (unsigned int i = 0; i < column_size; i++)
        {
            line.push_back(res_type->getString(i + 1));
        }
        os << (string_join(line, "      ")).str() << std::endl;
    }
    return os;
}

long long
getRandomValue(const long long &floor, const long long &ceil)
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_real_distribution<double> dist(floor, ceil);

    return (long long)std::ceil(dist(engine));
}

std::vector<std::string>
split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> list;

    std::string s = str;
    size_t pos = 0;
    std::string token;

    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);

    return list;
}