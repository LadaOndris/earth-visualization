#pragma once

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

    friend bool operator>(const Resolution &lhs, const Resolution &rhs) {
        return (lhs._width * lhs._height) > (rhs._width * rhs._height);
    }
};
