CC=clang
IPATHS=-I./mosek/10.0/tools/platform/linux64x86/h -I./mosek/10.0/tools/platform/linux64x86/include -I./nauty27r2
LPATHS=-L./mosek/10.0/tools/platform/linux64x86/bin -Wl,-rpath-link, ./mosek/10.0/tools/platform/linux64x86/bin '-Wl,-rpath=$$ORIGIN/mosek/10.0/tools/platform/linux64x86/bin'

fusion:
	make install -C ./mosek/10.0/tools/platform/linux64x86/src/fusion_cxx 

main: fusion main.cpp
	$(CC) -pthread -std=c++11 -g $(IPATHS) $(LPATHS) -o a.out main.cpp -fopenmp -L/usr/lib64 -lstdc++ -lfusion64 -lmosek64 ./nauty27r2/nauty.a -lm

all: fusion main
	./a.out
