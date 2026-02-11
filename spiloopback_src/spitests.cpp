#include "spitests.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cstring>

using namespace std;
using namespace std::chrono;

#define RP1_SPI0_BASE 0x1f00050000


int main( void )
{
    cout << "SPI_loopback 1.0" << endl;

    loadConfig();

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

    //INIT:
    // set ioctl parameters to default values
    if( ioctl(spi_handle, SPI_IOC_WR_MODE, &mode) == -1 )
        cout << "error" << endl;
    if( ioctl(spi_handle, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) == -1 )
        cout << "error" << endl;
    if(  ioctl(spi_handle, SPI_IOC_WR_MAX_SPEED_HZ, &gSpeed) == -1 )
        cout << "error" << endl;


    stressTest();

    close(spi_handle);
    return 0;
}

void loadConfig( void )
{
    //load config file if present and overwrite default global values:
    ifstream configFile;
    configFile.open("spiloopback.conf", ifstream::in);
    if ( configFile.is_open() )
    {
        string lineString, token, value;
        while ( configFile.good() )
        {
            getline(configFile, lineString); //fetches one line from file
            stringstream lineStream(lineString);

            getline(lineStream, token, '=');
            getline(lineStream, value);
            if ( token == "freq")
            {
                gSpeed = stoi(value);
                if( gSpeed >= 1000 )
                {
                    cout << "Frequency: " << gSpeed/1000 << " kHz" << endl;
                }
                else
                {
                    cout << "Frequency: " << value << " Hz" << endl;
                }

                gSpeed = stoi(value);
            }
            else if( token == "repeat")
            {
                cout << "Repeats: " << value << endl;
                gRepeats = stoi(value);
            }
            else if( token == "bytes")
            {
                cout << "# of bytes: " << value << endl;
                gBytes = stoi(value);
            }
        }
    }
}


void stressTest( void )
{
	cout << "*** spi_adc::test() start testing..." << endl;

    // set up the test transfer:
	uint32_t ret;
	uint8_t tx[ gBytes ];
    uint8_t rx[ gBytes ] = {0, };

    gtxPtr = &tx[0];    //points to first byte
    grxPtr = &rx[0];

    // randomize transmission once:
    mt19937 rng;
    rng.seed(42);

    uniform_int_distribution<uint8_t> uint_dist255(0,255);
    for (size_t i = 0; i < gBytes; i++)
    {
        tx[i] = (uint8_t)uint_dist255(rng);
    }

    spiTransfer = {
        .tx_buf = (uint64_t)&tx,    //transmitted
        .rx_buf = (uint64_t)&rx,    //received
        .len = gBytes,
        .speed_hz = gSpeed,
        .delay_usecs = gDelay,
        .bits_per_word = bitsPerWord,
    };
/*
    // singular test:
	ret = ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer);
	if (ret < 1)
    {
        cout << "can't send spi message" << endl;
        return;
    }

    cout << "transmitted: ";
    for (ret = 0; ret < gBytes; ret++)
	{
		if (!(ret % 6))
				cout << endl;
		cout << hex << setfill('0') << setw(2) << static_cast<int>(tx[ret]) << " ";
		//printf("%.2X ", rx[ret]);
	}
    cout << endl;

	cout << "received once: ";
	for (ret = 0; ret < gBytes; ret++)
	{
		if (!(ret % 6))
				cout << endl;
		cout << hex << setfill('0') << setw(2) << static_cast<int>(rx[ret]) << " ";
		//printf("%.2X ", rx[ret]);
	}
    cout << endl << endl;
    */


    // using spiTransfer
    speedTest_ioctl(gRepeats);


	cout << "*** spi_adc::test() done testing..." << endl;
}


// using spiTransfer
void speedTest_ioctl( int repeats )
{
    uint8_t received_all[repeats*gBytes];
    uint32_t failcount = 0;

	auto begin = std::chrono::high_resolution_clock::now();
	for( int i = 0; i < repeats; i++ )
	{
        if( ioctl(spi_handle, SPI_IOC_MESSAGE(1), &spiTransfer) < 1 )
		{
			failcount++;
		}
        memcpy(&received_all[i*gBytes], grxPtr, gBytes);    //should take almost no time
    //    auto test = std::chrono::high_resolution_clock::now();
    //    cout << 1;
    }
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<nanoseconds>(end-begin).count();

	float mean = 0;
	float variance = 0;
	float sigma  = 0;


    for (size_t i = 0; i < repeats; i++)
    {
        for (size_t j = 0; j < gBytes; j++)
        {
            if( received_all[i*gBytes + j] != gtxPtr[j] )
            failcount++;
        }
    }

    cout << "total duration: " << dec << duration << " ns" << endl;
	cout << "transfer duration: " << duration/repeats << " ns" << endl;
	cout << "send frequency: " << (double)repeats/(double)duration*(double)1000000 << " kHz" << endl;
    cout << "byte rate: " << (double)gBytes * (double)repeats/(double)duration*(double)1000000 << " kBytes/s" << endl;
    cout << "bit rate: " << 8 * (double)gBytes * (double)repeats/(double)duration*(double)1000000 << " kits/s" << endl;
	cout << "failcount: " << failcount << " of " << repeats*gBytes << endl;



/*	for( int i = 0; i < repeats; i++ )
	{
		mean += (float)dataset[i]/repeats;
	}
	for( int i = 0; i < repeats; i++ )
	{
		variance += ((float)dataset[i] - mean)*(dataset[i] - mean);
	}
	sigma = sqrt(variance/(repeats-1));

	cout << "mean value: " << mean << endl;
	cout << "sigma value: " << sigma << endl;*/
}
