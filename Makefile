all:
	g++ -I src/include -L src/lib -o AlienEvil main.cpp src/*.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer