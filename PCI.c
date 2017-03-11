/** @file
    A simple, basic, application showing how the Hello application could be
    built using the "Standard C Libraries" from StdLib.

    Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
    This program and the accompanying materials
    are licensed and made available under the terms and conditions of the BSD License
    which accompanies this distribution. The full text of the license may be found at
    http://opensource.org/licenses/bsd-license.

    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

/***
  Demonstrates basic workings of the main() function by displaying a
  welcoming message.

  Note that the UEFI command line is composed of 16-bit UCS2 wide characters.
  The easiest way to access the command line parameters is to cast Argv as:
      wchar_t **wArgv = (wchar_t **)Argv;

  @param[in]  Argc    Number of argument tokens pointed to by Argv.
  @param[in]  Argv    Array of Argc pointers to command line tokens.

  @retval  0         The application exited normally.
  @retval  Other     An error occurred.
***/

#include <Uefi.h>                              //MdePky\Include\Uefi.h
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>                   //MdePkg\Include\Library\UefiLib.h
#include <Library/UefiApplicationEntryPoint.h> //MdePkg\Include\Library\UefiApplicationEntryPoint.h
#include <Protocol/PciRootBridgeIo.h>          //MdePkg\Include\Protocol\PciRootBridgeIo.h
#include <Library/PciLib.h>                    //MdePkg\Include\Library\PciLib.h
#include <Library/PciExpressLib.h>             //MdePkg\Include\Library\PciExpressLib.h
#include "PCI.h"
#include <Library/ShellCEntryLib.h>
#include <stdio.h> 
#include <Library/PcdLib.h>
#include "pci_class.h"
#include "AsciiFileAPI.h"
#include <Protocol/SimpleFileSystem.h>
#include <Library/ShellLib.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

extern EFI_BOOT_SERVICES         *gBS;
extern EFI_SYSTEM_TABLE			 *gST;
extern EFI_RUNTIME_SERVICES 	 *gRT;

#define UTILITY_NAME	L"PCI.efi"
#define UTILITY_VERSION	L"V1.00.00"
#define UTILITY_DATE	L"2017/03/11"

UINT32 g_PCI_CNT  = 0;
UINT32 g_PCIE_CNT = 0;
char    gAction[40];

CHAR16 *DevicePortTypeTable[] = {
  L"PCI Express Endpoint",
  L"Legacy PCI Express Endpoint",
  L"Unknown Type",
  L"Unknonw Type",
  L"Root Port of PCI Express Root Complex",
  L"Upstream Port of PCI Express Switch",
  L"Downstream Port of PCI Express Switch",
  L"PCI Express to PCI/PCI-X Bridge",
  L"PCI/PCI-X to PCI Express Bridge",
  L"Root Complex Integrated Endpoint",
  L"Root Complex Event Collector"
};
const int BUFFER_SIZE=250;

struct PcieinfoStr
{
	CHAR16 vidstr[10];
	CHAR16 devstr[10];
	CHAR16 busstr[10];
	CHAR16 funcstr[10];
	CHAR16 maxstr[10];
	CHAR16 negostr[10];
	CHAR16 curlnkspd[20];
	CHAR16 clacodestr[15];
	CHAR16 devtystr[50];
	CHAR16 payload[10];
	CHAR16 pci_count[10];
	CHAR16 pcie_count[10];
};

struct Pcieinfo
{
	UINT16 vidstr;
	UINT8 devstr;
	UINT8 busstr;
	UINT8 funcstr;
	UINT16 maxstr;
	UINT16 negostr;
	UINT16 curlnkspd;
	UINT16 clacodestr;
	UINT16 devtystr;
	UINT16 payload;
	UINT16 dev_type;
};

struct Pcieinfo DataBuff[250];

