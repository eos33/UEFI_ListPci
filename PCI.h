#ifndef _PCIE_3D8611D0_F15E_11E5_8532_A072C24B4394_H
#define _PCIE_3D8611D0_F15E_11E5_8532_A072C24B4394_H

#include <Uefi.h>                               //MdePky\Include\Uefi.h
#include <Library/UefiLib.h>                    //MdePkg\Include\Library\UefiLib.h
#include <Library/UefiApplicationEntryPoint.h>  //MdePkg\Include\Library\UefiApplicationEntryPoint.h
//#include <Library/IoLib.h>                      //MdePkg\Include\Library\IoLib.h

#define BUS_MAX		256
#define DEVICE_MAX	32
#define FUNC_MAX	8

/*
//2016/03/24, 
//Barry found that VendorID is macro definition here, 
//but in source files, cannot define a same value for arithmetic operation (+/-/...)
//For avoid confliction, please use capital or others keywords !
//*/
#define VendorID_Offset		0x00
//#define VendorID			0x00
#define DeviceID_Offset		0x02
#define ClassID_Offset		0x08
#define CapaPtr_Offset		0x34	//Capability Pointer
#define LinkCapaReg_Offset	0x0C	//Link Capabilities Register
#define LinkStaReg_Offset	0x12	//Link Status Register
#define SlotStaReg_Offset	0x1A	//Slot Status Register
#define CapaReg_Lo_Offset	0x02	//Capabilities Register Low Byte
#define CapaReg_Hi_Offset	0x03	//Capabilities Register High Byte
#define DeviceCont_Offset	0x08	//Device Control 

//
// Link Capabilities Register
//
#define PCIE_CAP_SUP_LINK_SPEEDS(PcieLinkCap) \
    ((PcieLinkCap) & 0x0f)
#define PCIE_CAP_MAX_LINK_WIDTH(PcieLinkCap) \
    (((PcieLinkCap) >> 4) & 0x3f)
#define PCIE_CAP_ASPM_SUPPORT(PcieLinkCap) \
    (((PcieLinkCap) >> 10) & 0x3)
#define PCIE_CAP_L0s_LATENCY(PcieLinkCap) \
    (((PcieLinkCap) >> 12) & 0x7)
#define PCIE_CAP_L1_LATENCY(PcieLinkCap) \
    (((PcieLinkCap) >> 15) & 0x7)
#define PCIE_CAP_CLOCK_PM(PcieLinkCap) \
    (((PcieLinkCap) >> 18) & 0x1)
#define PCIE_CAP_SUP_DOWN_ERR_REPORTING(PcieLinkCap) \
    (((PcieLinkCap) >> 19) & 0x1)
#define PCIE_CAP_LINK_ACTIVE_REPORTING(PcieLinkCap) \
    (((PcieLinkCap) >> 20) & 0x1)
#define PCIE_CAP_LINK_BWD_NOTIF_CAP(PcieLinkCap) \
    (((PcieLinkCap) >> 21) & 0x1)
#define PCIE_CAP_PORT_NUMBER(PcieLinkCap) \
    (((PcieLinkCap) >> 24) & 0x0ff)
	
#define PCIE_CAP_DEVICEPORT_TYPE(PcieCapReg) \
    (((PcieCapReg) >> 4) & 0x0f)
#endif
      