/*
 * commonDeclaration.c
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */
#include "../incl/commonDeclaration.h"
#include "../incl/wifiHandleing.h"
#include <driver/gpio.h>

//Global Declarations
const char *fwVersion = "01.25.12.23"; //Last Edited On 25th December 2023

#if defined( APPLICATION_DEBUG_ENABLED )
const char *WIFI_DBG_TAG = "WiFi";
const char *TCP_DBG_TAG = "TCP";
const char *LEAK_DBG_TAG = "LEAK_TEST";
const char *LEAK_A_DBG_TAG = "LEAK_A";
const char *LEAK_B_DBG_TAG = "LEAK_B";
const char *CLK_SYNK_DBG_TAG = "CLOCK";
const char *CONTINUITY_DBG_TAG = "CONTI";
const char *COMMON_DEBUG_TAG = "COMMON";
const char *JSN_DBG_TAG = "JSN";

#endif //defined( APPLICATION_DEBUG_ENABLED )

const uint8_t AI1_PIN = 34;
const uint8_t AI2_PIN = 35;
const uint8_t RELAY1_PIN = 17;
const uint8_t RELAY2_PIN = 32;
const uint8_t LATCH_ENABLE_PINS[ TOTAL_LATCH_ENABLE_PINS ] = { 12, 16, 33 };
const uint8_t DATA_PINS[ TOTAL_DATA_PINS ] = { 4, 21, 14, 18, 19, 23, 27, 2 };
const uint8_t OUTPUT_ENABLE_PIN = 5;
const uint8_t BIPOLAR1_PIN = 13;
const uint8_t BIPOLAR2_PIN = 15;
const uint8_t BUFFER_ENABLE_PIN = 22;
const uint8_t GROUP_1_IC_ENABLE_PIN = 26;
const uint8_t GROUP_2_IC_ENABLE_PIN = 25;

/* Total Bits = 16
 * BS1 - BS8 ( LE1 )
 * BS9 - BS12( LE2 )
 * P1  - P4  ( LE2 ) */
uint16_t BS1ToBS12_P1ToP5_16BitData = 0;    /* To Set channel For Two Group of CD4051 */

/* Total Bits = 16
 * P5  - P12 ( LE3 ) */
uint16_t P5ToP12_16BitData = 0;             /* To Set channel For Third Group of CD4051 */

//Global Static Declaration
static nodeConfiguration_st nodeConfiguration;
static ledStatus_t ledStatus[ ALL_STATUS_LED ];
static uint8_t dataPinsDirection = 0xFF; //Any Random Value Other than GPIO_MODE_INPUT, GPIO_MODE_OUTPUT

static void setStatusLedState( statusLEDs_t _statusLED, enLEDOperation _operation );

//Function Definitions
long map( long x, long in_min, long in_max, long out_min, long out_max )
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void initNodeConfigParameters( void )
{
	memset( &nodeConfiguration, '\0', sizeof( nodeConfiguration_st ) );
}

uint8_t getNodeNumber( void )
{
	return nodeConfiguration.nodeNumber;
}

void setNodeNumber( uint8_t _number )
{
	nodeConfiguration.nodeNumber = _number;
}

uint8_t isNodeConfigured( void )
{
	return nodeConfiguration.isConfigured;
}

void setNodeConfiguredStatus( uint8_t _configured )
{
	nodeConfiguration.isConfigured = _configured;
}

uint16_t getRelayOnPeriod( void )
{
	return nodeConfiguration.relayOnPeriod;
}

void setRelayOnPeriod( uint16_t _period )
{
	nodeConfiguration.relayOnPeriod = _period;
}

uint16_t getConnectorLock1OnPeriod( void )
{
	return nodeConfiguration.connectorLock1Period;
}

void setConnectorLock1OnPeriod( uint16_t _period )
{
	nodeConfiguration.connectorLock1Period = _period;
}

uint16_t getConnectorLock2OnPeriod( void )
{
	return nodeConfiguration.connectorLock2Period;
}

void setConnectorLock2OnPeriod( uint16_t _period )
{
	nodeConfiguration.connectorLock2Period = _period;
}

void setDataPinsDirectionStatus( uint8_t _direction )
{
	if( _direction != GPIO_MODE_OUTPUT && _direction != GPIO_MODE_INPUT )
		dataPinsDirection = _direction;
}

uint8_t getDataPinsDirectionStatus( void )
{
	  return dataPinsDirection;
}

void getOutputGPIOConfig( uint8_t _PINNum )
{
	gpio_config_t io_conf;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = ( 1ULL << BIPOLAR2_PIN );
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
}

