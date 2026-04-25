
#pragma once

#include <glm/glm.hpp>

#include <utility>
#include <vector>
#include <string>

struct City {
    std::string name;
    float latitude;
    float longitude;
    int population;
};

class WorldCitiesReader {
public:
    explicit WorldCitiesReader(std::string filename) : _filename(std::move(filename)) {}

    std::vector<City> readData();

private:
    std::string _filename;
};
