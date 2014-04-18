#ifndef _BUS_MSG_H_
#define _BUS_MSG_H_

// define bus system message here

// system: broadcast bus => all slots
#define BUS_MSG_EXIT        BUS_MESSAGE_BASE_ID

// system: publish/subscribe slot->slot
#define BUS_MSG_TRACE       BUS_MESSAGE_BASE_ID + 1
#define BUS_MSG_TIMER       BUS_MESSAGE_BASE_ID + 2


// defined bus user custom message here
// ... #define BUS_MSG_USER_ID     BUS_MESSAGE_BASE_ID + ...

#endif
