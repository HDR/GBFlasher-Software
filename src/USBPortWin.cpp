#include "const.h"
#include "USBPortWin.h"
#include "Settings.h"

extern "C"{
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * Headers for hardware support under Windows
 */
#include <windows.h>
}

USBPortWin::USBPortWin()
	:AbstractPort()
	
{
	
}


bool USBPortWin::open_port (QString)
{
    if(FT_Open(0,&ftHandle) != FT_OK)
        return false;
/* choose speed */
    if(Settings::speed == STANDARD){
        if(FT_SetBaudRate(ftHandle,185000) != FT_OK)
           return false;
    }
    else if(Settings::speed == LOW){
        if(FT_SetBaudRate(ftHandle,125000) != FT_OK)
           return false;
    }
    else if(Settings::speed == HIGH){
        if(FT_SetBaudRate(ftHandle,375000) != FT_OK)
           return false;
    }
    else if(Settings::speed == ULTRA){
        if(FT_SetBaudRate(ftHandle,1500000) != FT_OK)
            return false;
    }

	if(FT_SetLatencyTimer(ftHandle, 2) != FT_OK)
		return false;
	if(FT_SetDataCharacteristics(ftHandle,FT_BITS_8,FT_STOP_BITS_1,FT_PARITY_NONE) != FT_OK)
		return false;
	if(FT_SetDataCharacteristics(ftHandle,FT_BITS_8,FT_STOP_BITS_1,FT_PARITY_NONE) != FT_OK)
		return false;
	if(FT_SetTimeouts(ftHandle,5000,0) != FT_OK)
		return false;
	return true;/* all ok */

  
}

bool USBPortWin::close_port ()
{
	    FT_Close(ftHandle);
        return true;
}

int USBPortWin::send_packet (unsigned char packet[PACKETSIZE])
{
    DWORD sendCount;
    FT_Write(ftHandle, packet, PACKETSIZE, &sendCount);
    return sendCount;

}


bool USBPortWin::send_char (unsigned char character)
{
  	DWORD sendCount;
    
    FT_Write(ftHandle, &character, 1, &sendCount);
	return sendCount;   

}

int USBPortWin::receive_char (void)
{
    time_t tp = time(nullptr);
    unsigned char character;
	DWORD cbInQue = 0;
    DWORD recivedCount=0;
    FT_GetQueueStatus(ftHandle,&cbInQue);
    do
    {
        if (cbInQue > 0)
            FT_Read(ftHandle, &character, 1, &recivedCount);
        if(recivedCount != 0)
            break;
        FT_GetQueueStatus(ftHandle,&cbInQue);
    }while(time(nullptr) - tp < SLEEPTIME);

    if (recivedCount == 0)
        return TIMEOUT;
    if(character == ACK || character == END)
        return character;
    else 
        return NAK;      
     
}

int USBPortWin::receive_packet (unsigned char *packet)
{
    time_t tp = time(nullptr);
    DWORD recivedCount=0, bytesToReceive;
	DWORD cbInQue = 0;
    FT_GetQueueStatus(ftHandle,&cbInQue);
    do
    {
        if (cbInQue > 0)
            FT_Read(ftHandle, packet, 1, &recivedCount);
        FT_GetQueueStatus(ftHandle,&cbInQue);
    } while(time(nullptr) - tp < SLEEPTIME && recivedCount == 0);
    if (recivedCount == 0)
        return TIMEOUT;
    else
    {
        if(packet[0] != DATA)
        {
            if(packet[0] == ACK)
                return ACK;
            else if(packet[0] == END)
                    return END;
                 else
                    return NAK;
        }
        else
        {
            unsigned int bytesLeft = PACKETSIZE-1;
            tp = time(nullptr);
            recivedCount = 0;
            FT_GetQueueStatus(ftHandle,&cbInQue);
            do
            {
                if (cbInQue > 0)
                {
                    if (cbInQue > bytesLeft)
                        bytesToReceive = bytesLeft;
                    else
                        bytesToReceive = cbInQue;

						FT_Read(ftHandle, &packet[PACKETSIZE-bytesLeft], bytesToReceive, &recivedCount);
                        bytesLeft -= recivedCount;
                        tp = time(nullptr);
                }
                
                   
                FT_GetQueueStatus(ftHandle,&cbInQue);
            }while(time(nullptr) - tp < SLEEPTIME && bytesLeft != 0);

            if(bytesLeft > 0)
                return TIMEOUT;
            else
                return DATA;
        }
    }
}
