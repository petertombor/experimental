/**
 * SPI port-ok az arduinokon
 Uno           Mega
 11       -      51            (MOSI)
 12       -      50            (MISO)
 13       -      52            (SCK)
 10       -      53            (CSN) !!!sorrend nem bizt jó !! TODO bef
 9         -      48 (Your choice) (CE)
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <MyRiasztoLakasUtil.h>
#include <Metro.h>
#include <LowPower.h>


RF24 radio(/*ce*/8, /*cs*/7); // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio); // Network uses that radio

int maxTryNum = 500;
int eventId = 0;
int lastEventId = 0;


void setupIRF(){
	//Serial.begin(57600);
	//Serial.println("Erzekelo init begin, V1.0");

	SPI.begin();
	//Serial.println("init 1");

	radio.begin();
	//Serial.println("init 2");

	network.begin(radioChannel, radioFentiErzekeloNode);
	//Serial.println("init 3");
	//Serial.println("Erzekelo init end");
}


void sendMessage(){

	network.update();

	//Serial.print("mozgas, eventId: ");
	//Serial.println(eventId);
	//Serial.println("Sending begin");

	payload_t payload = { payloadCode_movement, 1 };
	RF24NetworkHeader header(radioServerNode);
	int errCount = send(network, header, &payload, maxTryNum);
	//Serial.print("errCount: ");
	//Serial.println(errCount);
	//Serial.println("Sending end");
}


void setup(void) {
	delay(1000);
	setupIRF();

	sendMessage();
	delay(1000);

	sendMessage();
	delay(1000);

	sendMessage();
	delay(1000);
}


void loop(void) {
	// Enter power down state with ADC and BOD module disabled.
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	delay(1000);
}

