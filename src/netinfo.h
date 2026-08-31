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

#ifndef NETINFO_H
#define NETINFO_H

#include <stdatomic.h>

enum ConnectionStates {
  NET_NO_WAN,
  NET_NO_INTERNET,
  NET_HEALTHY,
  NET_HIGH_LOAD
};

struct NetworkState {
  // [jdsc] this controls the speed of the animation
  atomic_size_t packetSpeed;

  // this controls the color of the light and the animState
  _Atomic enum ConnectionStates connectionState;
};

#endif
