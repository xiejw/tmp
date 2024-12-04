CFLAGS         += -std=c23 -Wall -Werror -pedantic -Wfatal-errors
CFLAGS         += -Wconversion -Wshadow

CFLAGS         += $(shell pkg-config sqlite3 --cflags)
LDFLAGS        += $(shell pkg-config sqlite3 --libs)

SH              = sh
BUIlD           = .build

.PHONY: run 01

SQLS_TO_RUN     = "create table tbl1(one text, two int);   \
		   select count(*) from tbl1;              \
		   insert into tbl1 values('hello!',10);   \
		   insert into tbl1 values('goodbye', 20); \
		   select * from tbl1;"

run: 01 02
	echo "Run all"                                                                         && \
		echo "-> Run 01"                                                               && \
		${BUIlD}/01                                                                    && \
		echo "-> Run 02"                                                               && \
		${BUIlD}/02 ${SQLS_TO_RUN}                                                     && \
		echo "-> Run 03"                                                               && \
		${SH} 03_script_with_csv.sh                                                    && \
		echo "-> Done"

01: 01_cfg.c | ${BUIlD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUIlD}/$@ $^

02: 02_exec.c | ${BUIlD}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BUIlD}/$@ $^

fmt:
	clang-format -i --style=file *.c

${BUIlD}:
	mkdir -p ${BUIlD}

clean:
	rm -rf ${BUIlD}
