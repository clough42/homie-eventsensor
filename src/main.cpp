/*
 * Homie event sensor
 *
 * Uses Homie-ESP8266 framework to connect to the network and MQTT and report
 * an event.  It is assumed that the ESP8266 will be wired up so the event
 * resets the microcontroller, it boots and reports and then goes to deep sleep
 * forever (until the next reset).
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Homie.h>

// ESP-01
ADC_MODE(ADC_VCC);
#define VOLTAGE_COEFFICIENT 0.0010347

#define PROPERTY_VOLTAGE "voltage"
#define PROPERTY_TRIGGERED "triggered"

bool reported = false;
bool sleepRequested = false;

HomieNode batteryNode("battery2xAA", "voltage");
HomieNode eventNode("event", "event");

/*
 * Called once when Homie is ready to operate
 */
void setupHandler()
{

}

/*
 * Looped while Homie is online
 */
void loopHandler()
{
  if( ! reported ) {
    Homie.setNodeProperty(batteryNode, PROPERTY_VOLTAGE).send(String(VOLTAGE_COEFFICIENT * ESP.getVcc()));
    Homie.setNodeProperty(eventNode, PROPERTY_TRIGGERED).send("1");
    reported = true;
  } else {
    if( ! sleepRequested ) {
      sleepRequested = true;
      Homie.prepareForSleep();
    }
  }
}

/*
* Handle Homie events
 */
void eventHandler(HomieEvent event)
{
  if( event == HomieEvent::READY_FOR_SLEEP ) {
    Serial.println("Going to sleep...");
    ESP.deepSleep(0);
  }
}

void setup()
{
  Serial.begin(9600);

  Homie_setFirmware("eventsensor", "1.0.0");
  Homie_setBrand("clough42");

  batteryNode.advertise(PROPERTY_VOLTAGE);
  eventNode.advertise(PROPERTY_TRIGGERED);

  Homie
    .disableLedFeedback()
    .disableResetTrigger()
    .setSetupFunction(setupHandler)
    .setLoopFunction(loopHandler)
    .onEvent(eventHandler)
    .setup();
}

void loop()
{
  Homie.loop();
}
