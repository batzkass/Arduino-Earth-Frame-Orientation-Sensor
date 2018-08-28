//Copyright 2018 François Kneib <francois@kneib.fr>
//Licensed under LGPL-3.0-or-later

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

//You should have received a copy of the GNU Lesser General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.


//SERIAL MONITOR:
#define DEBUG
#define SERIAL_BAUD 9600

// GLOBAL PARAMS
float theta=0, phi=0;
int status_SGAM[4] = {0,0,0,0};

// SCREEN:
# include "T6963C_240x128.h"
T6963C_240x128 screen;

// SWITCH:
# include "Switch.h"
Switch calib_switch;

//ETHERNET:
#include <SPI.h>
#include <Ethernet2.h>
EthernetUDP udp;
//NOTE: change address and port in the following.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xA4, 0xB4 };
IPAddress local_ip(192, 168, 50, 1);
unsigned int local_port = 8888;
IPAddress remote_ip(192, 168, 50, 2);
unsigned int remote_port = 8888;


void send_message_udp(char *buffer){
#ifdef DEBUG
	Serial.print("Sending message to imu: ");
	Serial.println(buffer);
#endif
	strncpy(screen.udp_out, buffer, 50);
	udp.beginPacket(remote_ip, remote_port);
	udp.write(buffer);
	udp.endPacket();
}

void read_data_from_imu(){
	char inputBuffer[UDP_TX_PACKET_MAX_SIZE];
	memset(inputBuffer, 0, sizeof(inputBuffer));
#ifdef DEBUG
	Serial.println("Try to read data from imu...");
#endif
	//The following loop is used to empty the hardware buffer and only keep the last incoming packet (to have LIFO behavior)
	//Hence, we know that we will probably loose packets if the imu send frequency is high, but the display data will always be the last incoming ones (LIFO).
	bool get_something = false;
	while(udp.parsePacket()>0){ //while there is something in the hardware (ethernet shield) buffer...
		udp.read(inputBuffer, UDP_TX_PACKET_MAX_SIZE); //copy the hardware buffer in our software buffer (FIFO sequence)
		get_something = true;
	}
	if(get_something){
#ifdef DEBUG
		Serial.print("Received packet from ");
		IPAddress remote = udp.remoteIP();
		for (int i = 0; i < 4; i++) {
			Serial.print(remote[i], DEC);
			if (i < 3) Serial.print(".");
		}
		Serial.print(":");
		Serial.print(udp.remotePort());
		Serial.print(", message: ");
		Serial.println(inputBuffer);
#endif
		//Parse the incoming message:
		uint8_t i = 1;
		char *token = strtok(inputBuffer, " ");
		while( token != NULL ) {
			if(i==1) theta=atof(token);
			else if(i==2) phi=atof(token);
			else status_SGAM[i-3]=atoi(token);
			token = strtok(NULL, " ");
			i++;
		}
	}
}


void setup() {
#ifdef DEBUG
  Serial.begin(SERIAL_BAUD);
  Serial.println("Base receiver serial debug");
#endif
  screen.begin();
  screen.command = "sensor_values"; //initial display
  calib_switch.begin();
  Ethernet.begin(mac, local_ip);
  udp.begin(local_port);
#ifdef DEBUG
  Serial.print("Base is at: ");
  Serial.println(Ethernet.localIP());
#endif
}


void loop() {
	long t = millis();
	uint8_t current_switch_event = 0;
	current_switch_event = calib_switch.listen();
	
	if(current_switch_event){
#ifdef DEBUG
		Serial.print("Get switch input: ");
		Serial.println(current_switch_event);
#endif
		char buffer[50] ; memset(buffer, 0, sizeof(buffer));
		//if(current_switch_event==1) {sprintf(buffer,"load_calibration");} //short press
    if(current_switch_event==1) {sprintf(buffer,"write_offset_calibration");} //short press
		else if(current_switch_event==2) {sprintf(buffer,"save_calibration");} //long press
		else if(current_switch_event==3) {sprintf(buffer,"reset_calibration");} //long long press
		send_message_udp(&buffer[0]);
	}
	
	read_data_from_imu();
#ifdef DEBUG
	Serial.print("Spherical: ");
	Serial.print(theta);
	Serial.print(" ");
	Serial.print(phi);
	Serial.print(" ");
	Serial.print("Sys:");
	Serial.print(status_SGAM[0]);
	Serial.print(" G:");
	Serial.print(status_SGAM[1]);
	Serial.print(" A:");
	Serial.print(status_SGAM[2]);
	Serial.print(" M:");
	Serial.println(status_SGAM[3]);
#endif
    
	screen.draw();
	
	t = millis()-t;
	if(t<200) delay(200-t);
}
  

