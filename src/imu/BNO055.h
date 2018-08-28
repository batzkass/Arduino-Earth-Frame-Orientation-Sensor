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




#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <EEPROM.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);
const int calib_data_size = sizeof(adafruit_bno055_offsets_t)+sizeof(long);

class BNO055 {
  public:
    imu::Quaternion offset_ori;
    void begin();
    imu::Quaternion get_Orientation_Quaternion();
    imu::Vector<2> get_Orientation_Spherical();
    void get_cal_status(uint8_t *status_SGAM);
    void write_cal_data_to_EEPROM(int location);
    void read_cal_data_from_EEPROM(int location);
    void write_Offset_Ori_To_Epprom();
    void read_Offset_Ori_From_Epprom();
	void erase_calibration_data();
};

void BNO055::begin() {
  /* Initialise the sensor */
  if (!bno.begin())
  {
#ifdef DEBUG
    Serial.print("E: no BNO055 detected, check your wiring or I2C ADDR. Waiting for connexion with BNO055...");
#endif
    while (1);
  }
  delay(1000);
}

void BNO055::get_cal_status(uint8_t *status_SGAM)
{
  uint8_t system, gyro, accel, mag;
  for (int i = 0 ; i < 3 ; i++) status_SGAM[i] = 0;
  bno.getCalibration(&status_SGAM[0], &status_SGAM[1], &status_SGAM[2], &status_SGAM[3]);
}

void BNO055::write_cal_data_to_EEPROM(int location) {
  adafruit_bno055_offsets_t newCalib;
  bno.getSensorOffsets(newCalib);
#ifdef DEBUG
  if (!bno.isFullyCalibrated())
    Serial.println("W: Storing calibration data to EEPROM while not fully calibrated.");
#endif
  sensor_t sensor;
  bno.getSensor(&sensor);
  long bnoID = sensor.sensor_id;
  int address_offset = location*calib_data_size;
  EEPROM.put(address_offset, bnoID);
  address_offset += sizeof(long);
  EEPROM.put(address_offset, newCalib);
#ifdef DEBUG
  Serial.print("Calibration data stored to EEPROM at location ");
  Serial.print(location);
  Serial.print(" (byte ");
  Serial.print(location*calib_data_size);
  Serial.print(" to ");
  Serial.print(location*calib_data_size+calib_data_size);
  Serial.println(").");
#endif
  delay(25);
}

void BNO055::read_cal_data_from_EEPROM(int location) {
  sensor_t sensor;
  bno.getSensor(&sensor);
  long bnoID;
  int address_offset = location*calib_data_size;
  EEPROM.get(address_offset, bnoID);
  if (bnoID != sensor.sensor_id){
#ifdef DEBUG
    Serial.println("E: No calibration data for this sensor exists in EEPROM at this addess offset");
#endif
    return;
  }
#ifdef DEBUG
  else Serial.println("Found Calibration for this sensor in EEPROM at given address offset.");
#endif

  adafruit_bno055_offsets_t oldCalib;
  address_offset += sizeof(long);
  EEPROM.get(address_offset, oldCalib);
#ifdef DEBUG
  Serial.println("Restoring Calibration data...");
#endif
  bno.setSensorOffsets(oldCalib);
#ifdef DEBUG
  Serial.println("Success !");
#endif
  delay(25);
}

void BNO055::erase_calibration_data(){
	adafruit_bno055_offsets_t zeroCalib;
	zeroCalib.accel_offset_x=0;
	zeroCalib.accel_offset_y=0;
	zeroCalib.accel_offset_z=0;
	zeroCalib.mag_offset_x=0;
	zeroCalib.mag_offset_y=0;
	zeroCalib.mag_offset_z=0;
	zeroCalib.gyro_offset_x=0;
	zeroCalib.gyro_offset_y=0;
	zeroCalib.gyro_offset_z=0;
	zeroCalib.accel_radius=0;
	zeroCalib.mag_radius=0;
	bno.setSensorOffsets(zeroCalib);
	#ifdef DEBUG
	Serial.println("Calibration data erased in EEPROM and sensor.");
	#endif
}

imu::Quaternion BNO055::get_Orientation_Quaternion() {
  sensors_event_t event;
  bno.getEvent(&event);
  imu::Quaternion quat = bno.getQuat();
  quat.normalize();
  return quat;
}

void BNO055::read_Offset_Ori_From_Epprom(){
  double w,x,y,z;
  int address_offset = 10*calib_data_size;
  EEPROM.get(address_offset, w);
  EEPROM.get(address_offset+sizeof(double), x);
  EEPROM.get(address_offset+sizeof(double)*2, y);
  EEPROM.get(address_offset+sizeof(double)*3, z);
  offset_ori = imu::Quaternion(w,x,y,z);
}

void BNO055::write_Offset_Ori_To_Epprom(){
  int address_offset = 10*calib_data_size;
  offset_ori = get_Orientation_Quaternion();
  EEPROM.put(address_offset, offset_ori.w());
  EEPROM.put(address_offset+sizeof(double), offset_ori.x());
  EEPROM.put(address_offset+sizeof(double)*2, offset_ori.y());
  EEPROM.put(address_offset+sizeof(double)*3, offset_ori.z());
}

imu::Vector<2> BNO055::get_Orientation_Spherical() {
  imu::Quaternion quat = BNO055::get_Orientation_Quaternion();
  imu::Vector<3> point(0., 0., 1.);
  point = offset_ori.conjugate().rotateVector(point);
  point = quat.rotateVector(point);

  imu::Vector<2> theta_phi;
  theta_phi[0] = acos(point[2]) / PI * 180.F;
  theta_phi[1] = atan2(point[1], point[0]);
  theta_phi[1] = theta_phi[1] / PI * 180.F;
  theta_phi[1] = -theta_phi[1] + 270.F;
  theta_phi[1] = theta_phi[1] >= 360.F ? theta_phi[1] - 360.F : theta_phi[1];
  return theta_phi;
}


















