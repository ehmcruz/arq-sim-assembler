#ifndef __ASM_H__
#define __ASM_H__

#define DEBUG

#ifdef DEBUG
	#define dprintf(...) printf(__VA_ARGS__)
#else
	#define dprintf(...)
#endif

#define MAX_LABEL_LENGTH 128

#endif
