#!/usr/bin/env sh

if [ "$TARGET" == "linux64" ]; then
  cd ~
  curl -SLsO https://raw.githubusercontent.com/d3cod3/ofxVisualProgramming/fftw/fftw-3.3.2.tar.gz
  tar -xvf fftw-3.3.2.tar.gz
  cd fftw-3.3.2
  ./configure --prefix=`pwd` --enable-float --enable-sse2 --with-incoming-stack-boundary=2 --with-our-malloc16 --disable-shared --enable-static
  make
  cd .libs
  cp libfftw3f.a ~/openFrameworks/addons/ofxAudioAnalyzer/libs/fftw3f/lib/linux64/
fi
