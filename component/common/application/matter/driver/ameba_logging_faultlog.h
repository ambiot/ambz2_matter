#ifndef MATTER_AMEBA_FAULTLOG_H
#define MATTER_AMEBA_FAULTLOG_H

#ifdef __cplusplus
extern "C" {
#endif

void read_last_fault_log(void);

#if 1
void write_debug_userlog(char* msg);
#endif

#ifdef __cplusplus
}
#endif

#endif