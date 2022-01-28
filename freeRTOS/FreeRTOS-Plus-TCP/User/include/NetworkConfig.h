#ifndef NETINFOCONFIG__H
#define NETINFOCONFIG__H

BaseType_t IP_init( void );

/*
equal to
FreeRTOS_printf(...)
FreeRTOS_debug_printf(...)
*/
int lUDPLoggingPrintf( const char *fmt, ... );

#endif /*NETINFOCONFIG__H*/