OUTPUT = ./out/opentuner
CLIB = -I./lib/portaudio/include \
       -L./lib/portaudio/lib/.libs -lportaudio \
	   -L/opt/homebrew/Cellar/fftw/3.3.10_1/lib \
	   -I/opt/homebrew/Cellar/fftw/3.3.10_1/include \
       -framework CoreAudio \
	   -framework AudioToolbox \
	   -framework CoreFoundation \
	   -framework AudioUnit \
	   -framework CoreServices \
	   -pthread -lfftw3f -lm

${OUTPUT}: main.c
	clang -o $@ $^ $(CLIB)

clean:
	rm $(OUTPUT)
