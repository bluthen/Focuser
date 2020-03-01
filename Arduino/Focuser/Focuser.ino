#include "Moonlite.h"
#include "StepperControl_AMSv2.h"


const uint8_t port = 2; // 2 for port M3, M4, 1 for port M1, M2

StepperControl_AMSv2 *motorPtr;
Moonlite SerialProtocol;

float temp = 0;
long pos = 0;
bool pageIsRefreshing = false;
long tempCompCoeff = 0;
long tempComp = 0;

void processCommand() {
    MoonliteCommand_t command;
    switch (SerialProtocol.getCommand().commandID) {
        case ML_C:
            // Initiate temperature convertion
            // Not implemented
            break;
        case ML_FG:
            // Goto target position
            motorPtr->goToTargetPosition();
            break;
        case ML_FQ:
            // Motor stop movement
            motorPtr->stopMovement();
            break;
        case ML_GB:
            // Set the Red Led backligth value
            // Dump value necessary to run the official moonlite software
            SerialProtocol.setAnswer(2, 0x00);
        case ML_GC:
            // Return the temperature coefficient
            SerialProtocol.setAnswer(2, (long) tempCompCoeff);
            break;
        case ML_GD:
            // Return the current motor speed
            switch (motorPtr->getSpeed()) {
                case 500:
                    SerialProtocol.setAnswer(2, (long) 20);
                    break;
                case 1000:
                    SerialProtocol.setAnswer(2, (long) 10);
                    break;
                case 3000:
                    SerialProtocol.setAnswer(2, (long) 8);
                    break;
                case 5000:
                    SerialProtocol.setAnswer(2, (long) 4);
                    break;
                case 7000:
                    SerialProtocol.setAnswer(2, (long) 2);
                    break;
                default:
                    break;
            }
            break;
        case ML_GH:
            // Return the current stepping mode (half or full step)
            SerialProtocol.setAnswer(2, (long) (motorPtr->getStepMode() == SC_HALF_STEP ? 0xFF : 0x00));
            break;
        case ML_GI:
            // get if the motor is moving or not
            SerialProtocol.setAnswer(2, (long) (motorPtr->isInMove() ? 0x01 : 0x00));
            break;
        case ML_GN:
            // Get the target position
            SerialProtocol.setAnswer(4, (long) (motorPtr->getTargetPosition()));
            break;
        case ML_GP:
            // Return the current position
            SerialProtocol.setAnswer(4, (long) (motorPtr->getCurrentPosition()));
            break;
        case ML_GT:
            // Return the temperature
            SerialProtocol.setAnswer(4, (long) ((25.0 * 2)));
            break;
        case ML_GV:
            // Get the version of the firmware
            SerialProtocol.setAnswer(2, (long) (0x01));
            break;
        case ML_SC:
            // Set the temperature coefficient
            tempCompCoeff = SerialProtocol.getCommand().parameter;
            break;
        case ML_SD:
            // Set the motor speed
            switch (SerialProtocol.getCommand().parameter) {
                case 0x02:
                    motorPtr->setSpeed(7000);
                    break;
                case 0x04:
                    motorPtr->setSpeed(5000);
                    break;
                case 0x08:
                    motorPtr->setSpeed(3000);
                    break;
                case 0x10:
                    motorPtr->setSpeed(1000);
                    break;
                case 0x20:
                    motorPtr->setSpeed(500);
                    break;
                default:
                    break;
            }
            break;
        case ML_SF:
            // Set the stepping mode to full step
            motorPtr->setStepMode(SC_EIGHTH_STEP);
            if (motorPtr->getSpeed() >= 6000) {
                motorPtr->setSpeed(6000);
            }
            break;
        case ML_SH:
            // Set the stepping mode to half step
            motorPtr->setStepMode(SC_SIXTEENTH_STEP);
            break;
        case ML_SN:
            // Set the target position
            motorPtr->setTargetPosition(SerialProtocol.getCommand().parameter);
            break;
        case ML_SP:
            // Set the current motor position
            motorPtr->setCurrentPosition(SerialProtocol.getCommand().parameter);
            break;
        case ML_PLUS:
            // Activate temperature compensation focusing
            motorPtr->enableTemperatureCompensation();
            break;
        case ML_MINUS:
            // Disable temperature compensation focusing
            motorPtr->disableTemperatureCompensation();
            break;
        case ML_PO:
            // Temperature calibration
            tempComp = SerialProtocol.getCommand().parameter;
            break;
        default:
            break;
    }
}

void setup() {
    SerialProtocol.init(9600);
    motorPtr = new StepperControl_AMSv2(port);

    // Set the motor speed to a valid value for Moonlite
    motorPtr->setSpeed(7000);
    motorPtr->setMoveMode(SC_MOVEMODE_SMOOTH);
    //motorPtr->setMoveMode(SC_MOVEMODE_PER_STEP);
    // I like this to be in the middle because I set it near focus. Another option is to set it all the way extened to one side and so make current position 0
    motorPtr->setCurrentPosition(50000);
    //I don't see how you set brake mode in indi driver.
    //motorPtr->setBrakeMode(1);
}

void loop() {
    motorPtr->Manage();
    SerialProtocol.Manage();

    if (SerialProtocol.isNewCommandAvailable()) {
        processCommand();
    }

}
