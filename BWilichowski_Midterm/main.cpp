/*
Ben Wilichowski
10/18/2022
Four parts to the code
1. communicate using i2c with the color ID register
2. Obtain measurements from sensor
3. use mailbox to turn on LED per color
4. Calibrate colors with colors other than R,G,B
*/


#include "mbed.h"
#include "BWilichowski_binaryutils.h"
//#include "Ticker.h"
#include "USBSerial.h"
#include "I2C.h"
#include "DigitalOut.h"

#define LEDDIR (uint32_t*) 0x50000514  //In is ON, OUT is OFF
#define LEDSET (uint32_t*) 0x50000508 //Set LED Pin 
#define LEDCLEAR (uint32_t*) 0x5000050c //Clear LED Pin

I2C colors(I2C_SDA1, I2C_SCL1); //p0.14, p0.15
DigitalOut SetHigh(P1_0); //P1.0
DigitalOut SetHigh1(p22); //GPIO P0.22;

Thread thread1;
Thread thread2;
Thread thread3;
USBSerial MyMessage;

const int writeaddr = ((0x39 << 1) + 0); //write for board 0x72 //doesnt work
const int readaddr =  ((0x39 << 1) + 1); //read for board 0x73 //works

typedef struct 
{
 uint32_t Message;
 uint32_t Blue_grab;
 uint32_t Green_grab;
 uint32_t Red_grab;
 uint32_t Clear_grab;
 } mail_t;


Mail<mail_t, 20> mail_box;


void BlinkLED()
{

    PwmOut led(LED4); //BlueLED
    PwmOut led1(LED3);//green
    PwmOut led2(LED2); //red
    led.period_ms(4.0); 
    led1.period_ms(4.0);
    led2.period_ms(4.0);  

    uint16_t RedOut;
    uint16_t GreenOut;
    uint16_t BlueOut;
    uint16_t ClearOut;
    
    uint32_t Speed = 0;
     while (true) {
        
         if (!mail_box.empty()) //using the OS event from lab1 (maybe lab 0 i cant remember) did not work, so I tried this way.
         {
    mail_t *mail = mail_box.try_get();
    RedOut = mail -> Red_grab; //assigning my new OUT variables to the values in the mailbox
    GreenOut = mail -> Green_grab;
    BlueOut = mail -> Blue_grab;
    ClearOut = mail -> Clear_grab;
        
        
     if((RedOut == 512) && (BlueOut == 0) && (GreenOut == 0))
       {
           led2.write(0.5);
           led.write(1);
           led1.write(1); 
           
        MyMessage.printf("Red\n\r");
       }
       else if((BlueOut>GreenOut) && (BlueOut>RedOut) && (GreenOut>RedOut))
       {
           led.write(0.5);
           led2.write(1);
           led1.write(1);

         MyMessage.printf("Blue\n\r");
       }
       else if((GreenOut>BlueOut) && (GreenOut>RedOut))
       {
           led1.write(0.5);
           led2.write(1);
           led.write(1);

        MyMessage.printf("Green\n\r");
        
       }
       else if((BlueOut>GreenOut) && (RedOut>GreenOut)) //thought it would be clever to turn on both red and blue LED for a purple led
       {
           led.write(0.5);
           led1.write(1);
           led2.write(0.5);
           
         MyMessage.printf("Purple\n\r");
       }
       else if(((RedOut == 1024) | (RedOut == 768)) && (BlueOut == 256) && (GreenOut == 768))//((RedOut>BlueOut) && (RedOut >=GreenOut) && (GreenOut>BlueOut))
       {
           led.write(1);
           led1.write(0.5);
           led2.write(1);
           
         MyMessage.printf("Yellow\n\r");
       }
       else if((RedOut == 768) && (BlueOut == 256) && (GreenOut == 256))  //((RedOut>BlueOut) && (RedOut>GreenOut) && (BlueOut == GreenOut))
       {
           led.write(1);
           led1.write(1);
           led2.write(0.5);
           
          MyMessage.printf("Orange\n\r");
       }
            mail_box.free(mail);
         

        }
        thread_sleep_for(1000);
     }
     
}



void colorimetry() //thread to run communications with sensor. Show it is 'talking'
{
    
   // MyMessage.printf("Shift write %d\n\r", writeaddr);
   // MyMessage.printf("Shift read %d\n\r", readaddr);

    uint8_t data[2];
    char hold[1];
    char test1; //test ints to see if I am writing or reading properly
    char test2;

    uint16_t Red = 0;
    uint16_t Blue = 0;
    uint16_t Green = 0;
    uint16_t Clear = 0;

    uint16_t MSB = 0;
    uint16_t LSB = 0;
    uint16_t RedCombo = 0;
    uint16_t GreenCombo = 0;
    uint16_t BlueCombo = 0;
    uint16_t ClearCombo = 0;


    hold[0] = 0x00;
    data[0] = 0x92; //address of the ID register, output says 0xA8

    thread_sleep_for(1000);
    test1 = colors.write(writeaddr, (const char*) data, 1, true);
    test2 = colors.read(readaddr, hold, 1);
    //MyMessage.printf("is it writing properly? %d\n\r", test1);
    //MyMessage.printf("is it reading properly? %d\n\r", test2);
   // MyMessage.printf("Communication with sensor established\n\r");
   // MyMessage.printf("Is the output 0xA8? %d\n\r", hold[0]); //A8 = 168, AB = 171

//Turning on the sensor
    data[0] = 0x80; //write to this address
    data[1] = 0x13; //what is being written to address
    colors.write(writeaddr, (const char*) data, 2, true);
   // MyMessage.printf("Power to sensor on\n\r");


    while(true)
    {
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
   // MyMessage.printf("Red Ouptut: %d\n\r", RedCombo);


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
      // MyMessage.printf("Green Ouptut: %d\n\r", GreenCombo);

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
     // MyMessage.printf("Blue Ouptut: %d\n\r", BlueCombo);


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
        ClearCombo = ((MSB<<8)|LSB);
    // MyMessage.printf("Clear Ouptut: %d\n\r", ClearCombo);

        mail_t *mail = mail_box.alloc();

        mail-> Red_grab = RedCombo;
        mail-> Blue_grab = BlueCombo;
        mail-> Green_grab = GreenCombo;
        mail -> Clear_grab = ClearCombo;
        mail_box.put(mail);
    }
}

// main() runs in its own thread in the OS
int main()
{
    SetHigh = 1;
    SetHigh1 = 1;
    thread3.start(BlinkLED);
    thread1.start(colorimetry);
    while (true) 
    {
       thread_sleep_for(1000);
    }
}

