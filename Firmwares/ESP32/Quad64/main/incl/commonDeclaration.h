/*
 * commonDeclaration.h
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#ifndef MAIN_INCL_COMMONDECLARATION_H_
#define MAIN_INCL_COMMONDECLARATION_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "rtc_wdt.h"

#define FIRMWARE_VERSION_STR					"01.01.01.01"

//Debug Enable/Disable MACRO
#define APPLICATION_DEBUG_ENABLED

#if defined( APPLICATION_DEBUG_ENABLED )
//#define LEAK_TYPE_A_DEBUG_ENABLED
//#define LEAK_TYPE_B_DEBUG_ENABLED
#define CONTINUITY_DEBUG_ENABLED
#define COMMON_DEBUG_ENABLED
//#define UNIT_TESTING_ENABLED
#define WIFI_DEBUG_ENABLED
#define JSN_PARSING_DEBUG_ENABLED

#if defined( WIFI_DEBUG_ENABLED )
	//#define NON_TCP_BLOCKING_DEBUG_ENABLED
#endif //defined( WIFI_DEBUG_ENABLED )

#endif //define APPLICATION_DEBUG_ENABLED

#if defined( LEAK_TYPE_A_DEBUG_ENABLED ) || \
	defined( LEAK_TYPE_B_DEBUG_ENABLED )
	#define LEAK_DBG_LOG		ESP_LOGI
	#define LEAK_DBG_ERR		ESP_LOGE
#else
	#define LEAK_DBG_LOG( ... )
	#define LEAK_DBG_ERR( ... )
#endif

#if defined( CONTINUITY_DEBUG_ENABLED )
	#define CONTINUITY_DBG_LOG		ESP_LOGI
	#define CONTINUITY_DBG_ERR		ESP_LOGE
#else
	#define CONTINUITY_DBG_LOG( ... )
	#define CONTINUITY_DBG_ERR( ... )
#endif

#if defined( COMMON_DEBUG_ENABLED )
	#define COMMON_DEBUG_LOG		ESP_LOGI
	#define COMMON_DEBUG_ERROR		ESP_LOGE
#else
	#define COMMON_DEBUG_LOG( ... )
	#define COMMON_DEBUG_ERROR( ... )
#endif

#if defined( COMMON_DEBUG_ENABLED )
	#define COMMON_DEBUG_LOG		ESP_LOGI
	#define COMMON_DEBUG_ERROR		ESP_LOGE
#else
	#define COMMON_DEBUG_LOG( ... )
	#define COMMON_DEBUG_ERROR( ... )
#endif

#if defined( WIFI_DEBUG_ENABLED )
	#define WIFI_DBG_LOG		ESP_LOGI
	#define WIFI_DBG_ERR		ESP_LOGE

	#if defined( NON_TCP_BLOCKING_DEBUG_ENABLED )
		#define TCP_DBG_LOG				ESP_LOGI
		#define TCP_DBG_ERR				ESP_LOGE
	#endif //defined( NON_TCP_BLOCKING_DEBUG_ENABLED )
#else
	#define WIFI_DBG_LOG( ... )
	#define WIFI_DBG_ERR( ... )
	#if !defined( NON_TCP_BLOCKING_DEBUG_ENABLED )
		#define TCP_DBG_LOG( ... )
		#define TCP_DBG_ERR( ... )
	#endif
#endif // defined( WIFI_DEBUG_ENABLED )

#if defined( JSN_PARSING_DEBUG_ENABLED )
	#define JSN_DBG_LOG		ESP_LOGI
	#define JSN_DBG_ERR		ESP_LOGE
#else
	#define JSN_DBG_LOG( ... )
	#define JSN_DBG_ERR( ... )
#endif

#if defined( APPLICATION_DEBUG_ENABLED )
extern const char *LEAK_DBG_TAG;
extern const char *TCP_DBG_TAG;
extern const char *LEAK_A_DBG_TAG;
extern const char *LEAK_B_DBG_TAG;
extern const char *CLK_SYNK_DBG_TAG;
extern const char *WIFI_DBG_TAG;
extern const char *CONTINUITY_DBG_TAG;
extern const char *JSN_DBG_TAG;
#endif //defined( APPLICATION_DEBUG_ENABLED )

//Feature Or Process Enable Disable
#define DIGITAL_AVERAGING
//#define CHANNEL_DELAY
#define SYNC_NATIVE_CLOCK						( 1 )
#define UNIT_TESTING							( 1 )
#define NON_BLOCKING_TCP_CLIENT					( 1 )

//Delete when not in use
#define NODE_NUMBER								( 15 )
#define LOCK_CONN_STATE_1						( 1 )
#define LOCK_CONN_STATE_2						( 1 )
#define LOCK_CONN_STATE_3						( 1 )

#define BIT_WRITE( _data, nbit, status )		( ( status == 1 ) ? BIT_SET( _data, nbit ) : BIT_CLEAR( _data, nbit ) )
#define BIT_SET( _data, nbit )    				( ( _data ) |=  ( 1 << ( nbit ) ) )
#define BIT_CLEAR( _data, nbit )  				( ( _data ) &= ~( 1 << ( nbit ) ) )
#define BIT_TOGGLE( _data, nbit ) 				( ( _data ) ^=  ( 1 << ( nbit ) ) )
#define BIT_CHECK( _data, nbit )  				( ( ( _data ) &   ( 1 << ( nbit ) ) ) ? 1 : 0 )

//Default Values
#define NODE_NOT_CONFIGURED_OR_INITIALIZED		( 0x55 )
#define NODE_CONFIGURED_OR_INITIALIZED			( 0xAA )

#define DEFAULT_CONNECTOR_LOCK_PERIOD			( 2000 )
#define DEFAULT_RELAY_ON_PERIOD					( 2000 )

#define DEFAULT_NODE_NUMBER						( 0 )

#define GPIO_PIN_HIGH							( 1 )
#define GPIO_PIN_LOW							( 0 )

#define TOTAL_DATA_PINS                   		( 8 )
#define TOTAL_OP_UNDER_1_4051             		( 8 )

#define MAX_SUPPORTED_NODES						( 80 )

#define MAX_32_BIT_VALUE						( 4294967295 )
#define NULL_VALUE								( 0 )

#define DIGITAL_INPUT_TYPE        				( 0 )

#define CURRENT_CLOCK_INSTANT_WITH_OFFSET		( getClockValueAfterOffset( ) )

typedef enum
{
	NO_ERROR = 0,

	//If expected time is expired where scanning must start. This is applicable to both Node doing both Operating Output and
	//Scanning or just scanning.@TODO:Will decide what to do when entire basic operation is tested
	TIME_FOR_SCANNING_ALREADY_EXPIRED,

	//When No Enough room to operate output, In such case Node doing Input Output Operation will stop operating outputs
	//will do broadcast to let other nodes to stop process.
	NOT_ENOUGH_TIME_TO_OPERATE_OUTPUT,

	CONTINUITY_ERROR_WITH_ERROR_DESCRIPTION,
}errorList_e;

typedef enum
{
    LATCH_ENABLE_PIN_1 = 0,
    LATCH_ENABLE_PIN_2,
    LATCH_ENABLE_PIN_3,
    TOTAL_LATCH_ENABLE_PINS
}enLatchEnablePins;

typedef enum
{
	WIFI_CONNECTION_STATUS_LED_P0 = 12,
	CONTINUITY_INOUT_STATUS_LED_P1 = 13,
	LEAK_TEST_RESULT_STATUS_LED_P2 = 14,
	CONTI_TEST_RESULT_STATUS_LED_P3 = 15,
	ALL_STATUS_LED = 255,
}statusLEDs_t;

typedef enum
{
    STATUS_LED_OFF = 0,
    STATUS_LED_ON,
    STATUS_LED_BLINKING,
	MAX_LED_PATTERN,
}enLEDOperation;

typedef struct
{
	uint8_t 	ledStatus; //Mapped to enTestStatusLed
	uint8_t 	blinkState;	//1= ON, else 0 = OFF
	uint16_t 	delayForBlink;
}ledStatus_t;

typedef struct
{
	uint8_t isConfigured;
	uint8_t nodeNumber;
	uint16_t relayOnPeriod;
	uint16_t connectorLock1Period;
	uint16_t connectorLock2Period;
}nodeConfiguration_st;

extern const uint8_t AI1_PIN;
extern const uint8_t AI2_PIN;
extern const uint8_t RELAY1_PIN;
extern const uint8_t RELAY2_PIN;
extern const uint8_t LATCH_ENABLE_PINS[TOTAL_LATCH_ENABLE_PINS];
extern const uint8_t DATA_PINS[ TOTAL_DATA_PINS ];
extern const uint8_t OUTPUT_ENABLE_PIN;
extern const uint8_t BIPOLAR1_PIN;
extern const uint8_t BIPOLAR2_PIN;
extern const uint8_t BUFFER_ENABLE_PIN;
extern const uint8_t GROUP_1_IC_ENABLE_PIN;
extern const uint8_t GROUP_2_IC_ENABLE_PIN;

/* Total Bits = 16
 * BS1 - BS8 ( LE1 )
 * BS9 - BS12( LE2 )
 * P1  - P4  ( LE2 ) */
