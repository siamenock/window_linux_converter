#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "big_integer.h"

#if NOT_TEST

#define		BUFFER_SIZE 256

//ECC Point
typedef struct EcPt {
	BigInt x, y;
} EcPt;

BigInt ZERO;
EcPt INFINITE_POINT;		// point on infinity (0,0)	// initialized by initGlobal4Ec()


void initGlobal4Ec();								// set point at infinity
BigInt inverseMod(BigInt* k, BigInt* prime);	// n/k(mod prime) == (n * inverse_mod(k, prime)) % prime
BigInt modM(BigInt* k, BigInt* prime);		// used instead of modB in ECC system									

bool isEqualEcPt(EcPt *p1, EcPt* p2);
EcPt newEcPt(BigInt *x, BigInt *y, BigInt *prime);
EcPt addEc(EcPt *p1, EcPt *p2, BigInt *prime);	// +	ECC


void initGlobal4Ec() {
	ZERO = newBigInt(0);
	INFINITE_POINT.x = ZERO;
	INFINITE_POINT.y = ZERO;
}

// used instead of modB, which is -5 % 2 = -1 as C language's % operation,
// -5 % 2 = +1 in modM. it is usefull for ECC
BigInt modM(BigInt* k, BigInt* prime) {	
	BigInt copy_k = modB(k, prime);
	k = &copy_k;
	if (cmp(k, &ZERO) < 0) 		// if  k < 0
		*k = addB(k, prime);
	
	return *k;
}
EcPt newEcPt(BigInt* x, BigInt* y, BigInt* prime) {
	EcPt p3;
	p3.x = modM(x, prime);
	p3.y = modM(y, prime);
	return p3;
}

bool isEqualEcPt(EcPt* p1, EcPt* p2) {
	if (cmp(&p1->x, &p2->x) != 0) return false;
	if (cmp(&p1->y, &p2->y) != 0) return false;
	return true;
}

// inner fuction for inverseMod(...)
void inverse_mod_inner_converter(BigInt* a1, BigInt*a2, BigInt* quotient) {
	BigInt mul = mulB(quotient, a1);
	BigInt sub = subB(a2, &mul);
	*a2 = *a1;
	*a1 = sub;
}

// n/k(mod prime) == (n * inverse_mod(k, prime)) % prime
BigInt inverseMod(BigInt *k, BigInt *prime) {
	if (cmp(k, &ZERO) == 0)
		return ZERO;
	BigInt* ori_k = k, *ori_p = prime;
	BigInt copy_k = *k;
	BigInt copy_p = *prime;
	k		= &copy_k;	// now we can modify k without affecting "k" out of function
	prime	= &copy_p;
	
	BigInt r1 = *prime;
	BigInt r2 = *k;
	BigInt s1 = newBigInt(0);
	BigInt s2 = newBigInt(1);
	BigInt t1 = newBigInt(1);
	BigInt t2 = newBigInt(0);
	
	while (cmp(&r1, &ZERO) != 0) {
		BigInt quotient = divB(&r2, &r1);
		inverse_mod_inner_converter(&r1, &r2, &quotient);
		inverse_mod_inner_converter(&s1, &s2, &quotient);
		inverse_mod_inner_converter(&t1, &t2, &quotient);
	}
	BigInt one = newBigInt(1);
	BigInt mul = mulB(k, &s2);
	BigInt mod = modM(&mul, prime);

	BigInt ret = modM(&s2, prime);
	if (cmp(&r2, &one) != 0 || cmp(&mod, &one) != 0) {
		printf("inverse_mod go wrong : \n/%s\n*\n %s\nmod %s\n must be 1 but not\n", btoaDebug(ori_k), btoaDebug(&ret), btoaDebug(ori_p));
		exit(1);
	}


	return ret;
}

