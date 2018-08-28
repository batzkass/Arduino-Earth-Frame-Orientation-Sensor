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
//#define DEBUG
#define SERIAL_BAUD 9600

// IMU:
#include "BNO055.h"
BNO055 IMU;

//ETHERNET:
#include <SPI.h>
#include <Ethernet2.h>
EthernetUDP udp;

//CONFIG
//NOTE: change address and port in the following.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x11, 0x17, 0x4F };
IPAddress local_ip(192, 168, 50, 2);
unsigned int local_port = 8888;
IPAddress remote_ip(192, 168, 50, 1);
unsigned int remote_port = 8888;


void send_message_udp(char *buffer){
	udp.beginPacket(remote_ip, remote_port);
	udp.write(buffer);
	udp.endPacket();
}

void send_imu_data_to_base() {
	//READ SENSOR VALUES
	imu::Vector<2> theta_phi = IMU.get_Orientation_Spherical(); //get spherical coords in theta & phi
	uint8_t status_SGAM[4] ; IMU.get_cal_status(&status_SGAM[0]); //get IMU self calibration status (SGAM=System Gyro Accelero Magneto)
	char  outputBuffer[50];
	memset(outputBuffer, 0, sizeof(outputBuffer));
	// Arduino sprintf function doesn't work with floats, so we have to use dtostrf. (see http://yaab-arduino.blogspot.fr/2015/12/how-to-sprintf-float-with-arduino.html)
	char theta_char[7];  dtostrf(theta_phi[0], 5, 1, &theta_char[0]);
	char phi_char[7];  dtostrf(theta_phi[1], 5, 1, &phi_char[0]);
	sprintf(outputBuffer, "%s %s %d %d %d %d", theta_char, phi_char, status_SGAM[0], status_SGAM[1], status_SGAM[2], status_SGAM[3]);
	//SEND DATA
	send_message_udp(&outputBuffer[0]);
#ifdef DEBUG
	Serial.print("Spherical: ");
	Serial.print((float)theta_phi[0]);
	Serial.print(" ");
	Serial.print((float)theta_phi[1]);
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
}

void read_commands_from_base() {
	char inputBuffer[UDP_TX_PACKET_MAX_SIZE];
	memset(inputBuffer, 0, sizeof(inputBuffer));
#ifdef DEBUG
	Serial.println("Try to read command from base...");
#endif
	int packetSize = udp.parsePacket();
	if (packetSize) { // if there is something in the buffer
		udp.read(inputBuffer, UDP_TX_PACKET_MAX_SIZE);
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
		if (strcmp(inputBuffer, "reset_calibration") == 0) {
			IMU.erase_calibration_data();
			IMU.write_cal_data_to_EEPROM(0);
		}
		else if (strcmp(inputBuffer,"save_calibration") == 0) {
			IMU.write_cal_data_to_EEPROM(0);
		}
		else if (strcmp(inputBuffer,"load_calibration") == 0) {
			IMU.read_cal_data_from_EEPROM(0);
		}
		else if (strcmp(inputBuffer,"load_default_calibration") == 0) {
			IMU.read_cal_data_from_EEPROM(1);
			IMU.write_cal_data_to_EEPROM(0);
		}
		else if (strcmp(inputBuffer,"write_default_calibration") == 0) {
			IMU.write_cal_data_to_EEPROM(1);
		}
   else if (strcmp(inputBuffer,"write_offset_calibration") == 0) {
     IMU.write_Offset_Ori_To_Epprom();
    }
		else {;
#ifdef DEBUG
			Serial.println("W: Input command not understood");
#endif
		}
	}
}


void setup() {
//#ifdef DEBUG //FIXME
	Serial.begin(SERIAL_BAUD);
	Serial.println("IMU sender serial debug");
//#endif
	IMU.begin();
	IMU.read_cal_data_from_EEPROM(0);
	IMU.read_Offset_Ori_From_Epprom();
	Ethernet.begin(mac, local_ip);
	udp.begin(local_port);
#ifdef DEBUG
	Serial.print("IMU is at: ");
	Serial.println(Ethernet.localIP());
#endif
}



void loop() {
	long t = millis();
	read_commands_from_base();
	send_imu_data_to_base();
	t = millis()-t;
	if(t<100) delay(100-t);
}













