#include "fractal_myflpt.h"
#include "swap.h"
#include "vga.h"
#include "cache.h"
#include <stddef.h>
#include <stdio.h>

// Constants describing the output device
const int SCREEN_WIDTH = 512;   //!< screen width
const int SCREEN_HEIGHT = 512;  //!< screen height

// Constants describing the initial view port on the fractal function
// -2.0 = -1.0 * 2^1 -> exp = 1, mant = 0
const flt_8exp_t CX_0 = FLT_MAKE(1, 1, 0);      //!< default start x-coordinate (-2.0)
// -1.5 = -1.5 * 2^0 -> exp = 0, mant = 0.5 (bit MANTISA_BITS - 1)
const flt_8exp_t CY_0 = FLT_MAKE(1, 0, 1U << (MANTISA_BITS - 1));      //!< default start y-coordinate (-1.5)
const uint16_t N_MAX = 64;    //!< maximum number of iterations

int main() {
   volatile unsigned int *vga = (unsigned int *) 0x50000020;
   volatile unsigned int reg, hi;
   rgb565 frameBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
   // delta = 3.0 / 512 = 1.5 * 2^(-8) -> exp = -8, mant = 0.5 (bit MANTISA_BITS - 1)
   volatile flt_8exp_t delta = FLT_MAKE(0, -8, 1U << (MANTISA_BITS - 1));
   int i;
   vga_clear();
   printf("Starting drawing a fractal\n");
#ifdef __OR1300__   
   /* enable the caches */
   icache_write_cfg( CACHE_DIRECT_MAPPED | CACHE_SIZE_8K | CACHE_REPLACE_FIFO );
   dcache_write_cfg( CACHE_FOUR_WAY | CACHE_SIZE_8K | CACHE_REPLACE_LRU | CACHE_WRITE_BACK );
   icache_enable(1);
   dcache_enable(1);
#endif
   // printf("");
   /* Enable the vga-controller's graphic mode */
   vga[0] = swap_u32(SCREEN_WIDTH);
   vga[1] = swap_u32(SCREEN_HEIGHT);
   vga[2] = swap_u32(1);
   vga[3] = swap_u32((unsigned int)&frameBuffer[0]);
   /* Clear screen */
   for (i = 0 ; i < SCREEN_WIDTH*SCREEN_HEIGHT ; i++) frameBuffer[i]=0;

   draw_fractal(frameBuffer,SCREEN_WIDTH,SCREEN_HEIGHT,&calc_mandelbrot_point_soft, &iter_to_colour,CX_0,CY_0,delta,N_MAX);
#ifdef __OR1300__
   dcache_flush();
#endif
   printf("Done\n");
}
