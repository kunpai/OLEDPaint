//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************


//*****************************************************************************
//
// Application Name     -   SSL Demo
// Application Overview -   This is a sample application demonstrating the
//                          use of secure sockets on a CC3200 device.The
//                          application connects to an AP and
//                          tries to establish a secure connection to the
//                          Google server.
// Application Details  -
// docs\examples\CC32xx_SSL_Demo_Application.pdf
// or
// http://processors.wiki.ti.com/index.php/CC32xx_SSL_Demo_Application
//
//*****************************************************************************


//*****************************************************************************
//
//! \addtogroup ssl
//! @{
//
//*****************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "utils.h"
#include "timer.h"
#include "uart.h"
#include "spi.h"

//Common interface includes
#include "pin_mux_config.h"
#include "gpio_if.h"
#include "i2c_if.h"
#include "common.h"
#include "uart_if.h"
#include "timer_if.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "test.h"

// HTTP Client lib
#include <http/client/httpcli.h>
#include <http/client/common.h>

// JSON Parser
#include "jsmn.h"

#define APPLICATION_VERSION "1.1.1"
#define APP_NAME            "HTTP Client"

#define POST_REQUEST_URI    "/draw"

#define DELETE_REQUEST_URI  "/delete"


#define PUT_REQUEST_URI     "/put"
#define PUT_DATA            "PUT request."

#define GET_REQUEST_URI     "/"


#define HOST_NAME           "192.168.1.2" //"<host name>"
#define HOST_PORT           80

#define PROXY_IP            <proxy_ip>
#define PROXY_PORT          <proxy_port>

#define READ_SIZE           1450
#define MAX_BUFF_SIZE       1460

//NEED TO UPDATE THIS FOR IT TO WORK!
#define DATE                3    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2022  /* Current year */
#define HOUR                3    /* Time - hours */
#define MINUTE              0    /* Time - minutes */
#define SECOND              0     /* Time - seconds */
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}
#define SPI_IF_BIT_RATE         100000
// macros of string sequences of button presses
#define ZERO "00000100101110110001100011100111"
#define ONE "00000100101110111000000001111111"
#define TWO "00000100101110110100000010111111"
#define THREE "00000100101110111100000000111111"
#define FOUR "00000100101110111010000001011111"
#define FIVE "00000100101110110110000010011111"
#define SIX "00000100101110111110000000011111"
#define SEVEN "00000100101110111001000001101111"
#define EIGHT "00000100101110110101000010101111"
#define NINE "00000100101110111101000000101111"
#define MUTE "00000100101110110010001011011101"
#define LAST "00000100101110111001001001101101"
#define VOLPLUS "00000100101110111000100001110111"
#define VOLMINUS "00000100101110110100100010110111"
#define CHANPLUS "00000100101110110000100011110111"
#define CHANMINUS "00000100101110111100100000110111"


const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

HTTPCli_Struct httpClient;

unsigned int colors[] = {RED, MAGENTA, YELLOW, WHITE, CYAN, GREEN, BLUE};
//unsigned int colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, MAGENTA, CYAN, DARKGREEN, DARKCYAN, MAROON, WHITE};
int colorIndex = 0;
unsigned int appliedColor;
int rad = 3;
extern unsigned int storedcolor[128][128];
int lineCoord[4];
int triangleCoord[6];
int quadCoord[8];
int press = 0;

char* body;

// Pixel for first line button press
//int LineX;
//int LineY;

// flags
int colorChange = 0;
int delete = 0;
int radInc = 0;
int radDec = 0;
int cursor = 1;
int brush = 0;
int clear = 0;
int fill = 0;
int screenshot = 0;

int line = 0;
// First line point marker
int linePoint = 0;

int triangle = 0;
// Triangle point markers
int triPoint1 = 0;
int triPoint2 = 0;

int quad = 0;
// quad point marker
int quadPoint1 = 0;
int quadPoint2 = 0;
int quadPoint3 = 0;

int circle = 0;

// To prevent circular paint to be overwritten immediately
int circtocur = 0;
// To prevent drawn circle to be overwitten immediately
int circtocur2 = 0;


