#include <stdint.h>

/* tsadc.h
 * Library for accessing TS-ADC16 from TS7xxx PC/104 SBCs
 * Users should only need to know about the function definitions that appear here.
 * The following assumptions are made:
 * - There is exactly one TS-ADC16 board in the stack.
 * - Jumper 3 is installed on the TS-ADC16.
 * - Processes running this code have root privileges.
 */

/* Exactly one of the following should be defined.
 * for 7200, 7250, 7260 or 7300: #define TS7200
 * for 7350 or 7370: #define TS7350
 * for 7800: #define TS7800
 */

/*
 * These are the supported SBC's
 */

typedef enum {
    SBC_NONE,
    SBC_TS7200,
    SBC_TS7350,
    SBC_TS7800
} sbctype;

typedef enum {
    TS_ADCINVALID = 0,
    TS_ADC16 = 0x3E,
    TS_ADC24
} adctype;

typedef enum {
    ADCRANGE_55S   = 0x0100,  // ADC16: -5 to +5V single ended
    ADCRANGE_05S   = 0x0140,  // ADC16: 0 to +5V  single ended
    ADCRANGE_1010S = 0x0180,  // ADC16: -10 to +10V single ended
    ADCRANGE_0010S = 0x01e0,  // ADC16: 0 to +10V single ended
    ADCRANGE_55D   = 0x0000,  // ADC16: -5 to +5V, differential
    ADCRANGE_05D   = 0x0040,  // ADC16: 0 to +5V (default) differential
    ADCRANGE_1010D = 0x0090,  // ADC16: -10 to +10V differential
    ADCRANGE_0010D = 0x00e0,  // ADC16: 0 to +10V differential

    ADCRANGE_0VS   = 0x0100,  // ADC24: 0 to +VRef (single ended)
    ADCRANGE_02VS  = 0x0140,  // ADC24: 0 to +2xVRef (single ended)
    ADCRANGE_02VAS = 0x0180,  // ADC24: 0 to +2xVRef (single ended)
    ADCRANGE_02VBS = 0x01e0,  // ADC24: 0 to +2xVRef (single ended)
    ADCRANGE_0VD   = 0x0000,  // ADC24: 0 to +VRef (differential)
    ADCRANGE_02VD  = 0x0040,  // ADC24: 0 to +2xVRef (differential)
    ADCRANGE_02VAD = 0x0080,  // ADC24: 0 to +2xVRef (differential)
    ADCRANGE_02VBD = 0x00e0   // ADC24: 0 to +2xVRef (differential)
} adcrange;

typedef enum {
    DACRANGE_0VU   = 0x0000,  // ADC16: 0 to vRef, unbuffered
    DACRANGE_0VB   = 0x1000,  // ADC16: 0 to vRef, buffered
    DACRANGE_02VU  = 0x2000,  // ADC16: 0 to 2 * vRef, unbuffered
    DACRANGE_02VB  = 0x3000   // ADC16: 0 to 2 * vRef, buffered
} dacrefrange;

typedef enum {
    ADC_BOARD0 = 0,
    ADC_BOARD1,
    ADC_BOARD2,
    ADC_BOARD3,
    ADC_BOARDMAX
} BoardIndex;

typedef enum {
    ADC_COUNTER0 = 0,
    ADC_COUNTER1,
    ADC_COUNTER2,
    ADC_COUNTER3,
    ADC_COUNTERMAX
} CounterIndex;

typedef enum {
    DAC0 = 0,
    DAC1,
    DAC2,
    DAC3,
    DACMAX
} DacIndex;

typedef enum {
    ADC_ACQUIRE,     //  Acquire data from 1 or 2 specific channels
    ADC_SAMPLE       //  Acquire bulk data from multiple channels
} AcquisutuinType;

typedef enum { false = 0, true } bool; //FALSE = 0, TRUE, 

/* After initialization, these macros are used to access hardware
 * registers.  ADC(base, x) where x ranges from 0 to 0x1E will access the 
 * TS-ADC16 control registers as defined in the getting started sheet.
 * ADC(base, x) is for 16 bit access.  For certain operations 8 bit access
 * is required, hence ADC8(base, x).
 */
