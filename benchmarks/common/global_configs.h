#ifndef GLOBAL_CONFIGS
#define GLOBAL_CONFIGS

#define uint unsigned int

#define VLEN 128
#define SEW  64
#define XLEN 64
#define ELEN 64
#define INT_INST  int32_t


#if XLEN == 8
    #define uINTXLEN uint8_t
    #define INTXLEN   int8_t
#endif
#if XLEN == 16
    #define uINTXLEN uint16_t
    #define INTXLEN   int16_t
#endif
#if XLEN == 32
    #define uINTXLEN uint32_t
    #define INTXLEN   int32_t
#endif
#if XLEN == 64
    #define uINTXLEN uint64_t
    #define INTXLEN   int64_t
#endif
    
#if SEW == 8
    #define intSEW int8_t
    #define uintSEW uint8_t
    #define intDSEW int16_t
    #define intDSEW uint16_t
    #define FSEW 0xFF
    
    #define instLSEW VLE8_V_INSTR
    #define instSSEW VSE8_V_INSTR
    #define floaSEW 


#endif
#if SEW == 16
    #define intSEW int16_t
    #define uintSEW uint8_t
    #define intDSEW int32_t
    #define uintDSEW uint32_t
    #define FSEW 0xFFFF

    #define instLSEW VLE16_V_INSTR
    #define instSSEW VSE16_V_INSTR

    #define floaSEW _Float16
#endif
#if SEW == 32
    #define intSEW   int32_t
    #define uintSEW  uint32_t
    #define intDSEW  int64_t
    #define uintDSEW uint64_t
    #define FSEW 0xFFFFFFFF

    #define instLSEW VLE32_V_INSTR
    #define instSSEW VSE32_V_INSTR

    #define floaSEW float

#endif
#if SEW == 64
    #define intSEW   int64_t
    #define uintSEW  uint64_t
    #define intDSEW  int64_t
    #define uintDSEW uint64_t
    #define FSEW 0xFFFFFFFFFFFFFFFF

    #define instLSEW VLE64_V_INSTR
    #define instSSEW VSE64_V_INSTR

    #define floaSEW double
#endif
#if SEW == 128
    #define intSEW   int128_t
    #define uintSEW  uint128_t
    #define intDSEW  int256_t
    #define uintDSEW uint256_t
    #define FSEW 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    //#define instLSEW VLE128_V_INSTR
    //#define instSSEW VSE128_V_INSTR
#endif
#if SEW == 256
    #define intSEW   int256_t
    #define uintSEW  uint256_t
    #define intDSEW  int512_t
    #define uintDSEW uint512_t
    #define FSEW 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    //#define instLSEW VLE256_V_INSTR
    //#define instSSEW VSE256_V_INSTR
#endif


extern int LMUL;

#define MAX_LMUL 8
#define MAX_EL_PER_BLOCK VLEN / SEW * MAX_LMUL

extern int PRINTS;

extern int EL_PER_BLOCK;

void update_LMUL(int new_LMUL);

void update_SEW(int new_SEW);

#endif