/**
 *
 * Innen szedtem le
 * http://playground.arduino.cc/Code/Keypad#Download
 *
 * A fekete nyomogomb lábkiosztása
 *
 Keypad lábainak rövidrezárása nyomógomb esetén
 Sor, oszlopok: keypad kivezetései
 [szám]: nyomogómb

 |    3   1   5
 --------------
 2 | [1] [2] [3]
 7 | [4] [5] [6]
 6 | [7] [8] [9]
 4 | [*] [0] [#]
 *
 * sorok: 2, 7, 6, 4
 * oszlopok: 3, 1, 5
 **/

#include <Keypad.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <MyRiasztoLakasUtil.h>
#include <Metro.h>


/*
 * KEZELO BEGIN
 */
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = { { '1', '2', '3' }, { '4', '5', '6' },
		{ '7', '8', '9' }, { '*', '0', '#' } };
/**
 * A sorok és lábak bekötése: A nyomogomb sorait, oszlopait melyik arduino pin-re kötöttem.
 * Fordított bekötés esetén: sorok: 3, 8, 7, 5, oszlopok: 4, 2, 6
 * Egyenes bekötés esetén:
 */
byte rowPins[ROWS] = { 9 - 2, 9 - 7, 9 - 6, 9 - 4 }; //connect to the row pinouts of the keypad 3, 8, 7, 5
byte colPins[COLS] = { 9 - 3, 9 - 1, 9 - 5 }; //connect to the column pinouts of the keypad 4, 2, 6

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/*
 * RF24 BEGIN
 *
 * SPI port-ok az arduinokon
 Uno           Mega
 11       -      51            (MOSI)
 12       -      50            (MISO)
 13       -      52            (SCK)
 10       -      53            (CSN)
 9         -      48 (Your choice) (CE)
 */
RF24 radio(/*ce*/10, /*cs*/9); // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio); // Network uses that radio
int maxTryNum = 10;
uint16_t lastId = 0;

//
// portok
//
#define port_buzzer	A0

int noiseFrequency = 1000;
int noiseDownCounter = 0;
int noiseDuration = 300;

Metro metro = Metro(1000);


void setup() {
	Serial.begin(56700);
	Serial.println("Kezelo init begin");

	SPI.begin();
	Serial.println("init 1");

	radio.begin();
	Serial.println("init 2");

	network.begin(radioChannel, radioKezeloNode);
	Serial.println("init 3");

	pinMode(A0, OUTPUT);
	tone(port_buzzer, 1000, 100);
	Serial.println("Kezelo init end");
}


void loop() {

	network.update();

	int key = keypad.getKey();
	if (key != NO_KEY) {
		payload_t payload = { payloadCode_keyPressed, key };
		RF24NetworkHeader header(radioServerNode);
		int errCount = send(network, header, &payload, maxTryNum);
		//tone(port_buzzer, 3000, 150);
		Serial.print("Sending key, errCount: ");
		Serial.println(errCount);
	}


	while (network.available()) { // Is there anything ready for us?

		RF24NetworkHeader header; // If so, grab it and print it out
		payload_t payload;
		network.read(header, &payload, sizeof(payload));

		if (lastId != header.id){
			lastId = header.id;

			Serial.print("Received packet from server, code:");
			Serial.print(payload.code);
			Serial.print(", value: ");
			Serial.println(payload.value);

			/*
			if (payload.code == payloadCode_serverAlarmingStatus){
				int serverStatus = payload.value;
				if (serverStatus == status_kiriasztva){
					noiseFrequency = 5000;
					noiseDownCounter = 3;
					noiseDuration = 300;
				} else {
					noiseFrequency = 3000;
					noiseDownCounter = 60;
					noiseDuration = 150;
				}
			}
			else
			*/

			if (payload.code == payloadCode_serverStatus){
				int serverStatus = payload.value;
				Serial.print("Server status: ");

				String statusString = getStatusAsString(serverStatus);
				Serial.print(statusString);
				Serial.print(", code: ");
				Serial.println(serverStatus);

				if (serverStatus == status_kiriasztva){
					noiseFrequency = 5000;
					noiseDownCounter = 1;
					noiseDuration = 300;
				}
				else if (serverStatus == status_elesitve){
					noiseFrequency = 7000;
					noiseDownCounter = 7;
					noiseDuration = 200;
				}
				else {
					noiseFrequency = 3000;
					noiseDownCounter = 3;
					noiseDuration = 200;
				}
			}

			else if (payload.code == payloadCode_ok){
				noiseFrequency = 100;
				noiseDownCounter = 1;
				noiseDuration = 100;
			}

			else if (payload.code == payloadCode_nok){
				noiseFrequency = 100;
				noiseDownCounter = 1;
				noiseDuration = 1000;
			}
			else if (payload.code == payloadCode_beep){
				noiseFrequency = 1000;
				noiseDownCounter = 1;
				noiseDuration = 100;
			}
		} else {
	    	Serial.print("Already received, id: ");
	    	Serial.println(header.id);
	    }
	}

	if (noiseDownCounter > 0 && (metro.check() == 1) ){
		metro.reset();
		tone(port_buzzer, noiseFrequency, noiseDuration);
		noiseDownCounter--;
	}

}