typedef enum{
 /* Choosing this number to avoid overlap with host-driver's error codes */
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,
    DEVICE_START_FAILED = DEVICE_NOT_IN_STATION_MODE - 1,
    INVALID_HEX_STRING = DEVICE_START_FAILED - 1,
    TCP_RECV_ERROR = INVALID_HEX_STRING - 1,
    TCP_SEND_ERROR = TCP_RECV_ERROR - 1,
    FILE_NOT_FOUND_ERROR = TCP_SEND_ERROR - 1,
    INVALID_SERVER_RESPONSE = FILE_NOT_FOUND_ERROR - 1,
    FORMAT_NOT_SUPPORTED = INVALID_SERVER_RESPONSE - 1,
    FILE_OPEN_FAILED = FORMAT_NOT_SUPPORTED - 1,
    FILE_WRITE_ERROR = FILE_OPEN_FAILED - 1,
    INVALID_FILE = FILE_WRITE_ERROR - 1,
    SERVER_CONNECTION_FAILED = INVALID_FILE - 1,
    GET_HOST_IP_FAILED = SERVER_CONNECTION_FAILED  - 1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


// checks color in the 2D array, converts it into a single character
// creates a string of 128 columns for every 8 rows, determined by the value of the multiplier

char *generateBodyBitmap(int multiplier){
    int i;
    int j;
    char map[5000];
    memset(map, '\0', sizeof(map));
    for (i = ((multiplier * 8)-8); i <= ((multiplier*8)-1); i++){
        for (j = 0; j<= 127; j++){
            if (storedcolor[i][j] == 63488) {
                //printf("RED\n");
                strcat(map, "a");
            }
            if (storedcolor[i][j] == 63519) {
                //printf("MAGENTA\n");
                strcat(map, "b");
            }
            if (storedcolor[i][j] == 65504) {
                //printf("YELLOW\n");
                strcat(map, "c");
            }
            if (storedcolor[i][j] == 65535) {
                //printf("WHITE\n");
                strcat(map, "d");
            }
            if (storedcolor[i][j] == 2047) {
                //printf("CYAN\n");
                strcat(map, "e");
            }
            if (storedcolor[i][j] == 2016) {
                //printf("GREEN\n");
                strcat(map, "f");
            }
            if (storedcolor[i][j] == 31) {
                //printf("BLUE\n");
                strcat(map, "g");
            }
            if (storedcolor[i][j] == 0) {
                //printf("BLACK\n");
                strcat(map, "h");
            }

        }
    }
    int nBytes =
      sprintf((char *)NULL, "{\n\"map\":\"%s\"\n}",
              map) +
      1;
    char *body = malloc(nBytes);
    sprintf(body, "{\n\"map\":\"%s\"\n}",
              map);
    return body;
}

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulDestinationIP; // IP address of destination server
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
unsigned char g_buff[MAX_BUFF_SIZE+1];
long bytesReceived = 0; // variable to store the file size
#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
unsigned long interval = 0;
char rset = 0;
char input[33] = "";
int bitlen = 0;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End: df
//*****************************************************************************
typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting IR_IN = { .port = GPIOA2_BASE, .pin = 0x2}; // pin 08 is GPIO input

typedef struct
{
   /* time */
   unsigned long tm_sec;
   unsigned long tm_min;
   unsigned long tm_hour;
   /* date */
   unsigned long tm_day;
   unsigned long tm_mon;
   unsigned long tm_year;
   unsigned long tm_week_day; //not required
   unsigned long tm_year_day; //not required
   unsigned long reserved[3];
}SlDateTime;

SlDateTime g_time;

//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static long WlanConnect();
static int set_time();
static void BoardInit(void);
static long InitializeAppVariables();
static int tls_connect();
static int connectToAccessPoint();
static int http_post(int, char *);

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    if(!pWlanEvent) {
        return;
    }

    switch(pWlanEvent->Event) {
        case SL_WLAN_CONNECT_EVENT: {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'.
            // Applications can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            printf("[WLAN EVENT] STA Connected to the AP: %s , "
                       "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                       g_ucConnectionSSID,g_ucConnectionBSSID[0],
                       g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                       g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                       g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT: {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code) {
                printf("[WLAN EVENT]Device disconnected from the AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else {
                printf("[WLAN ERROR]Device disconnected from the AP AP: %s, "
                           "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        default: {
            printf("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    if(!pNetAppEvent) {
        return;
    }

    switch(pNetAppEvent->Event) {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT: {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            printf("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                       "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
        }
        break;

        default: {
            printf("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse) {
    // Unused in this application
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    if(!pDevEvent) {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    printf("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {
    if(!pSock) {
        return;
    }

    switch( pSock->Event ) {
        case SL_SOCKET_TX_FAILED_EVENT:
            switch( pSock->socketAsyncEvent.SockTxFailData.status) {
                case SL_ECLOSE:
                    printf("[SOCK ERROR] - close socket (%d) operation "
                                "failed to transmit all queued packets\n\n",
                                    pSock->socketAsyncEvent.SockTxFailData.sd);
                    break;
                default:
                    printf("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                                "(%d) \n\n",
                                pSock->socketAsyncEvent.SockTxFailData.sd, pSock->socketAsyncEvent.SockTxFailData.status);
                  break;
            }
            break;

        default:
            printf("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
          break;
    }
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End breadcrumb: s18_df
//*****************************************************************************


//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    0 on success else error code
//!
//! \return None
//!
//*****************************************************************************
static long InitializeAppVariables() {
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    return SUCCESS;
}


//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState() {
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode) {
        if (ROLE_AP == lMode) {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again
        if (ROLE_STA != lRetVal) {
            // We don't want to proceed if the device is not coming up in STA-mode
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                                &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    printf("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    printf("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                                SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);



    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal) {
        // Wait
        while(IS_CONNECTED(g_ulStatus)) {
#ifndef SL_PLATFORM_MULTI_THREADED
              _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
            WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeAppVariables();

    return lRetVal; // Success
}


//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void) {
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


//****************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint
//!
//!  This function connects to the required AP (SSID_NAME) with Security
//!  parameters specified in te form of macros at the top of this file
//!
//! \param  None
//!
//! \return  0 on success else error code
//!
//! \warning    If the WLAN connection fails or we don't aquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect() {
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    printf("Attempting connection to access point: ");
    printf(SSID_NAME);
    printf("... ...");
    lRetVal = sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    printf(" Connected!!!\n\r");


    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))) {
        // Toggle LEDs to Indicate Connection Progress
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay(800000);
        _SlNonOsMainLoopTask();
        MAP_UtilsDelay(800000);
    }
    printf("WLAN event received.\n\r");

    return SUCCESS;

}




long printErrConvenience(char * msg, long retVal) {
    printf(msg);
    return retVal;
}

int connectToAccessPoint() {
    long lRetVal = -1;


    lRetVal = InitializeAppVariables();
    ASSERT_ON_ERROR(lRetVal);

    //
    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost
    //
    lRetVal = ConfigureSimpleLinkToDefaultState();
    if(lRetVal < 0) {
      if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
          printf("Failed to configure the device in its default state \n\r");

      return lRetVal;
    }

    printf("Device is configured in default state \n\r");

    CLR_STATUS_BIT_ALL(g_ulStatus);

    ///
    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    //
    printf("Opening sl_start\n\r");
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || ROLE_STA != lRetVal) {
        printf("Failed to start the device \n\r");
        return lRetVal;
    }

    printf("Device started as STATION \n\r");

    //
    //Connecting to WLAN AP
    //
    lRetVal = WlanConnect();
    if(lRetVal < 0) {
        printf("Failed to establish connection w/ an AP \n\r");
        return lRetVal;
    }

    printf("Connection established w/ AP and IP is aquired \n\r");
    return 0;
}

static int FlushHTTPResponse(HTTPCli_Handle httpClient)
{
    const char *ids[2] = {
                            HTTPCli_FIELD_NAME_CONNECTION, /* App will get connection header value. all others will skip by lib */
                            NULL
                         };
    char buf[128];
    int id;
    int len = 1;
    bool moreFlag = 0;
    char ** prevRespFilelds = NULL;


    /* Store previosly store array if any */
    prevRespFilelds = HTTPCli_setResponseFields(httpClient, ids);

    /* Read response headers */
    while ((id = HTTPCli_getResponseField(httpClient, buf, sizeof(buf), &moreFlag))
            != HTTPCli_FIELD_ID_END)
    {

        if(id == 0)
        {
            if(!strncmp(buf, "close", sizeof("close")))
            {
                printf("Connection terminated by server\n\r");
            }
        }

    }

    /* Restore previosuly store array if any */
    HTTPCli_setResponseFields(httpClient, (const char **)prevRespFilelds);

    while(1)
    {
        /* Read response data/body */
        /* Note:
                moreFlag will be set to 1 by HTTPCli_readResponseBody() call, if more
                data is available Or in other words content length > length of buffer.
                The remaining data will be read in subsequent call to HTTPCli_readResponseBody().
                Please refer HTTP Client Libary API documenation @ref HTTPCli_readResponseBody
                for more information.
        */
        HTTPCli_readResponseBody(httpClient, buf, sizeof(buf) - 1, &moreFlag);
        ASSERT_ON_ERROR(len);

        if ((len - 2) >= 0 && buf[len - 2] == '\r' && buf [len - 1] == '\n'){
            break;
        }

        if(!moreFlag)
        {
            /* There no more data. break the loop. */
            break;
        }
    }
    return 0;
}

int ParseJSONData(char *ptr)
{
    long lRetVal = 0;
    int noOfToken;
    jsmn_parser parser;
    jsmntok_t   *tokenList;


    /* Initialize JSON PArser */
    jsmn_init(&parser);

    /* Get number of JSON token in stream as we we dont know how many tokens need to pass */
    noOfToken = jsmn_parse(&parser, (const char *)ptr, strlen((const char *)ptr), NULL, 10);
    if(noOfToken <= 0)
    {
        printf("Failed to initialize JSON parser\n\r");
        return -1;

    }

    /* Allocate memory to store token */
    tokenList = (jsmntok_t *) malloc(noOfToken*sizeof(jsmntok_t));
    if(tokenList == NULL)
    {
        printf("Failed to allocate memory\n\r");
        return -1;
    }

    /* Initialize JSON Parser again */
    jsmn_init(&parser);
    noOfToken = jsmn_parse(&parser, (const char *)ptr, strlen((const char *)ptr), tokenList, noOfToken);
    if(noOfToken < 0)
    {
        printf("Failed to parse JSON tokens\n\r");
        lRetVal = noOfToken;
    }
    else
    {
        printf("Successfully parsed %ld JSON tokens\n\r", noOfToken);
    }

    free(tokenList);

    return lRetVal;
}


static int readResponse(HTTPCli_Handle httpClient)
{
    long lRetVal = 0;
    int bytesRead = 0;
    int id = 0;
    unsigned long len = 0;
    int json = 0;
    char *dataBuffer=NULL;
    bool moreFlags = 1;
    const char *ids[4] = {
                            HTTPCli_FIELD_NAME_CONTENT_LENGTH,
                            HTTPCli_FIELD_NAME_CONNECTION,
                            HTTPCli_FIELD_NAME_CONTENT_TYPE,
                            NULL
                         };

    /* Read HTTP POST request status code */
    lRetVal = HTTPCli_getResponseStatus(httpClient);
    if(lRetVal > 0)
    {
        switch(lRetVal)
        {
        case 200:
        {
            printf("HTTP Status 200\n\r");
            /*
                 Set response header fields to filter response headers. All
                  other than set by this call we be skipped by library.
             */
            HTTPCli_setResponseFields(httpClient, (const char **)ids);

            /* Read filter response header and take appropriate action. */
            /* Note:
                    1. id will be same as index of fileds in filter array setted
                    in previous HTTPCli_setResponseFields() call.
                    2. moreFlags will be set to 1 by HTTPCli_getResponseField(), if  field
                    value could not be completely read. A subsequent call to
                    HTTPCli_getResponseField() will read remaining field value and will
                    return HTTPCli_FIELD_ID_DUMMY. Please refer HTTP Client Libary API
                    documenation @ref HTTPCli_getResponseField for more information.
             */
            while((id = HTTPCli_getResponseField(httpClient, (char *)g_buff, sizeof(g_buff), &moreFlags))
                    != HTTPCli_FIELD_ID_END)
            {

                switch(id)
                {
                case 0: /* HTTPCli_FIELD_NAME_CONTENT_LENGTH */
                {
                    len = strtoul((char *)g_buff, NULL, 0);
                }
                break;
                case 1: /* HTTPCli_FIELD_NAME_CONNECTION */
                {
                }
                break;
                case 2: /* HTTPCli_FIELD_NAME_CONTENT_TYPE */
                {
                    if(!strncmp((const char *)g_buff, "application/json",
                            sizeof("application/json")))
                    {
                        json = 1;
                    }
                    else
                    {
                        /* Note:
                                Developers are advised to use appropriate
                                content handler. In this example all content
                                type other than json are treated as plain text.
                         */
                        json = 0;
                    }
                    printf(HTTPCli_FIELD_NAME_CONTENT_TYPE);
                    printf(" : ");
                    printf("application/json\n\r");
                }
                break;
                default:
                {
                    printf("Wrong filter id\n\r");
                    lRetVal = -1;
                    goto end;
                }
                }
            }
            bytesRead = 0;
            if(len > sizeof(g_buff))
            {
                dataBuffer = (char *) malloc(len);
                if(dataBuffer)
                {
                    printf("Failed to allocate memory\n\r");
                    lRetVal = -1;
                    goto end;
                }
            }
            else
            {
                dataBuffer = (char *)g_buff;
            }

            /* Read response data/body */
            /* Note:
                    moreFlag will be set to 1 by HTTPCli_readResponseBody() call, if more
                    data is available Or in other words content length > length of buffer.
                    The remaining data will be read in subsequent call to HTTPCli_readResponseBody().
                    Please refer HTTP Client Libary API documenation @ref HTTPCli_readResponseBody
                    for more information
             */
            bytesRead = HTTPCli_readResponseBody(httpClient, (char *)dataBuffer, len, &moreFlags);
            if(bytesRead < 0)
            {
                printf("Failed to received response body\n\r");
                lRetVal = bytesRead;
                goto end;
            }
            else if( bytesRead < len || moreFlags)
            {
                printf("Mismatch in content length and received data length\n\r");
                goto end;
            }
            dataBuffer[bytesRead] = '\0';

            if(json)
            {
                /* Parse JSON data */
                lRetVal = ParseJSONData(dataBuffer);
                if(lRetVal < 0)
                {
                    goto end;
                }
            }
            else
            {
                /* treating data as a plain text */
            }

        }
        break;

        case 404:
            printf("File not found. \r\n");
            /* Handle response body as per requirement.
                  Note:
                    Developers are advised to take appopriate action for HTTP
                    return status code else flush the response body.
                    In this example we are flushing response body in default
                    case for all other than 200 HTTP Status code.
             */
        default:
            /* Note:
              Need to flush received buffer explicitly as library will not do
              for next request.Apllication is responsible for reading all the
              data.
             */
            FlushHTTPResponse(httpClient);
            break;
        }
    }
    else
    {
        printf("Failed to receive data from server.\r\n");
        goto end;
    }

    lRetVal = 0;

end:
    if(len > sizeof(g_buff) && (dataBuffer != NULL))
    {
        free(dataBuffer);
    }
    return lRetVal;
}


static int HTTPPostMethod(HTTPCli_Handle httpClient, char *body) {
  bool moreFlags = 1;
  bool lastFlag = 1;
  char tmpBuf[4];
  long lRetVal = 0;
  HTTPCli_Field fields[4] = {
      {HTTPCli_FIELD_NAME_HOST, HOST_NAME},
      {HTTPCli_FIELD_NAME_ACCEPT, "*/*"},
      {HTTPCli_FIELD_NAME_CONTENT_TYPE, "application/json"},
      {NULL, NULL}};

  /* Set request header fields to be send for HTTP request. */
  HTTPCli_setRequestFields(httpClient, fields);

  /* Send POST method request. */
  /* Here we are setting moreFlags = 1 as there are some more header fields need
     to send other than setted in previous call HTTPCli_setRequestFields() at
     later stage. Please refer HTTP Library API documentaion @ref
     HTTPCli_sendRequest for more information.
  */
  moreFlags = 1;
  lRetVal = HTTPCli_sendRequest(httpClient, HTTPCli_METHOD_POST,
                                POST_REQUEST_URI, moreFlags);
  if (lRetVal < 0) {
    //printf("First\n");
    printf("Failed to send HTTP POST request header.\n\r");
    return lRetVal;
  }
  printf("%d %d\r\n", sizeof(POST_DATA), strlen(body));
  sprintf((char *)tmpBuf, "%d", strlen(body));

  /* Here we are setting lastFlag = 1 as it is last header field.
     Please refer HTTP Library API documentaion @ref HTTPCli_sendField for more
     information.
  */
  lastFlag = 1;
  lRetVal = HTTPCli_sendField(httpClient, HTTPCli_FIELD_NAME_CONTENT_LENGTH,
                              (const char *)tmpBuf, lastFlag);
  if (lRetVal < 0) {
    //printf("Second\n");
    printf("Failed to send HTTP POST request header.\n\r");
    return lRetVal;
  }

  /* Send POST data/body */
  lRetVal =
      HTTPCli_sendRequestBody(httpClient, body, strlen(body));
  if (lRetVal < 0) {
    //printf("Third\n");
    printf("Failed to send HTTP POST request body.\n\r");
    return lRetVal;
  }

  lRetVal = readResponse(httpClient);
  free(body);

  return lRetVal;
}

static int ConnectToHTTPServer(HTTPCli_Handle httpClient)
{
    long lRetVal = -1;
    struct sockaddr_in addr;


#ifdef USE_PROXY
    struct sockaddr_in paddr;
    paddr.sin_family = AF_INET;
    paddr.sin_port = htons(PROXY_PORT);
    paddr.sin_addr.s_addr = sl_Htonl(PROXY_IP);
    HTTPCli_setProxy((struct sockaddr *)&paddr);
#endif

    /* Resolve HOST NAME/IP */
    lRetVal = sl_NetAppDnsGetHostByName((signed char *)HOST_NAME,
                                          strlen((const char *)HOST_NAME),
                                          &g_ulDestinationIP,SL_AF_INET);
    if(lRetVal < 0)
    {
        ASSERT_ON_ERROR(GET_HOST_IP_FAILED);
    }

    /* Set up the input parameters for HTTP Connection */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HOST_PORT);
    addr.sin_addr.s_addr = sl_Htonl(g_ulDestinationIP);

    /* Testing HTTPCli open call: handle, address params only */
    HTTPCli_construct(httpClient);
    lRetVal = HTTPCli_connect(httpClient, (struct sockaddr *)&addr, 0, NULL);
    if (lRetVal < 0)
    {
        printf("Connection to server failed. error(%d)\n\r", lRetVal);
        ASSERT_ON_ERROR(SERVER_CONNECTION_FAILED);
    }
    else
    {
        printf("Connection to server created successfully\r\n");
    }

    return 0;
}

char* decodeInput () { // decodes IR input
    if(strcmp(ZERO, input) == 0){ // send screenshot
        screenshot = 1;
        return "0";
    }
    else if (strcmp(ONE, input) == 0){ // cursor
        printf("Cursor tool selected\n");
        delete = 0;
        cursor = 1;
        brush = 0;
        line = 0;
        triangle = 0;
        circle = 0;
        quad = 0;
        press = 0;
        return "1";
    }
    else if (strcmp(TWO, input) == 0){ // brush
        printf("Brush tool selected\n");
        delete = 0;
        cursor = 0;
        brush = 1;
        line = 0;
        triangle = 0;
        circle = 0;
        quad = 0;
        press = 0;
        return "2";
    }
    else if (strcmp(THREE, input) == 0){ // delete
        printf("Erase tool selected\n");
        delete = 1;
        cursor = 0;
        brush = 0;
        line = 0;
        triangle = 0;
        circle = 0;
        quad = 0;
        press = 0;
        return "3";
    }
    else if (strcmp(FOUR, input) == 0){ // line
        if (press == 0) {
            printf("Line tool selected. Please select 2 points...\n");
        }
        delete = 0;
        cursor = 0;
        brush = 0;
        line = 1;
        triangle = 0;
        circle = 0;
        quad = 0;
        press++;
        return "4";
    }
    else if (strcmp(FIVE, input) == 0) { // triangle
        if (press == 0) {
            printf("Triangle tool selected. Please select 3 points...\n");
        }
        delete = 0;
        cursor = 0;
        brush = 0;
        line = 0;
        triangle = 1;
        circle = 0;
        quad = 0;
        press++;
        return "5";
    }
    else if (strcmp(SIX, input) == 0){ // circle
        if (press == 0) {
            printf("Circle tool selected. Please select 1 point...\n");
        }
        delete = 0;
        cursor = 0;
        brush = 0;
        line = 0;
        triangle = 0;
        circle = 1;
        quad = 0;
        press++;
        return "6";
    }
    else if (strcmp(SEVEN, input) == 0){ // quad
        if (press == 0) {
            printf("Quadrilateral tool selected. Please select 4 points...\n");
        }
        delete = 0;
        cursor = 0;
        brush = 0;
        line = 0;
        triangle = 0;
        circle = 0;
        quad = 1;
        press++;
        return "7";
    }
    else if (strcmp(EIGHT, input) == 0){ // fillScreen color
        printf("Filling screen...\n");
        fill = 1;
        return "8";
    }
    else if (strcmp(NINE, input) == 0) { // clear screen
        printf("Clearing screen...\n");
        clear = 1;
        return "9";
    }
    else if (strcmp(MUTE, input) == 0) {
        return "MUTE";
    }
    else if (strcmp(LAST, input) == 0) {
        return "LAST";
    }
    else if (strcmp(VOLPLUS, input) == 0) { // radius increase
        printf("Cursor size increased\n");
        radInc = 1;
        //radDec = 0;
        return "VOLPLUS";
    }
    else if (strcmp(VOLMINUS, input) == 0) { // radius decrease
        //radInc = 0;
        printf("Cursor size decreased\n");
        radDec = 1;
        return "VOLMINUS";
    }
    else if (strcmp(CHANPLUS, input) == 0) { // color change +
        printf("Color changed\n");
        colorChange = 1;
        colorIndex++;
        return "CHANPLUS";
    }
    else if (strcmp(CHANMINUS, input) == 0) { // color change -
        printf("Color changed\n");
        colorChange = 1;
        colorIndex--;
        return "CHANMINUS";
    }
    else
        return NULL;
}

void display(void)
{
    //printf("%s\n", input);
    char* input = decodeInput();
    //printf("%s\n", input);

    /*
    if (input != NULL) { // invalid button pressed
        printf("\r%s\n\r", input);
    }
    */
    //ball();
}

void getData(void)
{
    if (interval >= 3) // 1.64 ms ~ > 3 times 0.5 ms
        strcat(input, "1");
    else
        strcat(input, "0");

    bitlen++;

    if (bitlen == 32) // end of signal
        display();

}

void TimerA0IntHandler(void) { // timer interrupt

    Timer_IF_InterruptClear(TIMERA0_BASE);
    interval++; // marking every 0.5 ms interval
}

static void GPIOA2IntHandler(void) {
    unsigned long ulStatus;
    long read;

    ulStatus = MAP_GPIOIntStatus (IR_IN.port, true);
    MAP_GPIOIntClear(IR_IN.port, ulStatus);     // clear interrupts on pin 8

    read = GPIOPinRead(IR_IN.port, IR_IN.pin);


    if (read == 0x2) {  //rising edge
        interval = 0;
        MAP_TimerEnable(TIMERA0_BASE, TIMER_A); // enable 0.5 ms interval counter

    } else if (read == 0) {  //falling edge

        if (rset == 1) {
                getData();
        }

        if (interval >= 8) { // reset signal received (long singal in beginning)
                input[0] = '\0';
                rset = 1;
                bitlen = 0;
        }
    }

}

int xCoor = 64;
int yCoor = 64; // origin coordinates

void ball(){
    unsigned char ucDevAddr = 0x18, xOffset = 0x3, yOffset = 0x5, ucRdLen = 1; //hardcoded values from earlier part of lab
    unsigned char XaucRdDataBuf; // to find tilt in y
    unsigned char YaucRdDataBuf; // to find tilt in x

    // OLED Paint Ready to go
    fillRect(24, 50, 80 , 40, BLACK);
    setCursor(46, 64);
    Outstr("Ready!");
    delay(25);
    fillRect(4, 4, 120, 120, BLACK);

    // Initialize board with all black
    int i;
    int j;
    for (i=0; i <= 127; i++){
        for (j=0; j <= 127; j++){
            storedcolor[i][j] = BLACK;
        }
    }

    xCoor = 64;
    yCoor = 64;
    while(1){
        I2C_IF_Write(ucDevAddr, &xOffset, 1, 0);
        I2C_IF_Read(ucDevAddr, &XaucRdDataBuf, ucRdLen);
        I2C_IF_Write(ucDevAddr, &yOffset, 1, 0);
        I2C_IF_Read(ucDevAddr, &YaucRdDataBuf, ucRdLen);
        appliedColor = colors[colorIndex % 7];

        if (colorChange == 1){
            appliedColor = colors[colorIndex % 7];
            colorChange = 0;
        }
        if (delete == 1){
            drawCursor(xCoor, yCoor, rad, GRAY);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCircle(xCoor, yCoor, rad, BLACK);


            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }

            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, GRAY);
        }
        if (radInc == 1){
            fillCursor(xCoor, yCoor, rad);
            rad++;
            radInc = 0;
        }
        if (radDec == 1){
            fillCursor(xCoor, yCoor, rad);
            if (rad != 1)
                rad--;
            radDec = 0;
        }

        if (fill == 1) {
            fillScreen(appliedColor);
            fill = 0;
        }

        if (clear == 1) {
            fillScreen(BLACK);
            clear = 0;
        }

        if (screenshot == 1) {
            screenshot = 0;
            int mul;
            // 16 times 8 columns = 128 columns
            for (mul = 1; mul <= 16; mul++){
                long lRetVal = -1;
    //        // Connect the CC3200 to the local access point
                lRetVal = connectToAccessPoint();

                // Set time so that encryption can be used
                printf("Setting time\n\r");
                lRetVal = set_time();
                if (lRetVal < 0) {
                  printf("Unable to set time in the device");
                  LOOP_FOREVER();
                }
                printf("Time set in the device\n\r");

                lRetVal = ConnectToHTTPServer(&httpClient);
                if (lRetVal < 0) {
                  LOOP_FOREVER();
                }
                body = generateBodyBitmap(mul);
                HTTPPostMethod(&httpClient, body);
            }
            fillScreen(BLACK);
            cursor = 1;

        }

        if (cursor == 1){
            drawCursor(xCoor, yCoor, rad, appliedColor);

            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            fillCursor(xCoor, yCoor, rad);

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }
            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, appliedColor);
        }
        if (brush == 1) {
            fillCircle(xCoor, yCoor, rad, appliedColor);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCircle(xCoor, yCoor, rad, appliedColor);

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }

            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            fillCircle(xCoor, yCoor, rad, appliedColor);
        }
        if (line == 1){
            drawCursor(xCoor, yCoor, rad, appliedColor);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCursor(xCoor, yCoor, rad); // point
            if (press == 3) {
                lineCoord[2] = xCoor;
                lineCoord[3] = yCoor;
                fillCircle(lineCoord[2], lineCoord[3], 1, appliedColor); // point
                press = 0;
                drawLine(lineCoord[0], lineCoord[1], lineCoord[2], lineCoord[3], appliedColor);
                // Remove first point marker
                fillCircle(lineCoord[0], lineCoord[1], 1, BLACK);
                fillCircle(lineCoord[2], lineCoord[3], 1, BLACK);
                memset(lineCoord, 0, 4*sizeof(unsigned int));
                printf("Point 2 placed\n");
                printf("Line drawn!\n");
                cursor = 1;
                line = 0;
            }

            if (press == 2){
                lineCoord[0] = xCoor;
                lineCoord[1] = yCoor;
                fillCircle(lineCoord[0], lineCoord[1], 1, appliedColor);
                printf("Point 1 placed\n");
                line = 0;
                cursor = 1;
            }

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }

            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, appliedColor);
        }
        if (triangle == 1)
        {
            drawCursor(xCoor, yCoor, rad, appliedColor);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCursor(xCoor, yCoor, rad); // point
            if (press == 4) {
                triangleCoord[4] = xCoor;
                triangleCoord[5] = yCoor;
                drawTriangle(triangleCoord[0], triangleCoord[1], triangleCoord[2], triangleCoord[3], triangleCoord[4], triangleCoord[5], appliedColor);
                // Remove triangle markers
                fillCircle(triangleCoord[0], triangleCoord[1], 1, BLACK);
                fillCircle(triangleCoord[2], triangleCoord[3], 1, BLACK);
                memset(triangleCoord, 0, 6*sizeof(unsigned int));
                printf("Point 3 placed\n");
                printf("Triangle drawn!\n");
                press = 0;
                cursor = 1;
                triangle = 0;
            }

            if (press == 3){
                triangleCoord[2] = xCoor;
                triangleCoord[3] = yCoor;
                fillCircle(triangleCoord[2], triangleCoord[3], 1, appliedColor);
                printf("Point 2 placed\n");
                triangle = 0;
                cursor = 1;
            }

            if (press == 2){
                triangleCoord[0] = xCoor;
                triangleCoord[1] = yCoor;
                fillCircle(triangleCoord[0], triangleCoord[1], 1, appliedColor);
                printf("Point 1 placed\n");
                triangle = 0;
                cursor = 1;
            }

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }
            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, appliedColor);
        }
        if (quad == 1){
            drawCursor(xCoor, yCoor, rad, appliedColor);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCursor(xCoor, yCoor, rad); // point
            if (press == 5) {
                quadCoord[6] = xCoor;
                quadCoord[7] = yCoor;
                //fillCircle(xCoor, yCoor, 1, appliedColor); // point
                press = 0;
                drawQuad(quadCoord[0], quadCoord[1], quadCoord[2], quadCoord[3], quadCoord[4], quadCoord[5],
                         quadCoord[6], quadCoord[7], appliedColor);
                // Remove quad markers
                fillCircle(quadCoord[0], quadCoord[1], 1, BLACK);
                fillCircle(quadCoord[2], quadCoord[3], 1, BLACK);
                fillCircle(quadCoord[4], quadCoord[5], 1, BLACK);
                fillCircle(quadCoord[6], quadCoord[7], 1, BLACK);
                memset(quadCoord, 0, 8*sizeof(unsigned int));
                printf("Point 4 placed\n");
                printf("Quadrilateral drawn!\n");
                cursor = 1;
                quad = 0;
            }

            if (press == 4){
                quadCoord[4] = xCoor;
                quadCoord[5] = yCoor;
                fillCircle(quadCoord[4], quadCoord[5], 1, appliedColor);
                quad = 0;
                printf("Point 3 placed\n");
                cursor = 1;
            }

            if (press == 3){
                quadCoord[2] = xCoor;
                quadCoord[3] = yCoor;
                fillCircle(quadCoord[2], quadCoord[3], 1, appliedColor);
                quad = 0;
                printf("Point 2 placed\n");
                cursor = 1;
            }

            if (press == 2){
                quadCoord[0] = xCoor;
                quadCoord[1] = yCoor;
                fillCircle(quadCoord[0], quadCoord[1], 1, appliedColor);
                quad = 0;
                printf("Point 1 placed\n");
                cursor = 1;
            }

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }
            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, appliedColor);
        }
        if (circle == 1) {
            drawCursor(xCoor, yCoor, rad, appliedColor);
            // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
            int xSpeed = (int) ((signed char) YaucRdDataBuf);
            int ySpeed = (int) ((signed char) XaucRdDataBuf);

            // to give the impression that the ball is moving, filling original position with black
            fillCursor(xCoor, yCoor, rad);
            if (press == 2) {
                drawCircle(xCoor, yCoor, rad, appliedColor); // point
                press = 0;
                printf("Point 1 placed\n");
                printf("Circle drawn!\n");
                circle = 0;
                cursor = 1;
            }

            xCoor = xCoor + (int)xSpeed*0.1;

            //checking bounds
            if (xCoor>=123){
                xCoor = 123;
            }
            if (xCoor<=4){
                xCoor = 4;
            }
            yCoor = yCoor + (int)ySpeed*0.1;

            //checking bounds
            if (yCoor>=123){
                yCoor = 123;
            }
            if (yCoor<=4){
                yCoor = 4;
            }

            // filling new positon with ball
            drawCursor(xCoor, yCoor, rad, appliedColor);
        }
    }
}


