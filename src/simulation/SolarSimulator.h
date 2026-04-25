
#ifndef EARTH_VISUALIZATION_SOLARSIMULATOR_H
#define EARTH_VISUALIZATION_SOLARSIMULATOR_H


#include <cmath>
#include <ctime>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "LightSource.h"

const double DAYS_IN_YEAR = 365.25;

class SolarSimulator : public LightSource {
public:
    /**
     * It takes 23 hours, 56 minutes, and 4.1 seconds (86164.1 secs) for the Earth to complete one full rotation
     * with respect to the other stars. When combined the with the orbit rotation, this becomes 24 hours.
     */
    explicit SolarSimulator(float sunDistance) :
            _inclinationAngle(glm::radians(23.44)),
            _earthRotationSpeed(360.0 / 86164.1), _sunSpeed(360.0 / DAYS_IN_YEAR) {
        _basePosition = glm::vec3(sunDistance, 0.f, 0.f);

        _simulationTime.tm_year = 2023 - 1900; // Years since 1900
        _simulationTime.tm_mon = 0;             // January (0-based)
        _simulationTime.tm_mday = 1;            // 1st day of the month

    }

    [[nodiscard]] glm::vec3 getLightPosition() const override {
        return _sunPosition;
    }

    [[nodiscard]] glm::mat4 getTransformationMatrix() const override {
        return _transformationMatrix;
    }

    [[nodiscard]] std::tm getCurrentSimulationTime() const {
        return _simulationTime;
    }

    // Function to calculate the Sun's position
    void updateSunPosition(float deltaTime, float simulationSpeed) {
        assert(deltaTime >= 0);
        updateSimulationTime(deltaTime, simulationSpeed);
        double earthRotationAngle = calculateEarthRotationAngle(_simulationTime);

        glm::mat4 rotationMatrixForFirstOfJanuary = calcRotationMatrixToPlaceAtTheStart();
        glm::mat4 rotationMatrix = calcRotationMatrix(earthRotationAngle);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), _basePosition);

        _transformationMatrix = rotationMatrix * rotationMatrixForFirstOfJanuary * translationMatrix;
        _sunPosition = rotationMatrix * rotationMatrixForFirstOfJanuary * glm::vec4(_basePosition, 1.f);
    }


private:
    const double _inclinationAngle;
    const double _earthRotationSpeed;
    const double _sunSpeed;
    glm::vec3 _basePosition;
    glm::vec3 _sunPosition = glm::vec3(0.f);
    glm::mat4 _transformationMatrix = glm::mat4(1.f);
    std::tm _simulationTime = {};
    double _simulationTimeMillis = 0.0; // The std::tm structure doesn't support milliseconds. Store it separately.

    [[nodiscard]] double calculateEarthRotationAngle(const std::tm &datetime) const {
        double seconds = datetime.tm_hour * 3600 + datetime.tm_min * 60 + datetime.tm_sec;
        double rotationDegs = seconds * _earthRotationSpeed + 250;
        double rotationYears = std::floor(rotationDegs / 360);
        double rotationRads = glm::radians(rotationDegs - rotationYears * 360);
        return rotationRads;
    }

    [[nodiscard]] double calculateEarthOrbitAngle(const std::tm &datetime) const {
        double daysInYear = (datetime.tm_yday + 1) + (datetime.tm_year - 70) * DAYS_IN_YEAR;
        double rotationDegs = daysInYear * _sunSpeed;
        double rotationYears = std::floor(rotationDegs / 360);
        double rotationRads = glm::radians(rotationDegs - rotationYears * 360);
        return rotationRads;
    }

    [[nodiscard]] glm::mat4 calcRotationMatrix(double earthRotationAngle) const {
        double currentInclinationAngle = calcCurrentInlination();

        // Apply an additional rotation to account for the Earth's axial tilt (inclination).
        glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(currentInclinationAngle),
                                                  glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 earthRotationMatrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(earthRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

        return earthRotationMatrix * inclinationMatrix;
    }

    [[nodiscard]] double calcCurrentInlination() const {
        // Calculate the inclination angle (axial tilt) as a function of the time of the year.
        // This is a simplified model and doesn't account for the Earth's elliptical orbit.
        const double elapsedDays = getElapsedDaysInCurrentYear();
        const double summmerSolsticeDay = 171.5;
        // Calculate the difference in days from the vernal equinox to the current date and time.
        double daysFromVernalEquinox = elapsedDays - summmerSolsticeDay;
        double currentInclination = _inclinationAngle * cos(glm::radians(daysFromVernalEquinox * 360.0 / DAYS_IN_YEAR));
        return currentInclination;
    }

    [[nodiscard]] double getElapsedDaysInCurrentYear() const {
        // Obtain the day of the year from the provided date and time
        int dayOfYear = _simulationTime.tm_yday;

        // Calculate the time in hours (fractional part of the day)
        auto hours = static_cast<double>(_simulationTime.tm_hour);
        auto minutes = static_cast<double>(_simulationTime.tm_min);
        auto seconds = static_cast<double>(_simulationTime.tm_sec);

        double fractionalDay = hours + (minutes / 60.0) + (seconds / 3600.0);

        return dayOfYear + fractionalDay / 24.0;
    }

    [[nodiscard]] glm::mat4 calcRotationMatrixToPlaceAtTheStart() {
        // Constants for the approximation
        const double daysFromVernalEquinoxToJanuary1 = 286.0;
        const double earthOrbitDegrees = 360.0; // Full orbit in degrees

        // Calculate the Earth's angular position in its orbit on the 1st of January 2023
        double earthAngularPosition = (earthOrbitDegrees / DAYS_IN_YEAR) * daysFromVernalEquinoxToJanuary1;

        // Create a rotation matrix based on the Earth's angular position
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(earthAngularPosition)),
                                               glm::vec3(0.0f, 1.0f, 0.0f));

        return rotationMatrix;
    }

    void updateSimulationTime(double simPassedTimeSecs, double simulationSpeed) {
        // Define the time scale
        double secsInRealDay = 24 * 60 * 60;
        double secsInSimDay = secsInRealDay / simulationSpeed;

        double simulationSecs = secsInRealDay * simPassedTimeSecs / secsInSimDay;
        double simulatiomMs = simulationSecs * 1000;

        // Add seconds and milliseconds to the starting time
        _simulationTimeMillis += simulatiomMs;

        if (_simulationTimeMillis >= 1000) {
            int secondsToAdd = static_cast<int>(_simulationTimeMillis / 1000);
            _simulationTime.tm_sec += secondsToAdd;
            _simulationTimeMillis -= static_cast<double>(secondsToAdd) * 1000;
        }

        // Normalize the time struct, taking care of overflow
        mktime(&_simulationTime);
    }

};

#endif //EARTH_VISUALIZATION_SOLARSIMULATOR_H
