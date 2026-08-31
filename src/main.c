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

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "netinfo.h"
#include "rgbled.h"
#include "misc.h"

// update stats every 3 seconds
const size_t STAT_UPDATE_RATE = 3000000;

int main() {
  pthread_t networkThread;
  pthread_t ledThread;

  struct NetworkState netState = {
    .packetSpeed = 0,
    .connectionState = NET_NO_WAN
  };
  
  struct LedState ledState = {
    .R = 255,
    .G = 255,
    .B = 255,
    .A = 0,
    .animState = ANIM_SOLID
  };

  struct Data data = {
    .netState = &netState,
    .ledState = &ledState
  };

  pthread_create( &networkThread, NULL, net_thread, &data );
  pthread_create( &ledThread, NULL, anim_thread, &data );

  pthread_setname_np( networkThread, "network" );
  pthread_setname_np( ledThread, "led" );


  return 0;
}
