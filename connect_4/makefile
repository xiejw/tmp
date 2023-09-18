WEIGHT_FILE = data/golden_weights/2020-05-03.h5

.PHONY: run check default

default: run

run: check
	PYTHONPATH=lib python3 cmd/play.py

check:
	python3 -c "import torch"

