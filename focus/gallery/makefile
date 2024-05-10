compile:
	clang++ -std=c++20 `fltk-config --use-images   --cxxflags` -L`brew --prefix`/lib `fltk-config  --use-images  --ldflags` -Wfatal-errors main.cc
clean:
	rm a.out
fmt:
	~/Workspace/y/tools/clang_format_all.sh .
