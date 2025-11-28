/*
The LoRa communication code for this project was adapted from the Heltec Ping Pong example provided in the Heltec ESP32 LoRa library [1].
The original example demonstrates basic peer-to-peer LoRa transmission and reception. For this project, the code was modified to handle parking 
spot occupancy detection, including processing user input to update LED status and sending the corresponding state over LoRa.”
*/
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "esp_task_wdt.h"

#define RF_FREQUENCY        902500000
#define TX_OUTPUT_POWER     5
#define LORA_BANDWIDTH      0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE     1
#define LORA_PREAMBLE_LENGTH 8

#define LED_PIN 35
#define BUFFER_SIZE 64
#define WDT_TIMEOUT 10

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
bool occupied = false;
static RadioEvents_t RadioEvents;

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX
} States_t;

States_t state = STATE_IDLE;

// === SETUP ===
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Configure Task WDT
  esp_task_wdt_config_t wdt_config = {};
  wdt_config.timeout_ms = WDT_TIMEOUT * 1000;  // timeout in milliseconds
  esp_task_wdt_init(&wdt_config);

  // Add current task (loop) to WDT
  esp_task_wdt_add(NULL);
   
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, false,
                    true, 0, 0, false, 3000);

  Serial.println("Enter O for Occupied, E for Empty:");
}


// === LOOP ===
void loop() {

  esp_task_wdt_reset();

  // Check for user input
  if (Serial.available() > 0) {
    char input = Serial.read();

    if (input == 'O') {
      digitalWrite(LED_PIN, LOW);
      sprintf(txpacket, "OCCUPIED");
      state = STATE_TX;
    }
    else if (input == 'E') {
      digitalWrite(LED_PIN, HIGH);
      sprintf(txpacket, "EMPTY");
      state = STATE_TX;
    }

    // Ignore other characters
  }

  // Handle transmission
  if (state == STATE_TX) {
    Serial.printf("Sending: %s\n", txpacket);
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    state = STATE_IDLE;
  }

  // Process radio IRQs
  Radio.IrqProcess();
}


// === CALLBACKS ===
void OnTxDone(void) {
  Serial.println("TX Done");
  state = STATE_RX;
  Radio.Rx(0);
}


void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  payload[size] = '\0';
  Serial.printf("Received: %s | RSSI: %d\n", payload, rssi);
  char command = payload[0];

  if(command == 'R'){
    occupied = true;
    digitalWrite(LED_PIN, LOW);
    Serial.println("STATE: OCCUPIED");
     
  }
  state = STATE_IDLE;
}
