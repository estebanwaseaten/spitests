Tool for testing of SPI speed using ioctl (developed for raspberry pi)

transmits a random package of bytes repeatedly and checks if the transfer worked afterwards and returns the achieved bitrate.
it seems that reducing the number of calls and transmitting larger packages optimizes th ebit rate (less overhead).
the maximum bitrate on a pi5 seems to be achievable by requesting a frequency of 10 MHz (10Mbits/s) and results in a real bit rate of 6MHz (6Mbits/6). Further increasing the requested frequency towards 100MHz does not increase transfer speed, but increases the transfer error count quickly towards 100%

compile with
> ./make.sh

run with
> ./spistress

settings are in spistress.conf
> #requested frequency in Hz (bits per second):
> freq=2000000
> repeat=1000
> #bytes per transfer (single ioctl call transfers this amount of bytes):
> bytes=100


