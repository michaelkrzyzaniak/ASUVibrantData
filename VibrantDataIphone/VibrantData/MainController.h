//
//  MainController.h
//  VibrantData
//
//  Created by Michael Krzyzaniak on 6/23/15.
//  Copyright (c) 2015 ArizonaStateUniversity. All rights reserved.
//
////sudo tcpdump -l | nc -l 6543

#import <Foundation/Foundation.h>
#import "UIKit/UIKit.h" //for UIView and UIColor
#import "Synth.h"
#import "Network.h"

//#define TCP_HOST "127.0.0.1"
//#define TCP_PORT 6543

#define UDP_SEND_IP      "127.0.0.1"
#define UDP_SEND_PORT    6542
#define UDP_RECEIVE_PORT 6543

@interface MainController : NSObject
{
  Synth*   synth;
  Network* net  ;
  
  int wants_tcp;
  int wants_udp;
  int wants_outgoing;
  int wants_incoming;
  int wants_secure;
  int wants_unsecure;
  
  IBOutlet UIView* main_view;
}

-(IBAction) set_wants_tcp     : (id) sender;
-(IBAction) set_wants_udp     : (id) sender;
-(IBAction) set_wants_incoming: (id) sender;
-(IBAction) set_wants_outgoing: (id) sender;
-(IBAction) set_wants_secure  : (id) sender;
-(IBAction) set_wants_unsecure: (id) sender;


@end
