/*
 * Pierre Dahmani pd1528s 3215892
 * Jens Peter Dennigmann jd8389s 3190025
 * Leonhard Kipp lk2149s 3188047 
 */

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>
#include <magnetic_sensor/compass.h>
#include <iostream>
#include <thread>
#include <motor_engine/constants.h>
#include <motor_engine/motor_engine.h>
#include <ultrasonic_sensor/ultrasonic_sensor.h>
#include <chrono>
#include <applications/wasd_control.h>


//Test files
#include "test_inc/driving/test_automatic_movement.h"
#include "test_inc/calibration.h"

#define ULTRASONIC_BRAKE_LIGHT_PIN_WPI 0
#define ULTRASONIC_ECHO_PIN_WPI 1
#define ULTRASONIC_TRIGGER_PIN_WPI 2 

motor_engine* engine = nullptr;
wasd_control* wasd_controller = nullptr;
compass* _compass = nullptr;
/// Interrupt Routine for STRG-C
void release_resources(){
    if(engine){
        engine->emergency_stop();
        engine->release_engine();
    }
    if(_compass){
        _compass->release_resources();
    }
    if(wasd_controller){
        wasd_controller->release_resources();
    }
}
void signalHandler(int signum)
{
    release_resources();
    std::cout << "Strg-C Programmende" << std::endl;
    // Beenden Sie hier bitte alle Verbindung zu den Sensoren etc.
    exit(signum);
}


int main () {
    // Csignal für Abbruch über STRG-C
    signal(SIGINT, signalHandler);
    //init wiringPi
    wiringPiSetup();

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
    engine->set_frequency(500.);

    // init sensor
    ultrasonic_sensor ultrasonic(ULTRASONIC_TRIGGER_PIN_WPI, ULTRASONIC_ECHO_PIN_WPI,
                                 ULTRASONIC_BRAKE_LIGHT_PIN_WPI);
    ultrasonic.init();

    //Magnetic Sensor
    _compass = new compass{};
    /*
     * Test code for the ultrasonic sensor. 
     */

#ifdef TEST_ON
    bool ultrasonic_test = false;
    if(ultrasonic_test){
        //debug time diff
        for(int i=0 ;i<100;i++){
            ultrasonic.calc_distance() ;
        }
        for(int i=0 ;i<100;i++){
            auto dist = ultrasonic.calc_distance();
            if(!dist){
                std::cout << "No obstacle in front" << std::endl;
            }else{
                std::cout << "Obstacle in: " << dist->get() << " cm" << std::endl;
            }
        }
        return 0;
    };

    /*
     * Test code for the motor engine.
     */
    bool auto_movement = false;
    if (auto_movement) {
        std::cout << "driving forward" << std::endl;
        engine->set_speed(850);
        engine->forward();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        engine->smooth_stop();

        std::cout << "driving backwards" << std::endl;
        engine->set_speed(850);
        engine->backwards();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        engine->smooth_stop();

        engine->release_engine();
        delete engine;
        return 0;
    }

    //Test Case for compass: für das drehen des bugg
    bool test_magnetic_sensor = false;
    if (test_magnetic_sensor) {
        for (int i = 0; i < 10000; i++) {
//            _compass->check();
//            std::cout << "x,y regs: " << _compass->x << " " << _compass->y << std::endl;
            std::cout << "rot: " << _compass->get_rotation().value << std::endl;
            auto dir = _compass->get_direction();
            std::cout << "dir : " << dir.x << " " << dir.y << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }



    test_rotation_over_time(engine, _compass);
    //test_rectangle(engine, &ultrasonic, _compass);
//    test_move_to_point(engine, &ultrasonic, _compass);
//    test_turns(engine, &ultrasonic, magnet_sensor);

#endif //TEST_ON

	wasd_controller = new wasd_control{};
	wasd_controller->run(engine, &ultrasonic, _compass);



	// join thread
//	sensor_thread.join();
    release_resources();
}

