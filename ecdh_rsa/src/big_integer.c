#include "../include/big_integer.h"

// inner functions
BigInt u32tob(uint32_t val);
bool addDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, bool carryInput);
bool subDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, bool borrowInput);
uint32_t mulDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, uint32_t overflow);
char tochar(int i);


/*
char* btoaDebug(BigInt* i) {
	char* buff = malloc(256);
	btoa(i, buff, 16, 256);
	return buff;
}*/

// compare BigInt
// i1 < i2 : return -1
// i1 ==i2 : return 00
// i1 > i2 : return +1
// compare BigInt
// i1 < i2 : return -1
// i1 ==i2 : return 00
// i1 > i2 : return +1
int cmp(BigInt* i1, BigInt* i2) {
	if ((i1->sign == POSITIVE) ^ (i2->sign == POSITIVE)) { //if both sign are diffrent
		bool is_i1_0 = true;
		bool is_i2_0 = true;
		for (int i = i1->msbLoc; i < MAX; i++)
			if (i1->val[i] != 0) {
				is_i1_0 = false;
				break;
			}
		for (int i = i2->msbLoc; i < MAX; i++)
			if (i2->val[i] != 0) {
				is_i2_0 = false;
				break;
			}

		if (is_i1_0 && is_i2_0)
			return 0;
		else if (i1->sign == POSITIVE)
			return 1;
		else
			return -1;
	}

	// i1, i2 has the same sign
	int absolute_modifier = (i1->sign == POSITIVE) ? +1 : -1;
	int count1 = MAX - i1->msbLoc;
	int count2 = MAX - i2->msbLoc;
	if (count1 == count2) { // when count1 and count2 have same digit
		int d1 = i1->msbLoc;
		int d2 = i2->msbLoc;
		while (d1 < MAX) {
			if (i1->val[d1] < i2->val[d2])
				return -absolute_modifier;
			else if (i1->val[d1] > i2->val[d2])
				return +absolute_modifier;
			// else the same value on this digit
			d1++;
			d2++;
		}
		return 0;
	}
	else if (count1 < count2) // when count2 has more digit
		return -absolute_modifier;
	else // when count1 has more digit
		return +absolute_modifier;
}

int cmpAbs(BigInt* i1, BigInt* i2) {
	// don't care i1, i2  sign
	int count1 = MAX - i1->msbLoc;
	int count2 = MAX - i2->msbLoc;
	if (count1 == count2) { // when count1 and count2 have more digit
		int d1 = i1->msbLoc;
		int d2 = i2->msbLoc;
		while (d1 < MAX) {
			if (i1->val[d1] < i2->val[d2])
				return -1;
			else if (i1->val[d1] > i2->val[d2])
				return +1;
			// else the same value on this digit
			d1++;
			d2++;
		}
		return 0;
	}
	else if (count1 < count2) // when count2 has more digit
		return -1;
	else // when count1 has more digit
		return +1;
}


BigInt newBigInt(int32_t val) {
	BigInt temp;
	temp.msbLoc = MAX - 1;

	if (val >= 0)
		temp.sign = POSITIVE;
	else {
		temp.sign = NEGATIVE;
		val = -val;
	}
	temp.val[temp.msbLoc] = val;

	return temp;
}
int toInt(char num, int numberSystem) {
	
	if ('0' <= num && num <= '9')	return num - '0';
	if (numberSystem <= 10)
		return -1;

	if ('a' <= num && num <= 'z')	return num + 10 - 'a';
	if ('A' <= num && num <= 'Z')	return num + 10 - 'A';
	return -1;
}

BigInt atob(char* str) {
	int readPos = 0;
	int numberSystem = 10;
	bool sign = POSITIVE;
	if (str[0] == '+' || str[0] == '-') {
		readPos++;
		if (str[0] == '-')
			sign = NEGATIVE;
		
	}
	if (str[readPos] == '0' && str[readPos + 1] == 'x') {
		numberSystem = 16;
		readPos += 2;
	}

	BigInt result = newBigInt(0);
	BigInt radix = newBigInt(numberSystem);
	for (; str[readPos] != '\0'; readPos++) {
		int to_int = toInt(str[readPos], numberSystem);
		if (to_int < 0)		// invalid character
			return result;
		BigInt val = newBigInt(to_int);	//ascii str[readPos] -> BigInt

		result = mulB(&result, &radix);
		result = addB(&result, &val);
	}
	result.sign = sign;
	return result;
}

