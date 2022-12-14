#include "mbed.h"
#include "USBSerial.h"
#include <I2C.h>

#define LEDDIR (uint32_t*) 0x50000514 
#define SETTEMPERATURE (1UL << 8)
#define SETPROXIMITY (1UL << 4)
//#define SETCOLOR (1UL << 2)

unsigned int readReg = 0xEF; //Read and write registers for temperature sensor
unsigned int writeReg = 0xEE; 

unsigned int readRegProx = (0x39<<1) + 1; //Read and write registers for proximity sensor
unsigned int writeRegProx = 0x39<<1;

const int writeaddr = ((0x39 << 1) + 0); //write for board 0x72 //doesnt work
const int readaddr =  ((0x39 << 1) + 1); //read for board 0x73 //works

Ticker interruptTicker;
USBSerial test;
Thread thread, thread1, thread2;
EventFlags PTEvent;
I2C i2c(p31, p2); //(SDA, SCL)
I2C colors(I2C_SDA1, I2C_SCL1); //p0.14, p0.15
DigitalOut port22(p22);
DigitalOut pullupResistor(p32); //Pin 32 = P1_0
DigitalOut redLED(LED2);
DigitalOut blueLED(LED4);
DigitalOut greenLED(LED3);
DigitalOut SetHigh(P1_0); //P1.0

uint16_t result;

void read_temperature(){
    //test.printf("in read_temp now");
    char temperatureData[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //char == uint8
    unsigned short AC5, AC6; //Initializing constants and variables
    short MC, MD;
    long X1 = 0, X2 = 0, B5 = 0, currentT = 0, setT = 0;
    uint16_t MSB, LSB;
    int i = 0;

    temperatureData[0] = 0xD0;
    i2c.write(writeReg, temperatureData, 1, true); //Send 0xD0 = 208
    i2c.read(readReg, temperatureData, 1); //Expect a 0x55 = 85 back

    //test.printf("1: Array[0] = %i \r\n\r\n", temperatureData[0]); //Check for signal from sensor

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
            //printf("\n\r in while loop\n\r");
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

            if(result == 4){ 
                setT = 70;
                currentT = (1.8 * currentT) + 32;
                test.printf("\r\nTemperature %i", currentT);
            }
            else{
                setT = 40;
                test.printf("\r\nTemperature %i", currentT);
            }

            if(result == 4){
                if(currentT >= (setT + 1)){ //If detected temp is higher than set temp, LED = red
                    greenLED = 1;
                    blueLED = 1;
                    redLED = 0;
                }
                else if(currentT <= (setT - 1)){ //If detected temp is lower than set temp, LED = blue
                    greenLED = 1;
                    redLED = 1;
                    blueLED = 0;
                }
                else if (currentT > (setT - 1) || currentT < (setT + 1)){ //If detected temp is set temp, LED = green
                    redLED = 1;
                    blueLED = 1;
                    greenLED = 0;
                }
            }

            else if(result == (3)){
                if(currentT >= (setT + 0.5)){ //If detected temp is higher than set temp, LED = red
                    greenLED = 1;
                    blueLED = 1;
                    redLED = 0;
                }
                else if(currentT <= (setT - 0.5)){ //If detected temp is lower than set temp, LED = blue
                    greenLED = 1;
                    redLED = 1;
                    blueLED = 0;
                }
                else if (currentT > (setT - 0.5) || currentT < (setT + 0.5)){ //If detected temp is set temp, LED = green
                    redLED = 1;
                    blueLED = 1;
                    greenLED = 0;
                }
            }
        }
    }
}

