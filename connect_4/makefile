WEIGHT_FILE = data/golden_weights/2020-05-03.h5

.PHONY: run check default

default: run

run: check
	PYTHONPATH=lib C4_STATE_FILE=~/Desktop/resnet.pt.state python3 cmd/play_with_human.py

check:
	python3 -c "import torch"

