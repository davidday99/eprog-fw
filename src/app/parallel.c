#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "parallel.h"
#include "programmer.h"

static struct ParallelTimings Timings;
static struct ParallelControlSignals ControlSignals;

extern Programmer Prog;

void set_address(Programmer *prog, uint16_t address);
void set_data(Programmer *prog, uint8_t value);
uint8_t get_data(Programmer *prog);
void assert_CE(Programmer *prog);
void deassert_CE(Programmer *prog);
void assert_WE(Programmer *prog);
void deassert_WE(Programmer *prog);
void assert_OE(Programmer *prog);
void deassert_OE(Programmer *prog);
static void enable_io_as_input(Programmer *prog);
static void enable_io_as_output(Programmer *prog);


void DelayMicro(unsigned long i) {
    SysCtlDelay(i * 80);
}

void setParallelTimings(struct ParallelTimings *t) {
    Timings.set_address_wait = t->set_address_wait;
    Timings.set_data_wait = t->set_data_wait;
    Timings.ce_pulse_width = t->ce_pulse_width;
    Timings.min_write_time = t->min_write_time;
}

void setParallelControlSignals(struct ParallelControlSignals *cs) {
    ControlSignals.CE_active_low = cs->CE_active_low;
    ControlSignals.OE_active_low = cs->OE_active_low;
    ControlSignals.WE_active_low = cs->WE_active_low;
}

void parallelWrite(uint32_t address, uint8_t *buf, size_t count) {
    /*
     * For a general parallel EEPROM, the write process is as follows:
     * 1. Disable OE.
     * 2. Enable WE.
     * 3. Set address lines.
     * 4. Set data lines.
     * 5. Pulse CE.
     * 6. Increment address and repeat from step 3 until all bytes are written.
     * 7. Disable WE and wait the minimum write time. Writing may take 
     *    longer than the minimum, so some form of completion polling
     *    should be used if supported.
     */
    ControlSignals.OE_active_low ? assert_OE(&Prog) : deassert_OE(&Prog);
    ControlSignals.WE_active_low ? deassert_WE(&Prog) : assert_WE(&Prog);
    enable_io_as_output(&Prog);
    for (size_t i = 0; i < count; i++) {
        set_address(&Prog, address + i);
        DelayMicro(Timings.set_address_wait);       
        set_data(&Prog, buf[i]);
        DelayMicro(Timings.set_data_wait);
        ControlSignals.CE_active_low ? deassert_CE(&Prog) : assert_CE(&Prog);
        DelayMicro(Timings.ce_pulse_width); 
        ControlSignals.CE_active_low ? assert_CE(&Prog) : deassert_CE(&Prog);
    }
    ControlSignals.WE_active_low ? assert_WE(&Prog) : deassert_WE(&Prog);
    DelayMicro(Timings.min_write_time);
}

void parallelRead(uint32_t address, uint8_t *buf, size_t count) {
    /*
     * For a general parallel EEPROM, the read pcoess is as follows:
     * 1. Disable WE.
     * 2. Enable OE.
     * 3. Enable CE.
     * 4. Set address lines.
     * 5. Wait designated delay.
     * 5. Read data lines. 
     * 6. Increment address and repeat from step 4 until all bytes are read.
     * 7. Disable CE.
     * 8. Disable OE.
     *
     */
    enable_io_as_input(&Prog);
    ControlSignals.WE_active_low ? assert_WE(&Prog) : deassert_WE(&Prog);
    ControlSignals.OE_active_low ? deassert_OE(&Prog) : assert_OE(&Prog);
    ControlSignals.CE_active_low ? deassert_CE(&Prog) : assert_CE(&Prog);
    for (size_t i = 0; i < count; i++) {
        set_address(&Prog, address + i);
        DelayMicro(Timings.set_address_wait); 
        buf[i] = get_data(&Prog);
    }
    ControlSignals.CE_active_low ? assert_CE(&Prog) : deassert_CE(&Prog);
    ControlSignals.OE_active_low ? assert_OE(&Prog) : deassert_OE(&Prog);
}

void set_address(Programmer *prog, uint16_t address) {
    for (int i = 0; i < ADDRESS_WIDTH; i++) {
        GPIOPinWrite(prog->A[i].port, prog->A[i].pin, address & 1 ? prog->A[i].pin : 0);
        address >>= 1;
    }
}

void set_data(Programmer *prog, uint8_t value) {
    for (int i = 0; i < DATA_WIDTH; i++) {
        GPIOPinWrite(prog->IO[i].port, prog->IO[i].pin, (value & 1) ? prog->IO[i].pin : 0);
        value >>= 1;
    }
}

uint8_t get_data(Programmer *prog) {
    uint8_t data = 0;
    for (int i = 0; i < DATA_WIDTH; i++) {
        data |= (GPIOPinRead(prog->IO[i].port, prog->IO[i].pin) ? 1 : 0) << i;
    }
    return data;
}

void assert_CE(Programmer *prog) {
    GPIOPinWrite(prog->CEn.port, prog->CEn.pin, prog->CEn.pin);
}

void deassert_CE(Programmer *prog) {
    GPIOPinWrite(prog->CEn.port, prog->CEn.pin, 0); 
}

void assert_OE(Programmer *prog) {
    GPIOPinWrite(prog->OEn.port, prog->OEn.pin, prog->OEn.pin);
}

void deassert_OE(Programmer *prog) {
    GPIOPinWrite(prog->OEn.port, prog->OEn.pin, 0); 
}

void assert_WE(Programmer *prog) {
    GPIOPinWrite(prog->WEn.port, prog->WEn.pin, prog->WEn.pin);
}

void deassert_WE(Programmer *prog) {
    GPIOPinWrite(prog->WEn.port, prog->WEn.pin, 0);
}

static void enable_io_as_input(Programmer *prog) {
    for (int i = 0; i < DATA_WIDTH; i++) {
        GPIOPinTypeGPIOInput(prog->IO[i].port, prog->IO[i].pin);
    }
}

static void enable_io_as_output(Programmer *prog) {
    for (int i = 0; i < DATA_WIDTH; i++) {
        GPIOPinTypeGPIOOutput(prog->IO[i].port, prog->IO[i].pin);
    }
}

