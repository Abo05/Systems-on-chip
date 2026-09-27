#ifndef FRACTAL_FXPT_H
#define FRACTAL_FXPT_H
#define DECIMAL_BITS 28
#define FIXED_MAX ((1 << (32 - DECIMAL_BITS)) - 1)
#define FIXED_MIN (-(1 << (32 - DECIMAL_BITS)))
#define DOUBLE_TO_FXPT_4_28(a)                                                 \
  ((fxpt_4_28)(((a) >= (FIXED_MAX))   ? (INT32_MAX)                            \
               : ((a) <= (FIXED_MIN)) ? (INT32_MIN)                            \
                                      : ((a) * (1 << DECIMAL_BITS))))
#include <stdint.h>

//! Colour type (5-bit red, 6-bit green, 5-bit blue)
typedef uint16_t rgb565;
// Q4.28
typedef int32_t fxpt_4_28;

fxpt_4_28 fixed_add(fxpt_4_28 a, fxpt_4_28 b);

//! \brief Pointer to fractal point calculation function
typedef uint16_t (*calc_frac_point_p)(fxpt_4_28 cx, fxpt_4_28 cy,
                                      uint16_t n_max);

uint16_t calc_mandelbrot_point_soft(fxpt_4_28 cx, fxpt_4_28 cy, uint16_t n_max);

//! Pointer to function mapping iteration to colour value
typedef rgb565 (*iter_to_colour_p)(uint16_t iter, uint16_t n_max);

rgb565 iter_to_bw(uint16_t iter, uint16_t n_max);
rgb565 iter_to_grayscale(uint16_t iter, uint16_t n_max);
rgb565 iter_to_colour(uint16_t iter, uint16_t n_max);

void draw_fractal(rgb565 *fbuf, int width, int height, calc_frac_point_p cfp_p,
                  iter_to_colour_p i2c_p, fxpt_4_28 cx_0, fxpt_4_28 cy_0,
                  fxpt_4_28 delta, uint16_t n_max);

#endif // FRACTAL_FXPT_H