//*****************************************************************************
//
//! Main
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
 void main() {
  unsigned long ulStatus;
  long lRetVal = -1;
  //
  // Initialize board configuration
  //
  BoardInit();

  PinMuxConfig();
  MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);

  InitTerm();
  ClearTerm();

  MAP_PRCMPeripheralReset(PRCM_GSPI);

   //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    Adafruit_Init();
    fillScreen(BLACK);

    setCursor(38, 8);
    Outstr("OLED Paint");
    delay(50);
    setCursor(38 , 64);
    Outstr("Loading...");

    // UNCOMMENT FOR WIFI FUNCTIONALITY

    // Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();

    // Set time so that encryption can be used
    printf("Setting time\n\r");
    lRetVal = set_time();
    if (lRetVal < 0) {
      printf("Unable to set time in the device");
      LOOP_FOREVER();
    }
    printf("Time set in the device\n\r");

    lRetVal = ConnectToHTTPServer(&httpClient);
    if (lRetVal < 0) {
      LOOP_FOREVER();
    }


    I2C_IF_Open(I2C_MASTER_MODE_FST);

    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC_UP, TIMER_A, 0);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerA0IntHandler);
    Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 40000);  //0.5ms
    // Register the interrupt handlers
    //
    MAP_GPIOIntRegister(IR_IN.port, GPIOA2IntHandler);

    //
    // Configure rising edge interrupt on IR_IN
    //
    MAP_GPIOIntTypeSet(IR_IN.port, IR_IN.pin, GPIO_BOTH_EDGES);

    ulStatus = MAP_GPIOIntStatus (IR_IN.port, false);
    MAP_GPIOIntClear(IR_IN.port, ulStatus);         // clear interrupts on GPIOA2

    // Enable GPIO Interrupt
    MAP_GPIOIntEnable(IR_IN.port, IR_IN.pin);

    ball();
  // LOOP_FOREVER();
}
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
