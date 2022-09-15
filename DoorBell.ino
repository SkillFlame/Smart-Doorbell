/*******************************************************************
  Smart Doorbell using an esp32

  Sends a message through telegram and 
  plays a sound through a speaker connected via i2s
 *******************************************************************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "AudioTools.h"
#include "Audio.h"


#define TELEGRAM_BUTTON_PIN 0

#define AUDIO_BCK_PIN 15
#define AUDIO_WS_PIN 18
#define AUDIO_DATA_PIN 12


// Wifi network station credentials
#define WIFI_SSID "*******"
#define WIFI_PASSWORD "*******"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "**********:******************************"

//Audio settings
uint8_t channels = 1;
uint16_t sample_rate = 32000;

// Use @myidbot (IDBot) to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "******"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

volatile bool telegramButtonPressedFlag = false;

I2SStream i2s;  // Output to I2S
MemoryStream music(audio_raw, audio_raw_len);
StreamCopyT<int16_t> copier(i2s, music); // copies sound into i2s


void setup() {
  Serial.begin(115200);

  pinMode(TELEGRAM_BUTTON_PIN, INPUT);
  attachInterrupt(TELEGRAM_BUTTON_PIN, telegramButtonPressed, RISING);

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());


  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  auto config = i2s.defaultConfig(TX_MODE);
  config.pin_bck = AUDIO_BCK_PIN;
  config.pin_ws = AUDIO_WS_PIN;
  config.pin_data = AUDIO_DATA_PIN;
  config.sample_rate = sample_rate;
  config.channels = channels;
  config.bits_per_sample = 16;
  i2s.begin(config);
}

void telegramButtonPressed() {
  Serial.println("telegramButtonPressed");
  int button = digitalRead(TELEGRAM_BUTTON_PIN);
  if(button == HIGH)
  {
    telegramButtonPressedFlag = true;
  }
  return;
}

void sendTelegramMessage() {
  String message = "DingDong";
  message.concat("\n");
  if(bot.sendMessage(CHAT_ID, message, "Markdown")){
    Serial.println("TELEGRAM Successfully sent");
  }
  telegramButtonPressedFlag = false;
}

void playSound() {
  MemoryStream music(audio_raw, audio_raw_len);
  StreamCopyT<int16_t> copier(i2s, music);
  
  for(;;) {
    if (!copier.copy2()){
      break;
    }
  }
}

void loop() {
  if ( telegramButtonPressedFlag ) {
    sendTelegramMessage();
    playSound();
  }
}