void proximity_sensor(){
   
    char Data[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //char == uint8
    int i = 0;
    long setProx = 0; //Set to result from speech to text
 
    thread_sleep_for(10);

    Data[0] = 0x92; //146
   
   colors.write(writeRegProx, Data, 1, true); //Check ID
   colors.read(readRegProx, Data, 1); //Expect a 0xAB = 171 back
    //test.printf("The sensor ID is: %d\r\n\r\n", Data[0]); //Check for connection
    //test.printf("%i, %i          ", testy, testy2);
    if(Data[0] == 171){
            Data[0] = 0x90; //Increasing distance
            Data[1] = 0x30;
            colors.write(writeRegProx, (const char *)Data, 2);

            Data[0] = 0x8F; //Increasing gain = improves readings of distant signals
            Data[1] = 0x0C;
            colors.write(writeRegProx, (const char *)Data, 2);

        while(true){
            thread_sleep_for(50); //5 millisecond buffer

            Data[0] = 0x80; //Send start signal
            Data[1] = 0x25;
            colors.write(writeRegProx, (const char *)Data, 2);

            thread_sleep_for(15); //Additional buffer time

            Data[0] = 0x9C; //Read data obtained
            colors.write(writeRegProx, (const char *)Data, 1, true);
            colors.read(readRegProx, Data, 1);

            //test.printf("PDATA is: %d\r\n", Data[i]); //Data from chip

            setProx = 50; //This = 10 cm

            if((Data[0] < 60) && (Data[0] > 40)) { //Print if detected distance is close to set distance
                greenLED = 0;
            }
            else{
                greenLED = 1;
            }
            test.printf("At %i cm?\r\n      ", setProx);
        }
    }
}

void color_sensor() {

    uint8_t data[2];
    char hold[1];
    char test1, test2;
    uint16_t Red = 0, Blue = 0, Green = 0, Clear = 0;
    uint16_t MSB = 0, LSB = 0, RedCombo = 0, GreenCombo = 0, BlueCombo = 0, ClearCombo = 0;
    int redInt, greenInt, blueInt;
    long remainder;
    int i = 0, j = 0;

    hold[0] = 0x00;
    data[0] = 0x92; //address of the ID register, output says 0xA8

    thread_sleep_for(1000);

    test1 = colors.write(writeaddr, (const char*) data, 1, true);
    test2 = colors.read(readaddr, hold, 1);

    //MyMessage.printf("Is the output 0xA8? %d\n\r", hold[0]); //A8 = 168, AB = 171

    //Turning on the sensor
    data[0] = 0x80; //write to this address
    data[1] = 0x13; //what is being written to address
    colors.write(writeaddr, (const char*) data, 2, true);

    while(true) {        

        //WAIT FOR COMMAND FROM USER TO ENTER RED VALUE
        //Reading red lower and upper bit
        thread_sleep_for(100);
        data[0] = 0x96;
        test1 = colors.write(writeaddr, (const char*) data, 1, true);
        test2 = colors.read(readaddr, hold, 1, false);
        MSB = hold[0];
        data[0] = 0x97;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        LSB = hold[0];

        RedCombo = ((MSB<<8)|LSB);
        redInt = RedCombo/4;

        //MyMessage.printf("Red Ouptut: %d\n\r", RedCombo);

        //WAIT FOR COMMAND FROM USER TO ENTER GREEN VALUE
        //Reading Green lower and upper bit
        thread_sleep_for(100);
        data[0] = 0x98;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        MSB = hold[0];
        data[0] = 0x99;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        LSB = hold[0];

        GreenCombo = ((MSB<<8)|LSB);
        greenInt = GreenCombo/4;
      
        //MyMessage.printf("Green Ouptut: %d\n\r", GreenCombo);

        //WAIT FOR COMMAND FROM USER TO ENTER BLUE VALUE
        //Reading Blue lower and upper bit
        thread_sleep_for(100);
        data[0] = 0x9A;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        MSB = hold[0];
        data[0] = 0x9B;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        LSB = hold[0];

        BlueCombo = ((MSB<<8)|LSB);
        blueInt = BlueCombo/4;
    
        //MyMessage.printf("Blue Ouptut: %d\n\r", BlueCombo);

        //Reading clear lower and upper bit
        thread_sleep_for(100);
        data[0] = 0x94;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        MSB = hold[0];
        data[0] = 0x95;
        colors.write(writeaddr, (const char*) data, 1, true);
        colors.read(readaddr, hold, 1, false);
        LSB = hold[0];

        ClearCombo = ((MSB<<8)|LSB); //1024 Maximum

        //test.printf("R%i G%i B%i\r\n", redInt, greenInt, blueInt);

        if((redInt > 300) && (blueInt < 100) && (greenInt < 100)) { 
           test.printf("Red\n\r              ");
        }
        else if((blueInt > greenInt) && (blueInt > redInt) && (greenInt > redInt)) {
             test.printf("Blue\n\r           ");
        }
        else if((greenInt > blueInt) && (greenInt > redInt)) {
            test.printf("Green\n\r            ");
        }
        else if((blueInt > greenInt) && (redInt > greenInt)) { 
            test.printf("Purple\n\r            ");
        }
        else if((redInt > blueInt) && (redInt > greenInt) && (greenInt > 200) && (blueInt > 100)) { 
            test.printf("Yellow\n\r            ");
        }
        else if((redInt > blueInt) && (redInt > greenInt) && (greenInt < 200) && (blueInt < 150)) { 
            test.printf("Orange\n\r             ");
        }
    }
}

int main() {

    redLED = 1; //Turns LED off when first setting bits, causes a short blink
    blueLED = 1;
    greenLED = 1;

    pullupResistor = 1;
    port22 = 1;

    result = test.getc();
    result = result - 48;
    //test.printf("Result: %i",result);
    
    if(result == 1){
        thread.start(color_sensor);
    }
    else if(result == 2){
        thread.start(proximity_sensor);  
    }
    else if(result == 3){
        thread.start(read_temperature);
    }
    else if(result == 4){
        thread.start(read_temperature);
    }
    else{
        test.printf("Error number    ");
    }

    while (true) {
        thread_sleep_for(1000);
    }
}