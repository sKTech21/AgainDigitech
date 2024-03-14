/*
 * wifiHandleing.c
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_pm.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "../incl/wifiHandleing.h"
#include "../incl/commonDeclaration.h"
#include "../incl/leakTestHandeling.h"
#include "../incl/continuity.h"

#define CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK 	( 1 )
#define CONFIG_EXAMPLE_IPV4				  	( 1 )
#define CONFIG_PM_ENABLE				  	( 1 )
#define EXAMPLE_ESP_MAXIMUM_RETRY  		  	( 5 )

/*set the AP SSID and PASSWORD*/
#define EXAMPLE_ESP_WIFI_SSID  				"Palak"
#define EXAMPLE_ESP_WIFI_PASS  				"11223344"

/*Application IP address and PORT settigns for UDP communication*/
#define UDP_APP_PORT		   				( 9999 )
#define P2P_IP_ADDRESS						"192.168.1.2"
#define BROADCAST_IP_ADDRESS				"192.168.1.255"

/*Application IP address and PORT settigns for TCP communication*/
#define TCP_SERVER_IP_ADDRESS				"192.168.1.2"
#if !defined( NON_BLOCKING_TCP_CLIENT )
	#define TCP_SERVER_PORT					( 8080 )
#else
	#define TCP_SERVER_PORT					"4242"
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - BIT0 = 1 -> we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT 					( BIT0 )
#define WIFI_FAIL_BIT      					( BIT1 )

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/**************************************************************************
 * Global Variable Declaration
 *************************************************************************/
/*Key Used in JSON Messages*/
const char* JSN_MESSAGE_CODE_STR					=	"mC";
const char* JSN_FREE_HEAP_MEMORY					=	"fHM";
const char* JSN_SENT_MESSAGE_AFTER_INTERVAL_STR		= 	"iNTVL";
const char* JSN_SHARE_BASIC_INFO_STR				=	"bIN";
const char* JSN_NODE_NUMBER_STR						=	"nN";
const char* JSN_ESP_RESET_REASON_STR				=	"CrR";
const char* JSN_NATIVE_CLOCK_STR					=	"nC";
const char* JSN_NATIVE_CLOCK_WITH_OFFSET_STR		=	"nCWO";
const char* JSN_CLOCK_OFFSETING_DONE_STR			=	"cOD";
const char* JSN_CLOCK_SYNC_BASE_OFFSET_STR			=	"bO";
const char* JSN_CONNECTOR_LOCK_STATE_STR			=   "ls";
const char* JSN_ASSIGNED_IP_ADDRESS_STR				= 	"iP";
const char* JSN_NATIVE_CLOCK_LSB32_STR              = 	"ncLSB";
const char* JSN_NATIVE_CLOCK_MSB32_STR              = 	"ncMSB";
const char* JSN_FIRMWARE_VERSION_STR			    =   "fwV";
const char* JSN_NODE_ASSIGNED_IP_STR				=   "iP";
const char* JSN_NODE_RESET_INDICATION_STR			=   "rIF";
const char* JSN_TCP_CONNECTION_STATE_STR			=	"TCP";
const char* JSN_UDP_CONNECTION_STATE_STR			=	"UDP";

const char* JSN_SCANNED_VALUE_STR					=	"val";
const char* JSN_ZERO_DETECTION_NODES_STR			= 	"zDN";
const char* JSN_SCANNED_VALUE_STATUS_STR			= 	"scSt";
const char* JSN_SCANNED_FOR_NODE_STR				=   "sFN";
const char* JSN_SCANNED_FOR_NO_OF_OUTPUTS_STR		= 	"tO";
const char* JSN_SCANNED_VALUE_BITSETTER_STR			= 	"bs";
const char* JSN_SCAN_STARTED_ON_STR					=   "sSO";
const char* JSN_SCAN_COMPLETED_ON_STR				=   "sCO";
const char* JSN_ZERO_DETAION_NODE_COUNT_STR			=   "zNC";
const char* JSN_NONZERO_DETAION_NODE_COUNT_STR		=   "nzNC";
const char* JSN_OUTPUT_OPERATED_ON_STR				=	"oOO";
const char* JSN_SACN_ERROR_STS_STR					=	"sES";
const char* JSN_SCAN_ERROR_HAPPENED_FOR_STR			=	"eHF";

const char* JSN_PRODUCT_TOTAL_NODES_STR				=	"tN";
const char* JSN_PRODUCT_IO_MAPPING_STR				=	"ioM";
const char* JSN_SCAN_VAL_RECEIVED_FOR_STR			=   "sVR";
const char* JSN_IO_ASSOCIATED_WITH_NODE_STR			=   "iAN";

const char* JSN_EXPECTED_PROCESS_START_TIME_STR		=   "pST";
const char* JSN_REPORTING_DELAY_STR					=	"rD";
const char* JSN_START_SCANNING_FROM_NODE_STR		=	"sSFN";
const char* JSN_NO_OF_NODES_TO_BE_SCANNED_STR		=	"nNTBS";
const char* JSN_ALLOWED_SCANNING_PERIOD_STR			=	"aSP";
const char* JSN_IDLE_PERIOD_AFTER_SCANNING_STR		=	"iPAS";
const char* JSN_TOTAL_SCAN_PERIOD_FOR_IP_STR		=	"tSPFOO";
const char* JSN_PRODUCT_UNIQUE_ID_STR				=	"pUID";

/**************************************************************************
 * Static Variable Declaration
 *************************************************************************/
static EventGroupHandle_t s_wifi_event_group;
static ip_event_got_ip_t* event;

static TickType_t wifiMonitoringTaskFrequency = 100; //Task execution frequency in ticks[ For WiFi State monitoring ]
static TickType_t wifiMonitoringTaskLastWakeTime;

static int udpSocketID = 0;
//static bool UDPPSocketReconnected = false;
//static uint8_t UDPSocketDisconnectionError = 0;
struct sockaddr_in dest_addrForUDPTx;
static char UDPTxMesg[ UDP_TX_MAX_MESAAGE_SIZE ];
static char TCPTxMesg[ UDP_TX_MAX_MESAAGE_SIZE ];
static UDPTxStatus_t txMesgStatus[ MAX_MESG_TO_APP ];

static int tcpSocketID = 0;
struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
struct addrinfo *address_info = NULL;
static bool TCPPSocketReconnected = false;
static uint8_t TCPSocketDisconnectionError = 0;

static TaskHandle_t wifiMonitoringTaskHandle = NULL;
static TaskHandle_t wifiUDPServerTaskhandle = NULL;

static TaskHandle_t tcpRxNonBlockingTaskHandle = NULL;
static TaskHandle_t txMonitoringTaskHandle = NULL;
/**************************************************************************
 * Static Function Declaration
 *************************************************************************/
static void udp_server_task( void *pvParameters );
static void tcp_server_task_1( void *pvParameters );

static void wifiTaskMonitoring( void );
static void tcpAndudpTxMonitoringTask( void *pvParameters );
static void sendMessageIfRegistered( void );

