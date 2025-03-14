
#pragma once

// #include "Server.hpp"
#include <iostream>
#include <vector>

class Channel
{
    private:
        std::string                 _name;
        std::vector<std::string>    _client;


    public:
        void                        set_name(std::string new_name);
        std::string                 get_name();
        void                        add_client_to_list(std::string client_name);
        void                        remove_client_from_list(std::string client_name);
        std::vector<std::string>    get_client_list();
};