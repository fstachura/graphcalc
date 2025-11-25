main: main.cpp
	g++ -lGL -lGLEW -lglfw -o main main.cpp

clean:
	rm main