static void initUDPTxMessagesStatus( void );
static void parseReceivedMessage( const char* const _mesg, uint16_t _mesgLength, uint8_t _tcpOrUDP );
static bool JSN_getUIntegerValueForTheKey( const cJSON *_rootJsnObj, const char* _key, uint32_t *_value );
static void prepareLiveStatusMessage( char *_mesg );
static void prepareExpectedProcessStartTimeResp( char* _mesg );
static void startTCPSocketMonitoringTimer( TickType_t _lastWakeTime, uint32_t _timerPerioTickCount );
static bool createTCPServerSocket( );

static void prepareSyncCommandResponse( char* _mesg );
static bool parseClockOffsetAdjustmentMesg( const cJSON *_jsnObj );
static bool parseJsnArrayMesg( uint8_t _totalNodes, cJSON *_jsnArrayObj, uint8_t *_uintArray );
static bool checkIfJsnObjIsAnArray( cJSON *_jsnObj );
static bool parseProcessStartCommand( const cJSON *_jsnObj, continuityProdConfiguration_t *_productConfig );
static bool validateAndProcessScanValueAckResp( const cJSON *_jsnObj );

/**************************************************************************
 * Function Definitions
 *************************************************************************/

//Call this function only at once during initialization after power restart
void createWifiMonitoringTask( void )
{
	if( wifiUDPServerTaskhandle == NULL )
	{
		startTCPSocketMonitoringTimer( xTaskGetTickCount(), 100 );
		xTaskCreatePinnedToCore( wifiTaskMonitoring, "wifi_monitoring", 4096, ( void* )AF_INET, 2, &wifiMonitoringTaskHandle, 0 );
		WIFI_DBG_LOG( WIFI_DBG_TAG, "[** createWifiMonitoringTask Task Created **]" );
	}
}

//This function can be used to set the period of calling the wifi task monitoring routine
static void startTCPSocketMonitoringTimer( TickType_t _lastWakeTime, uint32_t _timerPerioTickCount )
{
	wifiMonitoringTaskLastWakeTime = _lastWakeTime;
	wifiMonitoringTaskFrequency = (TickType_t)_timerPerioTickCount;
}

static void wifiTaskMonitoring( void )
{
	while( 1 )
	{
		vTaskDelayUntil( &wifiMonitoringTaskLastWakeTime, wifiMonitoringTaskFrequency );

		EventBits_t currentBits = xEventGroupGetBits( s_wifi_event_group );
		if( currentBits & WIFI_CONNECTED_BIT )
		{
			//TCP_DBG_LOG( TCP_DBG_TAG, "WiFi Connected : %d", tcpSocketID );

			/*Check for the Socket ID
			 * If TCP socket ID is set to zero means, Socket is not created and hence not connected with
			 * TCP server running on Application.
			 * Create the TCP socket using function "createTCPServerSocket()" and once TCP socket is created
			 * successfully and connected with application, create TCP Rx monitoring task "createTCPServerClientTask()"
			 */
			if( tcpSocketID == 0 )
			{
				TCP_DBG_LOG( TCP_DBG_TAG, "NULL tcpSocketID Found" );
				if( createTCPServerSocket( ) == true )
				{
					if( tcpSocketID !=0 )
					{
						TCP_DBG_LOG( TCP_DBG_TAG, "Socket connected Successfully" );
						tcpRxNonBlockingTaskHandle = NULL;
						createTCPServerClientTask();
					}
				}

				if( wifiUDPServerTaskhandle == NULL )
					createUDPServerTask();
			}
			startTCPSocketMonitoringTimer( xTaskGetTickCount(), 1500 );
		}
		else
		{
			if( tcpSocketID != 0 )
			{
				WIFI_DBG_LOG( WIFI_DBG_TAG, "tcpSocketID Not NULL, Hence Deleting Task" );
				TCPPSocketReconnected = true;
				tcpSocketID = 0;
			}
			startTCPSocketMonitoringTimer( xTaskGetTickCount(), 10 );
		}
	}
}

void createUDPServerTask( void )
{
	initUDPTxMessagesStatus( );
	if( wifiUDPServerTaskhandle == NULL )
	{
		xTaskCreatePinnedToCore( udp_server_task, "udp_server", 8192, ( void* )AF_INET, 5, &wifiUDPServerTaskhandle, 0 );
		xTaskCreatePinnedToCore( tcpAndudpTxMonitoringTask, "tx_monitoring", 4096, ( void* )AF_INET, 2, &txMonitoringTaskHandle, 0 );
	}
}

static void initUDPTxMessagesStatus( void )
{
	uint8_t ite = 0;

	memset( &txMesgStatus, '\0', sizeof( UDPTxStatus_t ) );
	for( ite = LIVE_STATUS_EXCHANE_MESG; ite < MAX_MESG_TO_APP; ite++ )
	{
		txMesgStatus[ ite ].mesgToBeSent = 0;
		txMesgStatus[ ite ].sentAfterMs = 0;
		txMesgStatus[ ite ].mesgFormationFncPtr = NULL;
	}
	//Assign Function Pointer to actual function
	txMesgStatus[ LIVE_STATUS_EXCHANE_MESG ].mesgFormationFncPtr = &prepareLiveStatusMessage;
	txMesgStatus[ EXPECTED_PROCESS_START_TIME_MESG ].mesgFormationFncPtr = &prepareExpectedProcessStartTimeResp;

	txMesgStatus[ START_OPERATING_OUTPUT_MESG ].mesgFormationFncPtr = &prepareOperateOutputStartEventMesg;
	txMesgStatus[ OUTPUT_OPERATING_DONE_MESG ].mesgFormationFncPtr = &prepareOperateOutputDoneEventMesg;

	txMesgStatus[ CONTINUITY_SCANNED_ZERO_DETECTION_MESG ].mesgFormationFncPtr = &prepareZeroDetectionMessage;
	txMesgStatus[ CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG ].mesgFormationFncPtr = &prepareScannedValueMesgForNextPossibleNode;

	txMesgStatus[ SYNC_COMMAND_RESPONSE_MESG ].mesgFormationFncPtr = &prepareSyncCommandResponse;

	txMesgStatus[ PROCESS_ERROR_MESG ].mesgFormationFncPtr = &prepareProcessErrorMesg;

	//Prepare structure for UDP Tx
	dest_addrForUDPTx.sin_family = AF_INET;
	dest_addrForUDPTx.sin_port = htons( UDP_APP_PORT );
}

static void event_handler( void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data )
{
    if( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START )
    {
    	WIFI_DBG_LOG( WIFI_DBG_TAG, "WIFI_EVENT_STA_START" );
        esp_wifi_connect();
    }
    else if( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED )
    {
		xEventGroupSetBits( s_wifi_event_group, WIFI_FAIL_BIT );
		xEventGroupClearBits( s_wifi_event_group, WIFI_CONNECTED_BIT );

		esp_wifi_connect();
		WIFI_DBG_LOG( WIFI_DBG_TAG, "Reconnecting.." );
    }
    else if( event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP )
    {
        xEventGroupSetBits( s_wifi_event_group, WIFI_CONNECTED_BIT );
        xEventGroupClearBits( s_wifi_event_group, WIFI_FAIL_BIT );

        event = ( ip_event_got_ip_t* ) event_data;
        WIFI_DBG_LOG( WIFI_DBG_TAG, "Connected IP:" IPSTR, IP2STR( &event->ip_info.ip ) );
        //WIFI_DBG_LOG( WIFI_DBG_TAG, "Connected IP**:%s", ip4addr_ntoa( &event->ip_info.ip ) );
    }
    EventBits_t bits = xEventGroupGetBits( s_wifi_event_group );
    WIFI_DBG_LOG( WIFI_DBG_TAG, "BITS:%ld, %ld, %ld", bits, event_id, ( uint32_t )event_base );
}