// + operation for ECC
EcPt addEc(EcPt* p1, EcPt* p2, BigInt* prime) {
	if (isEqualEcPt(p1, &INFINITE_POINT)) return *p2;
	if (isEqualEcPt(p2, &INFINITE_POINT)) return *p1;

	BigInt t1, t2, t3;		// we can't use malloc pointers.... need temporary values

	t1= subB(&p1->y, &p2->y);	
	t3 = subB(&p1->x, &p2->x);
	t3 = modM(&t3, prime);
	t2 = inverseMod(&t3, prime);
	BigInt m = mulB( &t1, &t2);	
	m = modM(&m, prime);				// m = [(y1-y2)/(x1-x2)]%prime

	t1 = mulB(&m, &m);
	t2 = addB(&p1->x, &p2->x);
	BigInt x = subB(&t1, &t2);	// x3 = mm -x1 -x2

	t3 = subB(&x, &p1->x);
	t3 = mulB(&m, &t3);
	BigInt y = addB(&p1->y, &t3);	// y3 = y1 + m*(x3 -x1)

	y.sign = (y.sign == POSITIVE) ? NEGATIVE : POSITIVE;	// y3 = -y3;
	return newEcPt(&x, &y, prime);
}

EcPt doubleEc(EcPt* p1, BigInt* prime, BigInt* a) {
	if (isEqualEcPt(p1, &INFINITE_POINT)) {
		return *p1;
	}
	
	BigInt t1, t2;	//temp values
	t1 = newBigInt(3);
	t2 = mulB(&p1->x, &p1->x);
	t1 = mulB(&t1, &t2);
	t1 = addB(&t1, a);
	t1 = modM(&t1, prime);
	t2 = addB(&p1->y, &p1->y);
	t2 = inverseMod(&t2, prime);
	BigInt m = mulB(&t1, &t2);	
	m = modM(&m, prime);			// m = [(3 * x1 * x1 + a) /(2 * y1)]%prime	

	t1 = mulB(&m, &m);
	t2 = addB(&p1->x, &p1->x);
	BigInt x = subB(&t1,&t2);		// x3 = mm -x1 -x1
	
	t2 = subB(&x, &p1->x);
	t2 = mulB(&m, &t2);
	BigInt y = addB(&p1->y, &t2);	// y3 = y1+ m*(x3-x1)
	y.sign = (y.sign == POSITIVE) ? NEGATIVE : POSITIVE;	// y = -y;
	x = modM(&x, prime);
	y = modM(&y, prime);
	return newEcPt(&x, &y, prime);
}

// return k * p1;
EcPt mulEc(EcPt* p1, BigInt* k, BigInt* prime, BigInt* a, uint32_t curve) {
	uint32_t limit = 0;
	EcPt result = INFINITE_POINT;	// = 0 = (0,0)
	uint32_t digit;
	for (int bit = curve; 0 <= bit; bit--) {
		result = doubleEc(&result, prime, a);	// no effect if result == (0,0)
		int pos = (MAX - 1) - bit/ 32;
		//k를 uint2048_t로 취급할 때 bit자리 비트가 1인지 여부를 구해야함
		if (pos < k->msbLoc)
			digit = 0;	// out of k range
		else 
			digit = k->val[pos];
		digit >>= (bit % 32);

		//k의 bit자리 비트가 1인가?
		if (digit % 2) {
			result = addEc(&result, p1, prime);
		}
	}
	return result;
}

// is point on Eliptic Curve [yy = xxx + ax + b]
bool isOnEc(EcPt* p1, BigInt* prime, BigInt* a, BigInt* b) {
	BigInt t1; // temp val
	t1 = mulB(&p1->x, &p1->x);
	t1 = addB(&t1, a);
	t1 = modM(&t1, prime);
	t1 = mulB(&p1->x, &t1);
	t1 = modM(&t1, prime);
	t1 = addB(&t1, b);					// t1 = xxx + ax + b -> x*((x*x + a)%prime) + b
	BigInt t2 = mulB(&p1->y, &p1->y);	// t2 = yy
	
	t1 = subB(&t1, &t2);
	t1 = modM(&t1, prime);		// t1 = (t1-t2) % prime
	
	if (cmpAbs(&t1, &ZERO) == 0) 
		return true;	// [-yy + xxx + ax + b -> x*((x*x + a)%prime) + b ]%prime == 0
	else
		return false;
}

