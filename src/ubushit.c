#include "ubushit.h"

char wan_dev[64] = {0};

// [jdsc] this shit might genuinely be more complicated than Vulkan
static const struct blobmsg_policy interface_policy[1] = {
  [0] = { .name = "l3_device", .type = BLOBMSG_TYPE_STRING },
};

static void get_wan_cb( struct ubus_request* request, int type, 
  struct blob_attr* message  )
{
  struct blob_attr* tb[1];
  if( !message )
    return;

  blobmsg_parse( interface_policy, 1, tb, blob_data( message ), 
    blob_len( message ) );
  
  if( tb[0] ) {
    strncpy( wan_dev, blobmsg_get_string( tb[0] ), 
      sizeof( wan_dev ) - 1 );
  }
}

static int ubus_do_a_bunch_of_shit_to_get_the_wan_dev_name()
{
  struct ubus_context* context = ubus_connect( NULL );
  uint32_t id;

  if( !context )
    return -1;

  if( ubus_lookup_id( context, "network.interface.wan", &id ) == 0 )
    ubus_invoke( context, id, "status", NULL, get_wan_cb, NULL, 1000 );

  ubus_free( context );

  if( strlen( wan_dev ) <= 0 )
    return -1;

  return 0;
}

int find_wan_device( char* buffer )
{
  int retval = ubus_do_a_bunch_of_shit_to_get_the_wan_dev_name();
  strncpy( buffer, wan_dev, sizeof( buffer ) - 1 );
  return retval;
}