void initGPIO( void )
{
	//gpio_set_direction( BIPOLAR2_PIN, GPIO_MODE_INPUT );
	gpio_config_t input_conf;
	input_conf.mode = GPIO_MODE_INPUT;
	input_conf.pin_bit_mask = ( 1ULL << BIPOLAR2_PIN );
	input_conf.intr_type = GPIO_INTR_DISABLE;
	input_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	input_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config( &input_conf );

	gpio_config_t output_conf = {
		.pin_bit_mask = 0,  // Initialize to 0, will be updated in the loop below
		.mode = GPIO_MODE_OUTPUT,
		.intr_type = GPIO_INTR_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE,
	};

	output_conf.pin_bit_mask |= ( 1ULL << BIPOLAR1_PIN );

	output_conf.pin_bit_mask |= ( 1ULL << LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ] );
	output_conf.pin_bit_mask |= ( 1ULL << LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ] );
	output_conf.pin_bit_mask |= ( 1ULL << LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_3 ] );

	output_conf.pin_bit_mask |= ( 1ULL << BUFFER_ENABLE_PIN );

	for( uint8_t ite = 0; ite < TOTAL_DATA_PINS; ite++ )
		output_conf.pin_bit_mask |= ( 1ULL << DATA_PINS[ ite ] );

	output_conf.pin_bit_mask |= ( 1ULL << RELAY1_PIN );
	output_conf.pin_bit_mask |= ( 1ULL << RELAY2_PIN );
	output_conf.pin_bit_mask |= ( 1ULL << OUTPUT_ENABLE_PIN );
	output_conf.pin_bit_mask |= ( 1ULL << GROUP_1_IC_ENABLE_PIN );
	output_conf.pin_bit_mask |= ( 1ULL << GROUP_2_IC_ENABLE_PIN );

	gpio_config( &output_conf );

	/*
	gpio_set_direction( BIPOLAR1_PIN, GPIO_MODE_OUTPUT );
	gpio_set_direction( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_MODE_OUTPUT );
	gpio_set_direction( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_MODE_OUTPUT );
	gpio_set_direction( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_3 ], GPIO_MODE_OUTPUT );

	gpio_set_direction( BUFFER_ENABLE_PIN, GPIO_MODE_OUTPUT );

	for( uint8_t ite = 0; ite < TOTAL_DATA_PINS; ite++ )
		gpio_set_direction( DATA_PINS[ ite ], GPIO_MODE_OUTPUT );

	gpio_set_direction( RELAY1_PIN, GPIO_MODE_OUTPUT );
	gpio_set_direction( RELAY2_PIN, GPIO_MODE_OUTPUT );

	gpio_set_direction( OUTPUT_ENABLE_PIN, GPIO_MODE_OUTPUT );

	gpio_set_direction( GROUP_1_IC_ENABLE_PIN, GPIO_MODE_OUTPUT );
	gpio_set_direction( GROUP_2_IC_ENABLE_PIN, GPIO_MODE_OUTPUT );
	*/

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "GPIO Init Done" );
}

void setDefaultGPIOValues( void )
{
	gpio_set_level( OUTPUT_ENABLE_PIN, GPIO_PIN_HIGH );
	delayMicroseconds( 50 );
	gpio_set_level( BIPOLAR1_PIN, GPIO_PIN_HIGH );

	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_LOW );
	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_LOW );
	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_3 ], GPIO_PIN_LOW );

	gpio_set_level( BUFFER_ENABLE_PIN, GPIO_PIN_HIGH );

	for( uint8_t ite = 0; ite < TOTAL_DATA_PINS; ite++ )
		gpio_set_direction( DATA_PINS[ ite ], GPIO_PIN_LOW );

	gpio_set_level( RELAY1_PIN, GPIO_PIN_LOW );
	gpio_set_level( RELAY2_PIN, GPIO_PIN_LOW );

	gpio_set_level( GROUP_1_IC_ENABLE_PIN, GPIO_PIN_HIGH );
	gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_HIGH );

	delayMicroseconds( 50 );
	gpio_set_level( OUTPUT_ENABLE_PIN, GPIO_PIN_LOW );

	delayMicroseconds( 50 );
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Default GPIO Value Set" );
}

