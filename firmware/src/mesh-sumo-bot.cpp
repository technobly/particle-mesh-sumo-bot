/**
 * Particle Mesh Sumo Bot
 * RC Receiver
 * Author: Brett Walach
 * Date: 2019-02-08
 */

#include "Particle.h"

SerialLogHandler logHandler(115200, LOG_LEVEL_INFO);

SYSTEM_THREAD(ENABLED);

String dataString;
int rcControl = 0;
volatile bool parseData = false;

/**
 *  Adafruit 2448 TB6612FNG Dual H-Bridge Motor Driver Breakout
 *  to Particle Xenon pinout
 *
 *  Xenon - Adafruit
 *  D8    - PWMB  (speed right)
 *  D7    - BIN2  (direction bits)
 *  D6    - BIN1   "
 *  D5    - /STBY (motor enable)
 *  D4    - AIN1  (direction bits)
 *  D3    - AIN2   "
 *  D2    - PMWA  (speed left)
 */
#define MOTOR_L (0)
#define MOTOR_R (1)
#define FWD     (0)
#define RVS     (1)
#define PWM_R   (D8)
#define AIN2    (D7)
#define AIN1    (D6)
#define STBY    (D5)
#define BIN1    (D4)
#define BIN2    (D3)
#define PWM_L   (D2)

#define BTN_NONE (0x0000) // no buttons pressed
#define BTN_UP   (0x0001) // blue button UP
#define BTN_DN   (0x0002) // yellow button DOWN
#define BTN_LF   (0x0004) // red button LEFT
#define BTN_RT   (0x0008) // green button RIGHT
#define BTN_MSK  (0x000F) // all buttons
#define JOY_NONE (0x0000) // no joystick pressed
#define JOY_UP   (0x0010) // joystick UP
#define JOY_DN   (0x0020) // joystick DOWN
#define JOY_LF   (0x0040) // joystick LEFT
#define JOY_RT   (0x0080) // joystick RIGHT
#define JOY_MSK  (0x00F0) // all directions

void parseRc(int data);
void rcHandler(const char *event, const char *data);
void initHardware(void);
void enableMotors(bool enable);
void setDirection(bool motor, bool dir);
void setSpeed(bool motor, int speed);

void parseRc(int data) {
    // Combine joystick and button commands, now we only have to test for buttons!
    data = ((data & JOY_MSK) >> 4) | (data & BTN_MSK);
    // Log.info("Recieved: %d", data);

    int _speed_r = 100;
    int _speed_l = 100;

    // FORWARD Only?
    if ((data & BTN_MSK) == BTN_UP) {
        Log.info("Forward");
        setDirection(MOTOR_L, FWD);
        setDirection(MOTOR_R, FWD);
    }
    // REVERSE Only?
    if ((data & BTN_MSK) == BTN_DN) {
        Log.info("Reverse");
        setDirection(MOTOR_L, RVS);
        setDirection(MOTOR_R, RVS);
    }
    // LEFT Only?
    if ((data & BTN_MSK) == BTN_LF) {
        Log.info("Left");
        setDirection(MOTOR_L, FWD);
        setDirection(MOTOR_R, RVS);
    }
    // RIGHT Only?
    if ((data & BTN_MSK) == BTN_RT) {
        Log.info("Right");
        setDirection(MOTOR_L, RVS);
        setDirection(MOTOR_R, FWD);
    }

    // FORWARD + LEFT ?
    if ((data & (BTN_UP | BTN_LF)) == (BTN_UP | BTN_LF)) {
        Log.info("Forward + Left");
        setDirection(MOTOR_L, FWD);
        setDirection(MOTOR_R, FWD);
        _speed_l = 40;
    }
    // FORWARD + RIGHT ?
    if ((data & (BTN_UP | BTN_RT)) == (BTN_UP | BTN_RT)) {
        Log.info("Forward + Right");
        setDirection(MOTOR_L, FWD);
        setDirection(MOTOR_R, FWD);
        _speed_r = 40;
    }
    // REVERSE + LEFT ?
    if ((data & (BTN_DN | BTN_LF)) == (BTN_DN | BTN_LF)) {
        Log.info("Reverse + Left");
        setDirection(MOTOR_L, RVS);
        setDirection(MOTOR_R, RVS);
        _speed_l = 40;
    }
    // REVERSE + RIGHT ?
    if ((data & (BTN_DN | BTN_RT)) == (BTN_DN | BTN_RT)) {
        Log.info("Reverse + Right");
        setDirection(MOTOR_L, RVS);
        setDirection(MOTOR_R, RVS);
        _speed_r = 40;
    }

    // STOP ?
    if (data == BTN_NONE) {
        Log.info("Stop");
        setSpeed(MOTOR_L, 0);
        setSpeed(MOTOR_R, 0);
    } else {
        setSpeed(MOTOR_L, _speed_l);
        setSpeed(MOTOR_R, _speed_r);
    }
}

void rcHandler(const char *event, const char *data) {
    dataString = data;
    rcControl = dataString.toInt();
    parseData = true;
}

void initHardware(void) {
    for (int pin=D2; pin<=D8; pin++) {
        pinMode(pin, OUTPUT);
        digitalWriteFast(pin, LOW);
    }
    enableMotors(true);
}

void enableMotors(bool enable) {
    if (enable) digitalWrite(STBY, HIGH); // enable motor output
    else digitalWrite(STBY, LOW); // disable motor output
}

void setDirection(bool motor, bool dir) {
    if (motor == MOTOR_L) {
        if (dir == RVS) {
            digitalWriteFast(AIN2, LOW);
            digitalWriteFast(AIN1, HIGH);
        }
        else if (dir == FWD) {
            digitalWriteFast(AIN1, LOW);
            digitalWriteFast(AIN2, HIGH);
        }
    }
    else if (motor == MOTOR_R) {
        if (dir == FWD) {
            digitalWriteFast(BIN2, LOW);
            digitalWriteFast(BIN1, HIGH);
        }
        else if (dir == RVS) {
            digitalWriteFast(BIN1, LOW);
            digitalWriteFast(BIN2, HIGH);
        }
    }
    else {
        digitalWriteFast(AIN1, LOW);  // disable all motors
        digitalWriteFast(AIN2, LOW);
        digitalWriteFast(BIN1, LOW);
        digitalWriteFast(BIN2, LOW);
    }
}

void setSpeed(bool motor, int speed) {
    // Log.info("speed: %d", map(speed,0,100,0,255));
    if (motor == MOTOR_L) {
        analogWrite(PWM_L, map(speed,0,100,0,255) );
    }
    else if (motor == MOTOR_R) {
        analogWrite(PWM_R, map(speed,0,100,0,255) );
    }
    else {
        analogWrite(PWM_L, 0);  // stop all motors
        analogWrite(PWM_R, 0);
    }
}

void setup()
{
    dataString.reserve(622);
    Mesh.subscribe("rc-sumo", rcHandler);
    initHardware();
}

void loop()
{
    if (parseData) {
        parseRc(rcControl);
        parseData = false;
    }
}
