all:
	g++ -I src/include -L src/lib -o main main.cpp CustomClasses.cpp Physic2D.cpp Game.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer