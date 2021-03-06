/*
StepperControl_AMSv2.h - - Driver for a Stepper Motor with controler AMSv2- Version 1.0

History:
Version 1.0AMSv2 - Author Russell Valentine modified to support Adafruit motorshield v2.
Version 1.0 - Author Jean-Philippe Bonnet
   First release

This file is part of the StepperControl_AMSv2 library.

StepperControl_AMSv2 library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

StepperControl_AMSv2 library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with StepperControl library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "StepperControl_AMSv2.h"



//------------------------------------------------------------------------------------
// Constructors:
StepperControl_AMSv2::StepperControl_AMSv2(uint8_t port) {
    this->port = port;
    this->AFMS = Adafruit_MotorShield();
    this->motor = AFMS.getStepper(5, port);
    this->AFMS.begin();
    //this->motor->setSpeed(1);

    this->direction = SC_CLOCKWISE;
    this->inMove = false;
    this->startPosition = 0;
    this->currentPosition = 0;
    this->targetPosition = 0;
    this->moveMode = SC_MOVEMODE_PER_STEP;
    this->acceleration = SC_DEFAULT_ACCEL;
    this->speed = 0;
    this->timestamp = 0;
    this->accelTimestamp = 0;
    this->brakeMode = false;
    this->targetSpeedReached = false;
    this->positionTargetSpeedReached = 0;
    this->lastCompensatedTemperature = 0;
    this->temperatureCompensationIsInit = false;
    this->temperatureCompensationIsEnabled = false;
    this->temperatureCompensationCoefficient = 0;
    this->currentTemperature = 0;
    this->setStepMode(SC_FULL_STEP);
}

//------------------------------------------------------------------------------------
// Setters
void StepperControl_AMSv2::setTargetPosition(long position) {
    this->targetPosition = position;
}

void StepperControl_AMSv2::setCurrentPosition(long position) {
    this->currentPosition = position;
}

void StepperControl_AMSv2::setDirection(int direction) {
    this->direction = direction;
}

void StepperControl_AMSv2::setStepMode(int stepMode) {
    this->stepMode = stepMode;
}

void StepperControl_AMSv2::setMoveMode(int moveMode) {
    this->moveMode = moveMode;
}

void StepperControl_AMSv2::setSpeed(unsigned int speed) {
    if (this->stepMode == SC_SIXTEENTH_STEP && speed >= SC_MAX_SPEED_SIXTEENTH_STEP) {
        this->targetSpeed = SC_MAX_SPEED_SIXTEENTH_STEP;
    } else if (this->stepMode == SC_EIGHTH_STEP && speed >= SC_MAX_SPEED_EIGHTH_STEP) {
        this->targetSpeed = SC_MAX_SPEED_EIGHTH_STEP;
    } else if (this->stepMode == SC_QUATER_STEP && speed >= SC_MAX_SPEED_QUATER_STEP) {
        this->targetSpeed = SC_MAX_SPEED_QUATER_STEP;
    } else if (this->stepMode == SC_HALF_STEP && speed >= SC_MAX_SPEED_HALF_STEP) {
        this->targetSpeed = SC_MAX_SPEED_HALF_STEP;
    } else if (this->stepMode == SC_FULL_STEP && speed >= SC_MAX_SPEED_FULL_STEP) {
        this->targetSpeed = SC_MAX_SPEED_FULL_STEP;
    } else {
        this->targetSpeed = speed;
    }
}

void StepperControl_AMSv2::setBrakeMode(int brakeMode) {
    this->brakeMode = brakeMode;
}

void StepperControl_AMSv2::setTemperatureCompensationCoefficient(int coef) {
    this->temperatureCompensationCoefficient = coef;
}

void StepperControl_AMSv2::setCurrentTemperature(float curTemp) {
    this->currentTemperature = curTemp;
    if (!this->temperatureCompensationIsInit) {
        this->lastCompensatedTemperature = this->currentTemperature;
        this->temperatureCompensationIsInit = true;
    }
}

//------------------------------------------------------------------------------------
// Getters
long StepperControl_AMSv2::getCurrentPosition() {
    return this->currentPosition;
}

long StepperControl_AMSv2::getTargetPosition() {
    return this->targetPosition;
}

int StepperControl_AMSv2::getDirection() {
    return this->direction;
}

int StepperControl_AMSv2::getStepMode() {
    return this->stepMode;
}

int StepperControl_AMSv2::getMoveMode() {
    return this->moveMode;
}

unsigned int StepperControl_AMSv2::getSpeed() {
    return this->speed;
}

int StepperControl_AMSv2::getBrakeMode() {
    return this->brakeMode;
}

int StepperControl_AMSv2::getTemperatureCompensationCoefficient() {
    return this->temperatureCompensationCoefficient;
}

//------------------------------------------------------------------------------------
// Other public members
void StepperControl_AMSv2::Manage() {
    if (this->inMove) {
        this->moveMotor();
    } else if (this->temperatureCompensationIsEnabled) {

    }
}

void StepperControl_AMSv2::goToTargetPosition() {
    if (this->currentPosition != this->targetPosition) {
        if (this->moveMode == SC_MOVEMODE_SMOOTH) {
            this->speed = 0;
            this->targetSpeedReached = false;
            this->positionTargetSpeedReached = 0;
        } else {
            this->speed = this->targetSpeed;
        }
        this->startPosition = this->currentPosition;
        this->inMove = true;
    }
}

void StepperControl_AMSv2::stopMovement() {
    if (!this->getBrakeMode())
        this->motor->release();
    this->inMove = false;
    this->speed = 0;
    this->positionTargetSpeedReached = 0;
}

int StepperControl_AMSv2::isInMove() {
    return this->inMove;
}

void StepperControl_AMSv2::compensateTemperature() {
    long correction = 0;

    if (this->temperatureCompensationIsInit && !this->inMove) {
        correction = (long) (1.0 * (this->lastCompensatedTemperature
                                    - this->currentTemperature)
                             * (float) this->temperatureCompensationCoefficient);

        if (correction) {
            this->lastCompensatedTemperature = this->currentTemperature;
            this->dbg_correction = this->getCurrentPosition() + (long) correction;
            this->setTargetPosition(this->getCurrentPosition() + (long) correction);
            this->goToTargetPosition();
        }
    }
}

//------------------------------------------------------------------------------------
// Privates
void StepperControl_AMSv2::moveMotor() {
    uint8_t style = MICROSTEP; // Only one speed for now.
    uint8_t dir = FORWARD;
    if (this->moveMode == SC_MOVEMODE_SMOOTH) {
        this->calculateSpeed();
    }

    if ((this->targetPosition != this->currentPosition)) {
        if ((this->speed != 0) && (micros() - this->timestamp)
                                  >= ((unsigned long) ((1 / ((float) this->speed + 1)) * 1000000))) {
            if ((this->targetPosition - this->currentPosition) > 0) {
                if(this->direction == SC_CLOCKWISE) {
                    dir = FORWARD;  
                } else {
                  dir = BACKWARD;
                }
                this->currentPosition++;
            } else {
              if(this->direction == SC_CLOCKWISE) {
                dir = BACKWARD;
              } else {
                dir = FORWARD;
              }
                this->currentPosition--;
            }
            delayMicroseconds(5);
            this->motor->onestep(dir, style);
            delayMicroseconds(5);

            if (this->speed >= this->targetSpeed) {
                if (!this->targetSpeedReached) {
                    this->positionTargetSpeedReached = this->startPosition
                                                       - this->currentPosition;
                    if (this->positionTargetSpeedReached < 0) {
                        this->positionTargetSpeedReached *= -1;
                    }
                }
                this->speed = this->targetSpeed;
                this->targetSpeedReached = true;
            }
            this->timestamp = micros();
        }
    } else {
        this->stopMovement();
    }
}

void StepperControl_AMSv2::calculateSpeed() {
    if ((millis() - this->accelTimestamp) >= 50) {
        long midway = (this->targetPosition - this->startPosition);
        // avoid miday == 0 in case of movement of only one step
        if (abs(midway) == 1) {
            midway *= 2;
        }

        midway /= 2;

        if (midway > 0) {
            midway += this->startPosition;
            if (!this->targetSpeedReached && (this->currentPosition < midway)) {
                this->speed += this->acceleration;
            } else {
                if ((!this->targetSpeedReached && (this->currentPosition > midway))
                    || (this->currentPosition >= (this->targetPosition - this->positionTargetSpeedReached))) {

                    if ((this->targetPosition != this->currentPosition)
                        && (this->speed > this->acceleration)) {
                        this->speed -= this->acceleration;
                    } else {
                        this->speed = this->acceleration;
                    }

                }
            }
        } else {
            midway = this->startPosition - -1 * midway;
            if (!this->targetSpeedReached && (this->currentPosition > midway)) {
                this->speed += this->acceleration;
            } else {
                if ((!this->targetSpeedReached && (this->currentPosition < midway))
                    || (this->currentPosition <= (this->positionTargetSpeedReached + this->targetPosition))) {

                    if ((this->targetPosition != this->currentPosition)
                        && (this->speed > this->acceleration)) {
                        this->speed -= this->acceleration;
                    } else {
                        this->speed = this->acceleration;
                    }
                }
            }
        }
        this->accelTimestamp = millis();
    }
}


bool StepperControl_AMSv2::isTemperatureCompensationEnabled() {
    return this->temperatureCompensationIsEnabled;
}

void StepperControl_AMSv2::enableTemperatureCompensation() {
    this->temperatureCompensationIsInit = false;
    this->temperatureCompensationIsEnabled = true;
}

void StepperControl_AMSv2::disableTemperatureCompensation() {
    this->temperatureCompensationIsEnabled = false;
}
