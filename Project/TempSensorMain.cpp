#include "mbed.h"
#include "USBSerial.h"
#include <I2C.h>

#define LEDDIR (uint32_t*) 0x50000514 
//#define SETPRESSURE (1UL << 4)
#define SETTEMPERATURE (1UL << 8)

unsigned int readReg = 0xEF;
unsigned int writeReg = 0xEE; 

Ticker interruptTicker;
USBSerial test;
Thread thread;
EventFlags PTEvent;
I2C i2c(p31, p2); //(SDA, SCL)
DigitalOut pullupResistor(p32); //Pin 32 = P1_0
DigitalOut redLED(LED2);
DigitalOut blueLED(LED4);
DigitalOut greenLED(LED3);

void read_temperature(){
    char temperatureData[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //char == uint8
    unsigned short AC5, AC6; //Initializing constants and variables
    short MC, MD;
    long X1 = 0, X2 = 0, B5 = 0, currentT = 0, setT = 0;
    uint16_t MSB, LSB;
    int i = 0;

    //setT = PLACEHOLDER FOR COMMAND ENTERED; //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    temperatureData[0] = 0xD0;
    i2c.write(writeReg, temperatureData, 1, true); //Send 0xD0 = 208
    i2c.read(readReg, temperatureData, 1); //Expect a 0x55 = 85 back

    test.printf("1: Array[0] = %i \r\n\r\n", temperatureData[0]);

    if(temperatureData[0] == 85){
        //AC5
        temperatureData[0] = 0xB2;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        MSB = temperatureData[0];
        temperatureData[0] = 0xB3;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        LSB = temperatureData[0];
        AC5 = ((LSB<<8)|MSB);

        //AC6
        temperatureData[0] = 0xB4;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        MSB = temperatureData[0];
        temperatureData[0] = 0xB5;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        LSB = temperatureData[0];
        AC6 = ((MSB<<8)|LSB);

        //MC
        temperatureData[0] = 0xBC;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        MSB = temperatureData[0];
        temperatureData[0] = 0xBD;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        LSB = temperatureData[0];
        MC = ((MSB<<8)|LSB);

        //MD
        temperatureData[0] = 0xBE;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        MSB = temperatureData[0];
        temperatureData[0] = 0xBF;
        i2c.write(writeReg, temperatureData, 1, true);
        i2c.read(readReg, temperatureData, 1);
        LSB = temperatureData[0];
        MD = ((MSB<<8)|LSB);

        while(true){
            int waitTemp = PTEvent.wait_any(SETTEMPERATURE); //Wait for event flag
        
            temperatureData[0] = 0xF4;
            temperatureData[1] = 0x2E;
            i2c.write(writeReg, (const char *)temperatureData, 2); //Send 0xF4 = 244 and 0x2E = 46 to start temperature reading

            thread_sleep_for(5); //Wait for 4.5 ~= 5 milliseconds

            temperatureData[0] = 0xF6;
            i2c.write(writeReg, (const char *)temperatureData, 1, true); //Send 0xF6 = 246 to read MSB
            i2c.read(readReg, temperatureData, 1);

            MSB = temperatureData[0];

            temperatureData[0] = 0xF7; //Send 0xF7 = 247 to read LSB
            i2c.write(writeReg, (const char *)temperatureData, 1, true);
            i2c.read(readReg, temperatureData, 1);

            LSB = temperatureData[0];
            currentT = ((MSB<<8)|LSB); //Combine two bytes

            X1 = (currentT - AC6) * (AC5 / (32768));
            X2 = (MC * 2048) / (X1 + MD);
            B5 = X1 + X2;
            currentT = (B5 + 8)/16;
            currentT = currentT/10;

            /* //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if(FARENHEIT COMMAND == TRUE){ 
                T = (1.8 * T) + 32;
                test.printf("Reading temperature: %i degrees F\r\n\r\n", T);
            }
            else{
                test.printf("Reading temperature: %i degrees C\r\n\r\n", T);
            }
            if(FARENEIT COMMAND == TRUE){
                if(CURRENTTEMP >= (SETTEMP + 1)){
                    greenLED = 0;
                    blueLED = 0;
                    redLED = 1;
                }
                else if(CURRENT TEMP <= (SETTEMP - 1)){
                    greenLED = 0;
                    redLED = 0;
                    blueLED = 1;
                }
                else if (CURRENTTEMP > (SETTEMP - 1) || CURRENTTEMP < (SETTEMP + 1)){
                    redLED = 0;
                    blueLED = 0;
                    greenLED = 1;
                }
            }
            else if(CELCIUS COMMAND == TRUE){
                 if(CURRENTTEMP >= (SETTEMP + 0.5)){
                    greenLED = 0;
                    blueLED = 0;
                    redLED = 1;
                }
                else if(CURRENT TEMP <= (SETTEMP - 0.5)){
                    greenLED = 0;
                    redLED = 0;
                    blueLED = 1;
                }
                else if (CURRENTTEMP > (SETTEMP - 0.5) || CURRENTTEMP < (SETTEMP + 0.5)){
                    redLED = 0;
                    blueLED = 0;
                    greenLED = 1;
                }
            }
            */
        }
    }
}

void setEFlag(){ //Send event flag to read_temperature
    PTEvent.set(SETTEMPERATURE);
}

int main() {
    thread.start(read_temperature);
	*LEDDIR = *LEDDIR | (1 << 6);
    *LEDDIR = *LEDDIR | (1 << 24);

    redLED = 1; //Turns LED off when first setting bits, causes a short blink
    blueLED = 1;
    pullupResistor = 1;

    interruptTicker.attach(&setEFlag, 3.0);

    while (true) {
        thread_sleep_for(1000);
    }
}