BigInt u32tob(uint32_t val) { // change uint32_t to BigInt
	BigInt ret;
	ret.sign = POSITIVE;
	ret.msbLoc = MAX - 1;
	ret.val[ret.msbLoc] = val;

	return ret;
}

// return carry (overflow) not returning result! 
// memory for result must be prepaired
bool addDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, bool carryInput) {
	if (i1->msbLoc <= d1 && i2->msbLoc <= d2) {
		if (carryInput) {
			*result = (i1->val[d1] + i2->val[d2] + CARRY);   // uint never overflow (C standard)
			return  (i1->val[d1] < *result) ? 0 : 1;
		}
		else {
			*result = i1->val[d1] + i2->val[d2];      // uint never overflow (C standard)
			return (i1->val[d1] <= *result) ? 0 : 1;
		}
	}
	else if (i1->msbLoc <= d1) {
		*result = i1->val[d1];
		if (carryInput)
			(*result)++;
		return (i1->val[d1] <= *result) ? 0 : 1;
	}
	else if (i2->msbLoc <= d2) {
		*result = i2->val[d2];
		if (carryInput)
			(*result)++;
		return (i2->val[d2] <= *result) ? 0 : 1;
	}
	else {
		if (carryInput)
			*result = 1;
		else
			*result = 0;
		return 0;
	}
}

// return borrow (underflow)
// memory for result must be prepared
bool subDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, bool borrowInput) {
	if (i1->msbLoc <= d1 && i2->msbLoc <= d2) {
		if (borrowInput) {
			*result = i1->val[d1] - i2->val[d2] - BORROW;
			return (i1->val[d1] <= i2->val[d2]) ? 1 : 0;
		}
		else {
			*result = i1->val[d1] - i2->val[d2];
			return (i1->val[d1] < i2->val[d2]) ? 1 : 0;
		}
	}
	else if (i1->msbLoc <= d1) {
		*result = i1->val[d1];
		if (borrowInput) {
			(*result)--; // substract borrow
			if (i1->val[d1] == 0) {
				return 1;
			}
		}
		return 0;
	}
	else if (i2->msbLoc <= d2) {
		*result = 0 - i2->val[d2];
		if (borrowInput)
			(*result)--; // substract borrow
		if (i2->val[d2] || borrowInput)
			return 1;
		else
			return 0;
	}
	else {
		if (borrowInput) {
			*result = UINT32_MAX;
			return 1;
		}
		else {
			*result = 0;
			return 0;
		}
	}
}

// _multiply
// return carry (uint32 bit)
// memory for result must be prepaired
uint32_t mulDigit(BigInt* i1, BigInt* i2, int d1, int d2, uint32_t* result, uint32_t overflow) {

	if (d1 >= i1->msbLoc && d2 >= i2->msbLoc) {
		uint64_t value = i1->val[d1];
		value *= i2->val[d2];
		value += overflow;
		*result = (uint32_t)(value % 0x100000000);
		return (uint32_t)(value / 0x100000000);
	}
	else {
		*result = 0;
		return 0;
	}
}

char tochar(int i) {
	if (i < 0)   return '?';
	if (i < 10)   return '0' + i;
	else      return 'a' - 10 + i;
}



