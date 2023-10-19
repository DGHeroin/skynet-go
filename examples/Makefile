.PHONEY:

simple: simple.go
	CGO_ENABLED=1 go build -trimpath -o $@ $<
clean:
	go clean -cache ; rm simple