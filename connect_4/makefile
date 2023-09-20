# To create tar.gz for release
#   tar -czvf c4-resnet.pt.state.tar.gz c4-resnet.pt.state

ifndef ${C4_STATE_FILE}
	C4_STATE_FILE = ~/Desktop/c4-resnet-5x5.pt.state
endif

PY            = python3
PY_LIB_PATH   = PYTHONPATH=lib

.PHONY: run check default

default: check run

run:
	${PY_LIB_PATH} C4_STATE_FILE=${C4_STATE_FILE} ${PY} cmd/play_with_human.py

check:
	python3 -c "import torch"

