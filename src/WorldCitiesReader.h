
#ifndef EARTH_VISUALIZATION_WORLDCITIESREADER_H
#define EARTH_VISUALIZATION_WORLDCITIESREADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>
#include <iomanip>

#include <glm/glm.hpp>

struct City {
    std::string name;
    float latitude;
    float longitude;
    int population;
};

class WorldCitiesReader {
public:
    explicit WorldCitiesReader(std::string filename) : filename(std::move(filename)) {}

    std::vector<City> readData();

private:
    std::string filename;
};


#endif //EARTH_VISUALIZATION_WORLDCITIESREADER_H
