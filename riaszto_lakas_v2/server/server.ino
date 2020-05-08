#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <MyRiasztoLakasUtil.h>
#include <MyFifoUtil.h>
//#include <Metro.h>

#include "SIM900.h"
#include <SoftwareSerial.h>


//9,10

/* peti arduino bekotese
 * 	 ce		8
 	 csn	7
 * */

//8,7
RF24 radio(10,6); // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio); // Network uses that radio
int maxTryNum = 100;
uint16_t lastId = 0;

//const uint16_t this_node = 0;    // Address of our node
//const uint16_t other_node = 1;   // Address of the other node

const int fifoSize = 4;
MyFifoUtil fifo = MyFifoUtil(fifoSize);
int status;
long statusSetTime = 0;
long now;
const long elesedesiIdoMillis = 60000;
const long kiriasztasTurelmiIdoMillis = 20000;
int counter;

//Metro metro = Metro(1000);

void setup(void) {
	Serial.begin(57600);
	//Serial.println("Server init begin, V2017.12.03 ");

	SPI.begin();
	radio.begin();
	network.begin(radioChannel, radioServerNode);

	status = status_kiriasztva;
	statusSetTime = millis();

	//az elso hivas elott inicializalom
    if (gsm.begin(9600)){
      //Serial.println("\nstatus=READY");
      
      //Speaker bekapcsolasa
      //byte on = 1;
      //gsm.SetSpeaker(on);
      
    } else {
      //Serial.println("\nstatus=IDLE");
    }


    //Default legyen beelesitve !!!
    statusSetTime = millis();
    status = status_elesites_inditasa;
    

    //Serial.println("Server init end");

    counter=0;
}



void loop(void) {

	now = millis();

	network.update();

	while (network.available()) {
		RF24NetworkHeader header;
		payload_t payload;
		network.read(header, &payload, sizeof(payload));

		if (lastId != header.id){
	    	lastId = header.id;

			//Serial.print("Received packet, code:");
			//Serial.print(payload.code);
			//Serial.print(", value: ");
			//Serial.println(payload.value);

			//Kezelotol erkezett gombnyomas
			if (payload.code == payloadCode_keyPressed){
  
				if (payload.value == '*') {
					//jelszo osszehasonlitasa
					char buffer[ 100 /*fifoSize + 1*/];
					int size = fifo.getBuffer((byte*) &buffer[0]);

					//Serial.print("fifoBuffer.size: ");
					//Serial.println(size);

					buffer[fifoSize] = '\0';

					//Serial.print("fifo buffer: ");
					//Serial.println(buffer);

					fifo.clearBuffer();

					payload_t payloadResp;

                                        //Ha jó a beírt jelszó
					if (strcmp(buffer, "1234") == 0) {
						statusSetTime = millis();

						if (status == status_kiriasztva){
							status = status_elesites_inditasa;
							//Serial.println("elesites inditasa");
						} else {
							status = status_kiriasztva;
							//Serial.println("kiriasztva");
						}

						payloadResp.code = payloadCode_serverStatus;
						payloadResp.value = status;
					} else {
						payloadResp.code = payloadCode_nok;
						payloadResp.value = 2;
						//Serial.println("helytelen password");
					}
					RF24NetworkHeader headerResponse(header.from_node);
					//bool ok = network.write(headerResponse, &payloadResp, sizeof(payloadResp));

                                        //Uzenet a kliensnek
					int errCount = send(network, headerResponse, &payloadResp, maxTryNum);
					//Serial.print("errCount: ");
					//Serial.println(errCount);
				}
				else if (payload.value == '#') {
					payload_t payloadResp;
					payloadResp.code = payloadCode_serverStatus;
					payloadResp.value = status;
					RF24NetworkHeader headerResponse(header.from_node);
					int errCount = send(network, headerResponse, &payloadResp, maxTryNum);
					//Serial.print("status: ");
					//Serial.println(status);

					//Nem fér be a memóriába
					//String statusString = getStatusAsString(status);
					////Serial.print("status: ");
					////Serial.println(statusString);
				}
				else {
					fifo.add(payload.value);

                                        //send feedback about key press
					payload_t payloadResp;
					payloadResp.code = payloadCode_ok;
					payloadResp.value = payload.value;
					RF24NetworkHeader headerResponse(header.from_node);
					int errCount = send(network, headerResponse, &payloadResp, maxTryNum);

				}
			}
			else if (payload.code == payloadCode_movement){
				counter++;
				//Serial.print(counter);

				if (status == status_elesitve){
					status = status_beriasztas_inditasa;
					statusSetTime = millis();
					//Serial.println(". mozgas eszlelese, beriasztas inditasa");
				} else {
					//Serial.println(". mozgas eszlelese, nincs elesitve nincs problema");
				}
			} else {
				//Serial.print("ismeretlen command: ");
				//Serial.println(payload.code);
			}
		} else {
	    	//Serial.print("Already received, id: ");
	    	//Serial.println(header.id);
	    }
	}


	//
	//Ha volt elesites_inditasa, akkor ellenorozni kell az eltelt idot, hogy lehet-e elesiteni
	//
	if (status == status_elesites_inditasa){
		if (now - statusSetTime > elesedesiIdoMillis){
			status = status_elesitve;
			//Serial.println("rendszer elesitve");
		}
	}
	else if (status == status_beriasztas_inditasa){
		if (now - statusSetTime > kiriasztasTurelmiIdoMillis){
			//Serial.println("riasztas!!!, sms kuldes indul");
			doCall();

			//Elment az SMS
			//Visszaallitom a statust, mintha ujra lenne elesitve.
			//
			status = status_elesitve;
		}
	}
        
        //Pick up the phone if I call it!!!
        /*
        if (gsm.available() ){
           //Serial.write(gsm.read());
           
          int nlength = 300;
          char result[nlength];
           
          int len = gsm.read(result, nlength);
          if (len >= 0){
            result[len] = 0;
            Serial.print("len: ");
            Serial.print(len);
            
            String str = result;
            str.trim();
            Serial.print("(");
            Serial.print(str);
            Serial.println(")");
            
            int i = str.indexOf("+36204145031");
            
            //if (str.startsWith("+CLIP: \"+36204145031\"")) {
            if (i >= 0){
              Serial.println("Peti is calling");
              gsm.SimpleWriteln("ATA");
              gsm.SimpleRead();
            }
          }
        }
        */
}


void doCall(){

    //Serial.println("before call");
    //TODO szamomat visszairni !!!
    gsm.SimpleWriteln("ATD + +3620414....;");
    gsm.SimpleRead();

    //hagyom csorogni
    delay(11000);

    ////Serial.println("hivas megszakitas");
    gsm.SimpleWriteln("ATH"); //ATD + +36204145031;
    gsm.SimpleRead();
    ////Serial.println("end call");
}


