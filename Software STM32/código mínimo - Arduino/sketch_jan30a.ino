// --------------------
// Pines
// --------------------
const uint8_t PIN_ZC     = 2;   // D2 = INT0
const uint8_t PIN_TRIAC  = 9;   // Gate triac
const uint8_t PIN_POT    = A0;  // Potenciómetro
const uint8_t PIN_DEBUG  = 8;   // Salida para trigger osciloscopio

// --------------------
// Timing
// --------------------
const unsigned long ZC_HOLDOFF_US   = 2000;
const unsigned long ZC_OFFSET_US    = 600;
const unsigned long DIMMING_MAX_US  = 7500;
const unsigned long TRIAC_PULSE_US  = 300;

// --------------------
// Variables ISR
// --------------------
volatile bool zc_flag = false;
volatile unsigned long last_zc_us = 0;
volatile bool debug_state = false;

// --------------------
// Control dimming
// --------------------
volatile unsigned long dimming_us = 0;
unsigned long last_adc_ms = 0;
const unsigned long ADC_PERIOD_MS = 50;  // leer pote cada 50 ms

// --------------------
// ISR Zero Crossing
// --------------------
void ISR_zeroCrossing() {
  unsigned long now = micros();

  if (now - last_zc_us > ZC_HOLDOFF_US) {
    last_zc_us = now;

    zc_flag = true;

    // ---- TOGGLE de referencia para el osciloscopio ----
    debug_state = !debug_state;
    digitalWrite(PIN_DEBUG, debug_state);
  }
}

// --------------------
// Setup
// --------------------
void setup() {
  pinMode(PIN_ZC, INPUT);
  pinMode(PIN_TRIAC, OUTPUT);
  pinMode(PIN_DEBUG, OUTPUT);

  digitalWrite(PIN_TRIAC, LOW);
  digitalWrite(PIN_DEBUG, LOW);

  attachInterrupt(
    digitalPinToInterrupt(PIN_ZC),
    ISR_zeroCrossing,
    RISING
  );
}

// --------------------
// Loop
// --------------------
void loop() {

  // ---- Leer ADC cada tanto ----
  if (millis() - last_adc_ms > ADC_PERIOD_MS) {
    last_adc_ms = millis();

    int adc = analogRead(PIN_POT);  // 0–1023
    dimming_us = map(adc, 0, 1023, 0, DIMMING_MAX_US);
  }

  // ---- Evento de zero crossing ----
  if (zc_flag) {
    noInterrupts();
    zc_flag = false;
    interrupts();

    // Offset fijo post ZC
    delayMicroseconds(ZC_OFFSET_US);

    // Delay variable (dimming)
    if (dimming_us > 0) {
      delayMicroseconds(dimming_us);
    }

    // Pulso al gate
    digitalWrite(PIN_TRIAC, HIGH);
    delayMicroseconds(TRIAC_PULSE_US);
    digitalWrite(PIN_TRIAC, LOW);
  }
}