int32_t btoa(BigInt* i, char* buffer, int base, int buffer_len) {
	BigInt temp_i = *i;
	if (buffer_len <= 1)
		return -1;
	if (temp_i.sign == POSITIVE)
		buffer[0] = '+';
	else
		buffer[0] = '-';

	BigInt ZERO = newBigInt(0);
	BigInt BASE = newBigInt(base);
	int posBuff = 1;

	do {   // writing string backward
		int value = modB(&temp_i, &BASE).val[MAX - 1];
		buffer[posBuff] = tochar(value);
		temp_i = divB(&temp_i, &BASE);
		posBuff++;

		if (buffer_len <= posBuff)
			return -1;
	} while (cmpAbs(&temp_i, &ZERO) != 0);
	buffer[posBuff] = '\0';
	int start = 1;
	int end = posBuff - 1;
	for (; start < end; start++, end--) {   // backward -> forward
		char temp = buffer[end];
		buffer[end] = buffer[start];
		buffer[start] = temp;
	}
	return posBuff;   // strlen
}

BigInt addB(BigInt* i1, BigInt* i2) {
	if ((i1->sign == POSITIVE) ^ (i2->sign == POSITIVE)) {   //xor
		BigInt temp_i2 = *i2;
		temp_i2.sign = (temp_i2.sign == POSITIVE) ? NEGATIVE : POSITIVE;     // convert sign
		BigInt ret = subB(i1, &temp_i2);
		return ret;
	}

	// from now on, only calculate (+)+(+) or (-)+(-)
	BigInt i3 = newBigInt(0);
	int d1 = MAX - 1;
	int d2 = MAX - 1;
	uint32_t value = 0;
	bool carry = 0;

	while (d1 >= i1->msbLoc || d2 >= i2->msbLoc) {
		carry = addDigit(i1, i2, d1, d2, &value, carry);
		i3.val[i3.msbLoc] = value;
		i3.msbLoc--;

		if (d1 >= i1->msbLoc)
			d1--;   // move to bigger digit
		if (d2 >= i2->msbLoc)
			d2--;

		if (i3.msbLoc < 0)
			printf("add할 공간이 부족합니다. MAX값을 증가시켜 주십시오.");
	}
	if (carry)
		i3.val[i3.msbLoc] = 1;
	else
		i3.msbLoc++;

	while (i3.msbLoc < MAX - 1 && i3.val[i3.msbLoc] == 0)
		i3.msbLoc++;

	i3.sign = i1->sign;

	return i3;
}

BigInt subB(BigInt* i1, BigInt* i2) {
	if ((i1->sign == POSITIVE) ^ (i2->sign == POSITIVE)) {   //xor
		BigInt temp_i2 = *i2;
		temp_i2.sign = (temp_i2.sign == POSITIVE) ? NEGATIVE : POSITIVE;     // convert sign
		BigInt ret = addB(i1, &temp_i2);
		return ret;
	}

	// now,  i1->sign == i2->sign
	int d1 = MAX - 1;
	int d2 = MAX - 1;
	int value;
	BigInt i3 = newBigInt(0);
	bool borrow = false;

	while (d1 >= i1->msbLoc || d2 >= i2->msbLoc) {
		borrow = subDigit(i1, i2, d1, d2, &value, borrow);
		i3.val[i3.msbLoc] = value;
		i3.msbLoc--;

		if (d1)
			d1--;
		if (d2)
			d2--;
	}

	if (borrow) {
		i3.msbLoc++;
		for (int i = i3.msbLoc; i < MAX; i++)
			i3.val[i] ^= 0xFFFFFFFF;

		BigInt compVar = newBigInt(1);
		i3 = addB(&i3, &compVar);
		if (i2->sign == POSITIVE)
			i3.sign = NEGATIVE;
		else
			i3.sign = POSITIVE;
	}
	else {
		i3.msbLoc++;
		i3.sign = i1->sign;
	}

	while (i3.msbLoc < MAX - 1 && i3.val[i3.msbLoc] == 0)
		i3.msbLoc++;

	return i3;
}

