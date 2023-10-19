package main

import (
	"fmt"

	"github.com/DGHeroin/skynet-go"
)

type h int

func (h) OnMessage(id int, data []byte) {
	fmt.Println("[go] OnMessage:", id, data)
}
func (h) OnRequest(id int, data []byte) []byte {
	fmt.Println("[go] OnRequest:", id, data)
	return data
}
func main() {
	handler := h(0)
	lua := skynet.NewLua(&handler)
	lua.DoFile("simple.lua")

	lua.Start()

	// lua.SendMessage([]byte{1, 2, 3, 4, 5})
	lua.SendRequest([]byte{6, 7, 8}, 1, func(replyData []byte) {
		fmt.Println("[go] SendRequest reply: ", replyData)
	})

	// for i := 0; i < 1000*1000; i++ {
	// 	lua.SendMessage([]byte("1"), 2)
	// }
	lua.SendMessage([]byte("output"), 3)
	lua.Close()
}
