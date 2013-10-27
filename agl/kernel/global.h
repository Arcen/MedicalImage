////////////////////////////////////////////////////////////////////////////////
// Global Variable Definition

#ifdef __GLOBAL__

#define GLOBAL
#define GLOBALINITIAL(value)	= (value)

#else

#define GLOBAL	extern
#define GLOBALINITIAL(value)

#endif
