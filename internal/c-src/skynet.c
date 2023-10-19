#include "skynet.h"

static int SKSendMessage(lua_State* L) {
    size_t length;
    const char* message = luaL_checklstring(L, 3, &length);
    int id = (int)lua_tonumber(L, 2);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    SkynetSendMessage(sk->ptr, id, message, length);
    return 0;
}
static int SKRequest(lua_State* L) {
    size_t length;
    const char* message = luaL_checklstring(L, 3, &length); // 获取 Lua 参数
    int id = (int)lua_tonumber(L, 2);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    size_t replyLength;
    char* replyMessage;
    replyMessage = SkynetRequest(sk->ptr, id, message, length, &replyLength);
    // 返回值
    lua_pushlstring(L, replyMessage, replyLength);
    free(replyMessage);
    return 1;
}
// 注册回调 OnStart
static int SKOnStart(lua_State* L) {
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    sk->on_start_ref = callback_ref;
    return 0;
}
// 注册回调 OnStop
static int SKOnStop(lua_State* L) {
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    sk->on_stop_ref = callback_ref;
    return 0;
}
// 新的print函数实现
static int SKPrint(lua_State* L) {
    // 获取参数个数
    int n = lua_gettop(L);

    // 初始化一个空字符串
    lua_pushstring(L, "");

    for (int i = 1; i <= n; i++) {
        const char* str = lua_tostring(L, i); // 获取参数的字符串值
        if (str != NULL) {
            lua_pushstring(L, str); // 将参数字符串添加到已有字符串后面
            if (i+1<=n) {
                lua_pushstring(L, " ");
                lua_concat(L, 3); // 合并3个字符串    
            } else {
                lua_concat(L, 2); // 合并两个字符串    
            }
        }
    }
    // 
    size_t messageLength;
    const char* message = luaL_checklstring(L, -1, &messageLength);
   
    SkynetPrint(message, messageLength);
    return 0; // 返回0表示成功执行
}
// 注册回调 OnMessage
static int SKOnMessage(lua_State* L) {
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    sk->on_message_ref = callback_ref;
    return 0;
}
// 注册回调 OnRequest
static int SKOnRequest(lua_State* L) {
    int callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    // 获取 skynet_t 对象
    struct skynet_t* sk = (struct skynet_t*)lua_touserdata(L, 1);
    sk->on_request_ref = callback_ref;
    return 0;
}

