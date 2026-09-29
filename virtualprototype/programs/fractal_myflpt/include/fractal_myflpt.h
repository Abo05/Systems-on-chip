#ifndef FRACTAL_MYFLPT_H
#define FRACTAL_MYFLPT_H

#include <stdint.h>

typedef uint32_t flt_8exp_t;

static inline flt_8exp_t sum_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> 23) & 0xFFU;
    uint32_t exp2 = (flt2 >> 23) & 0xFFU;
    
    char exp1_big = exp1 > exp2;
    uint32_t diff = exp1_big ? (exp1 - exp2) : (exp2 - exp1);
    if (diff > 25) {
        return exp1_big ? flt1 : flt2;
    }

    uint64_t mant1 = flt1 & 0x7FFFFFU;
    if(exp1 == 0 && mant1 == 0){
        return flt2;
    }
    uint32_t mant2 = flt2 & 0x7FFFFFU;
    if(exp2 == 0 && mant2 == 0){
        return flt1;
    }
    mant1 |= (1U << 23);
    mant2 |= (1U << 23);
    
    uint32_t sign1 = flt1 & 0x80000000U;
    uint32_t sign2 = flt2 & 0x80000000U;
    
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
        if (mant & (1ULL << 24)) {
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
        while ((mant & (1ULL << 23)) == 0) {
            if (exp == 0) {
                return 0;
            }
            mant <<= 1;
            exp -= 1;
        }
    }

    mant &= 0x7FFFFFU;
    
    uint32_t res = sign;
    res |= exp << 23;
    res |= mant;

    return res;
}

static inline flt_8exp_t sub_float(flt_8exp_t flt1, flt_8exp_t flt2){
    flt2 ^= 1U << 31;
    return sum_float(flt1, flt2);

}

static inline flt_8exp_t mul_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> 23) & 0xFFU;
    uint32_t exp2 = (flt2 >> 23) & 0xFFU;

    uint64_t mant1 = flt1 & 0x7FFFFFU;
    if(exp1 == 0 && mant1 == 0){
        return 0;
    }
    uint32_t mant2 = flt2 & 0x7FFFFFU;
    if(exp2 == 0 && mant2 == 0){
        return 0;
    }
    mant1 |= (1U << 23);
    mant2 |= (1U << 23);
    
    uint32_t exp = exp1 + exp2;

    uint64_t mant = mant1 * mant2;
    
    if (mant & (1ULL << 47)) {
        exp += 1;
        mant >>= 24;
    } else {
        mant >>= 23;
    }
    mant &= 0x7FFFFFU;

    if(exp < 250U){
        return 0;
    }
    exp -= 250U;
    if (exp > 255) {
        exp = 0xFFU;
        mant = 0x7FFFFFU;
    }   

    uint32_t res = flt1 ^ flt2;
    res &= 0x80000000U;
    res |= exp << 23;
    res |= mant;

    return res;
}

static inline flt_8exp_t div_float(flt_8exp_t flt1, flt_8exp_t flt2){
    uint32_t exp1 = (flt1 >> 23) & 0xFFU;
    uint32_t exp2 = (flt2 >> 23) & 0xFFU;
    if (exp1+250 < exp2) {
        return 0;
    }

    uint32_t mant1 = flt1 & 0x7FFFFFU;
    if(exp1 == 0 && mant1 == 0){
        return 0; 
    }

    uint32_t mant2 = flt2 & 0x7FFFFFU;
    if(exp2 == 0 && mant2 == 0){
        uint32_t res = (flt1 ^ flt2) & 0x80000000U;
        res |= 0x7FFFFFFFU;
        return res;
    }

    mant1 |= (1U << 23);
    mant2 |= (1U << 23);

    uint32_t exp = 250 + exp1 - exp2;
    uint64_t mant = ((uint64_t)mant1 << 24) / mant2;

    if (mant1 >= mant2) {
        mant >>= 1;
    } else {
        if(exp == 0){
            return 0;
        }
        exp -= 1;
    }
    mant &= 0x7FFFFFU;

    if (exp > 255) {
        exp = 0xFF;
        mant = 0x7FFFFFU;
    }

    uint32_t res = (flt1 ^ flt2) & 0x80000000U;
    res |= ((uint32_t)exp) << 23;
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