void wifi_init_sta( void )
{
    s_wifi_event_group = xEventGroupCreate();

    xEventGroupSetBits( s_wifi_event_group, WIFI_FAIL_BIT );
    xEventGroupClearBits( s_wifi_event_group, WIFI_CONNECTED_BIT );

    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT, ESP_EVENT_ANY_ID,
    		                                              &event_handler, NULL, &instance_any_id ) );
    ESP_ERROR_CHECK( esp_event_handler_instance_register( IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                          &event_handler, NULL, &instance_got_ip ) );

    wifi_config_t wifi_config =
    {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &wifi_config ) );
    // Disable WiFi power save
    esp_wifi_set_ps( WIFI_PS_NONE );
    ESP_ERROR_CHECK( esp_wifi_start() );

    WIFI_DBG_LOG( WIFI_DBG_TAG, "wifi_init_sta finished." );

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits( s_wifi_event_group,
            								WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
											pdFALSE, pdFALSE, portMAX_DELAY );

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if( bits & WIFI_CONNECTED_BIT )
    	WIFI_DBG_LOG( WIFI_DBG_TAG, "connected to AP SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS );
    else if( bits & WIFI_FAIL_BIT )
    	WIFI_DBG_LOG( WIFI_DBG_TAG, "Failed to connect to SSID:%s, password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS );
    else
    {
    	WIFI_DBG_LOG( WIFI_DBG_TAG, "UNEXPECTED EVENT" );
    }

    esp_wifi_set_ps( WIFI_PS_NONE );
}

void udp_server_task(void *pvParameters)
{
    char rx_buffer[ 1024 ];
    char addr_str[ 128 ];
    int addr_family = ( int )pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    WIFI_DBG_LOG( WIFI_DBG_TAG, "[** udp_server_task TASK Created **]" );
    while (1)
    {
        if( addr_family == AF_INET )
        {
            struct sockaddr_in *dest_addr_ip4 = ( struct sockaddr_in * )&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl( INADDR_ANY ); //INADDR_ANY
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons( UDP_APP_PORT );
            ip_protocol = IPPROTO_UDP;
        }
        else if( addr_family == AF_INET6 )
        {
            bzero( &dest_addr.sin6_addr.un, sizeof( dest_addr.sin6_addr.un ) );
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons( UDP_APP_PORT );
            ip_protocol = IPPROTO_IPV6;
        }

        //Create UDP Socket
        udpSocketID = socket( addr_family, SOCK_DGRAM, ip_protocol );
        if( udpSocketID < 0 )
        {
        	WIFI_DBG_ERR( WIFI_DBG_TAG, "Unable to create UDP socket: errno %d", errno );
            break;
        }
        WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Socket created" );

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        int enable = 1;
        lwip_setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if( setsockopt( udpSocketID, SOL_SOCKET, SO_REUSEADDR, &timeout, sizeof timeout ) < 0 )
        {
        	WIFI_DBG_ERR( WIFI_DBG_TAG, "UDP setsockopt failed" );
        }

        int err = bind( udpSocketID, ( struct sockaddr * )&dest_addr, sizeof( dest_addr ) );
        if( err < 0 )
        {
        	WIFI_DBG_ERR( WIFI_DBG_TAG, "UDP Socket unable to bind: errno %d", errno );
        }
        //WIFI_DBG_LOG( WIFI_DBG_TAG, "Socket bound, port %d", UDP_APP_PORT );

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof( source_addr );
        //printf( "Time : %llu\n", ( esp_timer_get_time()/1000 ) );

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsgtmp;
        u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

        iov.iov_base = rx_buffer;
        iov.iov_len = sizeof(rx_buffer);
        msg.msg_control = cmsg_buf;
        msg.msg_controllen = sizeof(cmsg_buf);
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (struct sockaddr *)&source_addr;
        msg.msg_namelen = socklen;
#endif

        while( 1 )
        {
        	//CONTINUITY_DBG_LOG( WIFI_DBG_TAG, "CID:%d", xPortGetCoreID() );
        	//vTaskDelay( 10 / portTICK_PERIOD_MS );
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
            int len = recvmsg(sock, &msg, 0);
#else
            int len = recvfrom( udpSocketID, rx_buffer, sizeof( rx_buffer ) - 1, 0,
            		            ( struct sockaddr * )&source_addr, &socklen );
#endif
            // Error occurred during receiving
            if( len > 0 )
            {
                // Get the sender's ip address as string
                if( source_addr.ss_family == PF_INET )
                {
                    inet_ntoa_r( ( ( struct sockaddr_in * )&source_addr )->sin_addr,
                    		     addr_str, sizeof( addr_str ) - 1 );
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
                    for ( cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL; cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp) ) {
                        if ( cmsgtmp->cmsg_level == IPPROTO_IP && cmsgtmp->cmsg_type == IP_PKTINFO ) {
                            struct in_pktinfo *pktinfo;
                            pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgtmp);
                            ESP_LOGI( WIFI_DBG_TAG, "dest ip: %s\n", inet_ntoa(pktinfo->ipi_addr));
                        }
                    }
#endif
                }
                else if( source_addr.ss_family == PF_INET6 )
                {
                    inet6_ntoa_r( ( ( struct sockaddr_in6 * )&source_addr )->sin6_addr,
                    		      addr_str, sizeof( addr_str ) - 1 );
                }

                rx_buffer[ len ] = 0; //Null-terminate whatever we received and treat like a string...
                //WIFI_DBG_LOG( WIFI_DBG_TAG, "Mesg : %s\n", rx_buffer );
                parseReceivedMessage( rx_buffer, strlen( rx_buffer ), UDP_MESSAGE );
            }
        }

        if( udpSocketID != -1 )
        {
        	WIFI_DBG_ERR( WIFI_DBG_TAG, "Shutting down socket and restarting..." );
            shutdown( udpSocketID, 0 );
            close( udpSocketID );
        }
    }
    vTaskDelete( NULL );
}

static bool JSN_getUIntegerValueForTheKey( const cJSON *_rootJsnObj, const char* _key, uint32_t *_value )
{
	if( _rootJsnObj == NULL )
		return false;

	cJSON *commandObj = cJSON_GetObjectItem( _rootJsnObj, JSN_MESSAGE_CODE_STR );
	if( commandObj != NULL )
		*_value = commandObj->valueint;
	else
		return false;

	return true;
}

void createTCPServerClientTask( void )
{
	if( tcpRxNonBlockingTaskHandle == NULL )
	{
		xTaskCreatePinnedToCore( tcp_server_task_1, "tcp_rx_non_blocking_client",
								 4096, ( void* )AF_INET,
								 1, &tcpRxNonBlockingTaskHandle, 0 );
	}
}

