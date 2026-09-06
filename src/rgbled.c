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
#include <stdio.h>
#include "rgbled.h"
#include "netinfo.h"

static inline uint16_t local_quadratic_ease_out( uint16_t progress )
{
  uint32_t inverse = 65535 - progress;
  return 65535 - ( ( inverse * inverse ) >> 16 );
}

static inline uint16_t local_spline_wave( uint16_t progress )
{
  if ( progress < 32768 ) {
    return ( ( uint32_t )progress * progress ) >> 15;
  } else {
    uint32_t inverse = 65535 - progress;
    return 65535 - ( ( inverse * inverse ) >> 15 );
  }
}

void* anim_thread( void* arg )
{
  routerContext_s* rCtx = ( routerContext_s* )arg;
  uint16_t animPhase = 0;
  // xxx xxx xxx + NUL = 12 chars
  char multiIntensityBuffer[12] = {0};

  int ledFileDescriptor = open( "/sys/class/led/rgb:status/multi_intensity", O_WRONLY );
  
  while ( 1 ) {
    animPhase += 32;

    switch ( atomic_load( &rCtx->netState->connectionState ) ) {
      case NET_NO_WAN:
        rCtx->ledState->animState = ANIM_FLASH;
        rCtx->ledState->R = 255;
        rCtx->ledState->G = 0;
        rCtx->ledState->B = 0;
        break;

      case NET_NO_INTERNET:
        rCtx->ledState->animState = ANIM_PULSE;
        rCtx->ledState->R = 255;
        rCtx->ledState->G = 104;
        rCtx->ledState->B = 3;
        break;

      case NET_HEALTHY:
        rCtx->ledState->animState = ANIM_BREATHE;
        rCtx->ledState->R = 18;
        rCtx->ledState->G = 255;
        rCtx->ledState->B = 144;
        break;

      /*case NET_HIGH_LOAD:
        rCtx->ledState->animState = ANIM_BREATHE;
        rCtx->ledState->R = 251;
        rCtx->ledState->G = 255;
        rCtx->ledState->B = 18;
        break;*/
    }
    
    // [jdsc] update the animation:
    switch ( rCtx->ledState->animState ) {
      case ANIM_SOLID:
        rCtx->ledState->A = 255;
        break;

      case ANIM_BREATHE:
        rCtx->ledState->A = local_spline_wave( animPhase ) >> 8;
        break;
        
      case ANIM_PULSE:
        rCtx->ledState->A = local_quadratic_ease_out( 65535 - animPhase ) >> 8;
        break;

      // case ANIM_BEAT:

      case ANIM_FLASH:
        if ( animPhase < 32768 )
          rCtx->ledState->A = 255;
        else
          rCtx->ledState->A = 32;
        break;
    }

    size_t len = snprintf( multiIntensityBuffer, sizeof( multiIntensityBuffer ), 
      "%i %i %i", 
      rCtx->ledState->R, rCtx->ledState->G, rCtx->ledState->B );

    // multiIntensityBuffer[sizeof( multiIntensityBuffer ) - 1] = '\0';

    write( ledFileDescriptor, multiIntensityBuffer, len );
    
    usleep( ANIM_UPDATE_RATE );
  }
}
