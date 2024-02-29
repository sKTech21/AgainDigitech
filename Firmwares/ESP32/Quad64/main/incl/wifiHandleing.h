/*
 * wifiHandleing.h
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#ifndef MAIN_INCL_WIFIHANDLEING_H_
#define MAIN_INCL_WIFIHANDLEING_H_

#define UDP_IDLE_RX_TIMEOUT				( 5000 )
#define UDP_RX_TIMEOUT_DURING_PROCESS	( 2000 )
#define UDP_NO_OF_MESG_STORAGE			( 5 )
#define UDP_TX_MAX_MESAAGE_SIZE			( 2048 )

#define BROADCAST_MESG					( 0 )
#define P2P_MESG						( 1 )

#define UDP_MESSAGE						( 0 )
#define TCP_MESSAGE						( 1 )

//Key Used in JSON Messages
extern const char* JSN_MESSAGE_CODE_STR;
extern const char* JSN_SENT_MESSAGE_AFTER_INTERVAL_STR;
extern const char* JSN_SHARE_BASIC_INFO_STR;
extern const char* JSN_NODE_NUMBER_STR;
extern const char* JSN_ESP_RESET_REASON_STR;
extern const char* JSN_NATIVE_CLOCK_STR;
extern const char* JSN_NATIVE_CLOCK_WITH_OFFSET_STR;
extern const char* JSN_CONNECTOR_LOCK_STATE_STR;
extern const char* JSN_CLOCK_SYNC_BASE_OFFSET_STR;
extern const char* JSN_NATIVE_CLOCK_LSB32_STR;
extern const char* JSN_NATIVE_CLOCK_MSB32_STR;
extern const char* JSN_FIRMWARE_VERSION_STR;
extern const char* JSN_NODE_ASSIGNED_IP_STR;
extern const char* JSN_TCP_CONNECTION_STATE_STR;
extern const char* JSN_UDP_CONNECTION_STATE_STR;
extern const char* JSN_SCANNED_VALUE_STR;
extern const char* JSN_SCANNED_FOR_NODE_STR;
extern const char* JSN_ZERO_DETECTION_NODES_STR;
extern const char* JSN_SCANNED_FOR_NO_OF_OUTPUTS_STR;
extern const char* JSN_SCANNED_VALUE_STATUS_STR;
extern const char* JSN_SCANNED_VALUE_BITSETTER_STR;
extern const char* JSN_SCAN_STARTED_ON_STR;
extern const char* JSN_SCAN_COMPLETED_ON_STR;
extern const char* JSN_ZERO_DETAION_NODE_COUNT_STR;
extern const char* JSN_NONZERO_DETAION_NODE_COUNT_STR;
extern const char* JSN_IO_ASSOCIATED_WITH_NODE_STR;
extern const char* JSN_OUTPUT_OPERATED_ON_STR;
extern const char* JSN_SACN_ERROR_STS_STR;
extern const char* JSN_SCAN_ERROR_HAPPENED_FOR_STR;

extern const char* JSN_EXPECTED_PROCESS_START_TIME_STR;
extern const char* JSN_SCAN_VAL_RECEIVED_FOR_STR;

extern const char* JSN_REPORTING_DELAY_STR;
extern const char* JSN_START_SCANNING_FROM_NODE_STR;
extern const char* JSN_NO_OF_NODES_TO_BE_SCANNED_STR;
extern const char* JSN_ALLOWED_SCANNING_PERIOD_STR;
extern const char* JSN_IDLE_PERIOD_AFTER_SCANNING_STR;
extern const char* JSN_TOTAL_SCAN_PERIOD_FOR_IP_STR;
extern const char* JSN_PRODUCT_UNIQUE_ID_STR;

typedef struct
{
	uint64_t 	lastRxOn;
	char 	 	mesg[ 1024 ];
	uint8_t	 	hasNewMesgArrived;
	uint8_t  	isUDPRxTimeoutHappened;	//Refer to RX timeout set in Macro
}UDPRx_t;

typedef struct
{
	uint8_t     UDPOrTCPMesg;
	uint8_t 	P2POrBroadcast;				//0 = Broadcast else 1
	uint8_t  	mesgToBeSent;				//1 If message needs to sent to Application, 0 = Send Immediately
	uint32_t 	sentAfterMs;				//Message will be sent after once set mS period expired
	uint64_t 	registredOn;
	void ( *mesgFormationFncPtr )( char* _mesg );
}UDPTxStatus_t;

typedef enum
{
	LIVE_STATUS_EXCHANE_MESG = 0,
	CONTINUITY_SCANNED_ZERO_DETECTION_MESG,
	CONTINUITY_SCANNED_NON_ZERO_DETECTION_MESG,
	PRODUCT_CONFIG_RECEIVED_ACK_MESG,
	LEAK_TEST_RESULT_MESG,
	NATIVE_CLOCK_VALUE_MESG,
	START_SCAN_COMMAND_MESG,

	LOCK_CONNECTOR_STATE_CHANGE_MESG,
	NODE_RESET_INDICATION_MESG,
	NODE_BASIC_INFORMATION_MESG,

	EXPECTED_PROCESS_START_TIME_MESG,

	START_OPERATING_OUTPUT_MESG,
	OUTPUT_OPERATING_DONE_MESG,

	SYNC_COMMAND_RESPONSE_MESG,

	PROCESS_ERROR_MESG,

	MAX_MESG_TO_APP
}UDPTxMesgIdentifier_e;

typedef enum
{
    INVLAID_COMMAND = 0,
	LIVE_STATUS_EXCHANGE_CMD = 1,
	SYSTEM_MODE_AND_STATE_CMD = 2,
	ESP_RESET_INDICATION_CMD = 3,
	NODE_BASIC_INFO_CMD = 4,
	NODE_LOCK_CONNECTOR_STATUS_CMD = 5,
	CONFIGURE_LOCK_PERIOD_CMD = 6,

	//Scanning Related Commands
	MQTT_PROD_CONFIGURATION_CMD = 20,
    OPERATE_OUTPUT_CMD = 21,
	START_SCANNING_FOR_OUTPUT_COMMAND = 22,
    SHARE_SCANNED_INPUT_VALUES_CMD = 23,
    SCANNED_INPUT_VALUES_TO_APP_CMD = 24,
    SCANNED_INPUT_VALUE_ACK_CMD = 25,
    STOP_SCANIING_PROCESS_CMD = 26,
    LEAK_TEST_RESULT_TO_APP_CMD = 27,
	OPERATE_OUTPUT_DONE_CMD = 28,
	SCAN_ERROR_STATUS_CMD = 29,

	//Clock Synchronization Process Commands
    MQTT_GET_CLOCK_VALUE = 50,
    MQTT_CLOCK_OFFSET_ADJUSTMENT = 51,

	TCP_GET_EXPECTED_PROCESS_START_TIME = 101,
	TCP_EXPECTED_PROCESS_START_TIME_RESP = 102,

	TCP_PROCESS_START_CMD = 103,

	TCP_GET_SCANNED_VALUE = 104,
	TCP_SCANNED_VALUE_RESP = 105,

    MAX_MQTT_MESG_CODE,
}UDPCommands_e;

/*Function Declarations*/

//Call this function only at once during initialization after power restart
void createWifiMonitoringTask( void );

void wifi_init_sta( void );
void createUDPServerTask( void );
void registerTxMessage( UDPTxMesgIdentifier_e _mesgIdent, uint32_t _sentAfter, uint8_t _p2pOrBroadcast, uint8_t UDPOrTCP );
void clearRegisteredMesg( UDPTxMesgIdentifier_e _mesgIdent );
bool checkIfAnySimilarMesgRegistered( UDPTxMesgIdentifier_e _mesgIdent );
void createTCPServerClientTask( void );

#endif /* MAIN_INCL_WIFIHANDLEING_H_ */
