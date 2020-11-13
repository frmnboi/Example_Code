//to compile, run "gcc -g mainv2.c -o mainv2.o -lm"
//to run, run "./mainv2.o"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define BUFFERSIZE 5000
//these numbers may need to reduce, as more memory will need to be allocated for program running
#define DATASERIESSIZE 166666
#define NUMHYDROPHONES 3
//how many datapoints in each hydrophone buffer is filled before a thresholding check is performed
#define BUFFERREFRESHCHECK 100
#define MOVINGWINDOWSIZE 10
#define ADCFREQ 3.6 //samples per microsecond (microseconds to keep number small to prevent overflow)


//for readability, a macro or inline function might be made to simply run over all 3 hydrophones, because that for-loop appears to often in the code.


//the STM32 controller has 32MB of ram

//1e6 *8 _bits/3_channels/3.6e6_samples_per_second/16_bits_per_word = length of time that can be sampled
//30e3 (frequency of pulse)*length of time sampled=1388.8 oscillations minimum
//this works

struct noisefloor
{
    double average;
    double stdev;
};

//if this is ever converted to c++, make this a class
struct hydrophone
{
    unsigned char number;
    struct noisefloor newfloor;
    struct noisefloor oldfloor;
    unsigned short timeseries[BUFFERSIZE + DATASERIESSIZE]; //contains both the ring buffer and rest of data-series.
    unsigned int buffcounter;
    unsigned long timeseriescounter; //note, that an int may be used.  Also, direct pointer access also can produce a solution
    unsigned long pulsestart; //the beginning of the pulse
};


struct hydrophone hydrophones[3];

//function definitions
static void determineNoiseFloor(struct hydrophone *hydrophone);
static inline double mean(unsigned short *arr, int arraysize, unsigned int sampleperiod);
static inline double SD(unsigned short *arr, double mean, int arraysize, unsigned int sampleperiod);
static inline void updateBuffer(struct hydrophone *hydrophone);
static inline void updateData(struct hydrophone *hydrophone);
static void reorderData(struct hydrophone *hydrophone);
static void findStartPoint(struct hydrophone *hydrophone);

int main()
{
    //assign the hydrophone numbers (may not be neccisary)
    for (int i = 0; i < NUMHYDROPHONES; i++)
    {
        hydrophones[i].number = i + 1;
    }

    //External loop for data acquisition
    for (;;)
    {

        //reset to get ready for next pulse
        for (int j = 0; j < NUMHYDROPHONES; j++)
        {
            hydrophones[j].timeseriescounter = BUFFERSIZE;
            hydrophones[j].buffcounter = 0;
        }

        //polling on hydrophone
        do
        {
            //buffers need to be taken for all hydrophones, not just a single one, and as close to syncronously as possible.

            for (int j = 0; j < NUMHYDROPHONES; j++)
            {
                // newfloor=oldfloor; //note this is only a shallow copy and undesirable
                hydrophones[j].oldfloor.average = hydrophones[j].newfloor.average;
                hydrophones[j].oldfloor.stdev = hydrophones[j].newfloor.stdev;
            }

            //wait for the buffer to fill up at least partially before checking again(function to fill up buffer array)
            for (int i = 0; i < BUFFERREFRESHCHECK; i++)
            {
                //note, a for each loop here over the hydrophones would be better if it existed.
                for (int j = 0; j < NUMHYDROPHONES; j++)
                {
                    updateBuffer(&hydrophones[j]);
                }
            }
            for (int j = 0; j < NUMHYDROPHONES; j++)
            {
                determineNoiseFloor(&hydrophones[j]);
            }

        } while (!((fabs(hydrophones[0].newfloor.average - hydrophones[0].oldfloor.average) > 2 * hydrophones[0].oldfloor.stdev) || (fabs(hydrophones[1].newfloor.average - hydrophones[1].oldfloor.average) > 2 * hydrophones[1].oldfloor.stdev) || (fabs(hydrophones[2].newfloor.average - hydrophones[2].oldfloor.average) > 2 * hydrophones[2].oldfloor.stdev)));
        //bruh, why do lambdas not exist in c, making me type all this?

        //signal has breached the noisefloor.
        //start filling up the timeseries for each
        for (int i = 0; i < DATASERIESSIZE; i++)
        {
            for (int j = 0; j < NUMHYDROPHONES; j++)
            {
                updateData(&hydrophones[j]);
            }
        }

        //put the data in the correct order
        for (int j = 0; j < NUMHYDROPHONES; j++)
        {
            reorderData(&hydrophones[j]);
        }

        //now process data using a frequency domain technique.
        //inputs: hydrophone->timeseries
        //outputs: hydrophone->timeseries (modified and filtered), frequency of pulse (double)

        //find the starting point for each timepoint 
        for (int j = 0; j < NUMHYDROPHONES; j++)
        {
            findStartPoint(&hydrophones[j]);
        }
    }
}