// multiply
BigInt mulB(BigInt* i1, BigInt* i2) {
	int d1 = i1->msbLoc;
	int d2 = i2->msbLoc;
	BigInt i3 = newBigInt(0);
	uint32_t overflow = 0;
	uint32_t result = 0;
	int radix;
	i3.sign = POSITIVE;
	bool sign = POSITIVE;

	if ((i1->sign == POSITIVE) ^ (i2->sign == POSITIVE))
		sign = NEGATIVE;

	for (int i = MAX - 1; i >= d2; i--) {
		for (int j = MAX - 1; j >= d1; j--) {
			BigInt temp = newBigInt(0);
			temp.sign = POSITIVE;

			overflow = mulDigit(i1, i2, j, i, &result, overflow);

			radix = temp.msbLoc = temp.msbLoc - (MAX * 2 - (i + j + 2));
			if (radix < 0) {
				printf("공간이 부족합니다->\n");
				exit(0);
			};

			if (result != 0) {
				for (int k = temp.msbLoc; k < MAX; k++)
					temp.val[k] = 0;
				temp.val[temp.msbLoc] = result;
				i3 = addB(&i3, &temp);
			}
		}
		if (overflow) {
			if (--radix < 0) {
				printf("공간이 부족합니다->\n");
				exit(0);
			}
			if (i3.msbLoc == radix)
				i3.val[i3.msbLoc] += overflow;
			else {
				i3.msbLoc = radix;
				i3.val[i3.msbLoc] = overflow;
			}
			overflow = 0;
		}
	}

	while (i3.val[i3.msbLoc] == 0)
		i3.msbLoc++;

	i3.sign = sign;
	return i3;
}

BigInt divB(BigInt* i1, BigInt* i2) {
	BigInt ZERO = newBigInt(0);
	if (cmpAbs(i1, i2) < 0) //when if denominator is bigger than numerator(abs)
		return ZERO;

	if (cmpAbs(&ZERO, i2) == 0) { //when denominator is 0
		printf("cannot _divide by 0");
		return ZERO;
	}

	BigInt i3 = newBigInt(0);		// i3 : return value
	i3.sign = ((i1->sign == POSITIVE) && (i2->sign == POSITIVE))? POSITIVE : NEGATIVE;
	// i1, i2 wil be modified (but i2 will be roll backed)
	BigInt* original_i1 = i1;
	BigInt i1_copy = *i1;
	i1 = &i1_copy;
	i1->sign = i2->sign;

	int n;
	const int digit_gap_initial = (i2->msbLoc - i1->msbLoc);
	for (n = i2->msbLoc; n < MAX; n++) {
		i2->val[n - digit_gap_initial] = i2->val[n];
	}
	i2->msbLoc = i1->msbLoc;
	for (n = MAX - digit_gap_initial; n < MAX; n++) {
		i2->val[n] = 0;
	}

	for (n = 0; cmpAbs(i1, i2) >= 0; n++) {
		leftShift(i2);
	}
	n--;
	rightShift(i2);	// now i2 <= i1 < i2 * 2


	
	for (int k = MAX-1; k >= MAX - 1 - digit_gap_initial; k--) {
		i3.val[k] = 0;
	}
	i3.msbLoc = MAX - 1 - digit_gap_initial;
	uint32_t one = 1;
	const int bit_gap_initial = n + digit_gap_initial * 32;	//how many bit [leftshift]ed
	for (int bgap = bit_gap_initial; 0 < bgap; bgap--) {
		if (cmpAbs(i1, i2) >= 0) {	// if i1 >= i2
			*i1 = subB(i1, i2);				// i1 -= i2_original* pow(2,bgap)

			const int loc = MAX - 1 - (bgap / 32);
			const int bit = bgap % 32;
			i3.val[loc] |= (one << bit);	// i3 += 2^n
		}
		rightShift(i2);
	}
	if (cmpAbs(i1, i2) >= 0)	// if i1 >= i2
	{	// if i1 >= i2
		const int loc = MAX - 1;
		const int bit = 0;
		i3.val[loc] |= (one << bit);
	}
	while (i3.val[i3.msbLoc] == 0 && i3.msbLoc < MAX - 1) {	// for case of raising digit was too much
		i3.msbLoc++;
	}

	return i3;
}