BOOLEAN CheckPCIE(UINT8 Bus, UINT8 Device, UINT8 Func)
{
	int rtn=0, i=0;
	UINT8 ID=0, NextPtr=0, CurrentPtr=0, Hit_Addr=0, run=1, hit=0, capability_ptr=0;
	UINT16 maxlink=0, negolink=0, link_unknown=0, current_link_speed=0, payload=0, dev_type;
	UINT16 id_nextptr=0, vendor=0, device=0, temp=0;
	UINT32 ccode=0, vendev=0;
	UINT8 capareg_lo=0, capareg_hi=0;
	
	vendev=PciRead32(PCI_LIB_ADDRESS(Bus, Device, Func, 0x00)); //Offset-0x00, DWORD, Vendor+Device
	if( 0xFFFFFFFF != (vendev & 0xFFFFFFFF) ) {
	
		capability_ptr = PciRead8(PCI_LIB_ADDRESS(Bus, Device, Func, CapaPtr_Offset)); 
		if(capability_ptr != 0x00) {
			CurrentPtr = capability_ptr;
			id_nextptr = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, CurrentPtr));
			ID         = id_nextptr & 0x00FF;    //Capability ID
			NextPtr    = (id_nextptr&0xFF00)>>8; //Next Capability Pointer
			while(run) {
				switch(ID) {
					case 0x00:
						run=0;
						break;
					case 0x10: //Find a PCIE Device (Capability ID=10h)
						hit=1;
						run=0;
						Hit_Addr=CurrentPtr;						
						break;
					default:
						if( (NextPtr!=0) && (CurrentPtr!=NextPtr) ) {
							CurrentPtr = NextPtr;                //Avoid endless loop
							id_nextptr = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, NextPtr));
							ID         = id_nextptr & 0x00FF;    //Capability ID
							NextPtr    = (id_nextptr&0xFF00)>>8; //Next Capability Pointer
						} else {
							run=0;
						}
						break;
				}
			}
			
			if(hit) {
				payload = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, Hit_Addr+DeviceCont_Offset));
				maxlink = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, Hit_Addr+LinkCapaReg_Offset));
				negolink=PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, (Hit_Addr+LinkStaReg_Offset)));
				current_link_speed=PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, (Hit_Addr+LinkStaReg_Offset)));
				dev_type = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, Hit_Addr+CapaReg_Lo_Offset));
				
				DataBuff[g_PCIE_CNT].payload = payload;
				DataBuff[g_PCIE_CNT].maxstr = maxlink;
				DataBuff[g_PCIE_CNT].busstr = Bus;
				DataBuff[g_PCIE_CNT].devstr = Device;
				DataBuff[g_PCIE_CNT].funcstr = Func;
				DataBuff[g_PCIE_CNT].negostr = negolink;
				DataBuff[g_PCIE_CNT].curlnkspd = current_link_speed;
				DataBuff[g_PCIE_CNT].dev_type = dev_type;
				
				//Print(L"B%02X:D%02X:F%02X|psd=%04X|Spd=%04X|Wid=%04X|dev_t=%04X\n", Bus, Device, Func, payload, current_link_speed, negolink, dev_type); //Draw column frame
				return TRUE;
			}
		}
	}

	return FALSE;
}

void PrintMain()
{
	gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN); 
	Print(L"%s, %s, %s\n", UTILITY_NAME, UTILITY_VERSION, UTILITY_DATE);
	//Print(L"Check IIO for Intel Skylake\n");
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK); 
	Print(L"Copyright (C) Wistron Corporation 2001-2017. All Rights Reserved\n");
	//Print(L"\n");
}

void Help()
{
	Print(L"Cmd Line: %s [/?|/h] [-l]\n", UTILITY_NAME);
	Print(L"/l        - list all pci or pcie equipment \n");
	Print(L"/?|/h     - This help page\n");
	Print(L"Return  : 0 - OK, !0 - fail or error\n");
}


/*
EFI_STATUS PCIEEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
 */
