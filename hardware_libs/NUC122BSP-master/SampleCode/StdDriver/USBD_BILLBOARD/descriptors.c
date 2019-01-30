/******************************************************************************
 * @file     descriptors.c
 * @brief    NUC123 series USBD descriptor
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include "NUC122.h"
#include "billboard.h"


/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
const uint8_t gu8DeviceDescriptor[] =
{
    LEN_DEVICE,     /* bLength */
    DESC_DEVICE,    /* bDescriptorType */
    0x01, 0x02,     /* bcdUSB */
    0x11,           /* bDeviceClass */
    0x00,           /* bDeviceSubClass */
    0x00,           /* bDeviceProtocol */
    EP0_MAX_PKT_SIZE,   /* bMaxPacketSize0 */
    /* idVendor */
    USBD_VID & 0x00FF,
    (USBD_VID & 0xFF00) >> 8,
    /* idProduct */
    USBD_PID & 0x00FF,
    (USBD_PID & 0xFF00) >> 8,
    0x00, 0x00,     /* bcdDevice */
    0x01,           /* iManufacture */
    0x02,           /* iProduct */
    0x03,           /* iSerialNumber - no serial */
    0x01            /* bNumConfigurations */
};

/*!<USB Configure Descriptor */
const uint8_t gu8ConfigDescriptor[] =
{
    LEN_CONFIG,			/* bLength */
    DESC_CONFIG,		/* bDescriptorType */
    /* wTotalLength */
    (LEN_CONFIG) & 0x00FF,
    ((LEN_CONFIG) & 0xFF00) >> 8,
    0x00,						/* bNumInterfaces */
    0x01,						/* bConfigurationValue */
    0x02,						/* iConfiguration */
    0x80 | (USBD_SELF_POWERED << 6) | (USBD_REMOTE_WAKEUP << 5),/* bmAttributes */
    USBD_MAX_POWER,	/* MaxPower */

    /* BOS Descriptor */
    5, 											/* Size of descriptor */
    0x0f, 									/* BOS descriptor type */
    (73) & 0x00FF,				  /* Length of this descriptor and all of its sub descriptors */
    ((73) & 0xFF00) >> 8,
    2,                      /* The number of separate device capability descriptors in the BOS */

    /* CONTAINER_ID Descriptor */
    20,										  /* Size of this Descriptor in bytes */
    0x10,                   /* Device Capability */
                                                         
    0x04,		 							  /* Device Capability Type: CONTAINER_ID, Defines the instance unique ID used to identify the instance
                                                 across all operating modes */
    0x0,                      /* Reserved */                                                        
    0x00,0x0,0x0,0x00,	  /* ContainerID in UUID type, 128 bits */
    0x00,0x0,0x0,0x00,
    0x00,0x0,0x0,0x00,
    0x00,0x0,0x0,0x00,

    /* Billboard Capability Descriptor */
    48, 										/* Size of this Descriptor in bytes */
    0x10,            				/* Device Capability */
    0x0D,                   /* Billboard Capabiltiy Type */
    0x04,                   /* iAdditionalInfoURL Index of string descriptor of a URL */
    0x01,                   /* Number of Alternate Mode */
    0x00,                   /* Index of preferred Alternate Mode. */
    0x00, 0x00,             /* Vconn Power needed by the adapter for full functionality. 0=1w, 1.5w, 2w,3w,4w,5w,6w */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bmConfigured. Each bit pair indicates the state of Alternate Modes. */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, /* Reserved */
    0x16, 0x04,             /* wSVID. Standard or Vendor ID. */
    0x01,                   /* bAlternateMode. Index of the Alternate Mode within the SVID */
    0x00,                   /* iAlternateModeString */
	
};

/*!<USB Language String Descriptor */
const uint8_t gu8StringLang[4] =
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
const uint8_t gu8VendorStringDesc[] =
{
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
const uint8_t gu8ProductStringDesc[] =
{
    30,             /* bLength          */
    DESC_STRING,    /* bDescriptorType  */
    'B',0,'i',0,'l',0,'l',0,'b',0,'o',0,'a',0,'r',0,'d',0,' ',0,'T',0,'e',0,'s',0,'t',0
};



const uint8_t gu8StringSerial[26] =
{
    26,             // bLength
    DESC_STRING,    // bDescriptorType
    'A', 0, '0', 0, '2', 0, '0', 0, '1', 0, '4', 0, '0', 0, '9', 0, '0', 0, '3', 0, '0', 0, '4', 0
};

const uint8_t gu8StringURL[52] = 
{
	52,
	DESC_STRING,
	'h',0,'t',0,'t',0,'p',0,':',0,'/',0,'/',0,'w',0,'w',0,'w',0,'.',0,'n',0,'u',0,'v',0,'o',0,'t',0,'o',0,'n',0,'.',0,'c',0,
  'o',0,'m',0,'.',0,'t',0,'w',0
};


const uint8_t *gpu8UsbString[5] =
{
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    gu8StringSerial,
		gu8StringURL
};

const S_USBD_INFO_T gsInfo =
{
    gu8DeviceDescriptor,
    gu8ConfigDescriptor,
    gpu8UsbString,
    NULL
};