BigInt modB(BigInt* i1, BigInt* i2) {

	// i1, i2 wil be modified (but i2 will be roll backed)
	BigInt* original_i1 = i1;
	BigInt i1_copy = *i1;
	bool sign = POSITIVE;
	i1 = &i1_copy;

	if (cmpAbs(i1, i2) < 0) //when if denominator is bigger than numerator(abs)
		return *i1;

	if ((i1->sign == POSITIVE) ^ (i2->sign == POSITIVE))
		sign = NEGATIVE;

	i1->sign = i2->sign;

	BigInt ZERO = newBigInt(0);

	if (cmpAbs(&ZERO, i2) == 0) { //when denominator is 0
		printf("cannot _divide by 0");
		return ZERO;
	}

	///////////////////////////////////////
	int n;
	const int digit_gap_initial = (i2->msbLoc - i1->msbLoc);
	for (n = i2->msbLoc; n < MAX; n++) {
		i2->val[n - digit_gap_initial] = i2->val[n];
	}
	i2->msbLoc = i1->msbLoc;
	for (n = MAX - digit_gap_initial; n < MAX; n++) {
		i2->val[n] = 0;
	}

	for (n = 0; cmpAbs(i1, i2) >= 0; n++) {
		leftShift(i2);
	}
	n--;
	rightShift(i2);   // now i2 <= i1 < i2 * 2

	const int bit_gap_initial = n + digit_gap_initial * 32;
	//how many bit [leftshift]ed

	for (int bit_move = 0; bit_move < bit_gap_initial; bit_move++) {
		if (cmpAbs(i1, i2) >= 0)   // if i1 >= i2
			*i1 = subB(i1, i2);
		rightShift(i2);
	}
	if (cmpAbs(i1, i2) >= 0)   // if i1 >= i2
		*i1 = subB(i1, i2);

	i1->sign = sign;
	return *i1;
}
void leftShift(BigInt* i1) {
	bool carry = false;
	for (int i = MAX - 1; i >= i1->msbLoc; i--) {
		if (carry) {
			if (i1->val[i] < 0x80000000)
				carry = false;
			i1->val[i] <<= 1;
			i1->val[i]++;
		}
		else {
			if (i1->val[i] >= 0x80000000)
				carry = true;
			i1->val[i] <<= 1;
		}
	}
	if (carry == true) {
		i1->msbLoc--;
		i1->val[i1->msbLoc] = 1;
	}
}

