
#include "WorldCitiesReader.h"
#include "fast-cpp-csv-parser/csv.h"


std::vector<City> WorldCitiesReader::readData() {
    std::vector<City> data;

    io::CSVReader<4, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>,
            io::throw_on_overflow, io::no_comment> in(filename);

    in.read_header(io::ignore_extra_column, "city_ascii","lat","lng","population");
    std::string name;
    float latitude;
    float longitude;
    float population;
    while (in.read_row(name, latitude, longitude, population)) {
        City city = {.name = name, .latitude=latitude, .longitude=longitude, .population=(int)population};
        data.push_back(city);
    }

    return data;
}
