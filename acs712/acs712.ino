const float SENS_mV_PER_A = 185.0;   // <-- 5A board
const float VCC = 5.0;
const int PIN = A0;


const float ON_THRESHOLD_A  = 0.20; //set open threshold  
const float OFF_THRESHOLD_A = 0.12; //set close threshold 
const uint16_t MIN_ON_MS    = 120;    
const uint16_t MIN_OFF_MS   = 120;    

// Sampling
const int SAMPLES_PER_READ = 100;     
const uint16_t LOOP_DELAY_MS = 50;    

float zeroOffsetV = 2.5;

// State for counting
bool motorOn = false;
uint32_t stateChangeMs = 0;
uint32_t lastPrintMs = 0;
uint32_t sheetCount = 0;

void setup() {
  Serial.begin(9600);
  delay(200);


  const int N = 1000;
  long sum = 0;
  for (int i = 0; i < N; i++) { sum += analogRead(PIN); delayMicroseconds(200); }
  float avgCounts = sum / (float)N;
  zeroOffsetV = (avgCounts / 1023.0) * VCC;

  stateChangeMs = millis();
}

float readCurrentA() {
  long sum = 0;
  for (int i = 0; i < SAMPLES_PER_READ; i++) sum += analogRead(PIN);
  float avgCounts = sum / (float)SAMPLES_PER_READ;
  float v = (avgCounts / 1023.0) * VCC;
  float mvFromZero = (v - zeroOffsetV) * 1000.0;
  float amps = mvFromZero / SENS_mV_PER_A;

  // small deadband
  if (fabs(amps) < 0.03) amps = 0.0;
  return amps;
}

void loop() {
  float amps = readCurrentA();
  float a = fabs(amps);        
  uint32_t now = millis();

  if (!motorOn) {

    if (a >= ON_THRESHOLD_A) {
      if (now - stateChangeMs >= MIN_OFF_MS) {
        motorOn = true;
        stateChangeMs = now;
      }
    } else {
      stateChangeMs = now; 
    }
  } else {
   
    if (a <= OFF_THRESHOLD_A) {
      if (now - stateChangeMs >= MIN_ON_MS) {
        motorOn = false;
        stateChangeMs = now;
        sheetCount++;                       
        Serial.print("Paper out: ");
        Serial.println(sheetCount);
      }
    } else {
      stateChangeMs = now; 
    }
  }

  if (now - lastPrintMs > 500) {
    lastPrintMs = now;
    Serial.print("I = ");
    Serial.print(amps, 3);
    Serial.print(" A  | |I| = ");
    Serial.println(a, 3);
  }

  delay(LOOP_DELAY_MS);
}