//thresholding can also be done by checking for outlying values, but checks would need to be more frequent/computationally expensive.
static inline double mean(unsigned short *arr, int arraysize, unsigned int sampleperiod)
{
    long long sum = 0;
    for (int i = 0; i < arraysize; i += sampleperiod)
    {
        sum += arr[i];
    }
    return (double)sum / arraysize;
}

static inline double SD(unsigned short *arr, double mean, int arraysize, unsigned int sampleperiod)
{
    long long sum = 0;
    for (int i = 0; i < arraysize; i += sampleperiod)
    {
        sum += pow(arr[i] - mean, 2);
    }
    return sqrt((double)sum / (arraysize - 1));
}

static void determineNoiseFloor(struct hydrophone *hydrophone)
{
    //note that a smaller sample size, like every other element, can be used instead.
    hydrophone->newfloor.average = mean(hydrophone->timeseries, BUFFERSIZE, 1);
    hydrophone->newfloor.stdev = SD(hydrophone->timeseries, hydrophone->newfloor.average, BUFFERSIZE, 1);
}

static inline void updateBuffer(struct hydrophone *hydrophone)
{
    //updates a ring buffer for each hydrophone.
    //unsigned short newdata=Microcontroller_Library_Function_ADC_Sampler
    unsigned short newdata = 4; //delete this.  I only put this in to allow for compilation temporary.

    hydrophone->timeseries[hydrophone->buffcounter] = newdata; //faster is to assign it with no intermediary
    if (hydrophone->buffcounter < BUFFERSIZE - 1)
    {
        hydrophone->buffcounter++;
    }
    else
    {
        hydrophone->buffcounter = 0;
    }
}

static inline void updateData(struct hydrophone *hydrophone)
{
    //updates a time-series dataset for each hydrophone.
    //unsigned short newdata=Microcontroller_Library_Function_ADC_Sampler
    unsigned short newdata = 4;                                      //delete this.  I only put this in to allow for compilation temporary.
    hydrophone->timeseries[hydrophone->timeseriescounter] = newdata; //faster is to assign it with no intermediary
    hydrophone->timeseriescounter++;
}

static void reorderData(struct hydrophone *hydrophone)
{
    //the data in the buffer is out of order and needs to be reordered before it can be processed.
    //there is a computationally heavy method of swapping locations, that request only 1 memory swap unit
    //there is also a memory intensive unit that is light on cpu and easier to program.
    //I chose the memory intensive method, as although memory is scarce, it is not super scarce and the buffer is small.

    if (hydrophone->buffcounter == 0)
    {
        return;
    }
    int tempbuffsize=BUFFERSIZE - hydrophone->buffcounter;
    unsigned short *oldestdata = (unsigned short *)malloc(sizeof(unsigned short) * tempbuffsize);
    for (int i = hydrophone->buffcounter; i < BUFFERSIZE; i++)
    {
        //grab the oldest data and save it to memory
        oldestdata[i - hydrophone->buffcounter] = hydrophone->timeseries[i];
    }

    //shift the older data to new data spots
    for (int i = hydrophone->buffcounter; i > 0; i--)
    {
        hydrophone->timeseries[tempbuffsize + i] = hydrophone->timeseries[i];
    }

    //write in the memory held in swap
    for (int i = 0; i < tempbuffsize; i++)
    {
        hydrophone->timeseries[i] = oldestdata[i];
    }

    //prevent a memory leak
    free(oldestdata);
}



static void findStartPoint(struct hydrophone *hydrophone)
{
    //zero out the pulse start counter
    hydrophone->pulsestart=0;

    //check over the entire data series
    for (hydrophone->pulsestart;hydrophone->pulsestart<DATASERIESSIZE+BUFFERSIZE-MOVINGWINDOWSIZE;hydrophone->pulsestart++)
    {
        hydrophone->newfloor.average = mean(hydrophone->timeseries+hydrophone->pulsestart, MOVINGWINDOWSIZE, 1);
        hydrophone->newfloor.stdev = SD(hydrophone->timeseries+hydrophone->pulsestart, hydrophone->newfloor.average, MOVINGWINDOWSIZE, 1);

        //can compare the one after the same series as well.
        if (fabs(hydrophone->timeseries[hydrophone->pulsestart+MOVINGWINDOWSIZE] - hydrophones[0].newfloor.average) > 2 * hydrophones[0].newfloor.stdev)
        {
            //if the point is severely abberant, it is probably the start of the ping. (basic conception)
            hydrophone->pulsestart=hydrophone->pulsestart+MOVINGWINDOWSIZE;
        }
    }
}

