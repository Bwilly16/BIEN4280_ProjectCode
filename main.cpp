#include "mbed.h"
#include "USBSerial.h"
#include "Ticker.h"
#include "DigitalOut.h"
#include "I2C.h"
#include "Mutex.h"

#define TEMPERATURE (1UL << 0)
#define PRESSURE (1UL << 1)

USBSerial MyCereal; 

Thread thread;
Thread thread1;
Thread thread2;

Ticker flipper;

DigitalOut led (LED2);    // red LED
DigitalOut flash (LED3);  // green LED
DigitalOut high(p32);  // Setting the SCL and SDA to High, or 1

EventFlags event_flag; 

const int writeaddr8bit = 0xEE; // write address
const int readaddr8bit = 0xEF;  // read address

I2C i2c(I2C_SDA0,I2C_SCL0);

Mutex splitter_mutex;

/* Addresses for each of the registers
AC1 = 0xAA, 0xAB;
AC2 = 0xAC, 0xAD
AC3 = 0xAE, 0xAF
AC4 = 0xB0 0xB1
AC5 = 0xB2, 0xB3
AC6 = 0xB4, 0xB5
B1 = 0xB6, 0xB7
B2 = 0xB8, 0xB9
MB = 0xBA, 0xBB
MC = 0xBC, 0xBD
MD = 0xBE, 0xBF
*/


/* In this part, write routines that communicate with our onboard HTS221 temperature and humidity
sensor. You will find documentation on D2L for the sensor; this documentation contains addresses, rules 
for communication, as well as calibration instructions for your sensor. Use MBed’s I2C class for
communication. 

Obtain and establish the necessary calibration functions for temperature and humidity. Once you
have calculated the calibration constants, modify the read_humidity and read_temperature threads to
obtain one-shot humidity and temperature measurements from the sensor. 

Also, modify your threads to print the current temperature and humidity values to the console. 
Start the read_humidity and read_temperature threads. Test it out by breathing on your chip! Take screenshots of at least 3 
different environments (ideally with different values) to show that your code works.*/
/*
void notify(const char *name, int state)
{
    splitter_mutex.lock();
    printf("%s: %d\n\r", name, state);
    splitter_mutex.unlock();
}
*/

void setup()

{

char subaddr[8];
char data[1];

uint16_t eight;
uint16_t sixteen;
uint16_t test1;
uint16_t test2;


uint16_t AC5;   //Temperature
uint16_t AC6;   //Temperature
int16_t MC;     //Temperature
int16_t MD;     //Temperature
int16_t x1;     // 
int16_t x2;     // 
int32_t x3;     //
uint32_t B4;    //
int16_t B5;     //
int32_t B6;     //
int32_t B7;     //
int16_t Temperature; 
uint16_t MSB;
uint16_t LSB;
uint16_t XLSB;
uint32_t UT;
uint16_t oss;
uint16_t USLB;
int32_t UP;
int32_t t2;


char t[2];
char arr[2];
char t1[3];
char p[2];
t1[0] = 0xF4;
t1[1] = (0x34 + (oss << 6));



// ID "WHO AM I?"" register for testing
subaddr[0] = 0xD0;

    i2c.write(writeaddr8bit,(const char*)subaddr, 1,true); 
    i2c.read(readaddr8bit, data , 1 , false);
    MyCereal.printf("Who am I? Lord Flacko %d \r\n", data[0]);

 //   MyCereal.printf("Who am I? Lord Flacko W %i \r\n",  test1);
 //   MyCereal.printf("Who am I? Lord Flacko R %i \r\n",  test2);



////////////////////////////////////////////AC1 addy brosef./////////////////////////////////
subaddr[0] = 0xAA;

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf(" %d \r\n", eight);


subaddr[0] = 0xAB;
        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf(" %d \r\n", sixteen);

        AC1 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", AC1);

//////////////////////////////////////////AC2 addy brosef/////////////////////////////////////
subaddr[0] = 0xAC;

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);

subaddr[0] = 0xAD;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        AC2 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", AC2);
//////////////////////////////////////////AC3 addy brosef/////////////////////////////////////
subaddr [0] = 0xAE;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf(" %d \r\n", eight);


subaddr[0] = 0xAF;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen );

        AC2 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", AC2);
