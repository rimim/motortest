#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <unistd.h>
#include <sys/select.h>
#include "serialPort/SerialPort.h"
#include "unitreeMotor/unitreeMotor.h"

////////////////////////////////////////////////////////

static int readKeyIfAvailable() {
    fd_set readfds;
    struct timeval timeout;
    int retval;
    char ch;

    // Clear the set ahead of time
    FD_ZERO(&readfds);

    // Add our descriptor to the set
    FD_SET(STDIN_FILENO, &readfds);

    // Set timeout to 0, which makes select non-blocking
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // See if there is any data available to read from stdin
    retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    return (retval > 0 && read(STDIN_FILENO, &ch, 1) == 1) ? ch : -1;
}

static void setNonCanonicalMode(bool enable) {
    static struct termios oldt, newt;

    if (enable) {
        // Get the terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        // Copy settings to newt
        newt = oldt;
        // Disable canonical mode and local echo
        newt.c_lflag &= ~(ICANON | ECHO);
        // Set the new settings immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        // Restore the old settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

////////////////////////////////////////////////////////

static double degreesToRadians(double degrees) {
    return degrees * (M_PI / 180.0);
}

static double radiansToDegrees(double radians) {
    return radians * (180.0 / M_PI);
}

static int normalize(int degrees)
{
    degrees = fmod(degrees, 360);
    if (degrees < 0)
        degrees += 360;
    return degrees;
}

static int shortestDistance(int origin, int target)
{
    int result = 0.0;
    int diff = fmod(fmod(abs(origin - target), 360), 360);

    if (diff > 180)
    {
        //There is a shorter path in opposite direction
        result = (360 - diff);
        if (target > origin)
            result *= -1;
    }
    else
    {
        result = diff;
        if (origin > target)
            result *= -1;
    }
    return result;
}

////////////////////////////////////////////////////////

class GoMotor {
public:
    GoMotor(SerialPort& port, uint16_t motor_id) :
        fPort(port),
        fMotorID(motor_id)
    {}

    double currentAngle() {
        return fCurrentAngle;
    }

    bool update() {
        fCmd.motorType = MotorType::GO_M8010_6;
        fData.motorType = MotorType::GO_M8010_6;
        fCmd.id = fMotorID;
        if (fNewAngle != fCurrentAngle) {
            fCmd.mode = queryMotorMode(MotorType::GO_M8010_6,MotorMode::FOC);
            fCmd.kp   = 0.15;
            fCmd.kd   = 0.1;
            fCmd.q    = degreesToRadians(fNewAngle) * queryGearRatio(MotorType::GO_M8010_6);
        } else {
            fCmd.mode = queryMotorMode(MotorType::GO_M8010_6,MotorMode::BRAKE);
            fCmd.kp   = 0.00;
            fCmd.kd   = 0.00;
            fCmd.q    = 0;
        }
        fCmd.dq   = 0;
        fCmd.tau  = 0.0;
        if (!fPort.sendRecv(&fCmd, &fData)) {
            fprintf(stderr, "ERROR motor_id#%d\n", fMotorID);
            return false;
        }
        fCurrentAngle = radiansToDegrees(fData.q / queryGearRatio(MotorType::GO_M8010_6));
        return true;
    }

    int temp() const {
        return fData.temp;
    }

    int motorError() const {
        return fData.merror;
    }

    bool stop() {
        setPosition(fCurrentAngle + 0.1);
        return update();
    }

    void setPosition(double pos) {
        fNewAngle = pos;
    }

    void setAbsolutePosition(double pos) {
        setPosition(fCurrentAngle + shortestDistance(fCurrentAngle, normalize(pos)));
    }

private:
    SerialPort& fPort;
    uint16_t    fMotorID;
    double      fCurrentAngle = 0;
    double      fNewAngle = 0;
    MotorCmd    fCmd;
    MotorData   fData;
};

////////////////////////////////////////////////////////

int findMotorID(SerialPort &serial) {
    for (int id = 0; id < 15; id++)
    {
        GoMotor goMotor(serial, id);
        if (goMotor.update()) {
            return id;
        }
        usleep(200);
    }
    return -1;
}

int main(int argc, const char* argv[]) {
    SerialPort serial("/dev/ttyUSB0");
    int motorID = findMotorID(serial);
    if (motorID < 0) {
        fprintf(stderr, "Failed to find motor\n");
        return 1;
    }
    printf("\n\nFOUND MOTOR: %d\n", motorID);

    GoMotor goMotor(serial, motorID);
    if (!goMotor.update()) {
        printf("FAILED TO INIT MOTOR\n");
        return 1;
    }
    printf("Degrees: %f\n", goMotor.currentAngle());
    printf("Temp:    %d\n", goMotor.temp());

    int pos = 0;
    if (argc >= 2) {
        pos = normalize(atoi(argv[1]));
        // ENABLE IF YOU WANT TO TRY TO MOVE THE MOTOR
        setNonCanonicalMode(true);
        goMotor.setAbsolutePosition(pos);
        while (readKeyIfAvailable() == -1) {
            usleep(200);
            goMotor.update();
            printf("degrees: %f        \r", goMotor.currentAngle());
        }
        printf("\n");
        goMotor.stop();
        setNonCanonicalMode(false);
    }
    return 0;
}