/**
 * @brief Indicates that the file descriptor represents an invalid (uninitialized or closed) socket
 *
 * Used in the TCP server structure `sock[]` which holds list of active clients we serve.
 */
#define INVALID_SOCK 				( -1 )

/**
 * @brief Time in ms to yield to all tasks when a non-blocking socket would block
 *
 * Non-blocking socket operations are typically executed in a separate task validating
 * the socket status. Whenever the socket returns `EAGAIN` (idle status, i.e. would block)
 * we have to yield to all tasks to prevent lower priority tasks from starving.
 */
#define YIELD_TO_ALL_MS 			( 50 )
/**
 * @brief Utility to log socket errors
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket number
 * @param[in] err Socket errno
 * @param[in] message Message to print
 */
static void log_socket_error(const char *tag, const int sock, const int err, const char *message)
{
	TCP_DBG_ERR( tag, "[sock=%d]: %s, error=%d: %s", sock, message, err, strerror( err ) );

	//error=104: Connection reset by peer - In case Application is not running on PC
	//error=54 : Connection Closed - In case Socket is connected successfully and then application closed
	//error=113: When PC running the Application is not connected with WiFi
	if( err == 104 ||
        err == 54 ||
		err == 113 )
	{
		TCP_DBG_LOG( TCP_DBG_TAG, "Deleting Socket**:%d", tcpSocketID );

		if( tcpSocketID != 0 )
			close( tcpSocketID );

		if( address_info != NULL )
			free( address_info );

		tcpSocketID = 0;
	}
	TCPSocketDisconnectionError = err;
}
/**
 * @brief Tries to receive data from specified sockets in a non-blocking way,
 *        i.e. returns immediately if no data.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket for reception
 * @param[out] data Data pointer to write the received data
 * @param[in] max_len Maximum size of the allocated space for receiving data
 * @return
 *          >0 : Size of received data
 *          =0 : No data available
 *          -1 : Error occurred during socket read operation
 *          -2 : Socket is not connected, to distinguish between an actual socket error and active disconnection
 */
static int try_receive( const char *tag, const int sock, char * data, size_t max_len )
{
    int len = recv( sock, data, max_len, 0 );
    if( len < 0 )
    {
        if( errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK )
        {
            return 0;   // Not an error
        }
        if( errno == ENOTCONN )
        {
        	TCP_DBG_ERR( TCP_DBG_TAG, "[sock=%d]: Connection closed", sock );
            return -2;  // Socket has been disconnected
        }
        log_socket_error( tag, sock, errno, "Error occurred during receiving" );
        return -1;
    }
    return len;
}

/**
 * @brief Sends the specified data to the socket. This function blocks until all bytes got sent.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket to write data
 * @param[in] data Data to be written
 * @param[in] len Length of the data
 * @return
 *          >0 : Size the written data
 *          -1 : Error occurred during socket write operation
 */
static int socket_send( const char *tag, const int sock, const char * data, const size_t len )
{
    int to_write = len;
    while( to_write > 0 )
    {
        int written = send( sock, data + (len - to_write), to_write, 0 );
        if( written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK )
        {
            log_socket_error( tag, sock, errno, "Error occurred during sending" );
            return -1;
        }
        to_write -= written;
    }
    return len;
}

