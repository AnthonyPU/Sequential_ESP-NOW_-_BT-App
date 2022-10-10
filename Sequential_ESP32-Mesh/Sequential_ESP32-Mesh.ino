#include "Settings.h"
#include <esp_now.h>
#include <WiFi.h>

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;



bool state=0;
bool new_data = 0; // Flag to send data only once

// Universal MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool incomingLED_status;
char devLetter;

// Structure to send data
// Must match the receiver structure
typedef struct struct_message
{
  char device;
  bool avbl[20];
  bool state[20];
} struct_message;

// Create a struct_message called myData
struct_message myData;

struct_message incomingReadings;
// Create peer interface
esp_now_peer_info_t peerInfo;

esp_now_peer_num_t peer_num;
    

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  devLetter = incomingReadings.device;
  //Serial.println(incomingReadings.device);
  
  for(int i;i<20;i++){
    if(incomingReadings.avbl[i]!=myData.avbl[i]){
      myData.avbl[i]=1;
    }
    if(int(id)==65+i){
      //state=incomingReadings.state[i];
    }
      
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Send Callback Function
  esp_now_register_send_cb(OnDataSent);

  // Receive Callback Function
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  //Enviar el estado del dispositivo

  for(int i;i<20;i++){
    if(int(id)==65+i){
      myData.avbl[i]=1;
    }
  }
  //send
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  SerialBT.begin(String(id)); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop()
{
  //Serial.println(state);
  // Set values to send
  Serial.print("Dispositivos disponibles: ");
  for(int i;i<20;i++){
    Serial.print(myData.avbl[i]);
  }
  Serial.print("\n");
  if (SerialBT.available()) {
    
     char incomingChar = SerialBT.read();
     Serial.println(incomingChar);
    if(incomingChar =='1'){
      myData.state[int('A'-65)] = !myData.state[int('A'-65)] ;
      Serial.print("El estado de A es: ");
      Serial.println(myData.state[int('A'-65)]);
      state=myData.state[int('A'-65)];
    };
    if(incomingChar =='2'){
      myData.state[int('B'-65)] = !myData.state[int('B'-65)] ;
    };
    new_data = 0;
  }
  if (state)
  {
    Serial.println("gaaaa");
    digitalWrite(LED,1);
    new_data = 0;
  }
  else{
    digitalWrite(LED,0);
    new_data = 0;
  }

  esp_err_t result; // declaration

  // Send message via ESP-NOW
  if (new_data == 0)
  {
    result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    new_data = 1;
  }

  if (result == ESP_OK) 
  {
    //Serial.println("Sent with success");
  }
  else
  {
    //Serial.println("Error sending the data");
  }
}
