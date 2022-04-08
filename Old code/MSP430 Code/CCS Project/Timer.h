#ifndef TIMER_H_
#define TIMER_H_


void timerInitilization();

void startTimerA0(int endValue);
void stopTimerA0();
void startTimerA1(int endValue);
void stopTimerA1();
void startTimerA4(int endValue);
void stopTimerA4();
void startTimerB0(int endValue);
void stopTimerB0();

void handlerTimerA0();
void handlerTimerA1();
void handlerTimerA4();
void handlerTimerB0();


#endif