///////////////////////////////////////////AC4 addy brosef////////////////////////////////////
subaddr [0] = 0xB0;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xB1;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        AC4 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", AC4);
///////////////////////////////////////////AC5 addy brosef////////////////////////////////////

subaddr [0] = 0xB2;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xB3;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf(" %d \r\n", sixteen);

        AC5 = ((eight << 8) | sixteen);
        MyCereal.printf(" %d \r\n", AC5);

///////////////////////////////////////////AC6 addy brosef////////////////////////////////////
subaddr [0] = 0xB4;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf(" %d \r\n", eight);


subaddr[0] = 0xB5;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        AC6 = ((eight << 8) | sixteen);
        MyCereal.printf(" %d \r\n", AC6);
///////////////////////////////////////////B1 addy brosef////////////////////////////////////
subaddr [0] = 0xB6;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xB7;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        B1 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", B1);
///////////////////////////////////////////B2 addy brosef////////////////////////////////////
subaddr [0] = 0xB8;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xB9;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        B2 = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", B2);
///////////////////////////////////////////MB addy brosef////////////////////////////////////

subaddr [0] = 0xBA;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xBB;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        MB = ((eight << 8) | sixteen);
        MyCereal.printf("%d \r\n", MB);
///////////////////////////////////////////MC addy brosef////////////////////////////////////

subaddr [0] = 0xBC;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xBD;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        MC = ((eight) << 8 | sixteen);
        MyCereal.printf("%d \r\n", MC);
///////////////////////////////////////////MD addy brosef////////////////////////////////////
subaddr [0] = 0xBE;    

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        eight = data[0];
        MyCereal.printf("%d \r\n", eight);


subaddr[0] = 0xBF;      

        i2c.write(writeaddr8bit,(const char*)subaddr, 1,true);
        i2c.read(readaddr8bit, data , 1 , false);
        sixteen = data[0];
        MyCereal.printf("%d \r\n", sixteen);

        MD = ((eight) << 8 | sixteen);
        MyCereal.printf("%d \r\n", MD);




while(true)
{

    t[0] = 0xF4;    // Write address
    t[1] = 0x2E;    // Information written
    
    
  ///////////////////////////////Temperature Calculations////////////////////////////////   
    i2c.write(writeaddr8bit, (const char*) t, 2, true); // writing to the adress
    
    thread_sleep_for(2000); // allows proper time to mess with the chip
    t[0] = 0xF6;
    i2c.write(writeaddr8bit, (const char*) t, 1, true); // Writin gin the address
    
    i2c.read(readaddr8bit, p, 1, false); // reading in the address
    
    MSB = p[0];     
    LSB = p[1];
 
    UT = ((MSB << 8)|LSB);

    x1 = (UT - AC6)* AC5 / pow(2,15);
    x2 = MC * pow(2,11) / (x1 + MD);
    B5 = x1 + x2;
    Temperature = ((B5|8) / pow(2,4)*0.1); // Getting temp, 0.1 added since temp in 0.1*C

    MyCereal.printf("Chip Temperature: %d     \n\r", Temperature);

 ///////////////////////////////Pressure Calculations////////////////////////////////

    i2c.write(writeaddr8bit, (const char*) t, 2, true);
    
    thread_sleep_for(2000);

    t1[0] = 0xF6;
    t1[1] = 0xF7;
    t1[2] = 0xF8;

    i2c.read(readaddr8bit, t1, 3, false);

int i = 0;

void flip()
{
    if( i == 0)
    {
        event_flag.set(TEMPERATURE);
        i = 1;
    }

}

void read_temperature (void)

{  
              // attach the address of the flip function to the rising edge
    while(1) 
    {           // wait around, interrupts will interrupt this!
        splitter_mutex.lock();
        MyCereal.printf("LED ON TEMPERATURE \r\n"); 
        event_flag.wait_all(TEMPERATURE);
        flash = 0;
 

        thread_sleep_for(1000);


        flash = 1;


        MyCereal.printf("LED OFF TEMPERATURE \r\n"); 

        splitter_mutex.unlock(); 
    }
}


int main()
{
    // the address of the function to be attached (flip) and the interval (2 seconds)
    flipper.attach(flip, 2);  
    high = 1;

    thread2.start(setup);
    thread1.start(read_temperature);

    while (true) {
  
        ThisThread::sleep_for(5000);

    }
}

