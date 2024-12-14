UNAME           = $(shell uname)
BUIlD           = .build

compile: | ${BUIlD}
ifneq ($(UNAME), Darwin)   # macOs does not have good support for c23.
	${CC} -std=c23 -Wall -Werror main.c -o ${BUIlD}/demo
endif

${BUIlD}:
	mkdir -p ${BUIlD}

