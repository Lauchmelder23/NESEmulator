#ifndef _LOG_H_
#define _LOG_H_

struct Bus;

void logBusState(struct Bus* bus);

#ifndef NDEBUG
#define LOG_BUS(b) logBusState(b)
#else 
#define LOG_BUS(b)
#endif // NDEBUG

#endif // _LOG_H_