static bool createTCPServerSocket( )
{
	int res = getaddrinfo( TCP_SERVER_IP_ADDRESS, TCP_SERVER_PORT, &hints, &address_info );
	if( res != 0 || address_info == NULL )
	{
		TCP_DBG_ERR( TCP_DBG_TAG, "couldn't get hostname for `%s` "
					  "getaddrinfo() returns %d, addrinfo=%p", TCP_SERVER_IP_ADDRESS, res, address_info );
		return false;
		//goto error;
	}
	//ESP_LOGI( WIFI_DBG_TAG, "[** tcp_server_task Task Created **]" );

	tcpSocketID = socket( address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol );
	if( tcpSocketID < 0 )
	{
		log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Unable to create socket" );
		return false;
		//goto error;
	}
	ESP_LOGI( TCP_DBG_TAG, "Socket connecting to %s:%s", TCP_SERVER_IP_ADDRESS, TCP_SERVER_PORT );

	struct timeval timeout;
	timeout.tv_sec = 60;  // Set the timeout to 10 seconds
	timeout.tv_usec = 0;

	if (lwip_setsockopt( tcpSocketID, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		log_socket_error(TCP_DBG_TAG, tcpSocketID, errno, "Failed to set SO_RCVTIMEO");
		return false;
	}

	if (lwip_setsockopt( tcpSocketID, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
		log_socket_error(TCP_DBG_TAG, tcpSocketID, errno, "Failed to set SO_SNDTIMEO");
		return false;
	}

	int keepalive = 60;
	if (lwip_setsockopt( tcpSocketID, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0) {
		log_socket_error(TCP_DBG_TAG, tcpSocketID, errno, "Failed to set SO_KEEPALIVE");
		return false;
	}

//	// Set keep-alive parameters for the socket
//	if (lwip_setsockopt(tcpSocketID, SOL_SOCKET, SO_KEEPALIVE, &cfg, sizeof( cfg ) ) < 0 ) {
//	    log_socket_error(TCP_DBG_TAG, tcpSocketID, errno, "Failed to set TCP_KEEPALIVE");
//	    return false;
//	}

	// Marking the socket as non-blocking
	int flags = fcntl( tcpSocketID, F_GETFL );
	if( fcntl( tcpSocketID, F_SETFL, flags | O_NONBLOCK ) == -1 )
	{
		log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Unable to set socket non blocking" );
	}

	int connectError = connect( tcpSocketID, address_info->ai_addr, address_info->ai_addrlen );
	TCP_DBG_LOG( TCP_DBG_TAG, "connection in progress : connectError : %d", connectError );
	if( connectError != 0 )
	{
		if( errno == EINPROGRESS )
		{
			fd_set fdset;
			FD_ZERO( &fdset );
			FD_SET( tcpSocketID, &fdset );

			// Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
			res = select( tcpSocketID+1, NULL, &fdset, NULL, NULL );
			TCP_DBG_LOG( TCP_DBG_TAG, "connection in progress res : %d", res );
			if( res < 0 )
			{
				log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Error during connection: select for socket to be writable" );
				return false;
				//goto error;
			}
			else if( res == 0 )
			{
				log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Connection timeout: select for socket to be writable" );
				return false;
				//goto error;
			}
			else
			{
				socklen_t len = ( socklen_t )sizeof(int);
				int sockerr;
				if( getsockopt( tcpSocketID, SOL_SOCKET, SO_ERROR, (void*)( &sockerr ), &len ) < 0 )
				{
					log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Error when getting socket error using getsockopt()" );
					return false;
					//goto error;
				}

				if( sockerr )
				{
					log_socket_error( TCP_DBG_TAG, tcpSocketID, sockerr, "Connection error**" );
					return false;
					//goto error;
				}
			}
		}
		else
		{
			log_socket_error( TCP_DBG_TAG, tcpSocketID, errno, "Socket is unable to connect" );
			return false;
			//goto error;
		}
	}
	return true;
}

static void tcp_server_task_1( void *pvParameters )
{
	static char rx_buffer[ 1024 ];
	int len = 0;

	TCP_DBG_LOG( TCP_DBG_TAG, "[** tcp_server_task_1 Task Created **]" );
	do
	{
//		if( tcpSocketID == 0 )
//			continue;

		len = try_receive( TCP_DBG_TAG, tcpSocketID, rx_buffer, sizeof( rx_buffer ) );
		if( len < 0 )
		{
			TCP_DBG_ERR( TCP_DBG_TAG, "Error in try_receive" );
			break;
		}
		else if( len> 0 )
		{
			//TCP_DBG_LOG( TCP_DBG_TAG, "Received: %d-%s", len, rx_buffer );

			rx_buffer[ len ] = 0; //Null-terminate whatever we received and treat like a string...
			parseReceivedMessage( rx_buffer, strlen( rx_buffer ), TCP_MESSAGE );
		}
		vTaskDelay( 2 / portTICK_PERIOD_MS ); //2ms Sleep
	}while( len >= 0 );

	TCP_DBG_ERR( TCP_DBG_TAG, "TCP_Socket Error Found");
	vTaskDelete( NULL );
}

static void tcpAndudpTxMonitoringTask( void *pvParameters )
{
	TCP_DBG_LOG( TCP_DBG_TAG, "[** tcpAndudpTxMonitoringTask Task Created **]" );
	while( 1 )
	{
		sendMessageIfRegistered( );
		vTaskDelay( 2 / portTICK_PERIOD_MS );
	}
}

static void sendMessageIfRegistered( void )
{
	uint8_t ite = ( uint8_t )LIVE_STATUS_EXCHANE_MESG;

#if defined( MONITOR_HEAP_MEMORY )
	uint32_t heapMemoryBytes = 0;
#endif

	for( ite = LIVE_STATUS_EXCHANE_MESG; ite < MAX_MESG_TO_APP; ite++ )
	{
//		if( ite != LIVE_STATUS_EXCHANE_MESG && txMesgStatus[ ite ].mesgToBeSent == 1 )
//			WIFI_DBG_LOG( WIFI_DBG_TAG, "Mesg Found : %d, %ld", ite, txMesgStatus[ ite ].sentAfterMs );

		//Check If Message is not registered
		if( txMesgStatus[ ite ].mesgToBeSent == 0 )
			continue;

		//Check if instant arrived on which TCP message to be sent
		uint64_t currentTime = esp_timer_get_time()/1000;
		if( ( txMesgStatus[ ite ].sentAfterMs != 1 ) &&
			( ( currentTime - txMesgStatus[ ite ].registredOn ) < txMesgStatus[ ite ].sentAfterMs ) )
			return;

		//Check if message to be sent using TCP socket
		if( txMesgStatus[ ite ].UDPOrTCPMesg == TCP_MESSAGE )
		{
			//return from here if not a valid socket ID found
			if( tcpSocketID == 0 )
				return;

#if defined( MONITOR_HEAP_MEMORY )
			heapMemoryBytes = esp_get_free_heap_size();
#endif

			memset( TCPTxMesg, '\0', sizeof( TCPTxMesg ) );
			txMesgStatus[ ite ].mesgFormationFncPtr( &TCPTxMesg[ 0 ] );

#if defined( MONITOR_HEAP_MEMORY )
			if( esp_get_free_heap_size() != heapMemoryBytes )
				WIFI_DBG_LOG( WIFI_DBG_TAG, "**Mem Leak:%ld, %ld, %d********\n",
						      ( esp_get_free_heap_size() - heapMemoryBytes ), esp_get_free_heap_size(), ite );
#endif

			/*
			if( txMesgStatus[ ite ].UDPOrTCPMesg == TCP_MESSAGE )
				WIFI_DBG_LOG( WIFI_DBG_TAG, "TCP TX:%d,%s", strlen( TCPTxMesg ), TCPTxMesg );
			else
				WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP TX:%d,%s", strlen( TCPTxMesg ), TCPTxMesg );
			*/

			for( uint8_t retryCntr = 0; retryCntr < 3; retryCntr++ )
			{
				if( tcpSocketID == 0 )
					return;

				int err = socket_send( TCP_DBG_TAG, tcpSocketID, TCPTxMesg, strlen( TCPTxMesg ) );
				if (err < 0)
				{
					WIFI_DBG_ERR( WIFI_DBG_TAG, "Err In TCP Tx: %d", errno);
					vTaskDelay( 5 / portTICK_PERIOD_MS );
				}
				else
				{
					break;
				}
				//WIFI_DBG_LOG( WIFI_DBG_TAG, "TCP Tx Retrying..." );
			}
		}
		else if( txMesgStatus[ ite ].UDPOrTCPMesg == UDP_MESSAGE ) ////Check if message to be sent using UDP socket
		{
			//return from here if not a valid socket ID found
			if( udpSocketID == 0 )
				return;

#if defined( MONITOR_HEAP_MEMORY )
			heapMemoryBytes = esp_get_free_heap_size();
#endif
			memset( UDPTxMesg, '\0', sizeof( UDPTxMesg ) );
			txMesgStatus[ ite ].mesgFormationFncPtr( &UDPTxMesg[ 0 ] );

#if defined( MONITOR_HEAP_MEMORY )
			if( esp_get_free_heap_size() != heapMemoryBytes )
				WIFI_DBG_LOG( WIFI_DBG_TAG, "**Mem Leak:%ld, %ld, %d********\n",
							( esp_get_free_heap_size() - heapMemoryBytes ), esp_get_free_heap_size(), ite );
#endif
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP TX Mesg:%d,%s", strlen( UDPTxMesg ), UDPTxMesg );

			if( txMesgStatus[ ite ].P2POrBroadcast == BROADCAST_MESG )
				dest_addrForUDPTx.sin_addr.s_addr = inet_addr( BROADCAST_IP_ADDRESS );
			else
				dest_addrForUDPTx.sin_addr.s_addr = inet_addr( P2P_IP_ADDRESS );

			for( uint8_t retryCntr = 0; retryCntr < 3; retryCntr++ )
			{
				//return from here if not a valid socket ID found
				if( udpSocketID == 0 )
					return;

				int err = sendto( udpSocketID, UDPTxMesg, strlen( UDPTxMesg ), 0,
								  ( struct sockaddr * )&dest_addrForUDPTx, sizeof( dest_addrForUDPTx ) );
				if (err < 0)
				{
					WIFI_DBG_ERR( WIFI_DBG_TAG, "Err In UDP Tx: %d", errno);
					vTaskDelay( 5 / portTICK_PERIOD_MS );
				}
				else
				{
					break;
				}
				WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Tx Retrying..." );
			}
		}
	}
}

static void parseReceivedMessage( const char* const _mesg, uint16_t _mesgLength, uint8_t _tcpOrUDP )
{
	uint32_t command = 0;

	const cJSON *rootJsnObj = cJSON_Parse( _mesg );
	if( rootJsnObj == NULL )
		return;

	if( JSN_getUIntegerValueForTheKey( rootJsnObj, JSN_MESSAGE_CODE_STR, &command ) != true )
	{ cJSON_Delete(rootJsnObj); return; }

//	if( command != LIVE_STATUS_EXCHANGE_CMD )
//		WIFI_DBG_LOG( WIFI_DBG_TAG, "TCP MC: %ld", command );

	switch( command )
	{
		case LIVE_STATUS_EXCHANGE_CMD:
		{
			cJSON *msgIntervalObj = cJSON_GetObjectItem( rootJsnObj, JSN_SENT_MESSAGE_AFTER_INTERVAL_STR );
			cJSON *basicInfoNeedeObj = cJSON_GetObjectItem( rootJsnObj, JSN_SHARE_BASIC_INFO_STR );

			lockCurrentOffsettedClockValue( );
			registerTxMessage( LIVE_STATUS_EXCHANE_MESG, ( NODE_NUMBER * msgIntervalObj->valueint), P2P_MESG, UDP_MESSAGE );
			registerTxMessage( NODE_RESET_INDICATION_MESG, ( NODE_NUMBER * msgIntervalObj->valueint), P2P_MESG, UDP_MESSAGE );

			if( basicInfoNeedeObj->valueint == 1 )
				registerTxMessage( NODE_BASIC_INFORMATION_MESG, NODE_NUMBER, msgIntervalObj->valueint, UDP_MESSAGE );
		}
		break;

		case TCP_GET_EXPECTED_PROCESS_START_TIME:
		{
			cJSON *totalNodeJsnObj = cJSON_GetObjectItem( rootJsnObj, JSN_PRODUCT_TOTAL_NODES_STR );
			cJSON *IOMapJsnArrayObj = cJSON_GetObjectItem( rootJsnObj, JSN_PRODUCT_IO_MAPPING_STR );

			//Calculate the expected process time
			uint8_t totalNodes = totalNodeJsnObj->valueint;
			uint8_t ioMapArray[ MAX_SUPPORTED_NODES ];

			if( parseJsnArrayMesg( totalNodes, IOMapJsnArrayObj, ioMapArray ) != false )
			{
				calculateExpectedProcessTime( totalNodes, ioMapArray );
				registerTxMessage( EXPECTED_PROCESS_START_TIME_MESG, 1, P2P_MESG, TCP_MESSAGE );
			}
			else
				return;
		}
		break;

		case TCP_PROCESS_START_CMD:
		{
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "PROCESS_START_CMD:\n%s", _mesg );
			continuityProdConfiguration_t prodConfig;
			if( parseProcessStartCommand( rootJsnObj, &prodConfig ) == true )
				setProductCOnfigurationParameters( prodConfig );
		}
		break;

		case SHARE_SCANNED_INPUT_VALUES_CMD:
		{
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "SHARE_SCANNED_INPUT_VALUES_CMD : \n%s", _mesg );
			setApplReqToShareScannedValueStatus( 1 );
		}
		break;

		case SCANNED_INPUT_VALUE_ACK_CMD:
		{
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "SCANNED_INPUT_VALUE_ACK_CMD : \n%s", _mesg );
			validateAndProcessScanValueAckResp( rootJsnObj );
		}
		break;

		case MQTT_GET_CLOCK_VALUE:
		{
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "MQTT_GET_CLOCK_VALUE : \n%s", _mesg );
			registerTxMessage( SYNC_COMMAND_RESPONSE_MESG, ( getNodeNumber() * 2 ), P2P_MESG, TCP_MESSAGE );
		}
		break;

		case MQTT_CLOCK_OFFSET_ADJUSTMENT:
		{
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "MQTT_CLOCK_OFFSET_ADJUSTMENT : \n%s", _mesg );
			if( parseClockOffsetAdjustmentMesg( rootJsnObj ) == true )
			{
					//WIFI_DBG_LOG( WIFI_DBG_TAG, "SYNC_RESPONSE_MESG Registered" );
					registerTxMessage( SYNC_COMMAND_RESPONSE_MESG, ( getNodeNumber() * 2 ), P2P_MESG, TCP_MESSAGE );
			}
		}
		break;

		case STOP_SCANIING_PROCESS_CMD:
		{
			stopContinuityProcess( );
		}
		break;
	}
	cJSON_Delete( rootJsnObj );
}

