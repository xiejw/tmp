# In order to use sqlite3 in debian, install
# - sudo apt install libsqlite3-dev
BIN = a.out

.PHONY: release 01

release: 01

01:
	${CC} -o ${BIN} 01_version.c -lsqlite3 -std=c99 -Wall

fmt:
	clang-format -i --style=file *.c

clean:
	rm ${BIN}
