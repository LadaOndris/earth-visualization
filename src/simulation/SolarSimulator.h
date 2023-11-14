
#ifndef EARTH_VISUALIZATION_SOLARSIMULATOR_H
#define EARTH_VISUALIZATION_SOLARSIMULATOR_H


#include <cmath>
#include <ctime>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "LightSource.h"

const float DAYS_IN_YEAR = 365.25f;

class SolarSimulator : public LightSource {
public:
    /**
     * It takes 23 hours, 56 minutes, and 4.1 seconds (86164.1 secs) for the Earth to complete one full rotation
     * with respect to the other stars. When combined the with the orbit rotation, this becomes 24 hours.
     */
    explicit SolarSimulator(float sunDistance) :
            inclinationAngle(glm::radians(23.44f)),
            earthRotationSpeed(360.0f / 86164.1f), sunSpeed(360.0f / DAYS_IN_YEAR) {
        basePosition = glm::vec3(sunDistance, 0.f, 0.f);

        simulationTime.tm_year = 2023 - 1900; // Years since 1900
        simulationTime.tm_mon = 0;             // January (0-based)
        simulationTime.tm_mday = 1;            // 1st day of the month

    }

    [[nodiscard]] glm::vec3 getLightPosition() const override {
        return sunPosition;
    }

    [[nodiscard]] glm::mat4 getTransformationMatrix() const override {
        return transformationMatrix;
    }

    [[nodiscard]] std::tm getCurrentSimulationTime() const {
        return simulationTime;
    }

    // Function to calculate the Sun's position
    void updateSunPosition(float deltaTime, float simulationSpeed) {
        assert(deltaTime >= 0);
        updateSimulationTime(deltaTime, simulationSpeed);
        float earthRotationAngle = calculateEarthRotationAngle(simulationTime);
        float earthOrbitAngle = calculateEarthOrbitAngle(simulationTime);

        glm::mat4 rotationMatrixForFirstOfJanuary = calcRotationMatrixToPlaceAtTheStart();
        glm::mat4 rotationMatrix = calcRotationMatrix(earthOrbitAngle, earthRotationAngle);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), basePosition);

        transformationMatrix = rotationMatrix * rotationMatrixForFirstOfJanuary * translationMatrix;
        sunPosition = rotationMatrix * rotationMatrixForFirstOfJanuary * glm::vec4(basePosition, 1.f);
    }


private:
    const float inclinationAngle;
    const float earthRotationSpeed;
    const float sunSpeed;
    glm::vec3 basePosition;
    glm::vec3 sunPosition = glm::vec3(0.f);
    glm::mat4 transformationMatrix = glm::mat4(1.f);
    std::tm simulationTime = {};

    [[nodiscard]] float calculateEarthRotationAngle(const std::tm &datetime) const {
        float seconds = datetime.tm_hour * 3600 + datetime.tm_min * 60 + datetime.tm_sec;
        float rotationDegs = seconds * earthRotationSpeed + 250;
        float rotationYears = std::floor(rotationDegs / 360);
        float rotationRads = glm::radians(rotationDegs - rotationYears * 360);
        return rotationRads;
    }

    [[nodiscard]] float calculateEarthOrbitAngle(const std::tm &datetime) const {
        float daysInYear = (datetime.tm_yday + 1) + (datetime.tm_year - 70) * DAYS_IN_YEAR;
        float rotationDegs = daysInYear * sunSpeed;
        float rotationYears = std::floor(rotationDegs / 360);
        float rotationRads = glm::radians(rotationDegs - rotationYears * 360);
        return rotationRads;
    }

    [[nodiscard]] glm::mat4 calcRotationMatrix(float earthOrbitAngle, float earthRotationAngle) const {
        auto currentInclinationAngle = static_cast<float>(calcCurrentInlination());

        // Apply an additional rotation to account for the Earth's axial tilt (inclination).
        glm::mat4 inclinationMatrix = glm::rotate(glm::mat4(1.0f), currentInclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 earthRotationMatrix = glm::rotate(glm::mat4(1.0f), earthRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 orbitMatrix = glm::rotate(glm::mat4(1.0f), earthOrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        return earthRotationMatrix * /*orbitMatrix **/ inclinationMatrix;
    }

    [[nodiscard]] double calcCurrentInlination() const {
        // Calculate the inclination angle (axial tilt) as a function of the time of the year.
        // This is a simplified model and doesn't account for the Earth's elliptical orbit.
        const double elapsedDays = getElapsedDaysInCurrentYear();
        const double summmerSolsticeDay = 171.5;
        // Calculate the difference in days from the vernal equinox to the current date and time.
        double daysFromVernalEquinox = elapsedDays - summmerSolsticeDay;
        double currentInclination = inclinationAngle * cos(glm::radians(daysFromVernalEquinox * 360.0f / DAYS_IN_YEAR));
        return currentInclination;
    }

    [[nodiscard]] double getElapsedDaysInCurrentYear() const {
        // Obtain the day of the year from the provided date and time
        int dayOfYear = simulationTime.tm_yday;

        // Calculate the time in hours (fractional part of the day)
        auto hours = static_cast<double>(simulationTime.tm_hour);
        auto minutes = static_cast<double>(simulationTime.tm_min);
        auto seconds = static_cast<double>(simulationTime.tm_sec);

        double fractionalDay = hours + (minutes / 60.0) + (seconds / 3600.0);

        return dayOfYear + fractionalDay / 24.f;
    }

    [[nodiscard]] glm::mat4 calcRotationMatrixToPlaceAtTheStart() {
        // Constants for the approximation
        const float daysFromVernalEquinoxToJanuary1 = 286.0f;
        const float earthOrbitDegrees = 360.0f; // Full orbit in degrees

        // Calculate the Earth's angular position in its orbit on the 1st of January 2023
        float earthAngularPosition = (earthOrbitDegrees / DAYS_IN_YEAR) * daysFromVernalEquinoxToJanuary1;

        // Create a rotation matrix based on the Earth's angular position
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(earthAngularPosition),
                                               glm::vec3(0.0f, 1.0f, 0.0f));

        return rotationMatrix;
    }

    void updateSimulationTime(float simPassedTimeSecs, float simulationSpeed) {
        // Define the time scale
        float secsInRealDay = 24 * 60 * 60;
        float secsInSimDay = secsInRealDay / simulationSpeed;

        float simulationSecs = secsInRealDay * simPassedTimeSecs / secsInSimDay;

        // Add seconds and milliseconds to the starting time
        simulationTime.tm_sec += static_cast<int>(simulationSecs);

        // Normalize the time struct, taking care of overflow
        mktime(&simulationTime);
    }

};

#endif //EARTH_VISUALIZATION_SOLARSIMULATOR_H
