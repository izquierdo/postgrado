#ifndef PREPROC_FLAGS_H_
#define PREPROC_FLAGS_H_

#define TW_DEFAULT_TIME		INT_MAX
#define TW_DEFAULT_MEM		1800

//#define GATHER_STATS_DEBUG
#define GATHER_STATS

#define NDEBUG
//#define DEBUG_BFTW
//#define DEBUG_TW
//#define DEBUG_BREADTHFTW
//#define DEBUG_ALMGRAPH

// enable this to allow some algorithms to accept a graph of any size as input
//#define TW_ANY_VERTS

#define TW_MAX_VERTS		96 // should always be a multiple of 32
// don't change below here
#define TW_MIN_VERTS	        (TW_MAX_VERTS-32)
#define TW_NINTS	        (TW_MAX_VERTS/32)
#define TW_INDEX_MOD		32
#define DDD_MIN_VERTS		TW_MIN_VERTS
#define DDD_MAX_VERTS		TW_MAX_VERTS
#define DDD_HARD_MIN_VERTS	40
#if DDD_MIN_VERTS<40
#undef DDD_MIN_VERTS
#define DDD_MIN_VERTS		DDD_HARD_MIN_VERTS
#undef DDD_MAX_VERTS
#define DDD_MAX_VERTS		(((DDD_MIN_VERTS / 32) + 1)*32)
#endif
// don't change above here

#define DDD_MEMLIMIT_MB		600

#define _FILE_OFFSET_BITS 	64 // enables large file support (>2GB)

#define N_DDD_THREADS		1 // NOTE: values > 1 can seriously slow things down in current implementation

//#define BFHT_UB_AT_EVERY_NODE // define to make bfht compute an ub before expanding each node
//#define TRACK_EXP_BY_FVAL
#define BFHT_FASTGRAPH

#endif /*PREPROC_FLAGS_H_*/
