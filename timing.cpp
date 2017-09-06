#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>
#include <iomanip>


#define ECHOPIN 29
#define TRIGPIN 28
#define RELAYPIN 7

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


void delay_nanos(int nano_delay){

	auto delay_start = std::chrono::system_clock::now();

	bool sleep = true;
	while(sleep)
	{
		auto now = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - delay_start);
		if ( elapsed.count() > nano_delay )
			sleep = false;
	}

}

float take_reading(void){

	std::cout << "Taking reading...\n";
	std::cout << "Sending trigger pulse...\n";

	auto trig_start = std::chrono::system_clock::now();

	digitalWrite(TRIGPIN, HIGH);

	delay_nanos(20000);

	digitalWrite(TRIGPIN, LOW);

	auto trig_end = std::chrono::system_clock::now();

	auto trig_width = std::chrono::duration_cast<std::chrono::microseconds>(trig_end - trig_start);

	std::cout << "Waiting for echo pulse...\n";

	// wait for echo to go high
	while(digitalRead(ECHOPIN) == 0){}

	auto echo_start = std::chrono::system_clock::now();

	// wait for echo to go low
	while(digitalRead(ECHOPIN) == 1){}

	auto echo_end = std::chrono::system_clock::now();

	auto echo_delay = std::chrono::duration_cast<std::chrono::microseconds>(echo_start - trig_end);

	auto echo_width = std::chrono::duration_cast<std::chrono::microseconds>(echo_end - echo_start);

	float distance = (float)(echo_width.count()) / 58 * 0.3937;

	return distance;

}


int main(void){

	std::cout << "Booting up...\n";
	wiringPiSetup();
	pinMode(TRIGPIN, OUTPUT);
	pinMode(ECHOPIN, INPUT);
	pinMode(RELAYPIN, OUTPUT);

	digitalWrite(RELAYPIN, LOW);
	digitalWrite(TRIGPIN, LOW);

	// all heights in inches
	float full_height = 20;
	float empty_height = 5;
	float sensor_height = 30;
	float water_height = full_height;

	bool filling = false;

	int count = 1;
	float avg_distance = 0;
	float tot_distance = 0;

	while(1){

		count = 1;
		avg_distance = 0;
		tot_distance = 0;

		while(count <= 10){

			tot_distance += take_reading();
			avg_distance = tot_distance / count;

			delay(100);

			count++;

		}

		water_height = sensor_height - avg_distance;

		int percent = (int)(((water_height / full_height) * 100) / 5);

		char fill_bar[] = "[####################]";

		for(int i = 1; i <= 20; i++){
			if(i <= percent)
				fill_bar[i] = '#';
			else
				fill_bar[i] = ' ';
		}

		if(filling){
			if(water_height >= full_height){
				filling = false;
			}
		} else{
			if(water_height <= empty_height){
				filling = true;
			}
		}

		if(filling){
			digitalWrite(RELAYPIN, HIGH);
		} else{
			digitalWrite(RELAYPIN, LOW);
		}

		std::cout << "\n";

		std::cout << "Water Height: " << std::fixed << std::setprecision(1) << water_height << " in\n";

		std::cout << "\n";

		std::cout << fill_bar << "\n";

		std::cout << "\n";

		if(filling){
			std::cout << "Pump Status:  " << BOLDGREEN << "ON" << RESET << " /" << "\n";
		} else{
			std::cout << "Pump Status:     /  " << BOLDRED << "OFF" << RESET << "\n";
		}

	}

}
