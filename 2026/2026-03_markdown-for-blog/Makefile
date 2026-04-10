BIN = .build/md2html

compile:
	mkdir -p .build
	go build -o $(BIN) ./cmd/...

release: compile

test:
	go test ./src/...

fmt:
	go fmt md2html/...

clean:
	rm -rf .build

.PHONY: compile release test fmt clean
