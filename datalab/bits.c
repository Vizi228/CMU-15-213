/* 
 * CS:APP Data Lab 
 * 
 * <Myroslav 228>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

// typedef unsigned char *byte_pointer;
// void show_bytes(byte_pointer start, unsigned int len) {
//   int i;
//   for (i = 0; i < len; i++)
//     printf(" %.2x", start[i]);
//     printf("\n");
// }
// void show_int(int x) {
//   show_bytes((byte_pointer) &x, sizeof(int));
// }
// void show_float(float x) {
//   show_bytes((byte_pointer) &x, sizeof(float));
// }
// void show_pointer(void *x) {
//   show_bytes((byte_pointer) &x, sizeof(void *));
// }

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int nand = ~(x & y);
  return ~(~(x & nand) & ~(y & nand));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  int min = 0x01;
  return min << 31;
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
  return !(~(x ^ (x + 1))) & !!(x + 1);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int shift16 = (x >> 16) & x;
  int shift8 = (shift16 >> 8) & shift16;
  int shift4 = (shift8 >> 4) & shift8;
  int shift2 = (shift4 >> 2) & shift4;
  return (shift2 >> 1) & 1;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int top = 0x3a;
  int bottom = 0x2f;
  int lessThan = ((x + (~top + 1)) >> 31) & 1;
  int moreThan = ((bottom + (~x + 1)) >> 31) & 1;
  return lessThan & moreThan;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int cond = !!x;
  return (y & (~cond + 1)) | (z & ~(~cond + 1));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int signX = (x >> 31) & 1;
    int signY = (y >> 31) & 1;
    int diff = y + (~x + 1);
    return ((signY ^ 1) & signX) | ((!(signY ^ signX)) & (((diff >> 31) & 1) ^ 1));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  int shift16 = (x >> 16) | x;
  int shift8 = (shift16 >> 8) | shift16;
  int shift4 = (shift8 >> 4) | shift8;
  int shift2 = (shift4 >> 2) | shift4;
  int shift1 = (shift2 >> 1) | shift2;
  return (shift1 & 1) ^ 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int y = (~x)^(~(!(x>>31))+1); // if x less than 0 then revert it, because our shift would give us negative number
  int sign_bit = 1;
  int count = 0;

  int a = (!(y>>16)^0x0)<<4;
  count = count+a;
  y= y<<a;
  a = (!(y>>24)^0x0)<<3;
  count = count+a;
  y= y<<a;
  a = (!(y>>28)^0x0)<<2;
  count = count+a;
  y = y<<a;
  a = (!(y>>30)^0x0)<<1;
  count = count+a;
  y = y<<a;
  count = count+(!(y>>31));
  count = 32+(~count)+1; // 32(bits) minus count of bits that we should substract
  
  count = count + sign_bit; // add sign bit
  count = count^((count^1)&(~((!x)|(!(~x)))+1)); // if x == 1 or x == 0 then substract 1 from count because they don't have sign_bit
  return count;
}
//float
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  unsigned int exp_mask = 0x7F800000;  // mask to extract the exponent
  unsigned int frac_mask = 0x007FFFFF; // mask to extract the fraction

  // Extract the exponent and fraction
  unsigned int exp = (uf & exp_mask) >> 23;
  unsigned int frac = uf & frac_mask;

  // Check for special cases: +Inf, -Inf, NaN
  if (exp == 0xFF) {
    // If the exponent consists of all ones (0xFF), it's NaN or Inf.
    // In this case, return the same argument.
    return uf;
  } else if (exp == 0) {
    // If the exponent is zero, it's a denormalized number or zero.
    // Restore a normalized number by incrementing the exponent by 1.
    return (uf & 0x80000000) | ((uf << 1) & 0x7FFFFFFF);
  } else {
    // Increment the exponent by 1.
    exp += 1;

    // Check for overflow (if incrementing the exponent causes overflow)
    if (exp == 0xFF) {
      // Overflow: set the exponent to 0xFF and clear the fraction
      return (uf & 0x80000000) | 0x7F800000;
    } else {
      // Normal case: update the exponent in the original value
      return (uf & ~exp_mask) | (exp << 23) | frac;
    }
  }
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    unsigned sign, exp, frac, round;
    if (x == 0) {
        return 0;
    }

    sign = (x & 0x80000000);
    if (sign) {
        x = -x;
    }

    exp = 158;
    while ((x & 0x80000000) == 0) {
        x <<= 1;
        exp--;
    }

    frac = (x >> 8) & 0x007FFFFF;
    round = (x & 0xFF) > 0x80 || ((x & 0xFF) == 0x80 && (frac & 1));
    return sign + (exp << 23) + frac + round;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    unsigned sign = uf >> 31; // Extract sign bit
    unsigned frac = uf & 0x007FFFFF; // Extract fraction
    int exp = (uf >> 23) & 0xFF; // Extract exp

    // Special cases (NaN and Inf)
    if (exp == 0xFF) {
        return 0x80000000u; // Return 0x80000000u for NaN and Inf
    }
    exp = exp - 127;
    if (exp < 0) {
        return 0; // Fractional part is less than 1, return 0
    }
    if (exp >= 31) {
        return 0x80000000u; // Overflow, return 0x80000000u
    }
    if (exp > 23) {
        int res = (frac | 0x800000) << (exp - 23);
        return sign ? -res : res;
    } else {
        int res = (frac | 0x800000) >> (23 - exp);
        return sign ? -res : res;
    }
}
