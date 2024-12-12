#pragma warning disable 520,2053
#include "mcc_generated_files/system/system.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>


#define LF          ((uint8_t)'\n')
#define CR          ((uint8_t)'\r')

uint8_t radio_Buffer[100];
uint8_t debug_Buffer[100];
char radio_Msg[100]; // keep it uint8_t as per radio data type 
char debug_Msg[100];

uint8_t rxIndex_Radio = 0;
uint8_t rxIndex_UART2 = 0;

bool isRadioDataRecieved = false;
bool isDebugDataReceived = false;


void toggleLED(uint16_t n);
void UART2_sendString(const char *str);
void UART1_sendString(const char *str);
void ProcessDebugDataUART2(void);
void ProcessRadioDataUART1(void);
void UART1_Check(void);
void UART2_Check(void);

void toggleLED(uint16_t n) {

    for (uint16_t i = 0; i < n; i++) {
        LED_Toggle();

        __delay_ms(1);
    }
}

void UART2_sendString(const char *str) {
    while (*str) {
        while (!(UART2_IsTxReady()));
        UART2_Write(*str++);
        while (!UART2_IsTxDone());
    }
}

void UART1_sendString(const char *str) {
    while (*str) {
        while (!(UART1_IsTxReady()));
        UART1_Write(*str++);
        while (!UART1_IsTxDone());
    }
}

void ProcessDebugDataUART2(void) {

    while (EUSART2_IsRxReady()) {
        uint8_t newChar = UART2_Read();
        if (newChar != CR && newChar != LF) {
            if (rxIndex_UART2 < sizeof (debug_Buffer) - 1) { // Prevent buffer overflow
                debug_Buffer[rxIndex_UART2++] = newChar;
            } else {
                // need to write some code to print 1 time statement if limit is above 
            }
        }

        if (newChar == LF || newChar == CR) {
            debug_Buffer[rxIndex_UART2] = '\0'; // NUll-terminater for string
            char Debug_Buff[100];
            sprintf(Debug_Buff, "%s", debug_Buffer); //uint8_t to string
            memcpy(debug_Msg, debug_Buffer, sizeof (Debug_Buff)); //copy buffer  - memcpy(dst, src, size(src) )
            memset(debug_Buffer, '\0', sizeof (debug_Buffer)); //clear debug buffer
            memset(Debug_Buff, '\0', sizeof (Debug_Buff)); //clear Debug buff
            UART2_sendString("Got: "); // send to debug
            UART2_sendString(debug_Msg); // send to debug
            strcat(debug_Msg, "\r"); // add CR
            UART1_sendString(debug_Msg); // send to radio
            memset(debug_Msg, '\0', sizeof (debug_Msg)); //clear debug MSG
            rxIndex_UART2 = 0; // reset index
        }

    }
    UART2_sendString("\n");
}

void ProcessRadioDataUART1(void) {
    while (EUSART1_IsRxReady()) {

        uint8_t newChar = UART1_Read();

        if (newChar != CR && newChar != LF) {
            if (rxIndex_Radio < sizeof (radio_Buffer) - 1) { // Prevent buffer overflow
                radio_Buffer[rxIndex_Radio++] = newChar;
            } else {
                // need to write some code to print 1 time statement if limit is above 
            }

        }


        if (newChar == LF) {
            toggleLED(50);
            radio_Buffer[rxIndex_Radio] = '\0'; // NUll-terminater for string
            char Radio_Buff[100];
            memcpy(radio_Msg, radio_Buffer, sizeof (radio_Buffer)); //copy buffer  - memcpy(dst, src, size(src) )
            memset(radio_Buffer, '\0', sizeof (radio_Buffer)); //clear radio buffer
            memset(Radio_Buff, '\0', sizeof (Radio_Buff)); //clear radio buffer

            UART2_sendString(radio_Msg);
            memset(radio_Msg, '\0', sizeof (radio_Msg)); //clear radio buffer
            rxIndex_Radio = 0; // reset index
            UART2_sendString("\n");
        }

    }
//    UART2_sendString("\n");
}

void UART1_Check(void) {
    isRadioDataRecieved = true;
}

void UART2_Check(void) {
    isDebugDataReceived = true;
}

int main(void) {
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    UART1.RxCompleteCallbackRegister(&UART1_Check);
    UART2.RxCompleteCallbackRegister(&UART2_Check);

    RadioEN_SetHigh(); //Enable radio 
    UART2_sendString("\n---PCIE UARTS COMMUNICATION CODE---\n\n");
    UART2_sendString("\n---SETUP COMPLETE---\n\n");
    __delay_ms(1000);

    while (1) {

        if (isDebugDataReceived) {
            isDebugDataReceived = false;
            ProcessDebugDataUART2();

        }

        if (isRadioDataRecieved) {
            isRadioDataRecieved = false;
            ProcessRadioDataUART1();
        }



    }
}