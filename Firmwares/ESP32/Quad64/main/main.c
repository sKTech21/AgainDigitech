#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "incl/nvs_nodeConfiguration.h"
#include "incl/commonDeclaration.h"
#include "incl/wifiHandleing.h"
#include "incl/unitTesting.h"
#include "incl/continuity.h"
#include "incl/leakTestHandeling.h"

void app_main(void)
{
	rtc_wdt_disable();

	initGPIO( );
	delayMicroseconds( 500 );
	setDefaultGPIOValues( );
	delayMicroseconds( 500 );
	resetInputOutput( );
	delayMicroseconds( 500 );

	initNodeConfigParameters( );
	initNVSFlashMemory( );

	wifi_init_sta( );
	createWifiMonitoringTask( );

	continuityTestTaskInit( );
	leakTestTaskInit( );

	setNodeNumber( NODE_NUMBER );
#if defined( UNIT_TESTING_ENABLED )
	setNodeNumber( 1 );
	//prepareTestMessage( );
#endif
	CONTINUITY_DBG_LOG( CONTINUITY_DBG_TAG, "Node Number: %d", getNodeNumber() );
	while( 1 )
	{
		vTaskDelay( 10 / portTICK_PERIOD_MS );
	}
}

