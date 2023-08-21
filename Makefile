OUTPUT = ./out/opentuner
CLIB = ./lib/portaudio/lib/.libs/libportaudio.dylib -I./lib/portaudio/include -framework CoreAudio -framework AudioToolbox -framework CoreFoundation -framework AudioUnit -framework CoreServices -pthread

${OUTPUT}: main.c
	clang -o $@ $^ $(CLIB) -Wall -Werror -Wpedantic

clean:
	rm $(OUTPUT)