int main(int argc, char**argv) {
	initGlobal4Ec();
	printf("////////////////////////////////////////////////////////////////////////////////\n");
	printf("// Verification of ECDH over ECC-P192, ECC-P224, ECC-P256, ECC-P384, ECC-P521 //\n");
	printf("////////////////////////////////////////////////////////////////////////////////\n");

	printf("\n");
	printf("*********************************************************\n");
	printf("**** Enter Curve over prime field in Decimal format *****\n");
	printf("*********************************************************\n");


	BigInt prime;	// 해당 ECC에 해당되는 prime num
	BigInt a;		// ECC 정의 상수 (y^2 = x^3 + ax + b)
	BigInt b;		// ECC 정의 상수 (y^2 = x^3 + ax + b)
	BigInt r;		// 위수?? order
	BigInt gx;		// x 좌표(초기값)
	BigInt gy;		// y 좌표(초기값)
	int curve = 0;
	while (curve == 0) {
		scanf("%d", &curve);
		switch (curve) {
		case -1:	// for test
			curve = 256;
			prime = atob("223");
			a = atob("0");
			b = atob("7");
			r = atob("0");
			gx = atob("1");
			gy = atob("193");
			break;
		case 192:
			prime = atob("6277101735386680763835789423207666416083908700390324961279");
			a =		atob("6277101735386680763835789423207666416083908700390324961276");
			b =		atob("2455155546008943817740293915197451784769108058161191238065");
			r =		atob("6277101735386680763835789423176059013767194773182842284081");
			gx =	atob("602046282375688656758213480587526111916698976636884684818");
			gy =	atob("174050332293622031404857552280219410364023488927386650641");
			break;

		case 224:
			prime = atob("26959946667150639794667015087019630673557916260026308143510066298881");
			a = atob("26959946667150639794667015087019630673557916260026308143510066298878");
			b = atob("18958286285566608000408668544493926415504680968679321075787234672564");
			r = atob("26959946667150639794667015087019625940457807714424391721682722368061");
			gx = atob("19277929113566293071110308034699488026831934219452440156649784352033");
			gy = atob("19926808758034470970197974370888749184205991990603949537637343198772");
			break;

		case 256:
			prime = atob("115792089210356248762697446949407573530086143415290314195533631308867097853951");
			a = atob("115792089210356248762697446949407573530086143415290314195533631308867097853948");
			b = atob("41058363725152142129326129780047268409114441015993725554835256314039467401291");
			r = atob("115792089210356248762697446949407573529996955224135760342422259061068512044369");
			gx = atob("48439561293906451759052585252797914202762949526041747995844080717082404635286");
			gy = atob("36134250956749795798585127919587881956611106672985015071877198253568414405109");
			break;

		case 384:
			prime = atob("39402006196394479212279040100143613805079739270465446667948293404245721771496870329047266088258938001861606973112319");
			a = atob("39402006196394479212279040100143613805079739270465446667948293404245721771496870329047266088258938001861606973112316");
			b = atob("27580193559959705877849011840389048093056905856361568521428707301988689241309860865136260764883745107765439761230575");
			r = atob("39402006196394479212279040100143613805079739270465446667946905279627659399113263569398956308152294913554433653942643");
			gx = atob("26247035095799689268623156744566981891852923491109213387815615900925518854738050089022388053975719786650872476732087");
			gy = atob("8325710961489029985546751289520108179287853048861315594709205902480503199884419224438643760392947333078086511627871");
			break;

		case 521:
			prime = atob("6864797660130609714981900799081393217269435300143305409394463459185543183397656052122559640661454554977296311391480858037121987999716643812574028291115057151");
			a = atob("6864797660130609714981900799081393217269435300143305409394463459185543183397656052122559640661454554977296311391480858037121987999716643812574028291115057148");
			b = atob("1093849038073734274511112390766805569936207598951683748994586394495953116150735016013708737573759623248592132296706313309438452531591012912142327488478985984");
			r = atob("6864797660130609714981900799081393217269435300143305409394463459185543183397655394245057746333217197532963996371363321113864768612440380340372808892707005449");
			gx = atob("2661740802050217063228768716723360960729859168756973147706671368418802944996427808491545080627771902352094241225065558662157113545570916814161637315895999846");
			gy = atob("3757180025770020463545507224491183603594455134769762486694567779615544477440556316691234405012945539562144444537289428522585666729196580810124344277578376784");
			break;

		default:
			printf("invalid input. we only accept 192, 224, 256, 384, 521\n");
			curve = 0;
		}
	}
	EcPt publicPoint = newEcPt(&gx, &gy, &prime);

	printf("\n");
	printf("we get ECC curve y^2 = x^3 + ax + b    and point(x,y)\n a,b,x,y are weell known protocol data\n");
	char buff[256];
	btoa(&prime, buff, 16, 256);			printf("p == %s\n", buff);
	btoa(&a, buff, 16, 256);				printf("a == %s\n", buff);
	btoa(&b, buff, 16, 256);				printf("b == %s\n", buff);
	btoa(&publicPoint.x, buff, 16, 256);	printf("x == %s\n", buff);
	btoa(&publicPoint.y, buff, 16, 256);	printf("y == %s\n", buff);


	printf("\n");
	printf("**********************************************************************\n");
	printf("********** Enter Private Key k_a of Alice in Decimal format **********\n");
	printf("**********************************************************************\n");
	BigInt privateAlice = ZERO;
	char buffer[BUFFER_SIZE];
	while (cmpAbs(&privateAlice, &ZERO) == 0) {
		scanf("%s", buffer);
		privateAlice = atob(buffer);	// key of Alice
		if (cmp(&privateAlice, &ZERO) <= 0 || cmp(&privateAlice, &r) == 0) {
			privateAlice = ZERO;
			printf("invalid input. key must not equal with 0 or r\nand must be positive\n");
		}
	}
	printf("\n");
	printf("**********************************************************************\n");
	printf("*********** Enter Private Key k_a of Bob in Decimal format ***********\n");
	printf("**********************************************************************\n");
	BigInt privateBob = ZERO;
	while (cmpAbs(&privateBob, &ZERO) == 0) {
		scanf("%s", buffer);
		privateBob = atob(buffer);	// key of Alice
		if (cmp(&privateBob, &ZERO) <= 0 || cmp(&privateBob, &r) == 0) {
			privateBob = ZERO;
			printf("invalid input. key must not equal with 0 or r\nand must be positive\n");
		}
	}
	printf("\n");

	EcPt publicAlice = mulEc(&publicPoint, &privateAlice, &prime, &a, curve);	// public key of Alice
	EcPt publicBob = mulEc(&publicPoint, &privateBob, &prime, &a, curve);		// public key of Bob

	
	//print them
	printf("public key of Alice-----------\n");
	btoa(&publicAlice.x, buffer, 16, BUFFER_SIZE);
	printf("x\t%s\n", buffer);
	btoa(&publicAlice.y, buffer, 16, BUFFER_SIZE);
	printf("y\t%s\n", buffer);
	printf("\n");

	printf("public key of Bob-------------\n");
	btoa(&publicBob.x, buffer, 16, BUFFER_SIZE);
	printf("x\t%s\n", buffer);
	btoa(&publicBob.y, buffer, 16, BUFFER_SIZE);
	printf("y\t%s\n", buffer);
	printf("\n");
	
	// error check
	if (!isOnEc(&publicAlice, &prime, &a, &b)) {
		printf("FATAL ERROR : Alice's calculation is wrong");
		exit(1);
	}
	if (!isOnEc(&publicBob, &prime, &a, &b)) {
		printf("FATAL ERROR : Bob's calculation is wrong");
		exit(1);
	}

	printf("both public keys are on Eliptic Curve\n\n");

	EcPt sharedAlice = mulEc(&publicBob, &privateAlice, &prime, &a, curve);
	EcPt sharedBob = mulEc(&publicAlice, &privateBob, &prime, &a, curve);
	
	// error check
	if (!isEqualEcPt(&sharedAlice, &sharedBob) || ! isOnEc(&sharedAlice, &prime, &a, &b)) {
		printf("FATAL ERROR : diffrent key");
		exit(1);
	}
	//print them
	printf("shared key of Alice & Bob-------------\n");
	btoa(&sharedAlice.x, buffer, 16, BUFFER_SIZE);
	printf("x\n%s\n", buffer);
	btoa(&sharedAlice.y, buffer, 16, BUFFER_SIZE);
	printf("y\n%s\n", buffer);
	printf("\n");
	printf("they get the same shared key without exposing private key\n\n");
}
#endif
