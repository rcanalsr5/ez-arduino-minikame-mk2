#include <setjmp.h>
#include "GoBLE.h"
#include "MiniKame.h"

//#define __DEBUG__

#ifdef __DEBUG__
#include <SoftwareSerial.h>
SoftwareSerial softSerial(A0, A1);
#endif
//uncomment out below line if you want to use HC_SR04 sensor
//#define __HC_SR04__

#ifdef __HC_SR04__
#define HC_SR04_TRIGGER_PIN A4
#define HC_SR04_ECHO_PIN    A5
#define MIN_DISTANCE 10
#define MAX_DISTANCE MIN_DISTANCE + 10
#endif

#define CAL_TRIGGER_PIN 12
#define LED_PIN A6

#define TIME_INTERVAL 5000
#define SERIAL_DATA_PERIOD 200

#define FORWARD 'f'
#define LEFT 'l'
#define STAND 's'
#define RIGHT 'r'
#define BACKWARD 'b'
#define PUSH_UP 'p'
#define UP_DOWN 'u'
#define DANCE 'n'
#define OMNI_WALK_R 'o'
#define OMNI_WALK_L 'i'
#define MOON_WALK 'm'
#define FRONT_BACK 't'
#define HELLO 'h'

MiniKame robot;

bool auto_mode = true;
bool random_walk = false;
bool stopSerial = false;
unsigned long cur_time, prev_serial_data_time, perv_sensor_time;
char cmd = STAND;
jmp_buf jump_env;

