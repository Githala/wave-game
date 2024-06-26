// This program is written for the Arduino Nano using Port Change Interrupts to allow for more interrupt pins then the default 2 external interrupts.
// See https://dronebotworkshop.com/interrupts/ for information on Port Change Interrupts


#define ENC_A1 2
#define ENC_B1 3
#define ENC_A2 4
#define ENC_B2 5
#define ENC_A3 6
#define ENC_B3 7

#define SEL_WV0 9
#define SEL_WV1 10
#define SEL_WV2 11

volatile int8_t direction[] = {0,0,0};
volatile int8_t selectedWave = -1;

// detect interrupt for group B
ISR(PCINT0_vect) {
  read_wave_select();
}

// detect interrupt for group D
ISR(PCINT2_vect) {
  read_encoder_x(ENC_A1, ENC_B1, 0);
  read_encoder_x(ENC_A2, ENC_B2, 1);
  read_encoder_x(ENC_A3, ENC_B3, 2);
}

// Defined as per https://www.youtube.com/watch?v=fgOfSHTYeio
void read_encoder_x(int enca, int encb, int inputNr) {
  static uint8_t old_AB[] = {3,3,3};
  static int8_t encval[] = {0,0,0};
  static const int8_t enc_states[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };
  static int oldA[] = {1,1,1};
  static int oldB[] = {1,1,1};

  int a = digitalRead(enca);
  int b = digitalRead(encb);

  // only trigger if there is an actual change
  if (a != oldA[inputNr] || b != oldB[inputNr]) {

    // shift current state to the left 000000ab -> 0000ab00
    old_AB[inputNr] <<= 2;

    // add either 01 or 10 to as the last to bits
    if (a == 1) old_AB[inputNr] |= 0x02;  // 0000ab10
    if (b == 1) old_AB[inputNr] |= 0x01;  // 0000ab01

    uint8_t index = (old_AB[inputNr] & 0x0f);  // take the number of the right most 4 bits as the index (0-15)
    encval[inputNr] += enc_states[index];

    oldA[inputNr] = a;
    oldB[inputNr] = b;
  }

  // after 4 steps a full step was taken.
  // higher than 3 -> direction=1
  if (encval[inputNr] > 3) {
    direction[inputNr] = 1;
    encval[inputNr] = 0;
  }
  // lower than 3 -> direction=1
  if (encval[inputNr] < -3) {
    direction[inputNr] = -1;
    encval[inputNr] = 0;
  }
}

void read_wave_select() {
  static int old0 = 1;
  static int old1 = 1;
  static int old2 = 1;
  int wv0 = digitalRead(SEL_WV0);
  int wv1 = digitalRead(SEL_WV1);
  int wv2 = digitalRead(SEL_WV2);

  if (wv0 != old0) {
    if (wv0 == 0) {
      selectedWave = 0;
    }
    old0 = wv0;
  }

  if (wv1 != old1) {
    if (wv1 == 0) {
      selectedWave = 1;
    }
    old1 = wv1;
  }
    
  if (wv2 != old2) {
    if (wv2 == 0) {
      selectedWave = 2;
    }
    old2 = wv2;
  }
}

void setup() {
  PCICR |= B00000101;
  PCMSK0 |= B00001110;
  PCMSK2 |= B11111100;
  pinMode(ENC_A1, INPUT_PULLUP);
  pinMode(ENC_B1, INPUT_PULLUP);
  pinMode(ENC_A2, INPUT_PULLUP);
  pinMode(ENC_B2, INPUT_PULLUP);
  pinMode(ENC_A3, INPUT_PULLUP);
  pinMode(ENC_B3, INPUT_PULLUP);

  pinMode(SEL_WV0, INPUT_PULLUP);
  pinMode(SEL_WV1, INPUT_PULLUP);
  pinMode(SEL_WV2, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("start");
}

void loop() {
  static int oldSelectedWave = -1;
  static long lastOldSelectedWaveTime = 0;
  // If a direction was set, print it and set direction back to 0;
  for (int i=0; i<3; i++) {
    if (direction[i] != 0) {
      char strBuf[3];
      sprintf(strBuf, "rot:%d:%d", i, direction[i]);
      Serial.println(strBuf);
      direction[i] = 0;
    }
  }
  if(selectedWave > -1 && selectedWave != oldSelectedWave) {
    char strBuf[3];
    sprintf(strBuf, "wave:%d", selectedWave);
    Serial.println(strBuf);
    oldSelectedWave = selectedWave;
    lastOldSelectedWaveTime = millis();
    selectedWave = -1;
  }

  if (oldSelectedWave > -1 && lastOldSelectedWaveTime + 200 < millis()) {
    oldSelectedWave = -1;
  }
}