<img width="749" alt="image" src="https://github.com/TheStressTest/tuiner/assets/73505616/6c1b0548-5845-4c73-a3f7-b7d49c1d3ef7">

# Tuiner
A tiny little chromatic tuner for your terminal. This is a TUI app hence the
name.

## Building
Out of this box this only works on Macs, unfortunately. Getting it to work on
Linux should be as simple as changing the CoreAudio dependencies in the
Makefile. Building is as easy as running `make`. You also have to install the portaudio library to `lib/portaudio`

## How it works:
I have a firm belief that the best way to figure out something works, is to
build it yourself! I created this project in the spirit of curiosity and I figure I should leave some notes down here. The essential steps in the tuning process are:
### 1. Fourier transform
In this step, a single frame from your microphone is broken down into it's
frequencies. I am not going to attempt to explain the Fourier Transform in
this markdown document, partially because it is so complex, and partially
because I don't fully understand it myself. I have found that 3Blue1Brown
has a good [video](https://youtu.be/spUNpyF58BY?si=44HEBBy-mjjqWFGb) on this topic.
### 2. Filters
The next step is to apply some filters on the frequencies that the Fourier
Transform give you. The first is a small filter which takes out all the
noise, and the second filter is a HPS filter (Harmonic Product Spectrum).
From my understanding, this amplifies the fundamental frequency in relation
to the harmonic frequencies, by multiplying each frequency by the next few
harmonics. This ensure that the peak frequency is the actual fundamental
frequency, otherwise this tuner would only work on pure sine waves.

