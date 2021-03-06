#include "dvd.h"

volatile long *dvd = (volatile long *) 0xCC006000;

void DVD_CallFunc(u32 fnAddress)
{
	dvd[0] = 0x14; //DEINT clr, TCINT clr
//	dvd[0] = 0x2e;
	dvd[1] = 0;
	dvd[2] = 0xFE120000;	
	dvd[3] = fnAddress;
	dvd[4] = 0x66756e63;	
	dvd[8] = 0;
	dvd[7] = 1;

}

int DVD_WaitImmediate()
{
	u32 nCount = 0;

	while ((dvd[7] & 1)) {
		if(++nCount > 0xFFFFFF) {
			return 0;
		}
	}

	return 1;
}

int DVD_CustomDbgCommand(u32 dwCommand, u32 dwOffset, u32 dwLength, u32* pBuffer)
{
	dvd[0] = 0x10;//2e;
	dvd[1] = 0;
	dvd[2] = dwCommand;
	dvd[3] = dwOffset;
	dvd[4] = dwLength;
	dvd[5] = (u32) pBuffer;
	dvd[6] = dwLength;
	dvd[7] = 1;

	DVD_WaitImmediate();

	if (dvd[0] & 0x04) {
		return 1;
	}

	return 0;
}

u32 DVD_ReadDriveMemDword(u32 dwAddress)
{
	dvd[0] = 0x14;
//	dvd[0] = 0x2e;
	dvd[1] = 0;
	dvd[2] = 0xFE010000;	
	dvd[3] = dwAddress;
	dvd[4] = 0x00010000;	
	dvd[8] = 0;
	dvd[7] = 1;

	if (DVD_WaitImmediate() == 1)
	{
		return dvd[8];
	}
	return 0xDDDDDDDD;
}

u32 DVD_RequestError() {

	dvd[2] = 0xe0000000;
	dvd[7] = 1;

	return dvd[8];
}

int DVD_SetDebugMode1()
{
	dvd[0] = 0x10;
	dvd[1] = 0;
	dvd[2] = 0xff014d41;
	dvd[3] = 0x54534849;
	dvd[4] = 0x54410200;
	dvd[7] = 1;
	
	DVD_WaitImmediate();

	if (dvd[0] & 0x04) {
		return 1;
	}
	
	return 0;
}



int DVD_SetDebugMode2()
{
	dvd[0] = 0x10;
	dvd[1] = 0;
	dvd[2] = 0xff004456;
	dvd[3] = 0x442d4741;
	dvd[4] = 0x4d450300;
	dvd[7] = 1;
	
	DVD_WaitImmediate();

	if (dvd[0] & 0x04) {
		return 1;
	}
	
	return 0;
}


void dvd_unlock()
{

    dvd[0] |= 0x00000014;
    dvd[1] = 0;
    dvd[2] = 0xFF014D41;
    dvd[3] = 0x54534849;
    dvd[4] = 0x54410200;
    dvd[7] = 1;
    while ((dvd[0] & 0x14) == 0) { }
    dvd[0] |= 0x00000014;
    dvd[1] = 0;
    dvd[2] = 0xFF004456;
    dvd[3] = 0x442D4741;
    dvd[4] = 0x4D450300;
    dvd[7] = 1;
    while ((dvd[0] & 0x14) == 0) { }
}

int DVD_SetDebugMode()
{
	DVD_SetDebugMode1();
	DVD_SetDebugMode2();
//dvd_unlock();

	return 0;
}

int DVD_WriteDriveMemDword(u32 dwAddress, u32 dwData)
{
	int nCount = 0;

	dvd[0] = 0x2e;
	dvd[1] = 0;
	dvd[2] = 0xFE010100;	

	dvd[3] = dwAddress;
	dvd[4] = 0x00040000;	
	dvd[5] = 0;
	dvd[6] = 0;
	dvd[7] = 3;

	DVD_WaitImmediate();

	
	dvd[0] = 0x2e;
	dvd[1] = 0;
	dvd[2] = dwData;
	dvd[7] = 1;

	DVD_WaitImmediate();

	return 0;
}


int DVD_WriteDriveMemBlock(u32 dwAddress, void* pData, u32 dwSize)
{
	u32* pSource = (u32*)pData;
	u32 dwDest = dwAddress;
	int nLeft = dwSize;

	while (nLeft > 0)
	{
		DVD_WriteDriveMemDword(dwDest, *pSource++);//(dwAddress, *pSource++);
		dwDest += 4;
		nLeft -= 4;
	}
	return 0;
}

int DVD_ReadDriveMemBlock(u32 dwAddress, void* pData, u32 dwSize)
{
	u32* pSource = (u32 *) pData;
	u32 dwDest = dwAddress;
	int nLeft = dwSize;

	while(nLeft > 0) {
		*pSource++ = DVD_ReadDriveMemDword(dwDest);//(dwAddress);
//		dwAddress += 4;
		dwDest += 4;
		nLeft -= 4;
	}
	return 0;
}