#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <cstdint>      //uint...

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cstring>

#include <fcntl.h>      //open() and O_RDWR
#include <unistd.h>    //close()
#include <string.h>

#include <chrono>

#include <cmath>

using namespace std;

const char *devicePath = "/dev/spidev0.0";
int spi_handle;

struct spi_ioc_transfer spiTransfer;		//has to be initialized to zeros!


int main( int argc, char *argv[] )
{
    cout << "SPI_send 1.0" << endl;
    if( argv[1] == NULL || strcmp(argv[1], "") == 0 )
    {
        cout << "no params" << endl;
    }
    else
    {
        cout << "argument: " << argv[1] << endl;
    }

    spi_handle = open( devicePath, O_RDWR );
    if( spi_handle < 0 )
    {
        cout << "could not open SPI device" << endl;
        return -1;
    }
    else
    {
        cout << "open SPI device: Success!" << endl;
    }

    uint8_t mode = SPI_MODE_3;
    uint8_t bitsPerWord = 8;
    uint32_t speed = 10000;

    // set ioctl parameters to default values
    if( ioctl(spi_handle, SPI_IOC_WR_MODE, &mode) == -1 )
        cout << "error" << endl;
    if( ioctl(spi_handle, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) == -1 )
        cout << "error" << endl;
    if(  ioctl(spi_handle, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1 )
        cout << "error" << endl;


    // set up the test transfer:
	uint32_t ret;
	uint8_t tx[2] = {0xF, };    //10101010
    uint8_t rx[2] = {0, };      //00000000

    //gtxPtr = &tx[0];    //points to first byte
    //grxPtr = &rx[0];

    spiTransfer = {
        .tx_buf = (uint64_t)&tx,    //transmitted
        .rx_buf = (uint64_t)&rx,    //received
        .len = 1,
        .speed_hz = speed,
        .delay_usecs = 0,
        .bits_per_word = bitsPerWord,
    };

    if( ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer) < 1 )
    {
        cout << "transfer fail" << endl;
    }

    cout << "rx1: " << (uint32_t)rx[0] << endl;

    for( int i = 0; i < 1000; i++ )
    {
        if( ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer) < 1 )
        {
            cout << "transfer fail" << endl;
        }
    }

    cout << "rx2: " << (uint32_t)rx[0] << endl;

    return 0;
}
