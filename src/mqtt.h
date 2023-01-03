/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

String ssid = "TOCK AUTO";
String password = "tocktecnologia";
String mqtt_server = "192.168.0.32";
String pub_topic = "tock-commands";
String sub_topic = "tock-commands";
String broker_user = "tocktec.com.br";
String broker_pass = "tock30130tecnologia";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
long lastReconnectAttempt = 0;
long timeToConnect = 5000;
int countWiFiDisconnection = 0;

void setup_wifi(String ssid, String password)
{

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.print("password to ");
    Serial.println(password);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        count++;
        if (count > 200)
        {
            Serial.println("timeout ...");
            return;
        }
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

boolean reconnectMqtt()
{

    if (WiFi.status() != WL_CONNECTED)
    {
        countWiFiDisconnection++;
        if (countWiFiDisconnection >= 3)
            setup_wifi(ssid, password);
    }

    String thingName = deviceID();
    String clientId = "TOCK-" + thingName;
    Serial.print(String(" Try connecting to ") + mqtt_server + String(" ...\n"));

    if (client.connect(clientId.c_str(), broker_user.c_str(), broker_pass.c_str()))
    {
        Serial.println(clientId + String("connected. Broker ip: ") + mqtt_server + "!");
        client.publish(pub_topic.c_str(), (thingName + String(" connected")).c_str());
        // reportStates();
        client.subscribe(sub_topic.c_str());
    }

    return client.connected();
}

void paramsSetup(DynamicJsonDocument &configJson)
{
    if (configJson["configs"].containsKey("ssid"))
    {
        ssid = configJson["configs"]["ssid"].as<String>();
    }
    if (configJson["configs"].containsKey("password"))
    {
        password = configJson["configs"]["password"].as<String>();
    }
    if (configJson["configs"].containsKey("broker"))
    {
        mqtt_server = configJson["configs"]["broker"].as<String>();
    }
    if (configJson["configs"].containsKey("broker_user"))
    {
        broker_user = configJson["configs"]["broker_user"].as<String>();
    }
    if (configJson["configs"].containsKey("broker_pass"))
    {
        broker_pass = configJson["configs"]["broker_pass"].as<String>();
    }
}

void mqttSetup(DynamicJsonDocument &configJson)
{
    paramsSetup(configJson);

    setup_wifi(ssid.c_str(), password.c_str());
    client.setServer(mqtt_server.c_str(), 1883);
    lastReconnectAttempt = 0;
}

void mqttLoop()
{

    // if (!client.connected())
    // {
    //     // reconnect();
    //     reconnectMqtt();
    // }
    // client.loop();

    if (!client.connected())
    {
        long now = millis();
        if (now - lastReconnectAttempt > timeToConnect)
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnectMqtt())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        // Client connected
        client.loop();
    }
}
