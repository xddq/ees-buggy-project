/*
 * Pierre Dahmani pd1528s 3215892
 * Jens Peter Dennigmann jd8389s 3190025
 * Leonhard Kipp lk2149s 3188047 
 */

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>
#include <iostream>
#include <thread>
#include <motor_engine/constants.h>
#include <motor_engine/motor_engine.h>
#include <ultrasonic_sensor/ultrasonic_sensor.h>
#include <curses.h>
#include <thread>
#include <chrono>

#define ULTRASONIC_BRAKE_LIGHT_PIN_WPI 0
#define ULTRASONIC_ECHO_PIN_WPI 1
#define ULTRASONIC_TRIGGER_PIN_WPI 2 

motor_engine* engine = nullptr;
/// Interrupt Routine for STRG-C
void signalHandler(int signum)
{
    endwin();
    std::cout << "Strg-C Programmende" << std::endl;
    // Beenden Sie hier bitte alle Verbindung zu den Sensoren etc.
    if(engine){
        engine->emergency_stop();
        engine->release_engine();
    }
    exit(signum);
}


/*
 * Periodically checks distance using the ultrasonic sensor.
 */
void poll_distance(bool * end_ptr, ultrasonic_sensor * sensor, motor_engine * engine)
{

	// plot current speed
	double speed = 0;
	double distance = 0;
	double safety_threshold= 0;
	while(true){

		// Adapt threshold depending on current speed.
		speed = engine->get_speed();
		if(speed >= 2000){
			safety_threshold = 20;
		}
		else {
			safety_threshold = 10;
		}

		distance = sensor->calc_distance();
		// DEBUG CODE
		//std::cout << "current distance: " << distance << std::endl;
		//std::cout << "current speed: " << speed << std::endl;


		// Set stop condition to true if the distance is to the wall 
		// ois below our safety threshold. 
		if(distance <= safety_threshold){
			*end_ptr=true;
			sensor->set_brake_light(ON);
			engine->emergency_stop();

			/*
			 * TURN ROUTINE HERE
			 * TODO TURN 45 degrees for a maximum of 4 times (maybe some cat just ran by)
			 * If distance will be <=10 after one iteration we disable the brake light
			 * and enable remote wasd control again.
			 * otherwise we are stuck inside 4 walls.
			 * and just quit with a "you shall not pass" message? xd
			 */

			// after turn routine set *end_ptr to false again
			// mock routine by sleep timer
			// pause 2000 ms
			auto start = std::chrono::steady_clock::now();
			auto delay = std::chrono::steady_clock::now();
			while(delay - start < std::chrono::milliseconds(2000)){
				delay = std::chrono::steady_clock::now();
			}
			*end_ptr=false;

		}
		else {
			//TODO find better solution than turning on every iteration
			sensor->set_brake_light(OFF);
		}

		// pause 100 ms
		auto start = std::chrono::steady_clock::now();
		auto delay = std::chrono::steady_clock::now();
		while(delay - start < std::chrono::milliseconds(100)){
			delay = std::chrono::steady_clock::now();
		}
	}
}


