
#include <ArduinoJson.h>

DynamicJsonDocument wroomPins(int size)
{
    DynamicJsonDocument jsonPins(size);

    // ##### Lado 1 ###############################################
    /*1*/ // GND
    /*2*/ jsonPins["1"] = 23;
    /*3*/ jsonPins["2"] = 22;
    /*4*/ // TX
    /*5*/ // RX
    /*6*/ jsonPins["3"] = 21;
    /*7*/ // GND
    /*8*/ jsonPins["4"] = 19;
    /*9*/ jsonPins["5"] = 18;
    /*10*/ jsonPins["6"] = 5;
    /*11*/ jsonPins["7"] = 17;
    /*12*/ jsonPins["8"] = 16;
    /*13*/ jsonPins["9"] = 4;
    /*14*/ jsonPins["10"] = 0;
    /*15*/ jsonPins["11"] = 2;
    /*16*/ jsonPins["12"] = 15;
    /*17*/ // jsonPins["13"] = 8;  // CANT USE THIS PIN
    /*18*/ // jsonPins["14"] = 7; // CANT USE THIS PIN
    /*19*/ // jsonPins["15"] = 6; // CANT USE THIS PIN

    // ##### Lado 2 #################################################
    /*1*/ // VCC 5V
    /*2*/ // jsonPins["16"] = 11;  // CANT USE THIS PIN (CRASH)
    /*3*/ // jsonPins["17"] = 10;  // CANT USE THIS PIN (CRASH)
    /*4*/ // jsonPins["18"] = 9;   // CANT USE THIS PIN (CRASH)
    /*5*/ jsonPins["19"] = 13;
    /*6*/ // GND
    /*7*/ jsonPins["20"] = 12;
    /*8*/ jsonPins["21"] = 14;
    /*9*/ jsonPins["22"] = 27;
    /*10*/ jsonPins["23"] = 26;
    /*11*/ jsonPins["24"] = 25;
    /*12*/ jsonPins["25"] = 33;
    /*13*/ jsonPins["26"] = 32;
    /*14*/ // jsonPins["27"] = 35; //JUST CAN INPUT MODE (CRASH)
    /*15*/ // jsonPins["28"] = 34; //JUST CAN INPUT MODE (CRASH)
    /*16*/ // jsonPins["29"] = 39; // JUST CAN INPUT MODE
    /*17*/ // jsonPins["30"] = 36; // JUST CAN INPUT MODE
    /*18*/ // EN
    /*19*/ // VCC 3v3

    return jsonPins;
}

void wromTestPins(DynamicJsonDocument jsonPins)
{
    JsonObject jsonPinsObj;
    jsonPinsObj = jsonPins.as<JsonObject>();
    int aux = 0;
    for (JsonPair jsonPair : jsonPinsObj)
    {

        int softwarePin = atoi(jsonPair.key().c_str());
        int hardwarePin = jsonPair.value().as<String>().toInt();
        Serial.println("pin Sotfware: " + String(softwarePin));
        Serial.println("pin Hardware: " + String(hardwarePin));

        // TEST pinmode in/out
        Serial.println("> pinmode in output ...: " + String(jsonPair.key().c_str()));
        pinMode(hardwarePin, OUTPUT);
        Serial.println("> writing HIGH ...: ");
        digitalWrite(hardwarePin, HIGH);

        Serial.println("> pinmode in input ...: ");
        pinMode(hardwarePin, INPUT);
        Serial.println("> reading value: " + String(digitalRead(hardwarePin)));
        Serial.println();

        aux++;
        delay(500);
    }
    Serial.println("Test finished, " + String(aux) + String("pins available!"));
}

void wroomConfigPins(DynamicJsonDocument jsonPins)
{
}