void resetInputOutput( void )
{
	for( uint8_t ite = 0; ite < 12; ite++ )
		BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, ite,  0 );

	gpio_set_level( OUTPUT_ENABLE_PIN, GPIO_PIN_LOW );

	for( uint8_t ite = 0; ite < 8; ite++ )
		gpio_set_level( DATA_PINS[ ite ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite ) );

	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_HIGH );
	delayMicroseconds( 15 );
	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_1 ], GPIO_PIN_LOW );
	delayMicroseconds( 15 );

	for( uint8_t ite_2 = 8; ite_2 < 16; ite_2++ )
		gpio_set_level( DATA_PINS[ ite_2 - 8 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite_2 ) );

	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_HIGH );
	delayMicroseconds( 15 );
	gpio_set_level( LATCH_ENABLE_PINS[ LATCH_ENABLE_PIN_2 ], GPIO_PIN_LOW );
	delayMicroseconds( 15 );

	gpio_set_level( GROUP_1_IC_ENABLE_PIN, GPIO_PIN_HIGH );
	gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_HIGH );

	gpio_set_level( OUTPUT_ENABLE_PIN, GPIO_PIN_LOW );
	gpio_set_level( BIPOLAR1_PIN, GPIO_PIN_HIGH );

	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Reset Input Output Done" );
}

void setDataPinsDirection( uint8_t _direction )
{
	//Return from here if data pins direction is already set to expected direction
	if( dataPinsDirection == _direction )
		return;

    if( GPIO_MODE_OUTPUT == _direction )
    {
    	LEAK_DBG_LOG( COMMON_DEBUG_TAG, "GPIO MODE : OUTPUT" );
    	dataPinsDirection = GPIO_MODE_OUTPUT;

        for( uint8_t iterator = 0; iterator < TOTAL_DATA_PINS; iterator++ )
        	gpio_set_direction( DATA_PINS[ iterator ], GPIO_MODE_OUTPUT );

        delayMicroseconds( 15 );
    }
    else if( GPIO_MODE_INPUT == _direction )
    {
    	LEAK_DBG_LOG( COMMON_DEBUG_TAG, "GPIO MODE : INPUT" );
    	dataPinsDirection = GPIO_MODE_INPUT;

        for( uint8_t iterator = 0; iterator < TOTAL_DATA_PINS; iterator++ )
        	gpio_set_direction( DATA_PINS[ iterator ], GPIO_MODE_INPUT );

        gpio_set_level( BUFFER_ENABLE_PIN, GPIO_PIN_LOW );
        delayMicroseconds( 15 );
    }
}

void disableGroup2IC( void )
{
	gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_HIGH );
    delayMicroseconds( 500 );
}

void enableGroup2IC( void )
{
	gpio_set_level( GROUP_2_IC_ENABLE_PIN, GPIO_PIN_LOW );
    delayMicroseconds( 500 );
}

void operateStatusLeds( statusLEDs_t _statusLED, enLEDOperation _status )
{
	uint8_t updateRequired = 0;

	if( _status != STATUS_LED_ON && _status != STATUS_LED_OFF )
	    return;

	/* Set GPIO PIN direction as output */
	setDataPinsDirection( GPIO_MODE_OUTPUT );

    if( _statusLED == ALL_STATUS_LED )
    {
    	setStatusLedState( ALL_STATUS_LED, STATUS_LED_OFF );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, WIFI_CONNECTION_STATUS_LED_P0, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, CONTINUITY_INOUT_STATUS_LED_P1, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, LEAK_TEST_RESULT_STATUS_LED_P2, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, CONTI_TEST_RESULT_STATUS_LED_P3, _status );

        updateRequired = 1;
    }
    else if( _statusLED == WIFI_CONNECTION_STATUS_LED_P0 )
    {
    	setStatusLedState( WIFI_CONNECTION_STATUS_LED_P0, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, WIFI_CONNECTION_STATUS_LED_P0, _status );
        updateRequired = 1;
    }
    else if( _statusLED == CONTINUITY_INOUT_STATUS_LED_P1 )
    {
    	setStatusLedState( CONTINUITY_INOUT_STATUS_LED_P1, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, CONTINUITY_INOUT_STATUS_LED_P1, _status );
        updateRequired = 1;
    }
    else if( _statusLED == LEAK_TEST_RESULT_STATUS_LED_P2 )
    {
    	setStatusLedState( LEAK_TEST_RESULT_STATUS_LED_P2, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, LEAK_TEST_RESULT_STATUS_LED_P2, _status );
        updateRequired = 1;
    }
    else if( _statusLED == CONTI_TEST_RESULT_STATUS_LED_P3 )
    {
    	setStatusLedState( CONTI_TEST_RESULT_STATUS_LED_P3, _status );
    	BIT_WRITE( BS1ToBS12_P1ToP5_16BitData, CONTI_TEST_RESULT_STATUS_LED_P3, _status );
        updateRequired = 1;
    }

    if( updateRequired )
    {
        //Serial.printf( "%s : ", "LED Bits" );
        for( uint8_t ite = 8; ite < 16; ite++ )
        {
        	gpio_set_level( DATA_PINS[ ite - 8 ], BIT_CHECK( BS1ToBS12_P1ToP5_16BitData, ite ) );
            //Serial.printf( "%d, ", bitRead( BS1ToBS12_P1ToP5_16BitData, ite ) );
        }

        gpio_set_level( LATCH_ENABLE_PINS[ 1 ], GPIO_PIN_HIGH );
        delayMicroseconds( 15 );

        gpio_set_level( LATCH_ENABLE_PINS[ 1 ], GPIO_PIN_LOW );
        delayMicroseconds( 15 );
    }
}

