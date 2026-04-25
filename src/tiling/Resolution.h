//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_RESOLUTION_H
#define EARTH_VISUALIZATION_RESOLUTION_H


class Resolution {
private:
    int _width;
    int _height;
public:
    explicit Resolution(int width, int height) : _width(width), _height(height) {
    }

    int getWidth() const {
        return _width;
    }

    int getHeight() const {
        return _height;
    }
};


#endif //EARTH_VISUALIZATION_RESOLUTION_H
