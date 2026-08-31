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

#include <unistd.h>
#include "rgbled.h"

static inline uint16_t QuadraticEaseOut( uint16_t progress )
{
  uint32_t inverse = 65535 - progress;
  return 65535 - ( ( inverse * inverse ) >> 16 );
}

/*
static inline uint16_t QuadraticEaseIn( uint16_t progress )
{
  return ( ( uint32_t )progress * progress ) >> 16;
}


static inline uint16_t TriangleWave( uint16_t progress ) 
{
  if( progress < 32768 )
    return progress << 1;
  else
    return ( 65535 - progress ) << 1;
}
*/

static inline uint16_t SplineWave( uint16_t progress )
{
  if( progress < 32768 )
    return ( ( uint32_t )progress * progress ) >> 15;
  else {
    uint32_t inverse = 65535 - progress;
    return 65535 - ( ( inverse * inverse ) >> 15 );
  }
}

void* anim_thread( void* arg )
{
  struct Data* data = ( struct Data* )arg;
  uint16_t animPhase = 0;
  
  while( 1 ) {
    animPhase += 32;

    switch( atomic_load( &data->netState->connectionState ) ) {
      case NET_NO_WAN:
        data->ledState->animState = ANIM_FLASH;
        data->ledState->R = 255;
        data->ledState->G = 0;
        data->ledState->B = 0;
        break;

      case NET_NO_INTERNET:
        data->ledState->animState = ANIM_PULSE;
        data->ledState->R = 255;
        data->ledState->G = 104;
        data->ledState->B = 3;
        break;

      case NET_HEALTHY:
        data->ledState->animState = ANIM_BREATHE;
        data->ledState->R = 18;
        data->ledState->G = 255;
        data->ledState->B = 144;
        break;

      case NET_HIGH_LOAD:
        data->ledState->animState = ANIM_BREATHE;
        data->ledState->R = 251;
        data->ledState->G = 255;
        data->ledState->B = 18;
        break;
    }
    
    // [jdsc] update the animation:
    switch( data->ledState->animState ) {
      case ANIM_SOLID:
        data->ledState->A = 255;
        break;

      case ANIM_BREATHE:
        data->ledState->A = SplineWave( animPhase ) >> 8;
        break;
        
      case ANIM_PULSE:
        data->ledState->A = QuadraticEaseOut( 65535 - animPhase ) >> 8;
        break;

      // case ANIM_BEAT:

      case ANIM_FLASH:
        if( animPhase < 32768 )
          data->ledState->A = 255;
        else
          data->ledState->A = 32;
        break;
    }
    
    usleep( ANIM_UPDATE_RATE );
  }
}
