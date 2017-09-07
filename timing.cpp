#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>
#include <iomanip>
#include <cmath>
#include <algorithm>

// define pi pin numbers
#define ECHOPIN 29
#define TRIGPIN 28
#define RELAYPIN 7

// define char arrays for color coding terminal output
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


// use chrono to delay nano_delay nanoseconds
void delay_nanos(int nano_delay){

	auto delay_start = std::chrono::system_clock::now();
	bool sleep = true;

	while(sleep){
		auto now = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - delay_start);
		if (elapsed.count() > nano_delay)
			sleep = false;
	}

}


// takes a reading from the sensor and returns the distance in inches as a float
float take_reading(void){

	auto trig_start = std::chrono::system_clock::now();

	digitalWrite(TRIGPIN, HIGH);
	delay_nanos(20000);
	digitalWrite(TRIGPIN, LOW);

	auto trig_end = std::chrono::system_clock::now();
	auto trig_width = std::chrono::duration_cast<std::chrono::microseconds>(trig_end - trig_start);

	// wait for echo to go high
	while(digitalRead(ECHOPIN) == 0){}

	auto echo_start = std::chrono::system_clock::now();

	// wait for echo to go low
	while(digitalRead(ECHOPIN) == 1){}

	auto echo_end = std::chrono::system_clock::now();

	auto echo_delay = std::chrono::duration_cast<std::chrono::microseconds>(echo_start - trig_end);
	auto echo_width = std::chrono::duration_cast<std::chrono::microseconds>(echo_end - echo_start);

	// convert the pulse width from the sensor to a distance using 58 us/cm
	float distance = (float)(echo_width.count()) / 58 * 0.3937;

	return distance;

}


// main function
int main(void){

	// setting up and initializing pins
	wiringPiSetup();
	pinMode(TRIGPIN, OUTPUT);
	pinMode(ECHOPIN, INPUT);
	pinMode(RELAYPIN, OUTPUT);

	digitalWrite(RELAYPIN, LOW);
	digitalWrite(TRIGPIN, LOW);

	// set constants specific to tank
	// all heights in inches
	float full_height = 37;
	float empty_height = 18;
	float sensor_height = 50;
	float water_height = full_height;

	// initialize variables as if pump is off and tank is full
	bool filling = false;
	int count = 1;
	float avg_distance = 0;
	float readings_array[10] = {};

	auto last_pump_off_time = std::chrono::system_clock::now();
	auto last_pump_on_time = std::chrono::system_clock::now();

	// main infinite loop
	while(1){

		// reset variables
		count = 0;
		avg_distance = 0;
		readings_array[10] = {};

		// collect ten measurements over one second
		while(count <= 9){
			readings_array[count] = take_reading();
			delay(100);
			count++;
		}

		// order readings_array
		std::sort(readings_array, readings_array + 10);

		// use median as average to avoid outliers
		avg_distance = readings_array[4];

		// change measured distance to water height
		water_height = sensor_height - avg_distance;

		// make percent fill bar
		int percent = (int)((((water_height) / (full_height)) * 100) / 5);
		char fill_bar[] = "[####################]";
		for(int i = 1; i <= 20; i++){
			if(i <= percent)
				fill_bar[i] = '#';
			else
				fill_bar[i] = '.';
		}

		// check if water level is too low or too high
		if(filling){
			if(water_height >= full_height){
				filling = false;
			}
		} else{
			if(water_height <= empty_height){
				filling = true;
			}
		}

		// check if pump has been turned on for more than 70 minutes
		std::chrono::duration<float> seconds_pump_on = std::chrono::system_clock::now() - last_pump_off_time;
		if(seconds_pump_on.count() > 4200)
			filling = false;

		// format pump off time
		std::chrono::duration<float> seconds_pump_off = std::chrono::system_clock::now() - last_pump_on_time;

		// turn pump on or off with relay pin
		if(filling){
			digitalWrite(RELAYPIN, HIGH);
			last_pump_on_time = std::chrono::system_clock::now();
		} else{
			digitalWrite(RELAYPIN, LOW);
			last_pump_off_time = std::chrono::system_clock::now();
		}

		// output info on termnal
		std::cout << "\n";
		std::cout << "Water Height: " << std::fixed << std::setprecision(1) << water_height << " in\n";
		std::cout << fill_bar << "\n\n";
		if(filling){
			std::cout << "Pump Status:  " << BOLDGREEN << "ON" << RESET << " /" << "\n";
			std::cout << "for " << (int)(seconds_pump_on.count()/60) << " minutes\n";
		} else{
			std::cout << "Pump Status:     /  " << BOLDRED << "OFF" << RESET << "\n";
			std::cout << "for " << (int)(seconds_pump_off.count()/60) << " minutes\n";
		}

	}

}
