package skynet

/*
#cgo CFLAGS: -I/usr/include/lua5.3
#cgo LDFLAGS: -llua5.3

#include "internal/c-src/skynet.c"
#include <stdlib.h>
*/
import "C"
import (
	"fmt"
	"runtime/cgo"
	"sync"
	"unsafe"
)

type LuaState struct {
	L       unsafe.Pointer
	handle  cgo.Handle
	msgCh   chan *messageContext
	reqCh   chan *requestContext
	funCh   chan func()
	closeCh chan struct{}
	wg      sync.WaitGroup
	handler MessageHandler
}
type messageContext struct {
	data []byte
	id   int
}
type requestContext struct {
	data    []byte
	id      int
	reply   []byte
	err     error
	onReply func([]byte)
}
type MessageHandler interface {
	OnMessage(int, []byte)
	OnRequest(int, []byte) []byte
}

func NewLua(handler MessageHandler) *LuaState {
	state := &LuaState{
		msgCh:   make(chan *messageContext, 100),
		reqCh:   make(chan *requestContext, 100),
		funCh:   make(chan func(), 100),
		closeCh: make(chan struct{}),
		handler: handler,
	}

	// 创建lua状态机
	handle := cgo.NewHandle(state)
	state.handle = handle
	state.L = C.sn_NewLuaState(C.uintptr_t(handle))
	state.wg.Add(1)
	go func() {
		var closed bool
	exit_loop:
		for {
			select {
			case ctx := <-state.msgCh:
				if ctx == nil {
					continue
				}
				dPtr := (*C.char)(unsafe.Pointer(&ctx.data[0]))
				dSz := C.size_t(len(ctx.data))

				errMsg := C.sn_SendMessage(state.L, C.int(ctx.id), dPtr, dSz)
				if errMsg != nil {
					goStr := C.GoString(errMsg)
					fmt.Println("[LuaState] SendMessage Error:", goStr)
				}
			case ctx := <-state.reqCh:
				if ctx == nil {
					continue
				}
				dPtr := (*C.char)(unsafe.Pointer(&ctx.data[0]))
				dSz := C.size_t(len(ctx.data))

				var replyData *C.char
				var replyLength C.size_t

				errMsg := C.sn_Request(state.L, C.int(ctx.id), dPtr, dSz, &replyData, &replyLength)
				if errMsg != nil {
					goStr := C.GoString(errMsg)
					ctx.err = fmt.Errorf("%s", goStr)
					fmt.Println("[LuaState] SendMessage Error:", goStr)
				} else {
					replyDataBytes := C.GoBytes(unsafe.Pointer(replyData), C.int(replyLength))

					ctx.onReply(replyDataBytes)
				}
			case fn := <-state.funCh:
				if fn == nil {
					continue
				}
				fn()
			case <-state.closeCh:
				if !closed {
					closed = true
					close(state.msgCh)
					close(state.reqCh)
				}
			}
			if closed &&
				len(state.msgCh) == 0 &&
				len(state.reqCh) == 0 &&
				len(state.funCh) == 0 {
				break exit_loop
			}
		}
		// exit_loop
		handle.Delete()
		C.sn_CloseState(state.L)

		state.wg.Done()
	}()

	return state
}
func (s *LuaState) DoFile(filename string) error {
	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))

	C.sn_DoFile(s.L, cFilename)

	return nil
}

func (s *LuaState) SendMessage(data []byte, id int) {
	ctx := &messageContext{
		data: data,
		id:   id,
	}
	s.msgCh <- ctx
}
func (s *LuaState) SendRequest(data []byte, id int, onReply func([]byte)) {
	ctx := &requestContext{
		data: data,
		id:   id,
	}

	ctx.onReply = onReply
	s.reqCh <- ctx
}
func (s *LuaState) Close() {
	s.funCh <- func() {
		C.sn_OnStop(s.L)
	}
	close(s.closeCh)
	s.wg.Wait()
}
func (s *LuaState) Start() {
	s.funCh <- func() {
		C.sn_OnStart(s.L)
	}
}
