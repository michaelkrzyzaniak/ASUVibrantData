//
//  MainController.m
//  VibrantData
//
//  Created by Michael Krzyzaniak on 6/23/15.
//  Copyright (c) 2015 ArizonaStateUniversity. All rights reserved.
//
////sudo tcpdump -l | nc -l 6543

#import "MainController.h"
#import "OSC.h"

@implementation MainController

-(id) init
{
  self = [super init];
  if(self != NULL)
    {
      self->synth = synthNew(1);
      if(self->synth == NULL) 
        return NULL;
        
      self->net   = net_new();
      if(self->net == NULL)
        return NULL;
        
      auPlay((Audio*)self->synth);
    }
  return self;
}

/*-------------------------------------------*/
-(void) awakeFromNib
{
  [NSThread detachNewThreadSelector: @selector(tcp_receive:) toTarget: self withObject: nil];
}

/*-------------------------------------------*/
-(IBAction) set_wants_something: (id)sender what:(int*)what message:(char*)message
{
  int buffer_size = 100;
  char osc_buffer[buffer_size]; //more than plenty
  *what = !(*what);
  int val = (*what == YES) ? 1 : 0;
  int num_bytes = oscConstruct(osc_buffer, buffer_size, message, "i", val);
  net_udp_send(self->net, osc_buffer, num_bytes, UDP_SEND_IP, UDP_SEND_PORT);
  
  UIColor* c = (val) ? [UIColor darkGrayColor] : [UIColor lightGrayColor];
  [sender setBackgroundColor: c];
}

/*-------------------------------------------*/
-(IBAction) set_wants_tcp     : (id) sender
{
  [self set_wants_something: sender what: &wants_tcp message: "/asu/vibrant/wants_tcp"];
}

/*-------------------------------------------*/
-(IBAction) set_wants_udp     : (id) sender
{
  [self set_wants_something: sender what: &wants_udp message: "/asu/vibrant/wants_udp"];
}

/*-------------------------------------------*/
-(IBAction) set_wants_incoming: (id) sender
{
  [self set_wants_something: sender what: &wants_incoming message: "/asu/vibrant/wants_incoming"];
}

/*-------------------------------------------*/
-(IBAction) set_wants_outgoing : (id) sender
{
  [self set_wants_something: sender what: &wants_outgoing message: "/asu/vibrant/wants_outgoing"];
}

/*-------------------------------------------*/
-(IBAction) set_wants_secure  : (id) sender
{
  [self set_wants_something: sender what: &wants_secure message: "/asu/vibrant/wants_secure"];
}

/*-------------------------------------------*/
-(IBAction) set_wants_unsecure: (id) sender
{
  [self set_wants_something: sender what: &wants_unsecure message: "/asu/vibrant/wants_unsecure"];
}

#define NET_BUFFER_SIZE 100
char net_buffer[NET_BUFFER_SIZE];
-(void) tcp_receive: (void*) ignored
{
  //NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  int num_bytes;
  char* osc_address;
  char* osc_type_tag;
  int num_osc_values;
  oscValue_t* osc_values;

  for(;;)
    {
      //net_tcp_receive_connections(self->net, TCP_PORT);
      //BOOL success = net_tcp_connect_to_host(self->net, TCP_HOST, TCP_PORT);
      BOOL success = net_udp_connect(self->net, UDP_RECEIVE_PORT);
      if(success)
        {
          fprintf(stderr, "connected...\r\n");
          for(;;)
            {
              num_bytes = net_tcp_receive(self->net, net_buffer, NET_BUFFER_SIZE);
              if(num_bytes > 0)
                {
                  num_osc_values = oscParse(net_buffer, num_bytes, &osc_address, &osc_type_tag, &osc_values);
                  if (num_osc_values >= 0)
                    {
                      switch(oscHash((unsigned char*)osc_address))
                        {
                          case 4000250133: // "/asu/vibrant/magnitude"
                            if(num_osc_values >= 1)
                              {
                                float mag = oscValueAsFloat(osc_values[0], osc_type_tag[0]);
                                synth_set_vibration_magnitude(self->synth, mag);
                                //UIColor* c = [UIColor colorWithHue: 0.8 saturation: 1 brightness: mag alpha: 1];
                                //[main_view setBackgroundColor: c];
                                //[main_view setNeedsDisplay: YES];
                              }
                            break;
                        }
                    }
                  
                  fprintf(stderr, "recd %i\r\n", num_bytes);
                }
              else break;
            }
          fprintf(stderr, "disconnected...\r\n");
        }
      net_disconnect(self->net);
      sleep(3);
    }
  //[pool drain];
}

-(void)dealloc
{
  //pthread_cancel(udp_receive_thread);
  //pthread_join(udp_receive_thread, NULL);
  net_destroy(self->net);
  auPause  ((Audio*)self->synth);
  auDestroy((Audio*)self->synth);
  
  //[super dealloc];
}

@end
