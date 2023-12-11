
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

struct Text {
    std::string content;
    glm::vec2 geodeticPosition; // Latitude, longitude
};

class WorldCitiesReader {
public:
    explicit WorldCitiesReader(std::string filename) : filename(std::move(filename)) {}

    std::vector<Text> readData();

private:
    Text parseLine(const std::string &line);

    std::string filename;
};


#endif //EARTH_VISUALIZATION_WORLDCITIESREADER_H
