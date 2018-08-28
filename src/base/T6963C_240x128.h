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

/*  ________________________________________________________________
   T6963  LCD PIN   |    LCD PIN NAME  |     MEGA 2560            |
  ________________________________________________________________|
          1         |          FG      |       GND                |
          2         |         GND      |       GND                |
          3         |         VDD      |       +5V                |
          4         |          VO      |  10K POT WIPER – LEG 2   |
          5         |          WR      |        33                |
          6         |          RD      |        32                |
          7         |       CE/CS      |        31                |
          8         |         C/D      |        30                |
          9         |         RST      |        29                |
         10         |         DB0      |        28                |
         11         |         DB1      |        27                |
         12         |         DB2      |        26                |
         13         |         DB3      |        25                |
         14         |         DB4      |        24                |
         15         |         DB5      |        38                |
         16         |         DB6      |        40                |
         17         |         DB7      |        36                |
         18         |          FS      |       GND                |
         19         |         VEE      |    10K POT–LEG1          |
         20         |       LED A      |    +3.3V – 5V            |
         21         |       LED K      |        GND               |
  ________________________________________________________________|
*/

#include <U8g2lib.h>

class T6963C_240x128 {
	public:
		static const int width = 240;
		static const int height = 128;
		String command;
		static const long min_refresh_delay = 500;
		long last_refreshed = 0;
		char udp_out[50];

	struct sensor_values_screen_params {
		int left_circle_radius = height / 3 ;
		int left_circle_x = left_circle_radius + 5;
		int left_circle_y = left_circle_radius;
		int left_tick_length = 5;
	

		int right_circle_radius = height / 3 - 1;;
		int right_circle_x = left_circle_x + right_circle_radius + 20;
		int right_circle_y = right_circle_radius;
		int right_tick_length = 5;
	} svsp;

	U8G2_T6963_240X128_F_8080 u8g2 =
	//U8G2_T6963_240X128_F_8080(rotation,  d0, d1, d2, d3, d4, d5, d6, d7, enable (wr), cs, dc [, reset])
	  U8G2_T6963_240X128_F_8080(U8G2_R0 ,  28, 27, 26, 25, 24, 38, 40, 36,     33     , 31, 30  ,  29   );

	void begin();
	void draw_sensor_values();
	void draw_calibration_screen();
	void draw();
};

void T6963C_240x128::begin() {
	u8g2.begin();
	u8g2.enableUTF8Print();
	memset(udp_out, 0, sizeof(udp_out));
}

