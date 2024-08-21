GO_PKG = mlvm
BUILD  = .build

compile:
	go build ${GO_PKG}/... # go build -o ${BUILD}/pdb cmd/main.go

run: compile
	${BUILD}/pdb

fmt:
	go fmt ${GO_PKG}/... && go mod tidy

clean:
	rm -rf ${BUILD}

test:
	go test ${GO_PKG}/...

release: clean compile
