#include <magnetic_sensor/magnetic_sensor.h>
#include <iostream>
#include <wiringPiI2C.h>
#include <cmath>
#include <unistd.h>
#include <chrono>
#include <util/time_util.h>

#define PI		3.1415926535

compass::compass(){
	//Setup für den Filehandle für die Funktionen von wiringPiI2C
	this->fd = wiringPiI2CSetup(sensor_address);
	
	//Werte erstmals auf Null setzten
	this->x = 0;
	this->y = 0;
	this->z = 0;
	
	if(this->fd == -1){
		std::cout << "I2CSetup for Magnetic Sensor didn't work" << std::endl;
	}
	else {
        wiringPiI2CWriteReg8(this->fd, 0x0a, 0x80);
        busy_wait(std::chrono::milliseconds{1});
		//Testweise einstellen des Sensors für erste Werte
		//Empfolende Werte für das SET/RESET Register
		wiringPiI2CWriteReg8(this->fd, 0x0b, 0x01);
		//Hier erstmal ein Beispiel Setup aus dem Datenblatt
		wiringPiI2CWriteReg8(this->fd, 0x09, 0x1d);
        busy_wait(std::chrono::milliseconds{1});
	}
}

int compass::check(){
	int tmp = wiringPiI2CReadReg8(this->fd, 0x06);
	// erstes Bit des Statusregisters zeigt an ob neue Daten zur Verfügung stehen
	//Modulo hier also um einfach nachzuverfolgen ob dieses Bit gesetzt ist.
#ifdef DEBUG_MAGNETIC_SENSOR
	std::cout << "handle: " << fd << std::endl;
	std::cout << "Update: " << (tmp % 1) << std::endl;
#endif
	if((tmp % 2) != 0){
		//TO-DO Werte einlesen, für die 3 Achsen kombinieren und dann in die Klasse-attribute übertragen
		//gerade nur simples auslesen der Daten zum testen
		//Einlese der Register
		int data[6];
		data[0] = wiringPiI2CReadReg8(this->fd, X_LSB);
		data[1] = wiringPiI2CReadReg8(this->fd, X_MSB);
		data[2] = wiringPiI2CReadReg8(this->fd, Y_LSB);
		data[3] = wiringPiI2CReadReg8(this->fd, Y_MSB);
		data[4] = wiringPiI2CReadReg8(this->fd, Z_LSB);
		data[5] = wiringPiI2CReadReg8(this->fd, Z_MSB);
		
		//Kombinieren der Daten (x, y und z sind 16 Bit signed Integers, dadurch wird das zweier Komplement
		//ohne größere Umstände angewandt
		this->x = data[0] | (data[1] << 8);
		this->y = data[2] | (data[3] << 8);
		this->z = data[4] | (data[5] << 8);
		
		return 1;
	}

	return 0;
}

int compass::getX(){
	return this->x;
}
int compass::getY(){
	return this->y;
}
int compass::getZ(){
	return this->z;
}

double compass::testDirection(){
	double dX = this->x;
	double dY = this->y;
	std::cout << atan((dX/dY)) << std::endl;
	std::cout << atan(5)*180/PI << std::endl;
	if(this->y > 0){
		return 90 - atan(dX/dY)*180/PI;
	} else if(this->y < 0){
		return 270 - atan(dX/dY)*180/PI;
	} else{
		if(this->x < 0){
			return 180;	
		} else {
			return 0;
		}
	}
}

int compass::release_resources() {
    if(fd != -1){
        wiringPiI2CWriteReg8(this->fd, 0x09, 0x00);
        return close(fd);
    }
    return 0;
}

