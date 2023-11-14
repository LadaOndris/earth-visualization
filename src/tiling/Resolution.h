//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_RESOLUTION_H
#define EARTH_VISUALIZATION_RESOLUTION_H


class Resolution {
private:
    int width;
    int height;
public:
    explicit Resolution(int width, int height) : width(width), height(height) {
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }
};


#endif //EARTH_VISUALIZATION_RESOLUTION_H
