#include "NimBLEDevice.h"

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "ABCD"
#define COMMAND_CHARACTERISTIC_UUID "1234"
#define SENSOR_CHARACTERISTIC_UUID "5678"

NimBLECharacteristic *commandCharacteristic;
NimBLECharacteristic *sensorCharacteristic;

// Create a subclass for handling write events
class CommandCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override {
        std::string receivedValue = pCharacteristic->getValue();
        Serial.print("Received command: ");
        Serial.println(receivedValue.c_str());

        if (receivedValue == "READ_SENSOR") {
            //int sensorValue = analogRead(A0);  // Replace with actual sensor reading
            int sensorValue = 9;
            String sensorStr = String(sensorValue);
            sensorCharacteristic->setValue(sensorStr.c_str());
            sensorCharacteristic->notify();
            Serial.println("Sensor value sent!");
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("hello, world!");

    NimBLEDevice::init("ESP32_NimBLE");

    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Command characteristic (Client writes to it)
    commandCharacteristic = pService->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    commandCharacteristic->setCallbacks(new CommandCallbacks());  // Use our custom subclass

    // Sensor characteristic (Client reads and gets notified)
    sensorCharacteristic = pService->createCharacteristic(
        SENSOR_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    sensorCharacteristic->setValue("0"); // Default value

    pService->start();

    // Start BLE Advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setName("ESP32_NimBLE");
    pAdvertising->start();
    
    Serial.println("BLE Server started.");
}

void loop() {
    // No need for a loop, handled in callbacks
}
