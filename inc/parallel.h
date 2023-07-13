#ifndef __PARALLEL_H__
#define __PARALLEL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct ParallelTimings {
    uint32_t set_address_wait;
    uint32_t set_data_wait;
    uint32_t ce_pulse_width;
    uint32_t min_write_time;
};

struct ParallelControlSignals {
    bool CE_active_low;
    bool OE_active_low;
    bool WE_active_low;
};

void parallelWrite(uint32_t address, uint8_t *buf, size_t count);
void parallelRead(uint32_t address, uint8_t *buf, size_t count);
void setParallelTimings(struct ParallelTimings *t);
void setParallelControlSignals(struct ParallelControlSignals *cs);

#endif /* __PARALLEL_H__ */