void T6963C_240x128::draw_sensor_values() {
	u8g2.clearBuffer();

	//left half circle :
	u8g2.drawCircle(svsp.left_circle_x, svsp.left_circle_y, svsp.left_circle_radius);
	u8g2.setDrawColor(0);
	u8g2.drawBox(svsp.left_circle_x, 0, svsp.left_circle_radius + 1,2*svsp.left_circle_radius + 1 );
	u8g2.setDrawColor(1);
	u8g2.drawVLine(svsp.left_circle_x, 0, 2*svsp.left_circle_radius +1);
	for(int i=0;i<3;i++){
		float angle = i*PI/4. + 3.*PI/4.;
		float tick_start_x = svsp.left_circle_radius * cos(angle) + svsp.left_circle_x;
		float tick_end_x = (svsp.left_circle_radius-svsp.left_tick_length) * cos(angle) + svsp.left_circle_x;
		float tick_start_y = svsp.left_circle_radius * sin(angle) + svsp.left_circle_y;
		float tick_end_y = (svsp.left_circle_radius-svsp.left_tick_length) * sin(angle) + svsp.left_circle_y;
		u8g2.drawLine(tick_start_x, tick_start_y, tick_end_x, tick_end_y);
	}
	
// 	left hand :
	float hand_x = (svsp.left_circle_radius-svsp.left_tick_length - 2) * cos(radians(theta + 90.0 )) + svsp.left_circle_x;
	float hand_y = (svsp.left_circle_radius-svsp.left_tick_length - 2) * sin(radians(theta + 90.0 )) + svsp.left_circle_y;
	u8g2.drawLine(svsp.left_circle_x, svsp.left_circle_y, round(hand_x), round(hand_y));
	u8g2.drawDisc(svsp.left_circle_x, svsp.left_circle_y, svsp.left_circle_radius/10);

// 	left value :
	char theta_char[10];
	sprintf (theta_char, "%d.%d°", (int)theta, (int)((theta-(int)theta)*10.0));
	u8g2.setFont(u8g2_font_logisoso16_tn);
	u8g2.setCursor(svsp.left_circle_x - svsp.left_circle_radius/2 - u8g2.getUTF8Width(theta_char) / 2, 16 + 5 + svsp.left_circle_radius*2);
	u8g2.print(theta_char);
	
//	left label :
	u8g2.setFont(u8g2_font_helvR10_tf);
	u8g2.setCursor(svsp.left_circle_x/2-u8g2.getUTF8Width("Incidence")/2 + 6, height - 6 - 1);
	u8g2.print("Incidence");
	
	//right circle :
	u8g2.drawCircle(svsp.right_circle_x, svsp.right_circle_y, svsp.right_circle_radius);
	u8g2.setFont(u8g2_font_helvB08_tf);
	u8g2.setCursor(svsp.right_circle_x - u8g2.getUTF8Width("N") / 2.0, 10);
	u8g2.print("N");
	u8g2.setCursor(svsp.right_circle_x - u8g2.getUTF8Width("S") / 2.0, svsp.right_circle_radius*2-1);
	u8g2.print("S");
	u8g2.setCursor(svsp.right_circle_x - svsp.right_circle_radius + 2, svsp.right_circle_radius + 4);
	u8g2.print("O");
	u8g2.setCursor(svsp.right_circle_x + svsp.right_circle_radius - 2 -u8g2.getUTF8Width("E"), svsp.right_circle_radius + 4);
	u8g2.print("E");
	
	//right hand
	int x = (svsp.right_circle_radius - svsp.right_tick_length - 2) * cos(radians(phi - 90.0)) + svsp.right_circle_x;
	int y = (svsp.right_circle_radius - svsp.right_tick_length - 2) * sin(radians(phi - 90.0)) + svsp.right_circle_y;
	u8g2.drawLine(svsp.right_circle_x, svsp.right_circle_y, x, y);
	u8g2.drawDisc(svsp.right_circle_x, svsp.right_circle_y, svsp.right_circle_radius/10);

// 	right value :
	char phi_char[10];
	sprintf(phi_char, "%d.%d", (int)phi, (int)((phi-(int)phi)*10.0));
	u8g2.setFont(u8g2_font_logisoso16_tn);
	u8g2.setCursor(svsp.right_circle_x - u8g2.getUTF8Width(phi_char) / 2, 16 + 5 + svsp.left_circle_radius*2);
	u8g2.print(phi_char);
	
//	right label :
	u8g2.setFont(u8g2_font_helvR10_tf);
	u8g2.setCursor(svsp.right_circle_x-u8g2.getUTF8Width("Azimut")/2 , height - 6 - 1);
	u8g2.print("Azimut");
	
// 	vertical line :
 	int x_vert_line = svsp.right_circle_x+svsp.right_circle_radius+6;
	u8g2.drawVLine(x_vert_line,0,height);
	u8g2.drawVLine(x_vert_line-1,0,height);
	
//	text :
	uint8_t v_space = 7+4;
	u8g2.setFont(u8g2_font_6x10_tf);
	
	u8g2.setCursor(x_vert_line+4,v_space);
	char sys[20];sprintf(sys, "system    %d", status_SGAM[0]);
	u8g2.print(sys);
	
	u8g2.setCursor(x_vert_line+4,2*v_space);
	char gyr[20];sprintf(gyr, "gyroscope %d", status_SGAM[1]);
	u8g2.print(gyr);
	
	u8g2.setCursor(x_vert_line+4,3*v_space);
	char acc[20];sprintf(acc, "accéléro  %d", status_SGAM[2]);
	u8g2.print(acc);
	
	u8g2.setCursor(x_vert_line+4,4*v_space);
	char mag[20];sprintf(mag, "magnéto   %d", status_SGAM[3]);
	u8g2.print(mag);
	
	u8g2.setCursor(x_vert_line + (width-x_vert_line)/2 - u8g2.getUTF8Width("UDP OUTPUT:")/2,6*v_space);
	u8g2.print("UDP OUTPUT:");
	u8g2.setFont(u8g2_font_5x8_tf);
	u8g2.setCursor(x_vert_line+2,7*v_space);
	u8g2.print(udp_out);
	u8g2.sendBuffer();

}

void T6963C_240x128::draw_calibration_screen() {
	u8g2.setFont(u8g2_font_helvB14_tf);
	u8g2.clearBuffer();
	u8g2.setDrawColor(1);
	char calibration[] = "CALIBRATION";
	u8g2.print(calibration);
	u8g2.sendBuffer();
}

void T6963C_240x128::draw() {
	if( millis() - last_refreshed > min_refresh_delay){
		if (command == "sensor_values")draw_sensor_values();
		else if (command == "calibration")draw_calibration_screen();
		last_refreshed = millis();
	}
}















