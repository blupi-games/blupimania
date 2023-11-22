#!/bin/sh

echo "Configure VLC to use the Windows soundfont 'GS sound set (16 bit).sf2"
echo "Change for a gain of 0.20"

for a in *.mid; do
  cvlc "$a" --play-and-exit '--sout=#transcode{vcodec=none,acodec=flac,ab=512,channels=2,samplerate=44100}:std{access=file,mux=flac,dst=-' | ffmpeg -i pipe: -aq 0 "./${a%.mid}.ogg"
done
