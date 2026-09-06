/*
=======================================================================

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

=======================================================================

  misc.h

holds information that doesn't fit elsewhere, like netinfo or rgbled.

=======================================================================
*/

#ifndef MISC_H
#define MISC_H

#include "netinfo.h"
#include "rgbled.h"

// [jdsc] 3 sec
#define ERROR_RETRY_TIMER 3

typedef struct {
  networkState_s* netState;
  ledState_s* ledState;
} routerContext_s;

typedef enum {
  R_ERROR = -1, // how tragic...
  R_A_OK = 0,
  R_OK_BUT_NOT = 1, // function finished but with bad news
} funcResult_e;

#endif
