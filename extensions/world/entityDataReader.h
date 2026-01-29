
#pragma once

#include "json.hpp"


class EntityDataReader
{
    
    private:
        std::string path = "../../EntityData/";

    public:
        EntityDataReader();
        ~EntityDataReader();

        void get_entity_array(std::string name); // from json file
};
