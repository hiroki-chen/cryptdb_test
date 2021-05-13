#include <iostream>

#include "monitor.hh"
#include "helper.hh"

bool Monitor::start() const
{
    beforeQuery();
    char input;
    while (true)
    {
        std::cout << "-------------Welcome to Video Management Platform-------------\n";
        std::cout << "[0] quit\n";
        std::cout << "[1] create video\n";
        std::cout << "[2] delete video\n";
        std::cout << "[3] update video\n";
        std::cout << "[4] select video\n\n";
        std::cout << "[5] create user\n"; // We do not delete a user.
        std::cout << "[6] select user\n\n";
        std::cout << "[7] like video\n";
        std::cout << "[8] unlike video\n";
        std::cout << "[9] directive sql\n";
        std::cout << "-----------------------DO WHATEVER YOU WANT--------------------\n";

        std::cin >> input;
        try
        {
            if (0 == handleInput(input) && '0' == input)
            {
                break;
            }
        }
        catch (std::runtime_error e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

std::string
Monitor::getWhere(void) const
{
    std::string where = "";
    std::cout << "Do you want to add \'WHERE\' clause? (Y / N)\n";
    char choice;
    std::cin >> choice;

    if ('Y' == choice || 'y' == choice)
    {
        std::cout << "Input your \'WHERE\' clause.\n";
        std::cin.ignore();
        getline(std::cin, where);
    }

    return where;
}

bool Monitor::handleInput(const char &input) const
{
    switch (input)
    {
    case '0':
        std::cout << "Goodbye.\n";
        return false;
    case '1':
    {
        std::cout << "Please input the id, user_id, video_name in order." << std::endl;

        int id, user_id;
        std::string video_name;
        std::cin >> id >> user_id >> video_name;

        std::vector<std::unique_ptr<Argument>> values = std::move(pack_args(id, user_id, video_name, get_time()));

        return client->insertHandler("video", values, true);
    }
    case '2':
    {
        return client->deleteHandler("user", getWhere(), true);
    }
    case '3':
    {
        std::cout << "call procedure? (Y / N)\n";
        char c;
        std::cin >> c;
        if ('Y' == c || 'y' == c) {
            std::cout << "Input the new id of the video and old id thereof.\n";
            int new_id, old_id;
            std::cin >> new_id >> old_id;
            std::stringstream ss;
            ss << new_id << ", " << old_id;
            
            return client->updateHandler("", {}, "", true, true, 1, ss.str());
        }

        std::cout << "Input your update parameter lists, and split them by comma. Format: <attriburte> = <value>,more...\n";

        std::string field_value_pairs;
        std::cin.ignore();
        getline(std::cin, field_value_pairs);

        std::vector<std::string> pair = split(field_value_pairs, ",");
        std::vector<std::unique_ptr<Argument>> fv_pairs;
        std::transform(pair.begin(), pair.end(), std::back_inserter(fv_pairs), [](const std::string &pair) {
            return std::unique_ptr<Argument>(new Argument(pair, true));
        });

        return client->updateHandler("video", fv_pairs, getWhere(), true);
    }
    case '4':
    {
        std::cout << "Please input the columns you want to select and seperate them by comma \',\'."
                  << " Input \'*\' to select all.\n";

        std::string input;
        std::cin >> input;

        std::vector<std::string> columns;

        if (0 == input.compare("*"))
        {
            columns.push_back("*");
        }
        else
        {
            columns = split(input, ",");
        }

        std::unique_ptr<sql::ResultSet> res = client->selectHandler({"video", "user ON user.id = video.created_by"}, columns, getWhere(), true);
        std::cout << res;
        return true;
    }
    case '5':
    {
        std::cout << "Please input the id, sex, fans and username in order." << std::endl;

        int id, sex, fans;
        std::string username;
        std::cin >> id >> sex >> fans >> username;

        std::vector<std::unique_ptr<Argument>> values = pack_args(id, sex, fans, username);

        return client->insertHandler("user", values, true);
    }
    case '6':
    {
        std::cout << "Please input the user_id here." << std::endl;

        unsigned int user_id;
        std::cin >> user_id;
        std::string where = "id = ";
        where.append(std::to_string(user_id));

        std::unique_ptr<sql::ResultSet> res = client->selectHandler({"user"}, {"*"}, where, true);
        std::cout << res;
        return true;
    }
    case '7':
    {
        std::cout << "Please tell me who you are by inputting your user_id.\n";
        long long user_id;
        std::cin >> user_id;

        std::cout << "Please input the video_id you want to like.\n";
        long long video_id;
        std::cin >> video_id;

        std::vector<std::unique_ptr<Argument>> values =
            std::move(pack_args(getRandomValue(0, 0xfffffff), video_id, user_id, get_time()));

        return client->insertHandler("video_like", values, true);
    }
    case '8':
    {
        std::cout << "Please tell me who you are by inputting your user_id.\n";
        long long user_id;
        std::cin >> user_id;

        std::cout << "Please input the video_id you want to unlike.\n";
        long long video_id;
        std::cin >> video_id;

        std::string where = "";
        where.append("user_id = " + std::to_string(user_id));
        where.append(" AND video_id = " + std::to_string(video_id));

        return client->deleteHandler("video_like", where, true);
    }
    case '9':{
        std::cout << "Input your SQL here.\n";
        std::string sql;
        std::cin.ignore();
        getline(std::cin, sql);

        return client->directiveHandler(sql);
    }
    default:
        throw std::runtime_error("Error: your input is wrong.\n");
    }
}