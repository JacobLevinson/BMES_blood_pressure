// Micropressure setup

SparkFun_MicroPressure mpr; // Use default values with reset and EOC pins unused

// BLE setup

#include "NimBLEDevice.h"

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "ABCD"
#define COMMAND_CHARACTERISTIC_UUID "1234"
#define SYSTOLIC_CHARACTERISTIC_UUID "5678"
#define DIASTOLIC_CHARACTERISTIC_UUID "9123"

NimBLECharacteristic *commandCharacteristic;
NimBLECharacteristic *systolicCharacteristic;
NimBLECharacteristic *diastolicCharacteristic;

// Create a subclass for handling write events
class CommandCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override {
        std::string receivedValue = pCharacteristic->getValue();
        Serial.print("Received command: ");
        Serial.println(receivedValue.c_str());

        if (receivedValue == "READ_SENSOR") {
            findBloodPressure();
            
            String systolicString = String(systolicPressure);
            systolicCharacteristic->setValue(systolicString.c_str());
            systolicCharacteristic->notify();
            String diastolicString = String(diastolicPressure);
            diastolicCharacteristic->setValue(diastolicString.c_str());
            diastolicCharacteristic->notify();
            Serial.println("Sensor values sent!");
        }
    }
};

// Pins definitions
#define PUMP_SOLENOID_PIN 15

// Global Variables
bool systolicDetected = false;
bool diastolicDetected = false;
double systolicPressure = 0;
double diastolicPressure = 0;

// Pre-pressurize the cuff
void pressurize() {
  digitalWrite(PUMP_SOLENOID_PIN, HIGH);
  while(mpr.readPressure(INHG) / 25.4 < 165){
    // Do nothing
  }
  digitalWrite(PUMP_SOLENOID_PIN, LOW); // Allow to repressurize
  return;
}

// Knock-based systolic/diastolic detection
void detectBloodPressure(double pressure, double knock) {
    static double knockThreshold = 2;
    static int noKnockCount = 0;
    static const int quietWindow = 15;  // ~150ms if using delay(10)
    static double lastKnockPressure = 0;

    // Detect systolic
    if (!systolicDetected && knock > knockThreshold) {
        systolicPressure = pressure;
        systolicDetected = true;
        Serial.println("Systolic detected!");
    }

    // Track last knock after systolic
    if (systolicDetected && knock > knockThreshold) {
        lastKnockPressure = pressure;
        noKnockCount = 0;  // Reset quiet counter
    }

    // Count consecutive quiet samples
    if (systolicDetected && knock <= knockThreshold) {
        noKnockCount++;
        if (!diastolicDetected && noKnockCount >= quietWindow) {
            diastolicPressure = lastKnockPressure;
            diastolicDetected = true;
            Serial.println("Diastolic detected!");
        }
    }
}

// Low-pass filter (EMA)
double lowPassFilter(double newReading, bool reset = false) {
    static double smoothed = 0;
    static double alpha = 0.1;  // Smoothing factor

    if (reset) {
        smoothed = newReading;  // Start fresh from current value
        return smoothed;
    }

    smoothed = alpha * newReading + (1 - alpha) * smoothed;
    return smoothed;
}

// High-pass filter (simple difference)
double highPassFilter(double newReading, bool reset = false) {
    static double lastReading = 0;

    if (reset) {
        lastReading = newReading;
        return 0;
    }

    double knock = newReading - lastReading;
    lastReading = newReading;
    return abs(knock);
}

void resetFilters(double initialValue) {
    lowPassFilter(initialValue, true);
    highPassFilter(initialValue, true);
}



void setup() {
  // Initalize UART, I2C bus, and connect to the micropressure sensor
  Serial.begin(115200);
  Wire.begin();

  if(!mpr.begin())
  {
    Serial.println("Cannot connect to MicroPressure sensor.");
    while(1);
  }

  // Setup BLE
  NimBLEDevice::init("ESP32_NimBLE");

  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // Command characteristic (Client writes to it)
  commandCharacteristic = pService->createCharacteristic(
      COMMAND_CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::WRITE);
  commandCharacteristic->setCallbacks(new CommandCallbacks());  // Use our custom subclass

systolicCharacteristic = pService->createCharacteristic(
    SYSTOLIC_CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
systolicCharacteristic->setValue("0"); // Default value

diastolicCharacteristic = pService->createCharacteristic(
    DIASTOLIC_CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
diastolicCharacteristic->setValue("0"); // Default value

  pService->start();

  // Start BLE Advertising
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setName("ESP32_NimBLE");
  pAdvertising->start();
  
  Serial.println("BLE Server started.");
}

//  Main Blood Pressure Function

void findBloodPressure(){
  systolicDetected = false;
  diastolicDetected = false;
  pressurize();
  double initialPressure = mpr.readPressure(INHG) / 25.4;
  resetFilters(initialPressure);
  while(true){
    double raw_pressure_mmhg = mpr.readPressure(INHG) / 25.4; // Convert inches to mm
    Serial.println(raw_pressure_mmhg, 6);
    double filteredPressure = lowPassFilter(raw_pressure_mmhg);
    double knock = highPassFilter(raw_pressure_mmhg);  // Extract Korotkoff sounds

    detectBloodPressure(filteredPressure, knock);
    if (systolicDetected && diastolicDetected) {
        Serial.print("Systolic: "); Serial.println(systolicPressure);
        Serial.print("Diastolic: "); Serial.println(diastolicPressure);
        return;
    }
    delay(10); // Small Delay
  }
}

void loop() {
  // Nothing in the loop
}