// 创建lua state
void* sn_NewLuaState(uintptr_t ptr) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // 注册新的print函数为全局的print
    lua_pushcfunction(L, SKPrint);
    lua_setglobal(L, "print");

    struct skynet_t* userdata = (struct skynet_t*)lua_newuserdata(L, sizeof(struct skynet_t));
    // 将 sk 的内容复制到 userdata
    userdata->ptr = ptr;
    userdata->on_start_ref = LUA_REFNIL;
    userdata->on_stop_ref = LUA_REFNIL;
    userdata->on_message_ref = LUA_REFNIL;
    userdata->on_request_ref = LUA_REFNIL;

    luaL_newmetatable(L, "skynet_t");

    // 创建包含方法的表
    lua_newtable(L); // 创建一个空表
    // 添加方法到表
    lua_pushstring(L, "SendMessage");
    lua_pushcfunction(L, SKSendMessage);
    lua_settable(L, -3); // 将方法与表关联

    lua_pushstring(L, "Request");
    lua_pushcfunction(L, SKRequest);
    lua_settable(L, -3); // 将方法与表关联

    lua_pushstring(L, "OnStart");
    lua_pushcfunction(L, SKOnStart);
    lua_settable(L, -3); // 将方法与表关联

    lua_pushstring(L, "OnStop");
    lua_pushcfunction(L, SKOnStop);
    lua_settable(L, -3); // 将方法与表关联

    lua_pushstring(L, "OnMessage");
    lua_pushcfunction(L, SKOnMessage);
    lua_settable(L, -3); // 将方法与表关联

    lua_pushstring(L, "OnRequest");
    lua_pushcfunction(L, SKOnRequest);
    lua_settable(L, -3); // 将方法与表关联

    // 将包含方法的表与 __index 关联
    lua_setfield(L, -2, "__index");
    // 将元表关联到 userdata
    lua_setmetatable(L, -2);

    lua_setglobal(L, "skynet");
    return L;
}
void sn_CloseState(void*ptr) {
    lua_State* L = (lua_State*) ptr;
    lua_close(L);
}
// 执行文件
int sn_DoFile(void*ptr, char*filename) {
    lua_State* L = (lua_State*) ptr;
    return luaL_dofile(L, filename);
}
const char* sn_SendMessage(void *ptr, int id, char*data, size_t size){
    lua_State* L = (lua_State*) ptr;

    // 获取全局变量"skynet"的userdata
    lua_getglobal(L, "skynet");
    struct skynet_t* sk = (struct skynet_t*) lua_touserdata(L, -1);
    if (!sk) {
        return "skynet instance not found";
    }
    // 调用ref函数
    if (sk->on_message_ref == LUA_REFNIL) {
        return "on_message_ref is nil";
    }
    // 使用on_message_ref调用ref函数
    lua_settop(L, 0);
    lua_rawgeti(L, LUA_REGISTRYINDEX, sk->on_message_ref);
    lua_pushlstring(L, data, size);
    lua_pushnumber(L, id);
    if(lua_pcall(L, 2, 0, 0)) {
        const char* errorMessage = lua_tostring(L, -1); // 获取错误信息
        printf("Lua error: %s\n", errorMessage);
        lua_pop(L, 1); // 弹出错误信息 
        return errorMessage;
    }
    return NULL;
}
const char* sn_Request(void *ptr, int id, char*data, size_t size, const char** replyData, size_t*replyLength) {
    lua_State* L = (lua_State*) ptr;
    // 获取全局变量"skynet"的userdata
    lua_getglobal(L, "skynet");
    struct skynet_t* sk = (struct skynet_t*) lua_touserdata(L, -1);
    if (!sk) {
        return "skynet instance not found";
    }
    // 调用ref函数
    if (sk->on_request_ref == LUA_REFNIL) {
        return "on_request_ref is nil";
    }
    // 使用on_request_ref调用ref函数
    lua_settop(L, 0);
    lua_rawgeti(L, LUA_REGISTRYINDEX, sk->on_request_ref);
    lua_pushlstring(L, data, size);
    lua_pushnumber(L, id);
    if(lua_pcall(L, 2, 1, 0)) {
        const char* errorMessage = lua_tostring(L, -1); // 获取错误信息
        printf("Lua error: %s\n", errorMessage);
        lua_pop(L, 1); // 弹出错误信息
        return errorMessage;
    } else {
        size_t length;
        const char* reply = luaL_checklstring(L, 1, &length);
        *replyLength = length;
        *replyData = reply;
    }
    return NULL;
}
const char* sn_OnStart(void *ptr){
    lua_State* L = (lua_State*) ptr;

    // 获取全局变量"skynet"的userdata
    lua_getglobal(L, "skynet");
    struct skynet_t* sk = (struct skynet_t*) lua_touserdata(L, -1);

    if (!sk) {
        return "[c] OnStart skynet instance not found";
    }
    // 调用ref函数
    if (sk->on_start_ref == LUA_REFNIL) {
        return "on_start_ref is nil";
    }
    // 使用on_start_ref调用ref函数
    lua_rawgeti(L, LUA_REGISTRYINDEX, sk->on_start_ref);
    if(lua_pcall(L, 0, 0, 0)) {
        const char* errorMessage = lua_tostring(L, -1); // 获取错误信息
        printf("Lua error: %s\n", errorMessage);
        lua_pop(L, 1); // 弹出错误信息 
        return errorMessage;
    }
    return NULL;
}
const char* sn_OnStop(void *ptr){
    lua_State* L = (lua_State*) ptr;

    // 获取全局变量"skynet"的userdata
    lua_getglobal(L, "skynet");
    struct skynet_t* sk = (struct skynet_t*) lua_touserdata(L, -1);

    if (!sk) {
        return "[c] OnStop skynet instance not found";
    }
    // 调用ref函数
    if (sk->on_stop_ref == LUA_REFNIL) {
        return "on_stop_ref is nil";
    }
    // 使用on_start_ref调用ref函数
    lua_rawgeti(L, LUA_REGISTRYINDEX, sk->on_stop_ref);
    if(lua_pcall(L, 0, 0, 0)) {
        const char* errorMessage = lua_tostring(L, -1); // 获取错误信息
        printf("Lua error: %s\n", errorMessage);
        lua_pop(L, 1); // 弹出错误信息 
        return errorMessage;
    }
    return NULL;
}