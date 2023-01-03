#include <Arduino.h>
#include <ArduinoJson.h>
#include "utils.h"
#include "pins.h"

#include <FS.h> //this needs to be first, or it all crashes and burns...
#ifdef ESP32
#include <SPIFFS.h>
#endif
#define fileConfig "/configuration.json"
#define payloadSize 2048
#include "mqtt.h"

String payloadString;
DynamicJsonDocument configJson(payloadSize);

int getPinIdOutput(int softwarePin)
{

  if (!configJson.containsKey("configs"))
    return -1;
  else if (!configJson["configs"].containsKey("pins"))
    return -1;

  JsonArray pinsArray = configJson["configs"]["pins"].as<JsonArray>();
  for (JsonVariant pin : pinsArray)
  {
    if (pin["softwarePin"].as<int>() == softwarePin)
      return pin["hardwarePin"].as<int>();
  }
  return -1;
}

void writeJsonMemory(DynamicJsonDocument messageJson)
{

  // writing
  File configFile = SPIFFS.open(fileConfig, "w");
  if (!configFile)
  {
    Serial.println("failed to open config file for writing");
  }
  // Serial.println("writing in memory: \n");
  // serializeJson(messageJson, Serial);
  serializeJson(messageJson, configFile);
  configFile.close();
}

void configPins()
{

  // extract the values
  JsonArray pinsArray = configJson["configs"]["pins"].as<JsonArray>();
  for (JsonVariant pin : pinsArray)
  {

    // Serial.println("software Pin: " + pin["softwarePin"].as<String>());
    if (pin["mode"].as<String>().equals("out"))
    {
      pinMode(pin["hardwarePin"].as<int>(), OUTPUT);
      digitalWrite(pin["hardwarePin"].as<int>(), !pin["state"].as<int>());
    }
    else if (pin["mode"].as<String>().equals("out"))
      pinMode(pin["hardwarePin"].as<int>(), INPUT);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("device id: " + String(deviceID()) + String("##"));

  // SPIFFS.format();
  // Serial.println("Formated...");
  // read configuration from FS json
  Serial.println("mounting FS ...");
  if (SPIFFS.begin())
  {
    Serial.println("mounted file system.");
    if (SPIFFS.exists(fileConfig))
    {
      Serial.println("reading config file");
      File configFile = SPIFFS.open(fileConfig, "r");
      if (configFile)
      {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);

        auto deserializeError = deserializeJson(configJson, buf.get());
        if (!deserializeError)
        {
          serializeJson(configJson, Serial);
          configPins();
        }
        else
          Serial.println("failed to load json config");

        configFile.close();
      }
    }
    else
      Serial.println("File not exist yet.");
  }
  else
    Serial.println("failed to mount FS.");

  mqttSetup(configJson);
}

void updateStateConfigMemory(int hardwarePin, int state)
{

  JsonArray pinsArray = configJson["configs"]["pins"].as<JsonArray>();
  for (int i = 0; i < pinsArray.size(); i++)
  {
    if (configJson["configs"]["pins"][i]["hardwarePin"].as<int>() == hardwarePin)
      configJson["configs"]["pins"][i]["state"] = state;
  }

  // NEED SERIALIZATION AND DESERIALIZATION O.o
  String configJsonStr;
  serializeJson(configJson, configJsonStr);
  deserializeJson(configJson, configJsonStr);

  writeJsonMemory(configJson);

  // report
}

void loop()
{
  if (Serial.available())
  {

    // CHECK WORD CODE
    payloadString = Serial.readString();
    if (payloadString[payloadString.length() - 1] == '#' && payloadString[payloadString.length() - 2] == '#')
    {
      payloadString = payloadString.substring(0, payloadString.length() - 2);
      Serial.println("inputString :" + String(payloadString));
      DynamicJsonDocument payloadJson(payloadSize);
      deserializeJson(payloadJson, payloadString);

      // CONFIG
      if (payloadJson.containsKey("configs"))
      {
        configJson = payloadJson;
        // payloadJson.clear();
        // CHECK id
        if (configJson["configs"]["id"].as<String>().equals(deviceID()))
        {
          configPins();
          writeJsonMemory(configJson);
        }
      } // STATE
      else if (payloadJson.containsKey("state"))
      {

        // CHECK Desired
        if (payloadJson["state"].containsKey("desired"))
        {

          JsonObject fileJsonObj = payloadJson["state"]["desired"].as<JsonObject>();
          for (JsonPair jsonPair : fileJsonObj)
          {

            int softwarePin = String(jsonPair.key().c_str()).substring(3).toInt(); // jsonPair.key = "pin12" -> result = 12
            int hardwarePin = getPinIdOutput(softwarePin);
            Serial.println("softwarePin: " + String(softwarePin));
            Serial.println("hardwarePin: " + String(hardwarePin));

            if (hardwarePin >= 0)
            {

              // update pin output
              String state = jsonPair.value();
              if (state != "1" && state != "0")
                state = (String)digitalRead(hardwarePin);

              pinMode(hardwarePin, OUTPUT);
              digitalWrite(hardwarePin, !state.toInt());

              // // UPDATE configs
              updateStateConfigMemory(hardwarePin, state.toInt());

              String msg = "\"state\":{\"reported\":{\"" + String(jsonPair.key().c_str()) + "\": " + state + "}}";
              client.publish(pub_topic.c_str(), msg.c_str());

              // Serial.println("\"state\":{\"reported\":{\"" + String(jsonPair.key().c_str()) + "\": " + state + "}}##");
            }
          }
        }
      }
    }
  }

  mqttLoop();
}