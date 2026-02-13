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

#define RESP_WAIT 0xA3		// 10100011
#define RESP_ERR 0xAC		// 10101100
#define RESP_ACK 0xA6		// 10100110
#define RESP_DONE 0xA7		// 10100111

#define CMD_ADD 0x55		// 01010101		for debugging

using namespace std;
using namespace std::chrono;

const char *devicePath = "/dev/spidev1.0";
int spi_handle;

uint8_t mode = SPI_MODE_0;
uint8_t bitsPerWord = 8;
uint32_t speed = 10000000; //10000 works fine, 100000 is ok for transfer, but the stm is not fast enough providing data
// 100000 for 8MHz STM32 sysclk
// 1000000 for 72MHz STM32 sysclk
// 10000000 for 72MHz STM32 sysclk with optimised drivers... RESULTING in 30kHz 16 bit transfers (480 kBits per second)
// to go beyond that one needs DMA for sure
//could do a feedback output on some analog line... maybe

static inline uint8_t SPICMD(uint16_t spi)
{
    return (uint8_t)(((spi) >> 8) & 0xFF);
}

static inline uint8_t SPIDATA(uint16_t spi)
{
    return (uint8_t)((spi) & 0xFF);
}

struct spi_ioc_transfer spiTransfer;		//has to be initialized to zeros!

uint16_t simpleTransfer( uint16_t data );
int arrayTransfer( void );
void fetch( void );

