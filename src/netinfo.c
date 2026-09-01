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

int read_rx_or_tx_counter( char* path, uint64_t* result )
{
  int fileDescriptor = open( path, O_RDONLY );
  if( fileDescriptor == -1 )
    return -1;
  
  char buffer[64];

  ssize_t fRead = read( fileDescriptor, buffer, sizeof( buffer ) - 1 );
  if( fRead <= 0 ) {
    close( fileDescriptor );
    return -1;
  }

  close( fileDescriptor );

  buffer[fRead] = '\0';

  char* endptr;
  // [jdsc] i will say, this is the strangest way of tracking error codes...
  errno = 0;
  
  uint64_t num = ( uint64_t )strtoull( buffer, &endptr, 10 );

  if( errno != 0 || endptr == buffer )
    return -1;

  * result = num;
  return 0;
}

const uint16_t SYSFS_BUFFER_SIZE = 64 + sizeof( "/sys/class/net//statistics/rx_bytes" ) + 1;

int init_wan_dev( char* rxPath, char* txPath ) 
{
  char devBuffer[64] = {0};
  if( find_wan_device( devBuffer ) != 0 ) {
    printf("Fatal error looking for WAN dev name!\n");
    return -1;
  }

  snprintf( rxPath, SYSFS_BUFFER_SIZE, "/sys/class/net/%s/statistics/rx_bytes", devBuffer );
  snprintf( txPath, SYSFS_BUFFER_SIZE, "/sys/class/net/%s/statistics/tx_bytes", devBuffer );
  return 0;
}

void* net_thread( void* arg )
{
  struct Data* data = ( struct Data* )arg;

  char sysfsRxPath[SYSFS_BUFFER_SIZE];
  char sysfsTxPath[SYSFS_BUFFER_SIZE];

  uint8_t retryCount = 0;
  while( init_wan_dev( sysfsRxPath, sysfsTxPath ) != 0 ) {
    if( retryCount > 5 )
      return 0;
      
    printf( "Couldn't open WAN device info, trying to reinitialize...\n");
    sleep( 3 );

    ++retryCount;
  }
  retryCount = 0;
  
  uint64_t rxPackets1 = 0;
  uint64_t txPackets1 = 0;
  
  uint64_t rxPackets2 = 0;
  uint64_t txPackets2 = 0;

  uint64_t packetDelta = 0;

  while( 1 ) {
    if( read_rx_or_tx_counter( sysfsRxPath, &rxPackets2 ) != 0 )
    {
      printf( "Error reading WAN Rx sysfs counter\n" );
      pthread_exit( NULL );
    }

    if( read_rx_or_tx_counter( sysfsTxPath, &txPackets2 ) != 0 )
    {
      printf( "Error reading WAN Tx sysfs counter\n" );
      pthread_exit( NULL );
    }

    // [jdsc] using this it's possible to get an overall idea of how much data
    // the router is transferring. rxPackets uses the Linux kernel tracker
    // for tx/rx on a network interface, they're cumulative so it will not
    // cause a negative underflow, and if the counter overflows it wraps safely
    packetDelta = ( rxPackets2 - rxPackets1 ) + ( txPackets2 - txPackets1 );
    atomic_store( &data->netState->packetSpeed, packetDelta );

    rxPackets1 = rxPackets2;
    txPackets1 = txPackets2;

    sleep( 1 );
  }

  return 0;
}
