BUILD = .build

run:
	go run fzf
run_c:
	mkdir -p ${BUILD}
	cc c/fzf.c -o ${BUILD}/fzf_c && ${BUILD}/fzf_c
fmt:
	go fmt fzf
	~/Workspace/y/tools/scripts/clang_format_all.sh .


