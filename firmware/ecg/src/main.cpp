/*******************************************************************************
 *
 *	@brief	MAX30003 ECG
 *
 ********************************************************************************/

/*******************************************************************************
 * Includes
 ********************************************************************************/
#include <Arduino.h>
#include <max30003.h>

/*******************************************************************************
 * Variable declarations
 ********************************************************************************/
max30003 ecg;

const float RTOR_LSB_RES = 0.0078125f;

// D2 on Arduino Nano
const byte int1_pin = 2;
// D3 on Arduino Nano
const byte int2_pin = 3;

/*******************************************************************************
 * Functions
 ********************************************************************************/
// interrupt service routine
volatile bool ecg_int_flag = 0;
void ecg_callback()
{
    ecg_int_flag = 1;
}

void setup()
{
    // setup serial port for plotting
    Serial.begin(115200);

    // setup status LED
    pinMode(LED_BUILTIN, OUTPUT);

    // setup interrupt
    pinMode(int1_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(int1_pin), ecg_callback, CHANGE);

    // setup chipselect pin
    pinMode(MAX30003_CS_PIN, OUTPUT);

    // chip deselect
    digitalWrite(MAX30003_CS_PIN, HIGH);

    // setup SPI
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    // initialize max30003 chip
    ecg.max30003_init();
}

// send data to Protocental GUI
// it works only with ECG Gain 20V/V, otherwise too noisy
void send_data_to_pde_plot(int16_t ecg_sample, uint16_t r_to_r, uint16_t bpm)
{
    uint8_t data_bytes[20];

    data_bytes[0] = 0x0A;
    data_bytes[1] = 0xFA;
    data_bytes[2] = 0x0C;
    data_bytes[3] = 0;
    data_bytes[4] = 0x02;

    data_bytes[5] = ecg_sample >> 24;
    data_bytes[6] = ecg_sample >> 16;
    data_bytes[7] = ecg_sample >> 8;
    data_bytes[8] = ecg_sample;

    data_bytes[9] = r_to_r;
    data_bytes[10] = r_to_r >> 8;
    data_bytes[11] = 0x00;
    data_bytes[12] = 0x00;

    data_bytes[13] = bpm;
    data_bytes[14] = bpm >> 8;
    data_bytes[15] = 0x00;
    data_bytes[16] = 0x00;

    data_bytes[17] = 0x00;
    data_bytes[18] = 0x0b;

    for (int i = 0; i < 19; i++)
    {
        Serial.write(data_bytes[i]);
    }
}

/*******************************************************************************
 * ECG voltage readout for serial plotting
 *
 * R-R is always above 0 at 20V/V ECG Gain in the serial plotter, when no ac
 * noise is present ac noise in the system comes from laptop charger, monitor HDMI
 ********************************************************************************/
const int EINT_STATUS_MASK = 1 << 23;
const int FIFO_OVF_MASK = 0x7;
const int FIFO_VALID_SAMPLE_MASK = 0x0;
const int FIFO_FAST_SAMPLE_MASK = 0x1;
const int ETAG_BITS_MASK = 0x7;

void loop()
{
    // // read ecg data from max30003 fifo
    // uint32_t ecg_fifo = ecg.max30003_read_register(max30003::ECG_FIFO);
    //
    // // shift out ETG[5:3] & PTG[2:0] bits & extract 18 bits data as signed
    // // integer for serial plotting
    // int16_t ecg_sample = ecg_fifo >> 6;
    //
    // // print ecg voltage to display in serial plotter
    // // Serial.println(ecg_sample);
    //
    // // read r-to-r data from max30003
    // uint32_t r_to_r = ecg.max30003_read_register(max30003::RTOR);
    //
    // // extract 14 bits data from r_to_r register
    // r_to_r = ((r_to_r >> 10) & 0x3fff);
    // // Serial.print(r_to_r * 8);
    // // Serial.print(",");
    //
    // // calculate BPM
    // float bpm = 1.0f / (r_to_r * RTOR_LSB_RES / 60.0f);
    // // Serial.println(bpm);
    //
    // // r_to_r must be multiplied by 8 to get the time interval in millisecond
    // // 8ms resolution is for 32768Hz master clock
    // send_data_to_pde_plot(ecg_sample, (uint16_t)r_to_r * 8, (int16_t)bpm);
    //
    // delay(8);

    if (ecg_int_flag)
    {
        ecg_int_flag = 0;
        uint32_t status = ecg.max30003_read_register(max30003::STATUS);

        uint32_t ecg_fifo, sample_count, etag_bits[32];
        int16_t ecg_sample[32];

        if ((status & EINT_STATUS_MASK) == EINT_STATUS_MASK)
        {
            // reset sample counter
            sample_count = 0;

            do
            {
                // read ecg data from max30003 fifo
                ecg_fifo = ecg.max30003_read_register(max30003::ECG_FIFO);

                // shift out ETAG[5:3] & PTAG[2:0] bits & extract 18 bits data
                // as signed integer for serial plotting
                ecg_sample[sample_count] = ecg_fifo >> 8;

                // extract ETAG[5:3] bits
                etag_bits[sample_count] = (ecg_fifo >> 3) & ETAG_BITS_MASK;

                // increase sample count
                sample_count++;

                // if sample was last in FIFO, then exit
            } while (etag_bits[sample_count - 1] == FIFO_VALID_SAMPLE_MASK ||
                     etag_bits[sample_count - 1] == FIFO_FAST_SAMPLE_MASK);

            // if FIFO is overflowed, reset FIFO
            if (etag_bits[sample_count - 1] == FIFO_OVF_MASK)
                ecg.max30003_write_register(max30003::FIFO_RST, 0);

            // Print results
            for (int i = 0; (sample_count > 1 && i < sample_count); i++)
            {
                Serial.println(ecg_sample[i]);
            }
        }
    }
}
