#!/bin/sh

echo "Configure VLC for using 'Timbres of Heaven' sf2 file (fluidsynth)"
echo "provided by http://midkar.com/soundfonts/"
echo "Change for a gain of 0.10"

for a in *.mid; do
  cvlc "$a" --play-and-exit '--sout=#transcode{vcodec=none,acodec=flac,ab=512,channels=2,samplerate=44100}:std{access=file,mux=flac,dst=-' | ffmpeg -i pipe: -aq 0 "./${a%.mid}.ogg"
done
