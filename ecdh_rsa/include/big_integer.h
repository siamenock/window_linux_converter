#ifndef _BIGINT_H_
#define _BIGINT_H_

//#pragma warning(disable:4996)

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


//#define TEST 0
//#define NOT_TEST !TEST

#define MAX 150
#define POSITIVE false
#define NEGATIVE true
#define BORROW 1
#define CARRY 1

// bigint decimal scanning
#define DECI_SCAN_LEN 9         // uint32 max is 10 digit in decimal. scanning 9 digit is safe
#define DECI_SCAN_VAL 1000000000// 10 ^ DECI_SCAN_LEN
#define HEXA_SCAN_LEN 7         //   uint32 max is 0XFFFFFFFF (8digit)
#define HEXA_SCAN_VAL 268435456   // 16 ^ HEXA_SCAN_LEN

typedef struct BigInt {
	bool sign;
	int msbLoc;
	uint32_t val[MAX];
} BigInt;

//functions for user
BigInt newBigInt(int32_t val);
BigInt atob(char* str);            // ascii to big int (hexa 구현할것)
int32_t btoa(BigInt* i, char* buffer, int base, int buffer_len);
int cmp(BigInt* i1, BigInt* i2);      // <, =, >                              //
int cmpAbs(BigInt* i1, BigInt* i2);     // <, =, > in absolute value         //

BigInt addB(BigInt* i1, BigInt* i2);   // +
BigInt subB(BigInt* i1, BigInt* i2);   // -
BigInt mulB(BigInt* i1, BigInt* i2);   // *
BigInt divB(BigInt* i1, BigInt* i2);   // /
BigInt modB(BigInt* i1, BigInt* i2);   // %

BigInt powB(BigInt* i1, BigInt* i2);   // ^
void leftShift(BigInt* i1);
void rightShift(BigInt* i1);

//char* btoaDebug(BigInt* i);

#endif // !1

