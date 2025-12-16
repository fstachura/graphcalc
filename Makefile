main: main.cpp *.hpp
	g++ -g -lGL -lGLEW -lglfw -o main main.cpp

clean:
	rm -f main
