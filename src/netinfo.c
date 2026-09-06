/*
OpenWrt RGB LED control daemon
Copyright (C) 2026  Jão do Santo Cristo

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "ubushit.h"
#include "netinfo.h"
#include "misc.h"

static funcResult_e local_read_rx_or_tx_counter( char* path, uint64_t* result )
{
  int fileDescriptor = open( path, O_RDONLY );
  if ( fileDescriptor == -1 )
    return R_ERROR;
  
  char buffer[64];

  ssize_t fRead = read( fileDescriptor, buffer, sizeof( buffer ) - 1 );
  close( fileDescriptor );
  if ( fRead <= 0 )
      return R_ERROR;

  buffer[fRead] = '\0';

  char* endptr;
  // [jdsc] i will say, this is the strangest way of tracking error codes...
  errno = 0;
  
  uint64_t num = ( uint64_t )strtoull( buffer, &endptr, 10 );

  if ( errno != 0 || endptr == buffer )
    return R_ERROR;

  * result = num;
  return R_A_OK;
}

static const uint64_t SYSFS_BUFFER_SIZE = UNIX_MAX_DEV_NAME_SIZE + sizeof( "/sys/class/net//statistics/rx_bytes" ) + 1;

static funcResult_e local_init_interface( char* rxPath, char* txPath ) 
{
  char devBuffer[UNIX_MAX_DEV_NAME_SIZE] = {0};

  // TODO: ADD FUNCTION TO MANUALLY SPECIFY INTERFACE
  if ( find_wan_device( devBuffer, sizeof( devBuffer ) ) != R_A_OK ) {
    printf( "Fatal error looking for WAN device name!\n" );
    return R_ERROR;
  }

  snprintf( rxPath, SYSFS_BUFFER_SIZE, "/sys/class/net/%s/statistics/rx_bytes", devBuffer );
  snprintf( txPath, SYSFS_BUFFER_SIZE, "/sys/class/net/%s/statistics/tx_bytes", devBuffer );
  return R_A_OK;
}

void* net_thread( void* arg )
{
  routerContext_s* rCtx = ( routerContext_s* )arg;

  char sysfsRxPath[SYSFS_BUFFER_SIZE];
  char sysfsTxPath[SYSFS_BUFFER_SIZE];

  for ( int retryCount = 0; retryCount <= MAX_INTERFACE_RETRIES; ++retryCount ) {
    if ( local_init_interface( sysfsRxPath, sysfsTxPath ) == R_A_OK )
      break;

    if ( retryCount >= MAX_INTERFACE_RETRIES ) {
      printf( "Reached retry limit, exiting.");
      pthread_exit( NULL );
    }
  
    printf( "Couldn't open WAN interface info, trying to reinitialize...\n" );
    sleep( ERROR_RETRY_TIMER );
    continue;
  }
  
  uint64_t rxBytes1 = 0;
  uint64_t txBytes1 = 0;
  
  uint64_t rxBytes2 = 0;
  uint64_t txBytes2 = 0;

  uint64_t byteTrafficDelta = 0;

  while ( 1 ) {
    if ( local_read_rx_or_tx_counter( sysfsRxPath, &rxBytes2 ) != R_A_OK ) {
      printf( "Error reading interface Rx sysfs counter\n" );
      pthread_exit( NULL );
    }

    if ( local_read_rx_or_tx_counter( sysfsTxPath, &txBytes2 ) != R_A_OK ) {
      printf( "Error reading interface Tx sysfs counter\n" );
      pthread_exit( NULL );
    }

    // [jdsc] using this it's possible to get an overall idea of how much data
    // the router is transferring. rxBytes uses the Linux kernel tracker
    // for tx/rx on a network interface, they're cumulative so it will not
    // cause a negative underflow, and if the counter overflows it wraps safely
    byteTrafficDelta = ( rxBytes2 - rxBytes1 ) + ( txBytes2 - txBytes1 );
    atomic_store( &rCtx->netState->bpsTrafficSpeed, byteTrafficDelta );

    rxBytes1 = rxBytes2;
    txBytes1 = txBytes2;

    sleep( NETWORK_THREAD_TIMER );
  }

  return NULL;
}