void setup() {
  Serial.begin(115200);
#ifdef __DEBUG__
  softSerial.begin(115200);
  softSerial.println("debugging mode");
#endif

#ifdef __HC_SR04__
  pinMode(HC_SR04_TRIGGER_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
#endif
  randomSeed(analogRead(A7));
  //
  robot.init();
  delay(2000);

  { //begin: triggering delay for servo calibrating
    bool state = true;
    pinMode(CAL_TRIGGER_PIN, OUTPUT);
    digitalWrite(CAL_TRIGGER_PIN, 0);
    pinMode(CAL_TRIGGER_PIN, INPUT);
    while (digitalRead(CAL_TRIGGER_PIN)) {
      analogWrite(LED_PIN, 128 * state); // on calibarting indication LED
      delay(1000);
      state = !state;
    }
    analogWrite(LED_PIN, 0); // off calibarting indication LED
  }//end:
  perv_sensor_time = prev_serial_data_time = millis();
  //robot.run();
  //robot.turnL();
  //robot.turnR();
  //robot.pushUp();
  //robot.upDown();
  //robot.dance();
  //robot.frontBack();
  //robot.moonwalkL();
  //robot.omniWalk();
  //robot.hello();
  //robot.jump();
  if (auto_mode)
    cmd = FORWARD;
}

void loop() {
  //robot.refresh();  return;

  cur_time = millis();
  if (cur_time - prev_serial_data_time >= 100) {
    prev_serial_data_time = cur_time;
    if (stopSerial) {
#ifdef __DEBUG__
      //softSerial.println("enable serial again");
#endif
      Serial.begin(115200);
      stopSerial = false;
    }
  }
  check_goble();
  setjmp(jump_env);
#ifdef __DEBUG__
  softSerial.println(cmd);
#endif

#ifdef __HC_SR04__
  if (cur_time - perv_sensor_time >= SERIAL_DATA_PERIOD) {
    perv_sensor_time = cur_time;
    long cm = distance_measure();
    if (cm >= MIN_DISTANCE && cm <= MAX_DISTANCE) {
      cmd = BACKWARD;
    }
  }
#endif
  gaits(cmd);
  robot.refresh();
}

boolean check_goble() {
  bool rc = false;
  if (Goble.available()) {
    Serial.end();
    rc = stopSerial = true;
    int joystickX, joystickY;
    joystickY = Goble.readJoystickY();
    joystickX = Goble.readJoystickX();
    if (!auto_mode) {
      if (joystickX > 190) cmd = FORWARD;
      else if (joystickX < 80) cmd = BACKWARD;
      else if (joystickY > 190) cmd = RIGHT;
      else if (joystickY < 80) cmd = LEFT;
      else if (Goble.readSwitchUp() == PRESSED)
        cmd = PUSH_UP;
      else if (Goble.readSwitchDown() == PRESSED)
        cmd = MOON_WALK;
      else if (Goble.readSwitchLeft() == PRESSED)
        cmd = OMNI_WALK_L;
      else if (Goble.readSwitchRight() == PRESSED)
        cmd = OMNI_WALK_R;
      else if (Goble.readSwitchStart() == PRESSED)
        cmd = DANCE;
      else {
        cmd = STAND;
      }
    }
#ifdef __DEBUG__
    softSerial.print("recevied cmd ");
    softSerial.println(cmd);
#endif
    if (Goble.readSwitchSelect() == PRESSED) {
      auto_mode = !auto_mode;
      if (auto_mode)
        goto __auto;
      else
        cmd = STAND;
    }
  }

  if (auto_mode) {
    static char movements[] = {FORWARD, LEFT, OMNI_WALK_R, OMNI_WALK_L, RIGHT, BACKWARD, PUSH_UP, UP_DOWN, HELLO, DANCE, FRONT_BACK, MOON_WALK};
    static unsigned long old_time = cur_time;
    static int c = 0;
    if (cur_time - old_time >= TIME_INTERVAL ) {
      old_time = cur_time;
__auto:
      //c = (int)random(0, sizeof(movements));
      // cmd = movements[c];
      if (!random_walk) {
        c = c % (sizeof(movements)/sizeof(char));
        cmd = movements[c++];
      } else {
        c = (int)random(0, sizeof(movements)/sizeof(char));
        cmd = movements[c];
      }
    }

  }
  return rc;
}

boolean  gaits(char cmd) {
  static char prev_cmd = '.';
  bool taken = true;

  if (prev_cmd == cmd) return;
#ifdef __DEBUG__
  softSerial.print("dfsfs ");
  softSerial.println(cmd);
#endif
  switch (cmd) {
    case FORWARD:
      robot.run();
      break;
    case BACKWARD:
      robot.run(0);
      break;
    case RIGHT:
      robot.turnR(1, 550);
      break;
    case LEFT:
      robot.turnL(1, 550);
      break;
    case STAND:
      robot.home();
      break;
    case PUSH_UP:
      robot.pushUp();
      break;
    case UP_DOWN:
      robot.upDown();
      break;
    case HELLO:
      robot.hello();
      break;
    case MOON_WALK:
      robot.moonwalkL();
      break;
    case OMNI_WALK_L:
      robot.omniWalk(false);
      break;
    case OMNI_WALK_R:
      robot.omniWalk();
      break;
    case DANCE:
      robot.dance();
      break;
    case FRONT_BACK:
      robot.frontBack();
      break;
    default:
      taken = false;
  }
  if (taken) prev_cmd = cmd;
  return taken;
}

void pause(int period) {
  long timeout = millis() + period;
  do {
    if (check_goble())
      longjmp(jump_env, 1);
  } while (millis() <= timeout);
}

#ifdef __HC_SR04__
char detect_obstacle(char cmd) {
  char i = cmd;
  long cm = distance_measure();
  if (cm < MIN_DISTANCE) {
    i = STAND;
  } else if (cm >= MIN_DISTANCE && cm <= MAX_DISTANCE) {
    i = BACKWARD;
  }
  return i;
}

long distance_measure()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

  digitalWrite(HC_SR04_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_SR04_TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(HC_SR04_TRIGGER_PIN, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(HC_SR04_ECHO_PIN, HIGH);

  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  cm =  duration / 29 / 2;
  //softSerial.print(cm);
  //Serial.println("cm");
  // delay(100);
  return cm;
}
#endif
