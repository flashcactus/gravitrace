all: bin/main
	cd bin; for f in ../cfg/*; do echo $$f; ./main $$f; done

test: bin/main test_config.txt
	time ./main cfg/test_config.txt
	feh ./out.png

build: bin/main
dbg_build: bin/main_dbg
bin/main: src/main.cpp src/3d.cpp src/3d.h
	cd src; g++ -lpng main.cpp -o ../bin/main

bin/main_dbg: src/main.cpp src/3d.cpp src/3d.h
	cd src; g++ -lpng main.cpp -o ../bin/main_dbg -g

dbg: bin/main_dbg test_config.txt
	gdb bin/main_dbg cfg/test_config.txt

clean:
	rm -f bin/main bin/main_dbg bin/*.png
