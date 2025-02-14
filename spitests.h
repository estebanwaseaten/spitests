#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <cstdint>      //uint...

#include <fcntl.h>      //open() and O_RDWR
#include <unistd.h>    //close()

#include <chrono>

#include <cmath>

bool running = true;

bool valid;
uint8_t channelCount;

int spi_handle;
const char *devicePath = "/dev/spidev0.0";			//device path     could change depending on...

uint8_t mode = SPI_MODE_0;
/*  spiTransfer = {
            .tx_buf = (uint64_t)&bytesIn,
            .rx_buf = (uint64_t)&bytesOut,
            .len = bytes,
            .speed_hz = speed,
            .delay_usecs = 0,
            .bits_per_word = bitsPerWord,
        }; */

uint8_t bitsPerWord = 8;

//default global values:
uint32_t gSpeed = 90000000;
uint32_t gBytes = 20;
uint32_t gRepeats = 1000;
uint16_t gDelay = 0;



//speed:
//RASPI 4 max: 200.000.000 (selftest)
//RASPI 5 max: 90.000.000 (selftest)

// maximum: 10.000.000		--> transfer rate of  931,51 kbytes/s	(only for selftest)
                                // mcp6201
                                // 1.000.000 => 24 kHz DAQ at 3.3V
                                //  => 80 kHz      seems to be the sweet spot for 3.3V
                                // 10.000.000 => 100 kHz seems to work, but is already a bit strange
                                // 20.000.000 => strongly reduced intensity, not really more datapoints acquired

struct spi_ioc_transfer spiTransfer;		//has to be initialized to zeros!
uint8_t *grxPtr = NULL;                     //pointer to spiTransfer receive buffer
uint8_t *gtxPtr = NULL;                     //pointer to spiTransfer transmit buffer

void loadConfig( void );

void stressTest( void );

void speedTest_ioctl( int repeats );