int main( int argc, char *argv[] )
{
    char *ptr1;
    char *ptr2;
    long int param1 = 0;
    int repeat = 1;

    cout << "SPI_send 1.0" << endl;
    if( argv[1] == NULL || strcmp(argv[1], "") == 0 )
    {
        cout << "no params" << endl;
    }
    else
    {
        cout << "argument1: " << argv[1] << endl;
        param1 = strtol( argv[1], &ptr1, 0 );
    }
    if( !( argv[2] == NULL || strcmp(argv[2], "") == 0 ) )
    {
        cout << "argument2: " << argv[2] << endl;
        repeat = strtol( argv[2], &ptr2, 0 );
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

    // set ioctl parameters to default values
    if( ioctl(spi_handle, SPI_IOC_WR_MODE, &mode) == -1 )
        cout << "error" << endl;
    if( ioctl(spi_handle, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) == -1 )
        cout << "error" << endl;
    if(  ioctl(spi_handle, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1 )
        cout << "error" << endl;



    if( strcmp( argv[1], "fetch" ) == 0 )
    {
        fetch();
    }
    else
    {
        uint16_t *results;
        results = (uint16_t *)malloc( sizeof( uint16_t ) * repeat );


        auto begin = std::chrono::high_resolution_clock::now();
        for( int i = 0; i < repeat; i++ )
        {
            results[i] = simpleTransfer( param1 );
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(end-begin).count();

        cout << "16bit msg: 0x" << hex << results[0] << endl;
        cout << "16bit msg: 0x" << hex << results[1] << endl;
        cout << "16bit msg: 0x" << hex << results[2] << endl;
        for( int i = 3; i < repeat; i++ )
        {
            cout << "16bit msg: 0x" << hex << results[i] << " diff: " << dec << results[i]-results[i-1] << endl;
        }

        cout << "total duration: " << dec << duration << " ns" << endl;
        cout << "transfer duration: " << duration/repeat << " ns" << endl;
    	cout << "send frequency: " << (double)repeat/(double)duration*(double)1000000 << " kHz" << endl;

    }

    return 0;
}

uint16_t simpleTransfer( uint16_t data )
{
    // set up the test transfer:
	uint32_t ret;
	uint8_t tx[2] = {(data >> 8), (uint8_t)data };    //10011001
    uint8_t rx[2] = {0, 0};      //00000000

    //gtxPtr = &tx[0];    //points to first byte
    //grxPtr = &rx[0];
    //cout << "TRANSMIT: " << endl;
    //cout << "tx[0]: 0x" << hex << (uint32_t)tx[0] << endl;
    //cout << "tx[1]: 0x" << hex << (uint32_t)tx[1] << endl;

    spiTransfer = {
        .tx_buf = (uint64_t)&tx,    //transmitted
        .rx_buf = (uint64_t)&rx,    //received
        .len = 2,
        .speed_hz = speed,
        .delay_usecs = 0,
        .bits_per_word = bitsPerWord,
    };

    if( ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer) < 1 )
    {
        cout << "transfer fail" << endl;
    }

    uint16_t highbits = rx[0];
    uint16_t lowbits = rx[1];
    uint16_t result = (highbits << 8) + lowbits;

    return result;

}

void fetch( void )
{
    int errorCount = 0;
    uint16_t response = 0;

    response = simpleTransfer( 0x6000 );    //ask if data is ready
    response = simpleTransfer( 0x5000 );    //fetch reply
    if( SPIDATA( response ) == 0 )
    {
        cout << "no data ready!" << endl;
        return;
    }

    response = simpleTransfer( 0x6200 );   //request
    response = simpleTransfer( 0x5000 );   //check that this is an acknowledgement:
    switch( SPICMD(response) )
    {
        case RESP_ERR:
            cout << "ERR!" << endl;
            return;
        case RESP_WAIT:
            cout << "WAIT!" << endl;
            return;
        case RESP_ACK:
            cout << "ACK!" << endl;
            break;
    }
    int repeats = simpleTransfer( 0x5000 ); //size (can occupy full 16 bits)
    cout << "size: " << dec << repeats << endl;

    //buffer
    uint16_t *results;
    results = (uint16_t *)malloc( sizeof( uint16_t ) * repeats );

    auto begin = std::chrono::high_resolution_clock::now();
    for( int i = 0; i < repeats; i++ )
    {
        results[i] = simpleTransfer( 0x5000 );  //query
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end-begin).count();

    uint16_t previous = 0;

    for( int i = 0; i < repeats; i++ )
    {
        cout << "16bit msg: 0x" << hex << results[i] << " diff: " << dec << results[i] - previous << endl;
        if( (results[i] - previous) != 1 )
            errorCount++;

        previous = results[i];
    }
    cout << "error count: " << dec << errorCount << endl;       //only works if data is continuous 12345... for testing
    cout << "total duration: " << dec << duration << " ns" << endl;
    cout << "transfer duration: " << duration/repeats << " ns" << endl;
    cout << "send frequency: " << (double)repeats/(double)duration*(double)1000000 << " kHz" << endl;
}


int arrayTransfer( void )
{
    simpleTransfer( 0x6200 );
    uint16_t length = simpleTransfer( 0x5000 );
    length = simpleTransfer( 0x5000 );
    cout << "length: " << dec << length << endl << endl;


    if( true )
    {
        for( int i = 0; i < length; i++ )
        {
            uint16_t result = simpleTransfer( 0x5000 );
            //cout << dec << result << endl;
        }
    }
    else    //this might only work with DMA:
    {
        uint8_t *tx = NULL;
        uint8_t *rx = NULL;
        uint16_t *rx16;

        tx = (uint8_t*)malloc( length * sizeof(uint16_t) );
        rx = (uint8_t*)malloc( length * sizeof(uint16_t) );
        rx16 = (uint16_t*)rx;

        spiTransfer = {
            .tx_buf = (uint64_t)&tx,    //transmitted
            .rx_buf = (uint64_t)&rx,    //received
            .len = 2*length,
            .speed_hz = speed,
            .delay_usecs = 0,
            .bits_per_word = bitsPerWord,
        };

        if( ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer) < 1 )
        {
            cout << "transfer fail" << endl;
        }

        for( int i = 0; i < length; i++ )
        {
            cout << "0x" << hex << rx16[i] << endl;
        }
    }

    //uint16_t last = simpleTransfer( 0x5000 );
    //cout << "ack: " << hex << last << endl;
}
