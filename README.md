flv-repeater
================

This is a tool for generating flv file repead in any times. Note that you can control the repeated file from any offset(normaly at the begining of a FLV TAG), such as bypass flv header metadata. Of course you should parse the flv file in order to set these parameters.

# Compilation

Please execute the scripts below

```
git clone https://github.com/sharpbai/flv-repeater.git
cd flv-repeater
mkdir build
cd build
cmake ..
make
```

# Usage

```
Usage:  ./flv_repeater options [ inputfile ... ]
  -i  --input            Read from this file. Default is stdin.
  -s  --start_index      Repeat from this index.
  -r  --repeat_count     Repeat file count. 0 is infinite.
  --re                    Read input file in framerate.
  -h  --help             Display this usage information.
  -o  --output           Write output to file. Default is stdout.
  -v  --verbose          Print verbose messages.
```

# Sample usage

You can use it to create a infinite flv source, then push it to server for any usage.

```
./build/flv_repeater --re -s 400 -r 0 -i ./res/SampleVideo_720x480_1mb.flv | ffmpeg -f flv -i - -c:v copy -c:a copy -f flv "rtmp://xxx.xx.xx/xx"
```