static bool parseClockOffsetAdjustmentMesg( const cJSON *_jsnObj )
{
	cJSON *baseOffsetJsnArrObj = cJSON_GetObjectItem( _jsnObj, JSN_CLOCK_SYNC_BASE_OFFSET_STR );

	if( checkIfJsnObjIsAnArray( baseOffsetJsnArrObj ) != false )
	{
		int array_size = cJSON_GetArraySize( baseOffsetJsnArrObj );
		//WIFI_DBG_LOG( WIFI_DBG_TAG, "ArrSize: %d, NNum: %d", array_size, getNodeNumber() );
		if( array_size >= getNodeNumber() )
		{
			cJSON *array_item = cJSON_GetArrayItem( baseOffsetJsnArrObj, ( getNodeNumber() - 1 ) );
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "ArrSize : %d, NodeNum-1 : %d", cJSON_IsNumber( array_item ),
			//			array_item->valueint );
			if( cJSON_IsNumber( array_item ) )
			{
				COMMON_DEBUG_LOG( CLK_SYNK_DBG_TAG, "Offset: %d", array_item->valueint );
				setClockOffset( array_item->valuedouble );
				lockCurrentOffsettedClockValue( );
				//WIFI_DBG_LOG( WIFI_DBG_TAG, "CLK Offset Set : %s", cJSON_Print( _jsnObj )  );
				return true;
			}
		}
	}
	else
		JSN_DBG_ERR( JSN_DBG_TAG, "Invalid Jsn Arr in clk Offset" );

	return false;
}

static bool validateAndProcessScanValueAckResp( const cJSON *_jsnObj )
{
	uint8_t nodeNumber = cJSON_GetObjectItem( _jsnObj, JSN_NODE_NUMBER_STR )->valueint;
	cJSON *scanForNodeJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_SCANNED_FOR_NODE_STR );
	cJSON *scanValRecAckJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_SCAN_VAL_RECEIVED_FOR_STR );

	if( nodeNumber != getNodeNumber( ) )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "Invalid Ack Jsn-1" );
		return false;
	}

	if( ( checkIfJsnObjIsAnArray( scanForNodeJsnObj ) != true ) ||
		( checkIfJsnObjIsAnArray( scanValRecAckJsnObj ) != true ) ||
		( cJSON_GetArraySize( scanForNodeJsnObj ) != cJSON_GetArraySize( scanValRecAckJsnObj ) ) )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "Invalid Ack Jsn-2, %d, %d", cJSON_GetArraySize( scanForNodeJsnObj ),
				                                               cJSON_GetArraySize( scanValRecAckJsnObj ) );
		return false;
	}

	processScanValueReceivedAckResponse( scanForNodeJsnObj, scanValRecAckJsnObj );
	return true;
}