void rightShift(BigInt* i1) {
	bool send = false;
	for (int i = i1->msbLoc; i < MAX; i++) {
		if (send) {
			if (!(i1->val[i] & 1))
				send = false;
			i1->val[i] >>= 1;
			i1->val[i] += 0x80000000;
		}
		else {
			if (i1->val[i] & 0x00000001)
				send = true;
			i1->val[i] >>= 1;
		}
	}
	if (i1->msbLoc < MAX - 1 && i1->val[i1->msbLoc] == 0)
		i1->msbLoc++;
}
BigInt powB(BigInt* i1, BigInt* i2) {
	BigInt i3 = newBigInt(1);
	BigInt zero = newBigInt(0);

	if (cmpAbs(&zero, i1) == 0) { // if i1 == 0
		i3 = zero;
		return i3;
	}

	if (cmpAbs(&zero, i2) == 0) { // if(i2 == 0)
		i3 = newBigInt(1);
		return i3;
	}

	BigInt temp_i1;
	BigInt temp_i2 = *i2;
	BigInt two = newBigInt(2);
	BigInt count;
	while (cmp(&temp_i2, &zero) != 0) {
		temp_i1 = *i1;
		count = two;
		while (cmp(&temp_i2, &count) >= 0) {
			temp_i1 = mulB(&temp_i1, &temp_i1);
			count = mulB(&count, &two);
		}
		count = divB(&count, &two);
		temp_i2 = subB(&temp_i2, &count);
		i3 = mulB(&i3, &temp_i1);
	}
	return i3;
}
// test code-> you can modify if you want more test
// disable test main by #if 1 -> #if 0
#if 0
void main() {
	uint32_t testNums[10];
	int try_max = 10;
	int val = 1000000000;
	BigInt i1, i2, i3;

	srand(2);
	testNums[0] = 0xFFFFFFFF;
	testNums[1] = 0xFFFFFFFF;
	for (int i = 2; i < try_max; i++) {
		if (i % 2)
			testNums[i] = rand() * -13579;
		else
			testNums[i] = rand() * 13579;
	}
	char buffer[4][1000] = { "", "i1", "i2", "i3 will be written here" };
	char str[50][200] = { "0x1234560000000000", "1234567890123456789", "98765432109876543210987654321", "123456789012345678901234567890", "0",
		"", "0", "-22", "asdf", "-38276", "-1234567890", };

	i2 = atob("0x11112222333344445555666677778888");
	i1 = atob("0x1111");
	
	btoa(&i1, buffer[1], 16, 256);
	btoa(&i2, buffer[2], 16, 256);
	i3 = divB(&i1, &i2);
	btoa(&i3, buffer[3], 16, 256);
	printf(" %s\n/%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);




	for (int i = 0; i < try_max; i++) {
		i1 = atob(str[i]);
		btoa(&i1, buffer[1], 10, 256);
		printf(" %s==%s\n\n", str[i], buffer[1]);
	}


	i1 = newBigInt(testNums[0]);
	for (int i = 1; i < try_max; i++) {
		i2 = newBigInt(testNums[i]);
		i3 = addB(&i1, &i2);
		btoa(&i1, buffer[1], 10, 256);
		btoa(&i2, buffer[2], 10, 256);
		btoa(&i3, buffer[3], 10, 256);
		printf(" %s\n+%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);
		i1 = i3;
	}

	for (int i = try_max - 1; 0 <= i; i--) {
		i2 = newBigInt(testNums[i]);
		i3 = subB(&i1, &i2);
		btoa(&i1, buffer[1], 10, 256);
		btoa(&i2, buffer[2], 10, 256);
		btoa(&i3, buffer[3], 10, 256);
		printf(" %s\n-%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);
		i1 = i3;
	}

	i1 = u32tob(testNums[0]);
	for (int i = 1; i < try_max; i++) {
		i2 = u32tob(testNums[i]);
		i3 = mulB(&i1, &i2);
		btoa(&i1, buffer[1], 16, 256);
		btoa(&i2, buffer[2], 16, 256);
		btoa(&i3, buffer[3], 16, 256);
		printf(" %s\n*%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);
		i1 = i3;
	}

	for (int i = try_max - 1; 0 <= i; i--) {
		i1 = newBigInt(testNums[i]);
		i3 = divB(&i1, &i2);
		btoa(&i1, buffer[1], 10, 256);
		btoa(&i2, buffer[2], 10, 256);
		btoa(&i3, buffer[3], 10, 256);
		printf(" %s\n/%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);
		i2 = i3;   // little bit diffrent here!
	}

	i1 = atob("0x9D7D246CFF5423593B1A2E377C500D5A108714F9941A9962");
	i2 = atob("0xAA1260B4EE364ED84EA80090B5BB2F23D8A65D424BED0E68");
	btoa(&i1, buffer[1], 16, 256);
	btoa(&i2, buffer[2], 16, 256);

	i3 = mulB(&i1, &i2);
	btoa(&i3, buffer[3], 10, 256);
	printf(" %s\n*%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);


	i1 = i3;
	for (int i = 1; i < try_max; i++) {
		i2 = newBigInt(testNums[i]);
		i3 = modB(&i1, &i2);
		btoa(&i1, buffer[1], 16, 256);
		btoa(&i2, buffer[2], 16, 256);
		btoa(&i3, buffer[3], 16, 256);
		printf(" %s\n%%%s\n=%s\n\n", buffer[1], buffer[2], buffer[3]);
	}


	BigInt i = newBigInt(2);
	BigInt poww = newBigInt(10);
	BigInt result;

	result = powB(&i, &poww);
}
#endif // 0

