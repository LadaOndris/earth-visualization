
#include "WorldCitiesReader.h"


std::vector<Text> WorldCitiesReader::readData() {
    std::vector<Text> data;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return data;
    }

    std::string line;
    // Ignore the header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        Text text = parseLine(line);
        data.push_back(text);
    }

    file.close();
    return data;
}


std::string removeQuotes(const std::string& str) {
    std::string result = str;
    if (!result.empty() && result.front() == '"') {
        result.erase(0, 1); // remove opening quote
    }
    if (!result.empty() && result.back() == '"') {
        result.pop_back(); // remove closing quote
    }
    return result;
}

Text WorldCitiesReader::parseLine(const std::string &line) {
    std::istringstream ss(line);
    std::string token;
    Text text;

    // Read each field from the CSV line
    std::getline(ss, token, ','); // city
    // text.content = token;

    std::getline(ss, token, ','); // city_ascii
    text.content = removeQuotes(token);

    try {
        std::getline(ss, token, ','); // lat
        float latitude = std::stof(removeQuotes(token));

        std::getline(ss, token, ','); // lng
        float longitude = std::stof(removeQuotes(token));

        text.geodeticPosition = glm::vec2(latitude, longitude);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error: Invalid argument: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unexpected error occurred." << std::endl;
    }
    return text;
}
