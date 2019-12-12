
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* etatCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t valueNotify = 0;
uint32_t isOn = 0;
int LED = 2;
int MOTEUR = 25;


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_NOTIFY "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_WRITE "dd5befed-000f-483c-9fac-ef4faca741ca"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *commandeCharacteristic) {
      std::string value = commandeCharacteristic->getValue();

      if (value== "0") {
        Serial.println("*********");
        Serial.print("VIBREUR OFF");
        digitalWrite(MOTEUR, LOW);
        digitalWrite(LED, LOW);
        isOn = 0;
      }
      else if (value== "1") {        
        Serial.println("*********");
        Serial.print("VIBREUR ON");
        digitalWrite(MOTEUR, HIGH);
        digitalWrite(LED, HIGH);
        isOn = 1;
      }
      
      else {
        if (value.length() > 0) {
          Serial.println("*********");
          Serial.print("Valeur non attendue: ");
          for (int i = 0; i < value.length(); i++)
            Serial.print(value[i]);
  
          Serial.println();
          Serial.println("*********");
        }
      }


      
    }
};


void setup() {
  
  pinMode (LED, OUTPUT);
  pinMode (MOTEUR, OUTPUT);

  
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create notify BLE Characteristic
  etatCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_NOTIFY,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  etatCharacteristic->addDescriptor(new BLE2902());


  // Create write BLE Characteristic
  BLECharacteristic *commandeCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_WRITE,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  commandeCharacteristic->setCallbacks(new MyCallbacks());

  commandeCharacteristic->setValue("0");
  

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set valueNotify to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}




void loop() {
    // notify changed valueNotify
    if (deviceConnected) {
        etatCharacteristic->setValue((uint8_t*)&valueNotify, 4);
        etatCharacteristic->notify();
        valueNotify++;
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    if (isOn){
      //le vibreur s'allume
      

      
    }

    
}