extern uint16_t BS1ToBS12_P1ToP5_16BitData;    /* To Set channel For Two Group of CD4051 */

/* Totla Bits = 16
 * P5  - P12 ( LE3 ) */
extern uint16_t P5ToP12_16BitData;             /* To Set channel For Third Group of CD4051 */

//Function Declaration
long map( long x, long in_min, long in_max, long out_min, long out_max );

void initNodeConfigParameters( void );

void operateStatusLeds( statusLEDs_t _statusLED, enLEDOperation _status );

void delayMicroseconds( uint32_t _delayInuS );

void setDataPinsDirection( uint8_t _direction );

uint8_t getDataPinsDirectionStatus( void );
void setDataPinsDirectionStatus( uint8_t _direction );

void disableGroup2IC( void );
void enableGroup2IC( void );

uint8_t isNodeConfigured( void );
void setNodeConfiguredStatus( uint8_t _configured );

uint8_t getNodeNumber( void );
void setNodeNumber( uint8_t _number );

uint16_t getRelayOnPeriod( void );
void setRelayOnPeriod( uint16_t _period );

uint16_t getConnectorLock1OnPeriod( void );
void setConnectorLock1OnPeriod( uint16_t _period );

uint16_t getConnectorLock2OnPeriod( void );
void setConnectorLock2OnPeriod( uint16_t _period );

uint64_t getClockValueAfterOffset( void );
void setClockOffset( uint32_t _offsetValue );

bool hasClockOffsettingDone( void );
bool isOffsettedClockValueLocked( uint64_t *_value );
void resetOffsettedClockValue( void );
void lockCurrentOffsettedClockValue( void );

void initGPIO( void );
void setDefaultGPIOValues( void );
void resetInputOutput( void );

#endif /* MAIN_INCL_COMMONDECLARATION_H_ */