static bool parseProcessStartCommand( const cJSON *_jsnObj, continuityProdConfiguration_t *_productConfig )
{
	cJSON *scanPeriodJsnObj = NULL, *idlePeriodAfterScanningJsnObj = NULL, *noOfNodesToBeScannedJsnObj = NULL,
		  *processStartTimeJsnObj = NULL, *startScanningFromNodeJsnObj = NULL, *totalNodesJsnObj = NULL,
		  *scanTimeJsnObje = NULL, *IOMapJsnArrayObj = NULL, *productUIDJsnObj = NULL;

	scanPeriodJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_ALLOWED_SCANNING_PERIOD_STR );
	idlePeriodAfterScanningJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_IDLE_PERIOD_AFTER_SCANNING_STR );
	noOfNodesToBeScannedJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_NO_OF_NODES_TO_BE_SCANNED_STR );
	processStartTimeJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_EXPECTED_PROCESS_START_TIME_STR );
	startScanningFromNodeJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_START_SCANNING_FROM_NODE_STR );
	totalNodesJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_PRODUCT_TOTAL_NODES_STR );
	scanTimeJsnObje = cJSON_GetObjectItem( _jsnObj, JSN_TOTAL_SCAN_PERIOD_FOR_IP_STR );
	IOMapJsnArrayObj = cJSON_GetObjectItem( _jsnObj, JSN_PRODUCT_IO_MAPPING_STR );
	productUIDJsnObj = cJSON_GetObjectItem( _jsnObj, JSN_PRODUCT_UNIQUE_ID_STR );

	memset( _productConfig, '\0', sizeof( continuityProdConfiguration_t ) );
	if( parseJsnArrayMesg( totalNodesJsnObj->valueint, IOMapJsnArrayObj, _productConfig->IOassociatedWithNode ) == false ||
		scanPeriodJsnObj == NULL || idlePeriodAfterScanningJsnObj == NULL || noOfNodesToBeScannedJsnObj == NULL ||
		processStartTimeJsnObj == NULL || startScanningFromNodeJsnObj == NULL || totalNodesJsnObj == NULL ||
		scanTimeJsnObje == NULL )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "PROCESS_START_ERR-1" );
		return false;
	}

	_productConfig->totalProductNodes = totalNodesJsnObj->valueint;
	_productConfig->startScanningFromNode = startScanningFromNodeJsnObj->valueint;
	_productConfig->noOfNodesToBeScanned = noOfNodesToBeScannedJsnObj->valueint;
	_productConfig->allowedScanningPeriod = scanPeriodJsnObj->valueint;
	_productConfig->idlePeriodAfterScanning = idlePeriodAfterScanningJsnObj->valueint;
	uint16_t totalScanPeriodForOneOp = _productConfig->allowedScanningPeriod +
									   _productConfig->idlePeriodAfterScanning;

	if( totalScanPeriodForOneOp != scanTimeJsnObje->valueint )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "PROCESS_START_ERR-2" );
		return false;
	}

	_productConfig->totalScanningPeriodForOneOp = scanTimeJsnObje->valueint;
	_productConfig->productUniqueID = productUIDJsnObj->valueint;

	for( uint8_t node = 0; node < _productConfig->noOfNodesToBeScanned; node++ )
	{
		if( node == 0 )
			_productConfig->startScanningOn[ node ] = processStartTimeJsnObj->valuedouble;
		else
			_productConfig->startScanningOn[ node ] = _productConfig->startScanningOn[ node - 1 ] +
												      ( _productConfig->IOassociatedWithNode[ node - 1 ] * totalScanPeriodForOneOp ) +
													  GAURD_TIME_BETWEEN_TWO_NODE;
		JSN_DBG_LOG( JSN_DBG_TAG, "startScanningOn[ %d ] : %lld", node, _productConfig->startScanningOn[ node ] );
	}
	return true;
}

static bool parseJsnArrayMesg( uint8_t _totalNodes, cJSON *_jsnArrayObj, uint8_t *_uintArray )
{
	if( !checkIfJsnObjIsAnArray( _jsnArrayObj ) )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "Invalid JSN - 1" );
		return false;
	}

	uint8_t array_size = cJSON_GetArraySize( _jsnArrayObj );
	uint8_t expectedArraySize = ( _totalNodes %4 == 0 ) ? ( _totalNodes / 4 ) : ( ( _totalNodes / 4 ) + 1 );

	if( array_size != expectedArraySize )
	{
		JSN_DBG_ERR( JSN_DBG_TAG, "Invalid JSN - 2" );
		return false;
	}

	uint8_t arrayIndex, bitPosIn32Bit, nodeIOs;
	uint32_t ioValue = 0;
	cJSON *array_item = NULL;
	for( uint8_t node = 0; node < _totalNodes; node++ )
	{
		arrayIndex = node / 4;
		bitPosIn32Bit = node % 4;
		nodeIOs = ioValue = 0;
		array_item = NULL;

		array_item = cJSON_GetArrayItem( _jsnArrayObj, arrayIndex );
		if( cJSON_IsNumber( array_item ) )
			ioValue = array_item->valueint;

		if( bitPosIn32Bit == 0 )
			nodeIOs = ioValue & 0xFF;
		else if( bitPosIn32Bit == 1 )
			nodeIOs = ( ioValue >> 8 ) & 0xFF;
		else if( bitPosIn32Bit == 2 )
			nodeIOs = ( ioValue >> 16 ) & 0xFF;
		else if( bitPosIn32Bit == 3 )
			nodeIOs = ( ioValue >> 24 ) & 0xFF;

		_uintArray[ node ] = nodeIOs;

	}
	return true;
}

static bool checkIfJsnObjIsAnArray( cJSON *_jsnObj )
{
	if( _jsnObj == NULL || !cJSON_IsArray( _jsnObj ) )
	{
		return false;
	}
	return true;
}

void registerTxMessage( UDPTxMesgIdentifier_e _mesgIdent, uint32_t _sentAfter, uint8_t _p2pOrBroadcast, uint8_t UDPOrTCP )
{
	if( _mesgIdent < MAX_MESG_TO_APP )
	{
		//WIFI_DBG_LOG( WIFI_DBG_TAG, "Regstrd : %d", _mesgIdent );
		txMesgStatus[ _mesgIdent ].mesgToBeSent = 1;
		txMesgStatus[ _mesgIdent ].UDPOrTCPMesg = UDPOrTCP;

		if( _mesgIdent == LIVE_STATUS_EXCHANE_MESG ||
			_mesgIdent == LOCK_CONNECTOR_STATE_CHANGE_MESG ||
		    _mesgIdent == NODE_RESET_INDICATION_MESG ||
			_mesgIdent == NODE_BASIC_INFORMATION_MESG )
		{
			//txMesgStatus[ LIVE_STATUS_EXCHANE_MESG ].mesgToBeSent = 1;
			txMesgStatus[ LIVE_STATUS_EXCHANE_MESG ].registredOn = esp_timer_get_time()/1000;
			txMesgStatus[ LIVE_STATUS_EXCHANE_MESG ].sentAfterMs = _sentAfter;
			txMesgStatus[ LIVE_STATUS_EXCHANE_MESG ].P2POrBroadcast = P2P_MESG;
			//WIFI_DBG_LOG( WIFI_DBG_TAG, "Live status Exchange Message Registered" );
		}
		else
		{
			txMesgStatus[ _mesgIdent ].registredOn = esp_timer_get_time()/1000;
			txMesgStatus[ _mesgIdent ].sentAfterMs = _sentAfter;
			txMesgStatus[ _mesgIdent ].P2POrBroadcast = _p2pOrBroadcast;
		}
	}
}

