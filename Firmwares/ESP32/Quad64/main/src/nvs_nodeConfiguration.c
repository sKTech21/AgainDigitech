/*
 * nvs_nodeConfiguration.c
 *
 *  Created on: May 21, 2023
 *      Author: pbhum
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>

#include "nvs_flash.h"
#include "nvs.h"

#include "../incl/nvs_nodeConfiguration.h"
#include "../incl/commonDeclaration.h"

const char* NVS_CONFIGURATION_STORAGE		= "confStorage";
const char* NVS_NODE_INITIALIZED_STATUS_KEY = "nIni";
const char* NVS_NODE_NUMBER_KEY				= "nNum";
const char* NVS_WIFI_SSID_KEY 				= "ssid";
const char* NVS_WIFI_PASS_KEY				= "pass";
const char* NVS_APPLICATION_UDP_IP_KEY		= "appIP";
const char* NVS_LOCK1_PERIOD_KEY			= "l1Per";
const char* NVS_LOCK2_PERIOD_KEY			= "l2Per";
const char* NVS_RELAY_ON_PERIOD_KEY			= "rOnP";

//Private Function Declarations
bool createNVSFlashMemoryObject( nvs_handle_t *_handle );
bool eraseNVSNodeConfiguration( nvs_handle_t *nvs_handle );

//Function Declaration
bool initNVSFlashMemory( )
{
	esp_err_t err;
	nvs_handle_t nvs_handle;

	if( !createNVSFlashMemoryObject( &nvs_handle ) )
	{
		nvs_close( nvs_handle );
		return false;
	}

	//Read Node Initialization Status
	uint8_t isNodeInitializedOrConfigured = NODE_NOT_CONFIGURED_OR_INITIALIZED;
	err = nvs_get_u8( nvs_handle, NVS_NODE_INITIALIZED_STATUS_KEY, &isNodeInitializedOrConfigured );
	if( err != ESP_OK ) { printf( "Failed-9\n" ); return false; }

	if( isNodeInitializedOrConfigured != NODE_NOT_CONFIGURED_OR_INITIALIZED &&
		isNodeInitializedOrConfigured != NODE_CONFIGURED_OR_INITIALIZED )
	{
		if( eraseNVSNodeConfiguration( &nvs_handle ) != true )
		{
			nvs_close( nvs_handle );
			return false;
		}
		else
		{
			printf("Committing Default values in NVS ... ");
			err = nvs_commit( nvs_handle );
			if( err != ESP_OK ) { printf( "Failed-10\n" ); }

			nvs_close( nvs_handle );

			printf( "Restarting now. after resetting NVS Flash Memory\n" );
			fflush(stdout);
			esp_restart();
		}
	}
	setNodeConfiguredStatus( isNodeInitializedOrConfigured );

	//No need to fetch the other details from NVS if node is not initialized
	if( isNodeInitializedOrConfigured == NODE_NOT_CONFIGURED_OR_INITIALIZED )
		return true;

	//Read Node Number
	uint8_t nodeNumber = 0;
	err = nvs_get_u8( nvs_handle, NVS_NODE_NUMBER_KEY, &nodeNumber );
	if( err != ESP_OK ) { printf( "Failed-11\n" ); return false; }
	setNodeNumber( nodeNumber );

	//Read Relay On Period
	uint16_t relayOnPeriod = 0;
	err = nvs_get_u16( nvs_handle, NVS_RELAY_ON_PERIOD_KEY, &relayOnPeriod );
	if( err != ESP_OK ) { printf( "Failed-12\n" ); return false; }
	setRelayOnPeriod( relayOnPeriod );

	//Connector Lock 1 On Period
	uint16_t ConnectorLock1OnPeriod = 0;
	err = nvs_get_u16( nvs_handle, NVS_LOCK1_PERIOD_KEY, &ConnectorLock1OnPeriod );
	if( err != ESP_OK ) { printf( "Failed-13\n" ); return false; }
	setConnectorLock1OnPeriod( ConnectorLock1OnPeriod );

	//Connector Lock 2 On Period
	uint16_t ConnectorLock2OnPeriod = 0;
	err = nvs_get_u16( nvs_handle, NVS_LOCK2_PERIOD_KEY, &ConnectorLock2OnPeriod );
	if( err != ESP_OK ) { printf( "Failed-14\n" ); return false; }
	setConnectorLock2OnPeriod( ConnectorLock2OnPeriod );

	nvs_close( nvs_handle );

	return true;
}

bool createNVSFlashMemoryObject( nvs_handle_t *_handle )
{
	esp_err_t err = nvs_flash_init();
	if( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
	{
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK( nvs_flash_erase() );
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK( err );

	// Open NVS Configuration Storage
	printf( "\n" );
	printf( "Opening Non-Volatile Storage (NVS) handle... " );

	//nvs_handle_t my_handle;
	err = nvs_open( NVS_CONFIGURATION_STORAGE, NVS_READWRITE, _handle );
	if( err != ESP_OK )
	{
		printf( "Error (%s) opening NVS handle!\n", esp_err_to_name( err ) );
		return false;
	}
	return true;
}

bool eraseNVSNodeConfiguration( nvs_handle_t *nvs_handle )
{
	esp_err_t err;

	// Setting Default Values
	printf("Initializing NVS Storage with Default values....");

	err = nvs_set_u8( *nvs_handle, NVS_NODE_INITIALIZED_STATUS_KEY, NODE_NOT_CONFIGURED_OR_INITIALIZED );
	if( err != ESP_OK ) { printf( "Failed-1\n" ); return false; }

	err = nvs_set_u8( *nvs_handle, NVS_NODE_NUMBER_KEY , DEFAULT_NODE_NUMBER );
	if( err != ESP_OK ) { printf( "Failed-2\n" ); return false; }

	err = nvs_set_str( *nvs_handle, NVS_WIFI_SSID_KEY , "" );
	if( err != ESP_OK ) { printf( "Failed-3\n" ); return false; }

	err = nvs_set_str( *nvs_handle, NVS_WIFI_PASS_KEY , "" );
	if( err != ESP_OK ) { printf( "Failed-4\n" ); return false; }

	err = nvs_set_str( *nvs_handle, NVS_APPLICATION_UDP_IP_KEY , "" );
	if( err != ESP_OK ) { printf( "Failed-5\n" ); return false; }

	err = nvs_set_u16( *nvs_handle, NVS_LOCK1_PERIOD_KEY , DEFAULT_CONNECTOR_LOCK_PERIOD );
	if( err != ESP_OK ) { printf( "Failed-6\n" ); return false; }

	err = nvs_set_u16( *nvs_handle, NVS_LOCK2_PERIOD_KEY , DEFAULT_CONNECTOR_LOCK_PERIOD );
	if( err != ESP_OK ) { printf( "Failed-7\n" ); return false; }

	err = nvs_set_u16( *nvs_handle, NVS_RELAY_ON_PERIOD_KEY , DEFAULT_RELAY_ON_PERIOD );
	if( err != ESP_OK ) { printf( "Failed-8\n" ); return false; }

	return true;
}

bool NVS_setNodeConfiguration( uint8_t isConfigured, uint8_t _nodeNumber, uint16_t _relayOnPeriod,
		                       uint16_t connectorLock1Period, uint16_t connectorLock2Period )
{
	esp_err_t err;
	nvs_handle_t nvs_handle;

	if( !createNVSFlashMemoryObject( &nvs_handle ) )
	{
		nvs_close( nvs_handle );
		return false;
	}

	err = nvs_set_i32( nvs_handle, NVS_NODE_INITIALIZED_STATUS_KEY, NODE_CONFIGURED_OR_INITIALIZED );
	if( err != ESP_OK ) { printf( "Failed-15\n" ); return false; }

	err = nvs_set_i32( nvs_handle, NVS_NODE_NUMBER_KEY, _nodeNumber );
	if( err != ESP_OK ) { printf( "Failed-16\n" ); return false; }

	err = nvs_set_i32( nvs_handle, NVS_RELAY_ON_PERIOD_KEY, _relayOnPeriod );
	if( err != ESP_OK ) { printf( "Failed-17\n" ); return false; }

	err = nvs_set_i32( nvs_handle, NVS_LOCK1_PERIOD_KEY, connectorLock1Period );
	if( err != ESP_OK ) { printf( "Failed-18\n" ); return false; }

	err = nvs_set_i32( nvs_handle, NVS_LOCK2_PERIOD_KEY, connectorLock2Period );
	if( err != ESP_OK ) { printf( "Failed-19\n" ); return false; }

	err = nvs_commit( nvs_handle );
	if( err != ESP_OK ) { printf( "Failed-20\n" ); }

	nvs_close( nvs_handle );
	return true;
}

