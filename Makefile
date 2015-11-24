FLAGS=

all: bin/main
	cd bin; for f in ../cfg/*; do echo $$f; ./main $$f; done
time: bin/main
	cd bin; for f in ../cfg/*; do echo $$f; time ./main $$f; done

test: bin/main cfg/test_config.txt
	cd bin;time ./main ../cfg/test_config.txt
	feh bin/test.png

build: bin/main
dbg_build: bin/main_dbg
bin/main: src/*.cpp src/*.h
	cd src; g++ -I lib/libpng12 main.cpp -o ../bin/main -L. -lpng -lz -I lib

bin/main_dbg: src/main.cpp src/3d.cpp src/3d.h
	cd src; g++ -I lib.libpng12 main.cpp -o ../bin/main_dbg -g -L. -lpng -lz -I lib

dbg: bin/main_dbg cfg/test_config.txt
	cd bin; gdb main_dbg ../cfg/test_config.txt

clean:
	rm -f bin/main bin/main_dbg bin/*.png