void clearRegisteredMesg( UDPTxMesgIdentifier_e _mesgIdent )
{
	if( _mesgIdent < MAX_MESG_TO_APP )
	{
		//WIFI_DBG_LOG( WIFI_DBG_TAG, "Cleared : %d", _mesgIdent );
		txMesgStatus[ _mesgIdent ].mesgToBeSent = 0;
		txMesgStatus[ _mesgIdent ].registredOn = 0;
		txMesgStatus[ _mesgIdent ].sentAfterMs = 0;

		if( _mesgIdent == LIVE_STATUS_EXCHANE_MESG )
		{
			txMesgStatus[ LOCK_CONNECTOR_STATE_CHANGE_MESG ].mesgToBeSent = 0;
			txMesgStatus[ NODE_RESET_INDICATION_MESG ].mesgToBeSent = 0;
			txMesgStatus[ NODE_BASIC_INFORMATION_MESG ].mesgToBeSent = 0;
		}
	}
}

bool checkIfAnySimilarMesgRegistered( UDPTxMesgIdentifier_e _mesgIdent )
{
	if( _mesgIdent < MAX_MESG_TO_APP && txMesgStatus[ _mesgIdent ].mesgToBeSent )
		return true;

	return false;
}

static void prepareLiveStatusMessage( char* _mesg )
{
	cJSON *rootJSNObj = cJSON_CreateObject();
	cJSON *tcpJsonArray = cJSON_CreateArray();

	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, LIVE_STATUS_EXCHANGE_CMD );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );
	cJSON_AddNumberToObject(rootJSNObj, JSN_FREE_HEAP_MEMORY, (double)esp_get_free_heap_size() );


	if( checkIfAnySimilarMesgRegistered( NODE_RESET_INDICATION_MESG ) )
	{
		cJSON_AddNumberToObject(rootJSNObj, JSN_NODE_RESET_INDICATION_STR, 1);
		cJSON_AddNumberToObject(rootJSNObj, JSN_ESP_RESET_REASON_STR, (uint8_t)esp_reset_reason());
		cJSON_AddNumberToObject(rootJSNObj, JSN_CLOCK_OFFSETING_DONE_STR, hasClockOffsettingDone() ? 1 : 0);
		cJSON_AddNumberToObject(rootJSNObj, JSN_NATIVE_CLOCK_WITH_OFFSET_STR, (double)getClockValueAfterOffset());

		//Check if TCP Reconnection Happened
		if( TCPPSocketReconnected && TCPSocketDisconnectionError != 0 )
		{
		    cJSON_AddItemToArray( tcpJsonArray, cJSON_CreateNumber( TCPPSocketReconnected ? 1 : 0 ) );
		    cJSON_AddItemToArray( tcpJsonArray, cJSON_CreateNumber( TCPSocketDisconnectionError ) );
		    cJSON_AddItemToObject( rootJSNObj, JSN_TCP_CONNECTION_STATE_STR, tcpJsonArray );

		    TCPPSocketReconnected = false;
		    TCPSocketDisconnectionError = 0;
		}

		esp_netif_ip_info_t ip_info;
		esp_netif_get_ip_info( esp_netif_get_handle_from_ifkey( "WIFI_STA_DEF" ), &ip_info );
		cJSON_AddStringToObject( rootJSNObj, JSN_NODE_ASSIGNED_IP_STR, ip4addr_ntoa( &ip_info.ip ) );
	}

	if( checkIfAnySimilarMesgRegistered( LOCK_CONNECTOR_STATE_CHANGE_MESG ) ||
		checkIfAnySimilarMesgRegistered( NODE_RESET_INDICATION_MESG ) )
	{
		//@TODO:Replace with Original Connector Lock Status, As Connector lock status reading not implemented
		cJSON_AddNumberToObject( rootJSNObj, JSN_ESP_RESET_REASON_STR, 0 );
	}

//	if( checkIfAnySimilarMesgRegistered( NODE_BASIC_INFORMATION_MESG ) ||
//		checkIfAnySimilarMesgRegistered( NODE_RESET_INDICATION_MESG ) )
//	{
//		//@TODO:Replace with actual assigned IP address later
//		cJSON_AddStringToObject( rootJSNObj, JSN_ASSIGNED_IP_ADDRESS_STR, P2P_IP_ADDRESS );
//	}

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}

	//WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Tx-Live Status Mesg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	cJSON_Delete( tcpJsonArray );
	clearRegisteredMesg( LIVE_STATUS_EXCHANE_MESG );
}

static void prepareExpectedProcessStartTimeResp( char* _mesg )
{
	cJSON *rootJSNObj = NULL;

	rootJSNObj = cJSON_CreateObject();
	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, TCP_EXPECTED_PROCESS_START_TIME_RESP );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );

	cJSON *processStartTimeJsnObj = cJSON_CreateNumber( ( double )getProcessExpectedStartTime() );
	cJSON *nativeClkWithOffsetJsnObj = cJSON_CreateNumber( ( double )getClockValueAfterOffset() );

	if( processStartTimeJsnObj == NULL ||
		nativeClkWithOffsetJsnObj == NULL )
	{
		fprintf( stderr, "Error creating cJSON object for uint64_t value\n" );
		cJSON_Delete( rootJSNObj );
		return;
	}
	cJSON_AddItemToObject( rootJSNObj, JSN_EXPECTED_PROCESS_START_TIME_STR, processStartTimeJsnObj );
	cJSON_AddItemToObject( rootJSNObj, JSN_NATIVE_CLOCK_WITH_OFFSET_STR, nativeClkWithOffsetJsnObj );

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}
	//WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Tx-Process Start Time Mesg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( EXPECTED_PROCESS_START_TIME_MESG );
}

static void prepareSyncCommandResponse( char* _mesg )
{
	uint64_t nativeClock = esp_timer_get_time()/1000;
	cJSON *rootJSNObj = cJSON_CreateObject();

	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, getNodeNumber() );
	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, MQTT_GET_CLOCK_VALUE );

	uint64_t offsettedClockValue = 0;
	if( isOffsettedClockValueLocked( &offsettedClockValue ) == false )
	{
		resetOffsettedClockValue( );
		offsettedClockValue = ( double )getClockValueAfterOffset();
	}

	cJSON_AddItemToObject( rootJSNObj, JSN_NATIVE_CLOCK_STR, cJSON_CreateNumber( (double)nativeClock ) );
	cJSON_AddItemToObject( rootJSNObj, JSN_NATIVE_CLOCK_WITH_OFFSET_STR, cJSON_CreateNumber( (double)offsettedClockValue ) );
	cJSON_AddNumberToObject( rootJSNObj, JSN_CLOCK_OFFSETING_DONE_STR, hasClockOffsettingDone() ? 1 : 0 );

	char* jsonStr = cJSON_Print( rootJSNObj ); // Allocate memory for JSON string
	if( jsonStr != NULL )
	{
		strcpy( _mesg, jsonStr ); 	// Copy JSON string to _mesg
		free( jsonStr ); 			// Free allocated memory
	}
	//WIFI_DBG_LOG( WIFI_DBG_TAG, "UDP Get Clk Val Resp Mesg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( SYNC_COMMAND_RESPONSE_MESG );
}