#define ADC(base, index)    (*(base->io16 + (index)/sizeof(uint16_t)))
#define ADC8(base, index)   (*(base->io8 + (index)))
#define CONFIG(index) (*(allAdc.Io16 + (index)/sizeof(uint16_t))) 
/*
 *   tsadcinstance is a global variable used to hold pointers into 
 *   memory space & control and state information.
 */
typedef struct {
    unsigned operational:1;          /* 1 = operational, 0 = uninitialized */
    unsigned irq7:1;                 /* 1 = IRQ7, 0 = IRQ6 */
    unsigned twosComplementResult:1;  /* 1 = Sign extend & twos complemenet the result if 24 bit & set */
    unsigned adc16:1;                /* 1 = adc16, 0 = adc24 */

    uint16_t chana, chanb;
    int16_t cnt1, cnt2;
    int16_t *data1, *data2;
    int16_t **arrayData;
    int32_t sampleCnt, wSampleCnt;
    int16_t *sData1, *sData2, sSampleCnt;  //  Initial data for post acquisition fixup if ADC24 (12 bit)
    int16_t **dataArray;
    uint32_t sampleRate;
    uint32_t aggregateRate;
    uint32_t pollSleepTime;
    adctype whichadcboard;
    AcquisutuinType howAcquire;
    volatile unsigned short *io16; /* pointer to adc board io space */
    volatile unsigned char  *io8;  /* 8 bit access is required for certain operations */
} boarddata;

typedef struct {
    volatile uint16_t *Io16;  /* pointer to adc board 16 bit io space */
    volatile uint8_t  *Io8;   /* pointer to adc board 8 bit io space */
    volatile off_t IoBase8;
    volatile off_t IoBase16;
    uint16_t IoOffset;
    sbctype theSbc;            /* the type of SBC this is running on */
    uint16_t theMaxChannelRequest;
    uint16_t theMaxChannel;
    uint32_t theMaxBufferSize;
    uint32_t theTotalBufferSize;
    uint32_t fifoWorkCnt;
    uint8_t *fifoChannelBasePtr;
    uint8_t *fifoChannelWorkPtr;
    uint16_t *fifoDataBasePtr;
    size_t fifoDataBaseCnt;
    size_t fifoChannelBaseCnt;
    uint16_t *fifoDataWorkPtr;
    int memoryFd;     /* file descriptor used to open /dev/mem  */
#ifdef USING_IRQS
    int Irq6FileHandle;  /* file descriptor used to open /proc/irq/IRQ6 */
    int Irq7FileHandle;  /* file descriptor used to open /proc/irq/IRQ7 */
    uint8_t irqstring6[20];
    uint8_t irqstring7[20];
#endif
    unsigned startAcquisition:1;
    boarddata boards[ADC_BOARDMAX];
} tsadcinstance;

/*
 * Initialization error return codes
 */

#define TSADCSTATUS_NODATA     0
#define TSADCERR_CANTOPENMEM  -1
#define TSADCERR_CANTMAPIO16  -2
#define TSADCERR_CANTMAPIO8   -3
#define TSADCERR_NOINIT       -4
#define TSADCERR_HARDWARE     -5
#define TSADCERR_BADPARAMETER -6
#define TSADCERR_BADBOARD     -7
#define TSADCERR_CANTOPENIRQ6 -8
#define TSADCERR_CANTOPENIRQ7 -9
#define TSADCERR_FIFOOVERRUN  -10
#define TSADCERR_OUTOFMEMORY  -11
#define TSADCERR_MAXERROR     -12

#define BOARD_SPACING   0x20   /* Boards @ 0x100, 0x120, 0x140, 0x160 */

#define JP4 0x80
#define JP3 0x40
#define JP2 0x20
#define JP1 0x10

#define BOARD_ID      0x00
#define JUMPER_MASK   0xf000
#define JUMPER_SHIFT  12
#define PLDREV_MASK   0x0f00
#define PLDREV_SHIFT  8

#define BOARDID_INDEX      0x00
#define FPGA_CONFIGURATION 0x02
#define DELAY_MSB          0x04
#define DELAY_LSB          0x06
#define ADC_STATUS         0x08
#define FIFO_READ          0x0A
#define DIO_REG            0x18

