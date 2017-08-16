#include <iostream>
#include <chrono>
#include <ctime>
#include <wiringPi.h>


#define ECHOPIN 29
#define TRIGPIN 28
#define RELAYPIN 7
#define EMPTY 20
#define FULL 15 


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

	auto trig_start = std::chrono::system_clock::now();

	digitalWrite(TRIGPIN, HIGH);

	delay_nanos(10000);

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

	float distance = (float)(echo_width.count()) / 1000000 * 170 * 39.37;

	return distance;

}


int main(void){

	wiringPiSetup();
	pinMode(TRIGPIN, OUTPUT);
	pinMode(ECHOPIN, INPUT);
	pinMode(RELAYPIN, OUTPUT);

	digitalWrite(RELAYPIN, LOW);
	digitalWrite(TRIGPIN, LOW);

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

			// std::cout << "Total distance: " << tot_distance << " inches" << "\n";
			// std::cout << "Count: " << count << "\n";
			// std::cout << "Average distance: " << avg_distance << "inches" << "\n";

			delay(100);

			count++;

		}

		if(filling){
			if(avg_distance <= FULL){
				filling = false;
			}
		} else{
			if(avg_distance >= EMPTY){
				filling = true;
			}
		}

		if(filling){
			digitalWrite(RELAYPIN, HIGH);
		} else{
			digitalWrite(RELAYPIN, LOW);
		}

		std::cout << "Average distance: " << avg_distance << "\n";
		std::cout << "Filling: " << filling << "\n\n";

	}

}
