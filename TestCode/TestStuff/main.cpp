#include "mbed.h"
#include "Serial.h"
#include "USBSerial.h"

Serial pc(P1_3, P1_10);
USBSerial myMessage;

const int kMaxBufferSize = 100;
char buffer[kMaxBufferSize];
int len = 0;

int main(){

    buffer[0] = '\0';
    
    pc.printf("start...\n\n");

    while (true){
        while (pc.readable()) {
            char new_char = pc.getc();

            buffer[len++] = new_char;
            buffer[len] = '\0';

            if (new_char == '\n'){
               myMessage.printf("%s", buffer);
                len = 0;
            }
        }
    }
}