#ifndef KRI2S_H
#define KRI2S_H

extern void slavei2s_init();
extern void slavei2s_handle();
extern void slavei2s_sendheader();
extern bool slavei2s_buffered_enough();
extern void slavei2s_playchunk();

#endif