static void setStatusLedState( statusLEDs_t _statusLED, enLEDOperation _operation )
{
	if( ( _statusLED < WIFI_CONNECTION_STATUS_LED_P0 || _statusLED > ALL_STATUS_LED ) ||
	    ( _operation < STATUS_LED_OFF || _operation >= MAX_LED_PATTERN ) )
		return;

	if( _statusLED != ALL_STATUS_LED )
		ledStatus[ _statusLED ].ledStatus = _operation;
	else
	{
		ledStatus[ WIFI_CONNECTION_STATUS_LED_P0 ].ledStatus = _operation;
		ledStatus[ CONTINUITY_INOUT_STATUS_LED_P1 ].ledStatus = _operation;
		ledStatus[ LEAK_TEST_RESULT_STATUS_LED_P2 ].ledStatus = _operation;
		ledStatus[ CONTI_TEST_RESULT_STATUS_LED_P3 ].ledStatus = _operation;
	}
}

void delayMicroseconds( uint32_t _delayInuS )
{
	long long int usTimeSincePowerOn = esp_timer_get_time();

	while( 1 )
	{
		if( (esp_timer_get_time() - usTimeSincePowerOn ) >= _delayInuS )
			break;
	}
}

/*
void delayMicroseconds(uint32_t _delayInuS)
{
    vTaskDelay(pdMS_TO_TICKS(_delayInuS / 1000));
}
*/

#if defined( SYNC_NATIVE_CLOCK )
uint64_t clockOffset = 0;
bool clockOffsettingDone = false;
bool clockValueLocked = false;
uint64_t lockedClockValue = 0;
void prepareNativeClockValueResponseMessage( char *_mesg )
{
	uint64_t nativeClock = CURRENT_CLOCK_INSTANT_WITH_OFFSET;
	cJSON *rootJSNObj = NULL;

	ESP_LOGI( CLK_SYNK_DBG_TAG, "Preparing Message Native Clock Sync Response" );
	rootJSNObj = cJSON_CreateObject();
	cJSON_AddNumberToObject( rootJSNObj, JSN_MESSAGE_CODE_STR, MQTT_GET_CLOCK_VALUE );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NODE_NUMBER_STR, NODE_NUMBER );

	cJSON_AddNumberToObject( rootJSNObj, JSN_NATIVE_CLOCK_LSB32_STR, ( nativeClock & 0xFFFFFFFF ) );
	cJSON_AddNumberToObject( rootJSNObj, JSN_NATIVE_CLOCK_MSB32_STR, ( ( nativeClock >> 32 ) & 0xFFFFFFFF ) );

	memcpy( _mesg, cJSON_Print( rootJSNObj ), strlen( cJSON_Print( rootJSNObj ) ) );
	ESP_LOGI( CLK_SYNK_DBG_TAG, "P2P Mesg:%d,%s",strlen( cJSON_Print( rootJSNObj ) ), _mesg );
	cJSON_Delete( rootJSNObj );
	clearRegisteredMesg( NATIVE_CLOCK_VALUE_MESG );
}

void setClockOffset( uint32_t _offsetValue )
{
    clockOffset = _offsetValue;
    clockOffsettingDone = true;
}

uint64_t getClockValueAfterOffset( void )
{
    return ( ( esp_timer_get_time()/1000 ) + clockOffset );
}

bool hasClockOffsettingDone( void )
{
	return clockOffsettingDone;
}

bool isOffsettedClockValueLocked( uint64_t *_value )
{
	if( clockValueLocked == true )
	{
		*_value = lockedClockValue;
		return true;
	}
	return false;
}

void resetOffsettedClockValue( void )
{
	clockValueLocked = false;
	lockedClockValue = 0;
}

void lockCurrentOffsettedClockValue( void )
{
	lockedClockValue = getClockValueAfterOffset();
	clockValueLocked = true;
}
#endif




