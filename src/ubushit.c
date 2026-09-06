#include "ubushit.h"
#include "misc.h"

// [jdsc] this shit might genuinely be more complicated than Vulkan
static const struct blobmsg_policy interface_policy[1] = {
  [0] = { 
    .name = "l3_device", 
    .type = BLOBMSG_TYPE_STRING 
  },
};

static void _get_wan_cb( struct ubus_request* request, int type, 
  struct blob_attr* message  )
{
  // where is the documentation for UBUS??? WTFM??
  char* wanDev = request->priv;
  struct blob_attr* tb[1];
  
  if ( !wanDev || !message )
    return;

  blobmsg_parse( interface_policy, 1, tb, blob_data( message ), 
    blob_len( message ) );
  
  if ( tb[0] )
    snprintf( wanDev, UNIX_MAX_DEV_NAME_SIZE, "%s", 
      blobmsg_get_string( tb[0] ) );
}

static funcResult_e 
  _ubus_do_a_bunch_of_shit_to_get_the_wan_dev_name( char* wanDev )
{
  funcResult_e ret = R_ERROR;
  struct ubus_context* context = ubus_connect( NULL );
  uint32_t id = 0;

  if ( !context )
    return R_ERROR;
    
  if ( ubus_lookup_id( context, "network.interface.wan", &id ) != 
    UBUS_STATUS_OK )
    goto GoAwayAndDie;
    
  if ( ubus_invoke( context, id, "status", NULL, _get_wan_cb, wanDev,
    1000 ) != UBUS_STATUS_OK )
    goto GoAwayAndDie;

  if ( wanDev[0] == '\0' )
    goto GoAwayAndDie;

  ret = R_A_OK;

GoAwayAndDie:
  ubus_free( context );
  return ret;
}

funcResult_e find_wan_device( char* buffer, size_t size )
{
  if ( !buffer || size < UNIX_MAX_DEV_NAME_SIZE )
    return R_OK_BUT_NOT;

  buffer[0] = '\0';

  funcResult_e retVal = 
    _ubus_do_a_bunch_of_shit_to_get_the_wan_dev_name( buffer );
  
  return retVal;
}
