#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define POLL_MS 15
#define ENABLE_PROTOTYPE

#ifdef ENABLE_PROTOTYPE
    #define DEVICE_NAME "prototype"
#else /* ENABLE_PROTOTYPE */
    #define DEVICE_NAME "M416D"
#endif /* ENABLE_PROTOTYPE */

#endif /* _SETTINGS_H_ */