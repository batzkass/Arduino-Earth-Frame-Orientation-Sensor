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


class Switch{
	public:
		uint8_t input_pin = 49;
		bool current_state = false;
		long long_press_delay = 1500;
		long long_long_press_delay = 4000;
		long min_refresh_delay = 100;
		long pressed_time = 0;
		long last_event_time = 0;
		bool long_press = false;
		bool long_long_press = false;
		uint8_t current_event = 0; // 0:off | 1:short press | 2:long press

		void begin();
		uint8_t listen();
};

void Switch::begin(){
	pinMode(input_pin, INPUT);
}

uint8_t Switch::listen(){
	if( millis()-last_event_time > min_refresh_delay ){
		last_event_time=millis();
		if(digitalRead(input_pin)==HIGH){ //currently pressed
			if(!current_state){ //the press just happened
				current_state=true;
				pressed_time=millis(); //keep the pressed time in memory
			}
			if( pressed_time+long_press_delay<millis() && !(current_event==2)){ //the long press is triggered
				long_press=true;
			}
			if( pressed_time+long_long_press_delay<millis() && !(current_event==3)){ //the long press is triggered
				long_press=false;
				long_long_press=true;
			}
			return 0;
		}
		else{ //currently released
			if(current_state){ // the button was just released
				if(long_long_press){
					current_event=3;
					long_long_press=false;
				}
				else if(long_press){
					current_event=2;
					long_press=false;
				}
				else{
					current_event=1;
				}
				current_state=false;
			}
			else current_event=0;
			return current_event;
		}
	}
	else return 0;
}