int main(IN int Argc, IN char **Argv)
{
	UINT16		Bus;      //2016/03/21, Barry : Cannot use UINT8, bcz UINT8 is 0x00 ~ 0xFF, 0 ~ 255,  if Bus is 255 (0xFF) now, then Bus++ will be 0 (overflow!)
	UINT16		Device;
	UINT16		Func;
	UINT16		VendorID; //WORD
	UINT16		DeviceID;
	UINT32		DWData;   //DWORD
	EFI_STATUS	Status;
	UINT32      ClassCode;

	UINT16		OneTime=1;
	UINT16		Bus_FirstPCIE, Device_FirstPCIE, Func_FirstPCIE;
	PCI_CLASS_STRINGS ClassStrings;
	
	int		i,rc;
	UINTN   pld, spd, wid;

	char		strtmp[128];
	
	char		strtmp1[260], strbus[1028], strdev[1028], strfun[1028], strpld[1028], strspd[1028], strwid[1028];
	
	Status = EFI_SUCCESS;
	i=rc=0;
	memset(strtmp, 0x00, sizeof(strtmp));
	
	PrintMain();
	
	for(i=1; i<Argc; i++) {
		strcpy(strtmp, Argv[i]);

		strupr(strtmp);
		if( (strcmp(strtmp, "-H")==0) || (strcmp(strtmp, "/H")==0) || (strcmp(strtmp, "-?")==0) || (strcmp(strtmp, "/?")==0) || (strcmp(strtmp, "HELP")==0) || (strcmp(strtmp, "?")==0)) {
			Help();
			return 1;
		}
	}
	if( (strcmp(strupr(Argv[1]), "/L")==0) || (strcmp(Argv[1], "-L")==0) ) {
		strcpy(gAction, "l");
	}else{
		printf("Error! Invalid parameter,type /?|-?|/h|-h for help\n");
		return 1;
	}
	
	gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
	if(strcmp(gAction, "l")==0)
	{
		Print(L"List all PCI+PCIE device...\n");
		Print(L"Index BUS D/F  Vendor Device ClassName\n");
	}
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
	for(Bus=0; Bus<BUS_MAX; Bus++) { //2016/03/21, Barry : Cannot use UINT8, bcz UINT8 is 0x00 ~ 0xFF, 0 ~ 255,  if Bus is 255 (0xFF) now, then Bus++ will be 0 (overflow!)
		for(Device=0; Device<DEVICE_MAX; Device++) {
			for(Func=0; Func<FUNC_MAX; Func++) {
				DWData=PciRead32(PCI_LIB_ADDRESS(Bus, Device, Func, 0x00)); //Offset-0x00, DWORD, Vendor+Device
				if( 0xFFFFFFFF != (DWData & 0xFFFFFFFF) ) {
					g_PCI_CNT++;
					VendorID=PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, 0x00));
					DeviceID=PciRead16(PCI_LIB_ADDRESS(Bus, Device, Func, 0x02));
					ClassCode = PciRead32(PCI_LIB_ADDRESS(Bus, Device, Func, 0x09));
					ClassCode = (ClassCode&0xFFFFFF00)>>8;
					PciGetClassStrings (ClassCode&0xFFFFFFFF, &ClassStrings);
					if( CheckPCIE(Bus, Device, Func) ) {
						if(OneTime) {
							Bus_FirstPCIE = Bus;
							Device_FirstPCIE = Device;
							Func_FirstPCIE = Func;
							OneTime=0;
						}
						if(strcmp(gAction, "l")==0)
						{
							gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN);
							Print(L"%02X  |%02X %02X | %04X  %04X | %s - %s\n", g_PCI_CNT, Bus, (Device<<3)|(Func), VendorID, DeviceID, ClassStrings.BaseClass, ClassStrings.SubClass);
							gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);
						}
						g_PCIE_CNT++;
					} else {
						if(strcmp(gAction, "l")==0)
							Print(L"%02X  |%02X %02X | %04X  %04X | %s - %s\n", g_PCI_CNT, Bus, (Device<<3)|(Func), VendorID, DeviceID, ClassStrings.BaseClass, ClassStrings.SubClass);
					}
					//gBS->Stall(300*1000);
				}      
			}
		}
	}
	
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	Print(L"Total device count : PCI=%d ( PCIE=%d )\n", g_PCI_CNT, g_PCIE_CNT);
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLACK);

	
	return EFI_SUCCESS;
}




