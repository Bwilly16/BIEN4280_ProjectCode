/*#include "mbed.h"
#include "USBSerial.h"
#include <I2C.h>
#include <ECahill_binaryutils.h>

USBSerial test;
Thread thread, thread1;
I2C i2c(p14, p15); //(SDA, SCL)
DigitalOut pullupResistor(p32); //Pin 32 = P1_0
DigitalOut port22(p22);
DigitalOut greenLED(LED3);
PwmOut blueLED(LED_BLUE);

unsigned int readReg = (0x39<<1) + 1; //Read and write registers
unsigned int writeReg = 0x39<<1;

typedef struct { // Mail struct
    uint32_t state;
} mail_t;

Mail<mail_t, 16> mail_box; 

void proximity(){
    char Data[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //char == uint8
    int i = 0;

    test.printf("readReg: %i\r\n", readReg);
    test.printf("writeReg: %i\r\n\r\n", writeReg);

    thread_sleep_for(10);

    Data[0] = 0x92; //146
    i2c.write(writeReg, Data, 1, true); //Check ID
    i2c.read(readReg, Data, 1); //Expect a 0xAB = 171 back

    test.printf("The sensor ID is: %d\r\n\r\n", Data[0]);

    if(Data[0] == 171){
            Data[0] = 0x90; //Increasing distance
            Data[1] = 0x30;
            i2c.write(writeReg, (const char *)Data, 2);

            Data[0] = 0x8F; //Increasing gain = improves readings of distant signals
            Data[1] = 0x0C;
            i2c.write(writeReg, (const char *)Data, 2);

        while(true){
            thread_sleep_for(50); //5 millisecond buffer

            Data[0] = 0x80; //Send start signal
            Data[1] = 0x25;
            i2c.write(writeReg, (const char *)Data, 2);

            thread_sleep_for(15);

            Data[0] = 0x9C; //Read data obtained
            i2c.write(writeReg, (const char *)Data, 1, true);
            i2c.read(readReg, Data, 1);

            mail_t *mail = mail_box.try_alloc(); //Put data into mailbox
            mail->state = Data[0];
            mail_box.put(mail);

            //test.printf("PDATA is: %d\r\n", Data[i]); //Data from chip

            if((Data[0] < 256) && (Data[0] >= 250)){ //Estimated calibrations
                test.printf("<=3 cm\r\n");
            }            
        }
    }
}

void LED_intensity(){
    int sample;
    float testing;

    while(true){
        osEvent switchState = mail_box.get(0);  // Retrieve mail

        if(switchState.status == osEventMail){  // If mail is retrieved:

            mail_t *mail = (mail_t*)switchState.value.p;
                                            
            if(mail->state < 5){ //If distance is < 5, indicating there is nothing detected above the sensor:
                blueLED = 1;
                greenLED = 0;
            }
            else if(mail->state >= 5){ //If distance is checked, update duty cycle based on distance
                blueLED.period(1.0f);
                greenLED = 1;

                testing = (255 - (mail->state)) * 0.0025;

                blueLED.write(testing);
            }
            mail_box.free(mail);
        }
    }
}

// main() runs in its own thread in the OS
int main() {
    thread.start(proximity);
    thread1.start(LED_intensity);
    pullupResistor = 1;
    port22 = 1;

    while (true) {
        thread_sleep_for(1000);
    }
}
*/





















#include "mbed.h"
#include "USBSerial.h"
#include <I2C.h>
#include <ECahill_binaryutils.h>

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

    //setT = PLACEHOLDER FOR COMMAND ENTERED;

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

            /*
            if(FARENHEIT COMMAND == TRUE){
                T = (1.8 * T) + 32;
                test.printf("Reading temperature: %i degrees F\r\n\r\n", T);
            }
            else{
                test.printf("Reading temperature: %i degrees C\r\n\r\n", T);
            }

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
            */
        }
    }
}

void setEFlag(){ //Send event flag to read_temperature
    PTEvent.set(SETTEMPERATURE);
}

int main() {
    thread.start(read_temperature);
    setbit(LEDDIR, 6); 
    setbit(LEDDIR, 24);

    redLED = 1; //Turns LED off when first setting bits, causes a short blink
    blueLED = 1;
    pullupResistor = 1;

    interruptTicker.attach(&setEFlag, 3.0);

    while (true) {
        thread_sleep_for(1000);
    }
}