int main ()
{
	bool end = false;
	bool * end_ptr = & end;
	// Csignal für Abbruch über STRG-C
	signal(SIGINT, signalHandler);
	//init wiringPi
	wiringPiSetup () ;

	//test blink led
	/*
	   pinMode (0, OUTPUT) ;
	   do
	   {
	   digitalWrite (0, HIGH) ; delay (1000) ;
	   digitalWrite (0,  LOW) ; delay (1000) ;
	   }while(true);
	   */

	// init engine
	engine = new motor_engine{make_motor_engine()};
	engine->set_frequency(1600.);

	// init sensor
	ultrasonic_sensor ultrasonic(ULTRASONIC_TRIGGER_PIN_WPI, ULTRASONIC_ECHO_PIN_WPI,
			ULTRASONIC_BRAKE_LIGHT_PIN_WPI);
	ultrasonic_sensor * ultrasonic_ptr = & ultrasonic;
	ultrasonic.init();

	// periodically poll distance
	std::thread sensor_thread(poll_distance,end_ptr,ultrasonic_ptr,engine);

	//test thread is working
	//std::thread sensor_thread(fun1,end_ptr);



	bool ultrasonic_test = false;
	if(ultrasonic_test){
		//debug time diff
		for(int i=0 ;i<100;i++){
			ultrasonic.calc_distance() ;
		}
		for(int i=0 ;i<100;i++){
			std::cout << ultrasonic.calc_distance() << std::endl;
		}
		return 0;
	};

	bool auto_movement = false;
	if(auto_movement){
		std::cout << "driving forward" << std::endl;
		engine->set_speed(850);
		engine->forward();
		std::this_thread::sleep_for (std::chrono::milliseconds (500));
		engine->smooth_stop();

		std::cout << "driving backwards" << std::endl;
		engine->set_speed(850);
		engine->backwards();
		std::this_thread::sleep_for (std::chrono::milliseconds (500));
		engine->smooth_stop();

		engine->release_engine();
		delete engine;
		return 0;
	}

	// control buggy motor engine using wasd keys
	char user_cmd = 0;
	initscr();
	cbreak();
	noecho();
	while((user_cmd = getch()) != 'x'){
		/*
		 * Check if remote control is enabled. Which is the case if our buggy is 
		 * not below our safety distance threshold. 
		 */
		if(*end_ptr){
			std::cout << "remote control disabled while distance is below safety threshold!" << std::endl;
		}
		else {

			switch (user_cmd){
				case 'e':{
						 engine->emergency_stop();
						 break;
					 }
				case 'q':{
						 engine->smooth_stop();
						 break;
					 }
				case 'w':{
						 switch(engine->get_direction()){
							 case direction::IN_PLACE_TURN_RIGHT:
							 case direction::IN_PLACE_TURN_LEFT:
								 engine->smooth_stop();
								 engine->forward();
								 engine->increase_speed();
								 break;
							 case direction::BACKWARDS:
								 engine->decrease_speed();
								 break;
							 case direction::STOP:
								 engine->forward();
								 engine->increase_speed();
								 break;
							 case direction::FORWARD:
								 engine->increase_speed();
								 break;
						 }
						 break;
					 }
				case 's':{
						 switch(engine->get_direction()){
							 case direction::IN_PLACE_TURN_RIGHT:
							 case direction::IN_PLACE_TURN_LEFT:
								 engine->smooth_stop();
								 engine->backwards();
								 engine->increase_speed();
								 break;
							 case direction::FORWARD:
								 engine->decrease_speed();
								 break;
							 case direction::STOP:
								 engine->backwards();
								 engine->increase_speed();
								 break;
							 case direction::BACKWARDS:
								 engine->increase_speed();
								 break;
						 }
						 break;
					 }
				case 'd':{
						 switch(engine->get_direction()){
							 case direction::IN_PLACE_TURN_RIGHT:
								 engine->increase_speed();
								 break;
							 case direction::IN_PLACE_TURN_LEFT:
								 engine->decrease_speed();
								 break;
							 case direction::FORWARD:
							 case direction::BACKWARDS:
								 engine->turn_right();
								 break;
							 case direction::STOP:
								 engine->turn_in_place_right();
								 engine->increase_speed();
								 break;
						 }
						 break;
					 }
				case 'a':{
						 switch(engine->get_direction()){
							 case direction::IN_PLACE_TURN_RIGHT:
								 engine->decrease_speed();
								 break;
							 case direction::IN_PLACE_TURN_LEFT:
								 engine->increase_speed();
								 break;
							 case direction::FORWARD:
							 case direction::BACKWARDS:
								 engine->turn_left();
								 break;
							 case direction::STOP:
								 engine->turn_in_place_left();
								 engine->increase_speed();
								 break;
						 }
						 break;
					 }
				default: break;
			}
		}
	}

	// join thread
	sensor_thread.join();
	engine->smooth_stop();
	engine->release_engine();
	endwin();
}
