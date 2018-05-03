#include <EEPROM.h>
#include "MiniKame.h"

/*
   (servo index, pin to attach pwm)
   __________ __________ _________________
  |(3,9)_____)(1,8)      (0,2)(______(2,3)|
  |__|       |left FRONT right|        |__|
             |                |
             |                |
             |                |
   _________ |                | __________
  |(7,7)_____)(5,6)______(4,4)(______(6,5)|
  |__|                                 |__|

*/
//comment below manually setting trim in MiniKame() constructor
#define __LOAD_TRIM_FROM_EEPROM__

#define EEPROM_MAGIC  0xabcd
#define EEPROM_OFFSET 2   //eeprom starting offset to store trim[]


MiniKame::MiniKame():/* reverse{0, 0, 0, 0, 0, 0, 0, 0}, */trim{0, 0, 0, 0, 0, 0, 0, 0} {
  board_pins[FRONT_RIGHT_HIP] = 2; // front left inner
  board_pins[FRONT_LEFT_HIP] = 8; // front right inner
  board_pins[BACK_RIGHT_HIP] = 4; // back left inner
  board_pins[BACK_LEFT_HIP] = 6; // back right inner
  board_pins[FRONT_RIGHT_LEG] = 3; // front left outer
  board_pins[FRONT_LEFT_LEG] = 9; // front right outer
  board_pins[BACK_RIGHT_LEG] = 5; // back left outer
  board_pins[BACK_LEFT_LEG] = 7; // back right outer
}

void MiniKame::init() {
  /*
     trim[] for calibrating servo deviation,
     initial posture (home) should like below
     in symmetric
        \       / front left
         \_____/
         |     |->
         |_____|->
         /     \
        /       \ front right
  */
  /*
    trim[FRONT_LEFT_HIP] = 0;
    trim[FRONT_RIGHT_HIP] = -8;
    trim[BACK_LEFT_HIP] = 8;
    trim[BACK_RIGHT_HIP] = 5;

    trim[FRONT_LEFT_LEG] = 2;
    trim[FRONT_RIGHT_LEG] = -6;
    trim[BACK_LEFT_LEG] = 6;
    trim[BACK_RIGHT_LEG] = 5;
  */
#ifdef __LOAD_TRIM_FROM_EEPROM__
  int val = EEPROMReadWord(0);
  if (val != EEPROM_MAGIC) {
    EEPROMWriteWord(0, EEPROM_MAGIC);
    storeTrim();
  }
#endif

  for (int i = 0; i < 8; i++) {
    servo[i].attach(board_pins[i]);
#ifdef __LOAD_TRIM_FROM_EEPROM__
    int val = EEPROMReadWord(i * 2 + EEPROM_OFFSET);
    if (val >= -90 && val <= 90) {
      trim[i] = val;
    }
#endif
  }

  home();
}

