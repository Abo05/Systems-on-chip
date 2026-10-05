#ifndef FRACTAL_MYFLPT_H
#define FRACTAL_MYFLPT_H

#include <stdint.h>

#ifndef MANTISA_BITS
#define MANTISA_BITS 23U
#endif

#ifndef EXPONENT_BITS
#define EXPONENT_BITS 8U
#endif

#ifndef EXPONENT_BIAS
#define EXPONENT_BIAS 250U
#endif

#define MANTISSA_BITS MANTISA_BITS
#define EXPONENT_EXCESS EXPONENT_BIAS

#define SIGN_BIT (MANTISA_BITS + EXPONENT_BITS)
#define SIGN_MASK (1U << SIGN_BIT)

#define EXPONENT_MASK ((1U << EXPONENT_BITS) - 1U)
#define MAX_EXPONENT EXPONENT_MASK

#define MANTISA_MASK ((1U << MANTISA_BITS) - 1U)
#define MANTISSA_MASK MANTISA_MASK

#define MAX ((EXPONENT_MASK << MANTISA_BITS) | MANTISA_MASK)

// Helper macros for float constants
#define FLT_MAKE(sign, unbiased_exp, mant) \
    (((uint32_t)(sign) << SIGN_BIT) | \
     (((uint32_t)((int32_t)(EXPONENT_BIAS) + (unbiased_exp)) & EXPONENT_MASK) << MANTISA_BITS) | \
     ((uint32_t)(mant) & MANTISA_MASK))

#define FLOAT_FOUR      ((EXPONENT_BIAS + 2U) << MANTISA_BITS)

typedef uint32_t flt_8exp_t;

static inline flt_8exp_t sum_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> MANTISA_BITS) & EXPONENT_MASK;
    uint32_t exp2 = (flt2 >> MANTISA_BITS) & EXPONENT_MASK;
    
    char exp1_big = exp1 > exp2;
    uint32_t diff = exp1_big ? (exp1 - exp2) : (exp2 - exp1);
    if (diff > (MANTISA_BITS + 2U)) {
        return exp1_big ? flt1 : flt2;
    }

    uint64_t mant1 = flt1 & MANTISA_MASK;
    if(exp1 == 0 && mant1 == 0){
        return flt2;
    }
    uint32_t mant2 = flt2 & MANTISA_MASK;
    if(exp2 == 0 && mant2 == 0){
        return flt1;
    }
    mant1 |= (1U << MANTISA_BITS);
    mant2 |= (1U << MANTISA_BITS);
    
    uint32_t sign1 = flt1 & SIGN_MASK;
    uint32_t sign2 = flt2 & SIGN_MASK;
    
    uint32_t exp;
    uint32_t sign;
    if(exp1 > exp2){
        mant2 >>= diff;
        exp = exp1;
        sign = sign1;
    }
    else if(exp2 > exp1){
        mant1 >>= diff;
        exp = exp2;
        sign = sign2;
    }
    else{
        exp = exp1;
        if(mant1 > mant2){
            sign = sign1;
        }
        else{
            sign = sign2;
        }
    }
    
    uint32_t mant;
    if(sign1 == sign2){
        mant = mant1 + mant2;
        if (mant & (1U << (MANTISA_BITS + 1))) {
            if (exp >= MAX_EXPONENT) {
                return sign | MAX;
            }
            exp += 1;
            mant >>= 1;
        }
    }
    else{
        if(mant1 > mant2){
            mant = mant1 - mant2;
        }
        else{
            mant = mant2 - mant1;
            if (mant == 0) {
                return 0;
            }
        }
        while ((mant & (1U << MANTISA_BITS)) == 0) {
            if (exp == 0) {
                return 0;
            }
            mant <<= 1;
            exp -= 1;
        }
    }

    mant &= MANTISA_MASK;
    
    uint32_t res = sign;
    res |= exp << MANTISA_BITS;
    res |= mant;

    return res;
}

static inline flt_8exp_t sub_float(flt_8exp_t flt1, flt_8exp_t flt2){
    if ((flt2 & MAX) == 0) {
        return flt1;
    }
    flt2 ^= SIGN_MASK;
    return sum_float(flt1, flt2);
}

static inline flt_8exp_t mul_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> MANTISA_BITS) & EXPONENT_MASK;
    uint32_t exp2 = (flt2 >> MANTISA_BITS) & EXPONENT_MASK;

    uint64_t mant1 = flt1 & MANTISA_MASK;
    if(exp1 == 0 && mant1 == 0){
        return 0;
    }
    uint32_t mant2 = flt2 & MANTISA_MASK;
    if(exp2 == 0 && mant2 == 0){
        return 0;
    }
    mant1 |= (1U << MANTISA_BITS);
    mant2 |= (1U << MANTISA_BITS);
    
    uint32_t exp = exp1 + exp2;

    uint64_t mant = mant1 * mant2;
    
    if (mant & (1ULL << (MANTISA_BITS * 2 + 1))) {
        exp += 1;
        mant >>= (MANTISA_BITS + 1);
    } else {
        mant >>= MANTISA_BITS;
    }

    mant &= MANTISA_MASK;

    if(exp < EXPONENT_BIAS){
        return 0;
    }
    exp -= EXPONENT_BIAS;
    if (exp > MAX_EXPONENT) {
        exp = MAX_EXPONENT;
        mant = MANTISA_MASK;
    }   

    uint32_t res = flt1 ^ flt2;
    res &= SIGN_MASK;
    res |= exp << MANTISA_BITS;
    res |= (uint32_t)mant;

    return res;
}

static inline flt_8exp_t div_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> MANTISA_BITS) & EXPONENT_MASK;
    uint32_t exp2 = (flt2 >> MANTISA_BITS) & EXPONENT_MASK;
    if (exp1 + EXPONENT_BIAS < exp2) {
        return 0;
    }

    uint32_t mant1 = flt1 & MANTISA_MASK;
    if(exp1 == 0 && mant1 == 0){
        return 0; 
    }

    uint32_t mant2 = flt2 & MANTISA_MASK;
    if(exp2 == 0 && mant2 == 0){
        uint32_t res = (flt1 ^ flt2) & SIGN_MASK;
        res |= MAX;
        return res;
    }

    mant1 |= (1U << MANTISA_BITS);
    mant2 |= (1U << MANTISA_BITS);

    uint32_t exp = EXPONENT_BIAS + exp1 - exp2;
    uint64_t mant = ((uint64_t)mant1 << (MANTISA_BITS + 1)) / mant2;

    if (mant1 >= mant2) {
        mant >>= 1;
    } else {
        if(exp == 0){
            return 0;
        }
        exp -= 1;
    }
    mant &= MANTISA_MASK;

    if (exp > MAX_EXPONENT) {
        exp = MAX_EXPONENT;
        mant = MANTISA_MASK;
    }

    uint32_t res = (flt1 ^ flt2) & SIGN_MASK;
    res |= ((uint32_t)exp) << MANTISA_BITS;
    res |= (uint32_t)mant;

    return res;
}

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(flt_8exp_t cx, flt_8exp_t cy, uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(flt_8exp_t cx, flt_8exp_t cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height,
                  calc_frac_point_p cfp_p, iter_to_colour_p i2c_p,
                  flt_8exp_t cx_0, flt_8exp_t cy_0, flt_8exp_t delta, uint16_t n_max);

#endif // FRACTAL_MYFLPT_H
