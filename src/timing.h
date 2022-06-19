/*
 * timing.h
 *
 *  Created on: 22. 9. 2021
 *      Author: zmatkar
 */

#ifndef SRC_TIMING_H_
#define SRC_TIMING_H_

typedef struct{
    uint64_t start;
    uint64_t elapsed;
}timing_t;

void TimingInit(void);
void TimingStart(timing_t* s);
void TimingEnd(timing_t* s);

#endif /* SRC_TIMING_H_ */