void MiniKame::turnR(float steps, float T) {
  int x_amp = 15;
  int z_amp = 15;
  int ap = 15;
  //int hi = 23;
  int hi = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {0, 180, 90, 90, 180, 0, 90, 90};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::turnL(float steps, float T) {
  int x_amp = 15;
  int z_amp = 15;
  int ap = 15;
  int hi = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {180, 0, 90, 90, 0, 180, 90, 90};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::dance(float steps, float T) {
  int x_amp = 0;
  int z_amp = 40;
  int ap = 30;
  int hi = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {0, 0, 0, 270, 0, 0, 90, 180};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::frontBack(float steps, float T) {
  int x_amp = 30;
  int z_amp = 25;
  int ap = 20;
  int hi = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {0, 180, 270, 90, 0, 180, 90, 270};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::run(int dir, float steps, float T) {
  int x_amp = 15;
  int z_amp = 15;
  int ap = 15;
  int hi = 0;
  int front_x = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {    90 + ap - front_x,
                      90 - ap + front_x,
                      90 - hi,
                      90 + hi,
                      90 - ap - front_x,
                      90 + ap + front_x,
                      90 + hi,
                      90 - hi
                 };
  int phase[] = {0, 0, 90, 90, 180, 180, 90, 90};
  if (dir == 1) {
    phase[0] = phase[1] = 180;
    phase[4] = phase[5] = 0;
  }
  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::omniWalk(bool side, float T, float turn_factor) {
  int x_amp = 15;
  int z_amp = 15;
  int ap = 15;
  int hi = 23;
  int front_x = 6 * (1 - pow(turn_factor, 2));
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {    90 + ap - front_x,
                      90 - ap + front_x,
                      90 - hi,
                      90 + hi,
                      90 - ap - front_x,
                      90 + ap + front_x,
                      90 + hi,
                      90 - hi
                 };

  int phase[8];
  if (side) {
    int phase1[] =  {0,   0,   90,  90,  180, 180, 90,  90};
    int phase2R[] = {0,   180, 90,  90,  180, 0,   90,  90};
    for (int i = 0; i < 8; i++)
      phase[i] = phase1[i] * (1 - turn_factor) + phase2R[i] * turn_factor;
  }
  else {
    int phase1[] =  {0,   0,   90,  90,  180, 180, 90,  90};
    int phase2L[] = {180, 0,   90,  90,  0,   180, 90,  90};
    for (int i = 0; i < 8; i++)
      phase[i] = phase1[i] * (1 - turn_factor) + phase2L[i] * turn_factor + oscillator[i].getPhaseProgress();
  }

  execute(1, period, amplitude, offset, phase);
}

void MiniKame::moonwalkL(float steps, float T) {
  int z_amp = 45;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {0, 0, z_amp, z_amp, 0, 0, z_amp, z_amp};
  int offset[] = {90, 90, 90, 90, 90, 90, 90, 90};
  int phase[] = {0, 0, 0, 120, 0, 0, 180, 290};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::walk(int dir, float steps, float T) {
  int x_amp = 15;
  int z_amp = 20;
  int ap = 20;
  int hi = -10;
  float period[] = {T, T, T / 2, T / 2, T, T, T / 2, T / 2};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {   90 + ap,
                     90 - ap,
                     90 - hi,
                     90 + hi,
                     90 - ap,
                     90 + ap,
                     90 + hi,
                     90 - hi
                 };
  int  phase[] = {270, 270, 270, 90, 90, 90, 90, 270};
  if (dir == 0) { //backward
    phase[0] = phase[1] = 90;
    phase[4] = phase[5] = 270;
  }
  for (int i = 0; i < 8; i++) {
    oscillator[i].reset();
    oscillator[i].setPeriod(period[i]);
    oscillator[i].setAmplitude(amplitude[i]);
    oscillator[i].setPhase(phase[i]);
    oscillator[i].setOffset(offset[i]);
    oscillator[i].start();
  }
  unsigned long _init_time = millis();
  unsigned long _now_time = _init_time;
  unsigned long _final_time = _init_time + period[0] * steps;
  bool side;

  while (_now_time < _final_time) {
    side = (int)((_now_time - _init_time) / (period[0] / 2)) % 2;

    setServo(0, oscillator[0].refresh()); //FRONT_RIGHT_HIP
    setServo(1, oscillator[1].refresh()); //FRONT_LEFT_HIP
    setServo(4, oscillator[4].refresh()); //BACK_RIGHT_HIP
    setServo(5, oscillator[5].refresh()); //BACK_LEFT_HIP

    if (side == 0) {
      setServo(3, oscillator[3].refresh()); //FRONT_LEFT_LEG
      setServo(6, oscillator[6].refresh()); //BACK_RIGHT_LEG
    }
    else {
      setServo(2, oscillator[2].refresh()); //FRONT_RIGHT_LEG
      setServo(7, oscillator[7].refresh()); //BACK_LEFT_LEG
    }
    pause(1);
    _now_time = millis();
  }

}

void MiniKame::upDown(float steps, float T) {
  int x_amp = 0;
  int z_amp = 35;
  int ap = 20;
  //int hi = 25;
  int hi = 0;
  int front_x = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {    90 + ap - front_x,
                      90 - ap + front_x,
                      90 - hi,
                      90 + hi,
                      90 - ap - front_x,
                      90 + ap + front_x,
                      90 + hi,
                      90 - hi
                 };
  int phase[] = {0, 0, 90, 270, 180, 180, 270, 90};

  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::pushUp(float steps, float T) {
  int z_amp = 40;
  int x_amp = 65;
  int hi = 0;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {0, 0, z_amp, z_amp, 0, 0, 0, 0};
  int offset[] = {90, 90, 90 - hi, 90 + hi, 90 - x_amp, 90 + x_amp, 90 + hi, 90 - hi};
  int phase[] = {0, 0, 0, 180, 0, 0, 0, 180};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::home() {
  int ap = 20;
  int hi = 0;
  int position[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  for (int i = 0; i < 8; i++) {
    if (position[i] + trim[i] <= 180 && position[i] + trim[i] > 0) {
      oscillator[i].stop();
      setServo(i, position[i] + trim[i]);
    }
  }
}

void MiniKame::hello() {
  float sentado[] = {90 + 15, 90 - 15, 90 - 65, 90 + 65, 90 + 20, 90 - 20, 90 + 10, 90 - 10};
  moveServos(150, sentado);
  pause(200);

  int z_amp = 40;
  int x_amp = 60;
  int T = 350;
  float period[] = {T, T, T, T, T, T, T, T};
  int amplitude[] = {0, 50, 0, 50, 0, 0, 0, 0};
  int offset[] = {
    90 + 15, 40,
    90 - 10, 90 + 10,
    90 + 20, 90 - 20,
    90 + 65, 90
  };

  int phase[] = {0, 0, 0, 90, 0, 0, 0, 0};

  execute(4, period, amplitude, offset, phase);

  float goingUp[] = {160, 20, 90, 90, 90 - 20, 90 + 20, 90 + 10, 90 - 10};
  moveServos(500, goingUp);
  pause(200);

}

void MiniKame::jump() {
  //float sentado[] = {90 + 15, 90 - 15, 90 - 65, 90 + 65, 90 + 20, 90 - 20, 90 + 10, 90 - 10};
  float sentado[] = {
    90 + 15, 90 - 15, //front hips servos
    90 - 10, 90 + 10, //front leg servos
    90 + 10, 90 - 10, // back hip servos
    90 + 65, 90 - 65  // back leg servos
  };
  int ap = 20;
  int hi = 35;
  float salto[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap * 3, 90 + ap * 3, 90 + hi, 90 - hi};

  moveServos(150, sentado);
  pause(200);
  moveServos(0, salto);
  pause(100);
  home();
}


void MiniKame::moveServos(int time, float target[8]) {
  float _increment[8];
  float _servo_position[8] = {90, 90, 90, 90, 90, 90, 90, 90};
  unsigned long _final_time;
  unsigned long _partial_time;
  if (time > 10) {
    for (int i = 0; i < 8; i++)  _increment[i] = (target[i] - (_servo_position[i] + trim[i])) / (time / 10.0);
    _final_time =  millis() + time;

    while (millis() < _final_time) {
      _partial_time = millis() + 10;
      for (int i = 0; i < 8; i++) setServo(i, (_servo_position[i] + trim[i]) + _increment[i]);
      //while (millis() < _partial_time); //pause
      pause(_partial_time);
    }
  }
  else {
    for (int i = 0; i < 8; i++) setServo(i, target[i]);
  }
  for (int i = 0; i < 8; i++) _servo_position[i] = target[i];
}

void MiniKame::setServo(int id, float target) {
  servo[id].write(target + trim[id]);
}

void MiniKame::execute(float steps, float period[8], int amplitude[8], int offset[8], int phase[8]) {
  for (int i = 0; i < 8; i++) {
    oscillator[i].setPeriod(period[i]);
    oscillator[i].setAmplitude(amplitude[i]);
    oscillator[i].setPhase(phase[i]);
    oscillator[i].setOffset(offset[i]);
    oscillator[i].start();
    oscillator[i].setTime(millis());
  }
}

void MiniKame::refresh() {
  for (int i = 0; i < 8; i++) {
    if (oscillator[i].isStop()) continue;
    setServo(i, oscillator[i].refresh());
  }
}

void MiniKame::storeTrim() {
  for (int i = 0; i < 8; i++) {
    EEPROMWriteWord(i * 2 + EEPROM_OFFSET, trim[i]);
    delay(100);
  }
}

// load/send only trim of hip servo
void MiniKame::loadTrim() {
  //FRONT_LEFT/RIGHT_HIP
  for (int i = 0; i < 4; i++) {
    Serial.write(EEPROM.read(i + EEPROM_OFFSET));
  }

  //BACK_LEFT/RIGHT_HIP
  for (int i = 8; i < 12; i++) {
    Serial.write(EEPROM.read(i + EEPROM_OFFSET));
  }
}

int MiniKame::EEPROMReadWord(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void MiniKame::EEPROMWriteWord(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}



