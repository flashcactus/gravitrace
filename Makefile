test: main
	time ./main
	feh ./out.png

build: main
main: main.cpp 3d.cpp 3d.h
	g++ -lpng main.cpp -o main
