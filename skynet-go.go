package skynet

/*
#include <stdlib.h>
#include <string.h>
*/
import "C"
import (
	"fmt"
	"runtime/cgo"
	"unsafe"
)

//export SkynetSendMessage
func SkynetSendMessage(i uintptr, id C.int, s *C.char, size C.size_t) {
	var h interface{} = cgo.Handle(i).Value()
	state, ok := h.(*LuaState)
	if !ok {
		return
	}

	goBytes := C.GoBytes(unsafe.Pointer(s), C.int(size)) // 放心使用, goBytes数据为副本, 不用担心c端释放
	state.handler.OnMessage(int(id), goBytes)
}

//export SkynetRequest
func SkynetRequest(i uintptr, id C.int, s *C.char, size C.size_t, replyLen *C.size_t) *C.char {
	var h interface{} = cgo.Handle(i).Value()
	state, ok := h.(*LuaState)
	if !ok {
		return nil
	}
	goBytes := C.GoBytes(unsafe.Pointer(s), C.int(size)) // 放心使用, goBytes数据为副本, 不用担心c端释放
	data := state.handler.OnRequest(int(id), goBytes)

	dSz := C.size_t(len(data))
	dPtr := (*C.char)(C.malloc(dSz)) // 在C内存中分配空间
	if dPtr == nil {
		return nil // 或处理内存分配失败的情况
	}

	C.memcpy(unsafe.Pointer(dPtr), unsafe.Pointer(&data[0]), dSz) // 复制数据到C内存
	*replyLen = dSz

	return dPtr
}

//export SkynetPrint
func SkynetPrint(s *C.char, size C.int) {
	goBytes := C.GoStringN(s, size)
	fmt.Printf("%s\n", goBytes)
}
