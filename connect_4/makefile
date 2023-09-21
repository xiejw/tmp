# To create tar.gz for release
#   tar -czvf c4-resnet.pt.state.tar.gz c4-resnet.pt.state

ifndef ${C4_STATE_FILE}
	C4_STATE_FILE = ${HOME}/Desktop/c4-resnet-5x5.pt.state
endif

PY            = python3
PY_LIB_PATH   = PYTHONPATH=lib

DOCKER_ASSETS = .docker_assets

.PHONY: run check default

default: check run

run:
	${PY_LIB_PATH} C4_STATE_FILE=${C4_STATE_FILE} ${PY} cmd/play_with_human.py

check:
	python3 -c "import torch"

clean:
	rm -rf ${DOCKER_ASSETS}

build_docker:
	mkdir -p ${DOCKER_ASSETS} && \
		cp -r lib cmd misc ${DOCKER_ASSETS} && \
		cp ${C4_STATE_FILE} ${DOCKER_ASSETS}/resnet.pt.state && \
	docker build -t xiejw/connect_4_pt \
		--build-arg WEIGHT_FILE=resnet.pt.state \
		-f misc/docker/Dockerfile \
		${DOCKER_ASSETS}
