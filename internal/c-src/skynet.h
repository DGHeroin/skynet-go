#ifndef __skynet_h__
#define __skynet_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern void  SkynetSendMessage(uintptr_t, int, const char*, size_t);
extern char* SkynetRequest(uintptr_t, int, const char*, size_t, size_t*);
extern void SkynetPrint(const char*, size_t);
struct skynet_t {
    uintptr_t  ptr;
    int on_start_ref;
    int on_stop_ref;
    int on_message_ref;
    int on_request_ref;
};

#endif