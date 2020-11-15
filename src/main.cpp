#include <Arduino.h>
#include "config.h"
#include "TTN.h"

bool packetSent, packetQueued;
long lastMessage = 0;

void callback(uint8_t message)
{
  if (EV_JOINING == message)
    DEBUG_MSG("Joining TTN...\n");
  if (EV_JOINED == message)
  {
    DEBUG_MSG("TTN joined!\n");
  }
  if (EV_JOIN_FAILED == message)
    DEBUG_MSG("TTN join failed\n");
  if (EV_REJOIN_FAILED == message)
    DEBUG_MSG("TTN rejoin failed\n");
  if (EV_RESET == message)
    DEBUG_MSG("Reset TTN connection\n");
  if (EV_LINK_DEAD == message)
    DEBUG_MSG("TTN link dead\n");
  if (EV_MSG_ACK == message)
    DEBUG_MSG("ACK received\n");
  if (EV_MSG_PENDING == message)
    DEBUG_MSG("Message discarded\n");
  if (EV_MSG_QUEUED == message)
    DEBUG_MSG("Message queued\n");

  // We only want to say 'packetSent' for our packets (not packets needed for joining)
  if (EV_TXCOMPLETE == message && packetQueued)
  {
    DEBUG_MSG("Message sent\n");
    packetQueued = false;
    packetSent = true;
  }

  if (EV_MSG_RESPONSE == message)
  {

    DEBUG_MSG("[TTN] Response: ");

    size_t len = TTN.responseLen();
    uint8_t data[len];
    TTN.getResponse(data, len);

    char buffer[6];
    for (uint8_t i = 0; i < len; i++)
    {
      snprintf(buffer, sizeof(buffer), "%02X", data[i]);
      DEBUG_MSG(buffer);
    }
    DEBUG_MSG("\n");
  }
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  TTN.registerCallback(callback);
  // put your setup code here, to run once:
  if (!TTN.begin(SCK_GPIO, MISO_GPIO, MOSI_GPIO, NSS_GPIO, RESET_GPIO, DIO0_GPIO, DIO1_GPIO, DIO2_GPIO))
  {
    DEBUG_MSG("[ERR] Radio module not found!\n");
  }
  else
  {
    TTN.useABP(DEVADDR, APPSKEY, NWKSKEY);
    TTN.setSpreadingFactor(DR_SF7);
    TTN.setAdrEnabled(1);
    TTN.join();
  }
}

void loop()
{
  // Run the TTN loop !!IMPORTANT!!
  TTN.loop();

  //send a message every 30 seconds
  if (millis() - lastMessage > SEND_INTERVAL_MS)
  {
    lastMessage = millis();
    //If a packet is still queued, do not send another.
    if (packetQueued)
      return;

    uint8_t buffer[4] = {0x01, 0x02, 0x03, 0x04};

    //Send a unconfirmed uplink message to port 1
    TTN.sendMessage(buffer, 4, 1, false);

    //Set the packetQueued to true
    packetQueued = true;
  }
}