#define COUNTER_REG_BASE (0x10)
#define DAC_REG_BASE (0x0e)

#define DIO_REG_BIT (0x10) 
#define FIFO_CHANNEL(base) ((ADC(base, ADC_STATUS) >> 1) & 0x1f)
#define FIFO_COUNT(base) (ADC(base, ADC_STATUS) >> 6)
#define IRQ_ENABLE_BIT 0x0001
#define INTERRUPT_ENABLE(base)  (ADC(base, ADC_STATUS) |= IRQ_ENABLE_BIT)
#define INTERRUPT_DISABLE(base) (ADC(base, ADC_STATUS) &= (~IRQ_ENABLE_BIT))

#define FPGA_CLOCK 32000000
#define FPGA_RUNNING 0x01
#define FIFO_HALFFULL  0x4000  // OR 0x40

#define FIFO_SIZE 512


/* tsadclib1624_sample samples all channels up to max_chan and stores the accumulated
 * results in data[].  data[] must be an array of length 2*(max_chan+1) or
 * longer.  (If longer, elements beyond the required length will not be
 * touched.)  Each channel will be sampled at s_rate MHz if possible, otherwise
 * as fast as possible.  For each channel, the first n_discard values will be
 * discarded, and then the next n_accum values will be added up.  Thus each
 * value returned in data[] should be interpreted in a range from 0 to 
 * (0x10000)*(n_accum).  This function currently assumes a low number of 
 * samples per channel are needed.  Specifically, it is required that 
 * (n_discard + n_accum) * 2*(max_chan + 1) <= 512.
 * max_chan ranges from 0 to 7.
 */
int16_t tsadclib1624_sample(BoardIndex theBoard, int16_t maxChannel, int32_t sampleRate, int16_t sampleCnt, adcrange range, int16_t *channelBuffers[], bool contineOperation);

/* tsadclib1624_startAcquisution will attempt to acquire sampleCnt samples at sampleRate Hz.
 * It will return the number of samples actually acquired.
 * This function reads either a pair of channels or a single channel if data2 is set to NULL
 */
int16_t tsadclib1624_startAcquisution(BoardIndex theBoard, int8_t channel, uint32_t sampleRate, int16_t sampleCnt, adcrange range, uint16_t *data1, uint16_t *data2, bool contineOperation);

int32_t tsadclib1624_pollResults(BoardIndex theBoard, bool blocking);
int32_t tsadclib1624_bufferCleanup(BoardIndex theBoard);

/* tsadclib16_dacset sets a new analog output value.
 * theDac range is {0..3}
 * the reference and range (referenceRange) should be from the
 * enumeration dacrefrange in this file
 * value range is {0x0000..0x0fff}
 */
int16_t tsadclib16_dacset(BoardIndex theBoard, uint8_t theDac, dacrefrange referenceRange, uint16_t value);

/* tsadclib16_counterIn returns the current value stored in a counter
 * on the s3elected board.  The counter_id must be {0..3}
 */
uint16_t tsadclib16_counterIn(BoardIndex theBoard, CounterIndex theCounter);

/* tsadclib16_digitalIn returns an unsigned char value where bits 0-3 report the 
 * DIO values for the selected board (theBoard)
 */
uint8_t tsadclib16_digitalIn(BoardIndex theBoard);

/* tsadclib16_digitalOut toggles the digital output pin
 * output should be 0 or 1
 */
int tsadclib16_digitalOut(BoardIndex theBoard, bool state);

/* tsadclib1624_init: initializes the TS-ADC16, and if necessary configures the PC/104
 * port correctly.
 * It returns 0 if the operation is successful, otherwise -1.
 * Using it is currently optional, because if the adc is not initialized then
 * any of the functions above will automatically initialize it.
 */
int tsadclib1624_init(sbctype sbc, uint16_t maxChannel, uint32_t maxBufferSize);

adctype tsadclib1624_boardstatus(BoardIndex theBoard);

uint8_t *tsadclib1624_errors(int errorCode);

void  tsadclib1624_verbose(bool Verbose);

bool tsadclib1624_buffers(void);

