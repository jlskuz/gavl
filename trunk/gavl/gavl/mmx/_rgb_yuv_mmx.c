/*****************************************************************

  _rgb_yuv_mmx.c

  Copyright (c) 2001 by Burkhard Plaum - plaum@ipf.uni-stuttgart.de

  http://gmerlin.sourceforge.net

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

/*
 *  Support for mmxext
 *  this macro procudes another set of
 *  functions in ../mmxext
 */

#ifdef MMXEXT
#define MOVQ_R2M(reg,mem) movntq_r2m(reg, mem)
#else
#define MOVQ_R2M(reg,mem) movq_r2m(reg, mem)
#endif

static mmx_t rgb_yuv_rgb24_l = { 0x0000000000FFFFFFLL };
static mmx_t rgb_yuv_rgb24_u = { 0x0000FFFFFF000000LL };

static mmx_t rgb_yuv_rgb16_upper_mask =   { 0xf800f800f800f800LL };
static mmx_t rgb_yuv_rgb16_middle_mask =  { 0x07e007e007e007e0LL };
static mmx_t rgb_yuv_rgb16_lower_mask =   { 0x001f001f001f001fLL };

static mmx_t rgb_yuv_rgb15_upper_mask =   { 0x7C007C007C007C00LL };
static mmx_t rgb_yuv_rgb15_middle_mask =  { 0x03e003e003e003e0LL };
static mmx_t rgb_yuv_rgb15_lower_mask =   { 0x001f001f001f001fLL };


#define RGB_YUV_LOAD_RGB_24 movq_m2r(*src, mm0);/*            mm0: G2 R2 B1 G1 R1 B0 G0 R0 */\
                            movd_m2r(*(src+8), mm1);/*          mm1: 00 00 00 00 B3 G3 R3 B2 */\
                            movq_r2r(mm0, mm2);/*             mm2: G2 R2 B1 G1 R1 B0 G0 R0 */\
                            psrlq_i2r(48, mm2);/*             mm2: 00 00 00 00 00 00 G2 R2 */\
                            psllq_i2r(16 , mm1);/*            mm1: 00 00 B3 G3 R3 B2 00 00 */\
                            por_r2r(mm2, mm1);/*              mm1: 00 00 B3 G3 R3 B2 G2 R2 */\
                            movq_r2r(mm0, mm2);/*             mm2: 00 00 B3 G3 R3 B2 G2 R2 */\
                            pand_m2r(rgb_yuv_rgb24_l, mm0);/* mm0: 00 00 00 00 00 B0 G0 R0 */\
                            pand_m2r(rgb_yuv_rgb24_u, mm2);\
                            psllq_i2r(8, mm2);\
                            por_r2r(mm2, mm0);\
                            movq_r2r(mm1, mm2);\
                            pand_m2r(rgb_yuv_rgb24_l, mm1);\
                            pand_m2r(rgb_yuv_rgb24_u, mm2);\
                            psllq_i2r(8, mm2);\
                            por_r2r(mm2, mm1);


#define LOAD_RGB_32 movq_m2r(*src, mm0);\
                    movq_m2r(*(src+8),mm1);


#define LOAD_RGB_15  movq_m2r(*src, mm5);\
                     pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm5, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_yuv_rgb15_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm0);\
                     punpcklbw_r2r(mm3, mm0);/* mm0: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm1);\
                     punpckhbw_r2r(mm3, mm1);/* mm1: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm5, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_yuv_rgb15_middle_mask, mm2);\
                     psrlq_i2r(2, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm0);/*       mm0: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm1);/*       mm1: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_yuv_rgb15_upper_mask, mm5);\
                     psllq_i2r(1, mm5);\
                     movq_r2r(mm5, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm0);\
                     punpckhbw_r2r(mm3, mm5);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm5, mm1);


#define LOAD_RGB_16  movq_m2r(*src, mm5);\
                     pxor_r2r(mm3, mm3);/* Zero mm3 */\
                     movq_r2r(mm5, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_yuv_rgb16_lower_mask, mm2);\
                     psllq_i2r(3, mm2);\
                     movq_r2r(mm2, mm0);\
                     punpcklbw_r2r(mm3, mm0);/* mm0: 00 00 00 R1 00 00 00 R0 */\
                     movq_r2r(mm2, mm1);\
                     punpckhbw_r2r(mm3, mm1);/* mm1: 00 00 00 R3 00 00 00 R2 */\
                     movq_r2r(mm5, mm2);/* mm2:      P3 P3 P2 P2 P1 P1 P0 P0 */\
                     pand_m2r(rgb_yuv_rgb16_middle_mask, mm2);\
                     psrlq_i2r(3, mm2);\
                     punpcklbw_r2r(mm2, mm3);/* mm3: 00 00 G1 00 00 00 G0 00 */\
                     por_r2r(mm3, mm0);/*       mm0: 00 00 G1 R1 00 00 G0 R0 */\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpckhbw_r2r(mm2, mm3);/* mm3: 00 00 00 R3 00 00 00 R2 */\
                     por_r2r(mm3, mm1);/*       mm1: 00 00 G3 R3 00 00 G2 R2 */\
                     pand_m2r(rgb_yuv_rgb16_upper_mask, mm5);\
                     movq_r2r(mm5, mm2);\
                     pxor_r2r(mm3, mm3);/*      Zero mm3;*/\
                     punpcklbw_r2r(mm3, mm2);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm2, mm0);\
                     punpckhbw_r2r(mm3, mm5);/* mm2: 00 B1 00 00 00 B0 00 00 */\
                     por_r2r(mm5, mm1);


// RGB to Y conversion
// Input:

// mm0: 00 B1 G1 R1 00 B0 G0 R0
// mm1: 00 B3 G3 R3 00 B2 G2 R2



#define R_TO_Y (int16_t)( 0.257*32768.0)
#define G_TO_Y (int16_t)( 0.504*32768.0)
#define B_TO_Y (int16_t)( 0.098*32768.0)

#define R_TO_U (int16_t)(-0.148*32768.0)
#define G_TO_U (int16_t)(-0.291*32768.0)
#define B_TO_U (int16_t)( 0.439*32768.0)

#define R_TO_V (int16_t)( 0.439*32768.0)
#define G_TO_V (int16_t)(-0.368*32768.0)
#define B_TO_V (int16_t)(-0.071*32768.0)

static mmx_t rgb_to_y = { w: { R_TO_Y, G_TO_Y, B_TO_Y, 0 } };
static mmx_t rgb_to_u = { w: { R_TO_U, G_TO_U, B_TO_U, 0 } };
static mmx_t rgb_to_v = { w: { R_TO_V, G_TO_V, B_TO_V, 0 } };

static mmx_t bgr_to_y = { w: { B_TO_Y, G_TO_Y, R_TO_Y, 0 } };
static mmx_t bgr_to_u = { w: { B_TO_U, G_TO_U, R_TO_U, 0 } };
static mmx_t bgr_to_v = { w: { B_TO_V, G_TO_V, R_TO_V, 0 } };

static mmx_t rgb_yuv_y_add = { 0x0010001000100010LL };

static mmx_t lower_dword = { 0x00000000FFFFFFFFLL};
static mmx_t upper_dword = { 0xFFFFFFFF00000000LL};
static mmx_t chroma_mask = { 0x000000FF000000FFLL };

static mmx_t sign_mask =   { 0x0000008000000080LL };

#define RGB_TO_Y pxor_r2r(mm7, mm7);/*           Zero mm7 */\
                 movq_r2r(mm0, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpcklbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B0 00 G0 00 R0 */\
                 pmaddwd_m2r(rgb_to_y, mm2);/*   mm2: 00 00 BY*B0 GY*G0+RY*R0 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B0 GY*G0+RY*R0 */\
                 pand_m2r(lower_dword, mm3);/*   mm3: 00 00 00 00 GY*G0+RY*R0 */\
                 movq_r2r(mm3, mm4);/*           mm4: 00 00 00 00 GY*G0+RY*R0 */\
                 psrlq_i2r(32, mm2);/*           mm2: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm2, mm5);/*           mm5: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm0, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpckhbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B1 00 G1 00 R1 */\
                 pmaddwd_m2r(rgb_to_y, mm2);/*   mm2: 00 00 BY*B1 GY*G1+RY*R1 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B1 GY*G1+RY*R1 */\
                 psllq_i2r(32, mm3);/*           mm3: GY*G1+RY*R1 00 00 00 00 */\
                 por_r2r(mm3, mm4);/*            mm4: GY*G1+RY*R1 GY*G0+RY*R0 */\
                 pand_m2r(upper_dword, mm2);/*   mm2: 00 00 BY*B1 00 00 00 00 */\
                 por_r2r(mm2, mm5);/*            mm5: 00 00 BY*B1 00 00 BY*B0 */\
                 paddd_r2r(mm4, mm5);/*          mm5: 00 Y1 Y1 Y1 00 Y0 Y0 Y0 */\
                 psrld_i2r(15, mm5);/*           mm5: 00 00 00 Y1 00 00 00 Y0 */\
                 packssdw_r2r(mm7, mm5);/*       mm5: 00 00 00 00 00 Y1 00 Y0 */\
                 movq_r2r(mm5, mm6);/*           mm6: 00 00 00 00 00 Y1 00 Y0 */\
                 movq_r2r(mm1, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpcklbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B0 00 G0 00 R0 */\
                 pmaddwd_m2r(rgb_to_y, mm2);/*   mm2: 00 00 BY*B0 GY*G0+RY*R0 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B0 GY*G0+RY*R0 */\
                 pand_m2r(lower_dword, mm3);/*   mm3: 00 00 00 00 GY*G0+RY*R0 */\
                 movq_r2r(mm3, mm4);/*           mm4: 00 00 00 00 GY*G0+RY*R0 */\
                 psrlq_i2r(32, mm2);/*           mm2: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm2, mm5);/*           mm5: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm1, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpckhbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B1 00 G1 00 R1 */\
                 pmaddwd_m2r(rgb_to_y, mm2);/*   mm2: 00 00 BY*B1 GY*G1+RY*R1 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B1 GY*G1+RY*R1 */\
                 psllq_i2r(32, mm3);/*           mm3: GY*G1+RY*R1 00 00 00 00 */\
                 por_r2r(mm3, mm4);/*            mm4: GY*G1+RY*R1 GY*G0+RY*R0 */\
                 pand_m2r(upper_dword, mm2);/*   mm2: 00 00 BY*B1 00 00 00 00 */\
                 por_r2r(mm2, mm5);/*            mm5: 00 00 BY*B1 00 00 BY*B0 */\
                 paddd_r2r(mm4, mm5);/*          mm5: 00 Y1 Y1 Y1 00 Y0 Y0 Y0 */\
                 psrld_i2r(15, mm5);/*           mm5: 00 00 00 Y1 00 00 00 Y0 */\
                 packssdw_r2r(mm5, mm7);/*       mm7: 00 Y1 00 Y0 00 00 00 00 */\
                 por_r2r(mm7, mm6);/*            mm6: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                 paddusb_m2r(rgb_yuv_y_add, mm6);


#define RGB_TO_UV_2 pxor_r2r(mm7, mm7);/*         Zero mm7 */\
                    movq_r2r(mm0, mm2);/*         mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                    punpcklbw_r2r(mm7, mm2);/*    mm2: 00 00 00 B0 00 G0 00 R0 */\
                    pmaddwd_m2r(rgb_to_u, mm2);/* mm2: 00 00 BU*B0 GU*G0+RU*R0 */\
                    movq_r2r(mm2, mm4);/*         mm4: 00 00 BU*B0 GU*G0+RU*R0 */\
                    pand_m2r(lower_dword, mm4);/* mm4: 00 00 00 00 GU*G0+RU*R0 */\
                    psrlq_i2r(32, mm2);        /* mm2: 00 00 00 00 00 00 BU*B0 */\
                    paddd_r2r(mm2, mm4);/*        mm4: 00 00 00 00 00 U0 U0 U0 */\
                    punpcklbw_r2r(mm7, mm0);/*    mm2: 00 00 00 B0 00 G0 00 R0 */\
                    pmaddwd_m2r(rgb_to_v, mm0);/* mm2: 00 00 BV*B0 GV*G0+RV*R0 */\
                    movq_r2r(mm0, mm5);/*         mm3: 00 00 BV*B0 GV*G0+RV*R0 */\
                    pand_m2r(lower_dword, mm5);/* mm5: 00 00 00 00 GV*G0+RV*R0 */\
                    psrlq_i2r(32, mm0);        /* mm2: 00 00 00 00 00 00 BV*B0 */\
                    paddd_r2r(mm0, mm5);/*        mm5: 00 00 00 00 00 V0 V0 V0 */\
                    movq_r2r(mm1, mm2);/*         mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                    punpcklbw_r2r(mm7, mm2);/*    mm1: 00 00 00 B2 00 G2 00 R2 */\
                    pmaddwd_m2r(rgb_to_u, mm2);/* mm1: 00 00 BU*B2 GU*G2+RU*R2 */\
                    movq_r2r(mm2, mm3);/*         mm3: 00 00 BU*B2 GU*G2+RU*R2 */\
                    pand_m2r(lower_dword, mm3);/* mm4: 00 00 00 00 GU*G2+RU*R2 */\
                    psrlq_i2r(32, mm2);        /* mm2: 00 00 00 00 00 00 BU*B2 */\
                    paddd_r2r(mm2, mm3);/*        mm3: 00 00 00 00 00 U2 U2 U2 */\
                    psllq_i2r(32, mm3);        /* mm3: 00 U2 U2 U2 00 00 00 00 */\
                    por_r2r(mm3, mm4);/*          mm4: 00 U2 U2 U2 00 U0 U0 U0 */\
                    punpcklbw_r2r(mm7, mm1);/*    mm1: 00 00 00 B2 00 G2 00 R2 */\
                    pmaddwd_m2r(rgb_to_v, mm1);/* mm1: 00 00 BV*B2 GV*G2+RV*R2 */\
                    movq_r2r(mm1, mm3);/*         mm3: 00 00 BV*B2 GV*G2+RV*R2 */\
                    pand_m2r(lower_dword, mm3);/* mm5: 00 00 00 00 GV*G2+RV*R2 */\
                    psrlq_i2r(32, mm1);        /* mm2: 00 00 00 00 00 00 BV*B2 */\
                    paddd_r2r(mm1, mm3);/*        mm3: 00 00 00 00 00 V2 V2 V2 */\
                    psllq_i2r(32, mm3);        /* mm3: 00 V2 V2 V2 00 00 00 00 */\
                    por_r2r(mm3, mm5);/*          mm4: 00 V2 V2 V2 00 V0 V0 V0 */\
                    psrad_i2r(15, mm4);/*         mm4: 00 00 00 U2 00 00 00 U0 */\
                    psrad_i2r(15, mm5);/*         mm4: 00 00 00 V2 00 00 00 V0 */\
                    pand_m2r(chroma_mask, mm4);\
                    pand_m2r(chroma_mask, mm5);\
                    pxor_m2r(sign_mask, mm4);\
                    pxor_m2r(sign_mask, mm5); 

#define BGR_TO_Y pxor_r2r(mm7, mm7);/*           Zero mm7 */\
                 movq_r2r(mm0, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpcklbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B0 00 G0 00 R0 */\
                 pmaddwd_m2r(bgr_to_y, mm2);/*   mm2: 00 00 BY*B0 GY*G0+RY*R0 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B0 GY*G0+RY*R0 */\
                 pand_m2r(lower_dword, mm3);/*   mm3: 00 00 00 00 GY*G0+RY*R0 */\
                 movq_r2r(mm3, mm4);/*           mm4: 00 00 00 00 GY*G0+RY*R0 */\
                 psrlq_i2r(32, mm2);/*           mm2: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm2, mm5);/*           mm5: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm0, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpckhbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B1 00 G1 00 R1 */\
                 pmaddwd_m2r(bgr_to_y, mm2);/*   mm2: 00 00 BY*B1 GY*G1+RY*R1 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B1 GY*G1+RY*R1 */\
                 psllq_i2r(32, mm3);/*           mm3: GY*G1+RY*R1 00 00 00 00 */\
                 por_r2r(mm3, mm4);/*            mm4: GY*G1+RY*R1 GY*G0+RY*R0 */\
                 pand_m2r(upper_dword, mm2);/*   mm2: 00 00 BY*B1 00 00 00 00 */\
                 por_r2r(mm2, mm5);/*            mm5: 00 00 BY*B1 00 00 BY*B0 */\
                 paddd_r2r(mm4, mm5);/*          mm5: 00 Y1 Y1 Y1 00 Y0 Y0 Y0 */\
                 psrld_i2r(15, mm5);/*           mm5: 00 00 00 Y1 00 00 00 Y0 */\
                 packssdw_r2r(mm7, mm5);/*       mm5: 00 00 00 00 00 Y1 00 Y0 */\
                 movq_r2r(mm5, mm6);/*           mm6: 00 00 00 00 00 Y1 00 Y0 */\
                 movq_r2r(mm1, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpcklbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B0 00 G0 00 R0 */\
                 pmaddwd_m2r(bgr_to_y, mm2);/*   mm2: 00 00 BY*B0 GY*G0+RY*R0 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B0 GY*G0+RY*R0 */\
                 pand_m2r(lower_dword, mm3);/*   mm3: 00 00 00 00 GY*G0+RY*R0 */\
                 movq_r2r(mm3, mm4);/*           mm4: 00 00 00 00 GY*G0+RY*R0 */\
                 psrlq_i2r(32, mm2);/*           mm2: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm2, mm5);/*           mm5: 00 00 00 00 00 00 BY*B0 */\
                 movq_r2r(mm1, mm2);/*           mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                 punpckhbw_r2r(mm7, mm2);/*      mm2: 00 00 00 B1 00 G1 00 R1 */\
                 pmaddwd_m2r(bgr_to_y, mm2);/*   mm2: 00 00 BY*B1 GY*G1+RY*R1 */\
                 movq_r2r(mm2, mm3);/*           mm3: 00 00 BY*B1 GY*G1+RY*R1 */\
                 psllq_i2r(32, mm3);/*           mm3: GY*G1+RY*R1 00 00 00 00 */\
                 por_r2r(mm3, mm4);/*            mm4: GY*G1+RY*R1 GY*G0+RY*R0 */\
                 pand_m2r(upper_dword, mm2);/*   mm2: 00 00 BY*B1 00 00 00 00 */\
                 por_r2r(mm2, mm5);/*            mm5: 00 00 BY*B1 00 00 BY*B0 */\
                 paddd_r2r(mm4, mm5);/*          mm5: 00 Y1 Y1 Y1 00 Y0 Y0 Y0 */\
                 psrld_i2r(15, mm5);/*           mm5: 00 00 00 Y1 00 00 00 Y0 */\
                 packssdw_r2r(mm5, mm7);/*       mm7: 00 Y1 00 Y0 00 00 00 00 */\
                 por_r2r(mm7, mm6);/*            mm6: 00 Y3 00 Y2 00 Y1 00 Y0 */\
                 paddusb_m2r(rgb_yuv_y_add, mm6);

#define BGR_TO_UV_2 pxor_r2r(mm7, mm7);/*         Zero mm7 */\
                    movq_r2r(mm0, mm2);/*         mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                    punpcklbw_r2r(mm7, mm2);/*    mm2: 00 00 00 B0 00 G0 00 R0 */\
                    pmaddwd_m2r(bgr_to_u, mm2);/* mm2: 00 00 BU*B0 GU*G0+RU*R0 */\
                    movq_r2r(mm2, mm4);/*         mm4: 00 00 BU*B0 GU*G0+RU*R0 */\
                    pand_m2r(lower_dword, mm4);/* mm4: 00 00 00 00 GU*G0+RU*R0 */\
                    psrlq_i2r(32, mm2);        /* mm2: 00 00 00 00 00 00 BU*B0 */\
                    paddd_r2r(mm2, mm4);/*        mm4: 00 00 00 00 00 U0 U0 U0 */\
                    punpcklbw_r2r(mm7, mm0);/*    mm2: 00 00 00 B0 00 G0 00 R0 */\
                    pmaddwd_m2r(bgr_to_v, mm0);/* mm2: 00 00 BV*B0 GV*G0+RV*R0 */\
                    movq_r2r(mm0, mm5);/*         mm3: 00 00 BV*B0 GV*G0+RV*R0 */\
                    pand_m2r(lower_dword, mm5);/* mm5: 00 00 00 00 GV*G0+RV*R0 */\
                    psrlq_i2r(32, mm0);        /* mm2: 00 00 00 00 00 00 BV*B0 */\
                    paddd_r2r(mm0, mm5);/*        mm5: 00 00 00 00 00 V0 V0 V0 */\
                    movq_r2r(mm1, mm2);/*         mm2: 00 B1 G1 R1 00 B0 G0 R0 */\
                    punpcklbw_r2r(mm7, mm2);/*    mm1: 00 00 00 B2 00 G2 00 R2 */\
                    pmaddwd_m2r(bgr_to_u, mm2);/* mm1: 00 00 BU*B2 GU*G2+RU*R2 */\
                    movq_r2r(mm2, mm3);/*         mm3: 00 00 BU*B2 GU*G2+RU*R2 */\
                    pand_m2r(lower_dword, mm3);/* mm4: 00 00 00 00 GU*G2+RU*R2 */\
                    psrlq_i2r(32, mm2);        /* mm2: 00 00 00 00 00 00 BU*B2 */\
                    paddd_r2r(mm2, mm3);/*        mm3: 00 00 00 00 00 U2 U2 U2 */\
                    psllq_i2r(32, mm3);        /* mm3: 00 U2 U2 U2 00 00 00 00 */\
                    por_r2r(mm3, mm4);/*          mm4: 00 U2 U2 U2 00 U0 U0 U0 */\
                    punpcklbw_r2r(mm7, mm1);/*    mm1: 00 00 00 B2 00 G2 00 R2 */\
                    pmaddwd_m2r(bgr_to_v, mm1);/* mm1: 00 00 BV*B2 GV*G2+RV*R2 */\
                    movq_r2r(mm1, mm3);/*         mm3: 00 00 BV*B2 GV*G2+RV*R2 */\
                    pand_m2r(lower_dword, mm3);/* mm5: 00 00 00 00 GV*G2+RV*R2 */\
                    psrlq_i2r(32, mm1);        /* mm2: 00 00 00 00 00 00 BV*B2 */\
                    paddd_r2r(mm1, mm3);/*        mm3: 00 00 00 00 00 V2 V2 V2 */\
                    psllq_i2r(32, mm3);        /* mm3: 00 V2 V2 V2 00 00 00 00 */\
                    por_r2r(mm3, mm5);/*          mm4: 00 V2 V2 V2 00 V0 V0 V0 */\
                    psrad_i2r(15, mm4);/*         mm4: 00 00 00 U2 00 00 00 U0 */\
                    psrad_i2r(15, mm5);/*         mm4: 00 00 00 V2 00 00 00 V0 */\
                    pand_m2r(chroma_mask, mm4);\
                    pand_m2r(chroma_mask, mm5);\
                    pxor_m2r(sign_mask, mm4);\
                    pxor_m2r(sign_mask, mm5); 



#define OUTPUT_YUY2 psllq_i2r(8, mm4);\
                    por_r2r(mm4, mm6);\
                    psllq_i2r(24, mm5);\
                    por_r2r(mm5, mm6);\
                    MOVQ_R2M(mm6, *dst);

#define OUTPUT_PLANAR_Y pxor_r2r(mm7, mm7);\
                        packuswb_r2r(mm7, mm6);\
                        movd_r2m(mm6, *dst_y);

#define OUTPUT_PLANAR_UV2 pxor_r2r(mm7, mm7);\
                          packssdw_r2r(mm7, mm4);\
                          packssdw_r2r(mm7, mm5);\
                          packuswb_r2r(mm7, mm4);\
                          packuswb_r2r(mm7, mm5);\
                          movd_r2m(mm4, i_tmp);\
                          *((uint16_t*)(dst_u))=(uint16_t)i_tmp;\
                          movd_r2m(mm5, i_tmp);\
                          *((uint16_t*)(dst_v))=(uint16_t)i_tmp;


/***************************************
 * -> YUY2
 ***************************************/

#define FUNC_NAME   rgb_15_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_15 \
    BGR_TO_Y \
    BGR_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_15_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_15 \
    RGB_TO_Y \
    RGB_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_16_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_16 \
    BGR_TO_Y \
    BGR_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_16_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  8
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_16 \
    RGB_TO_Y \
    RGB_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"


#define FUNC_NAME   rgb_24_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  12
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    RGB_YUV_LOAD_RGB_24 \
    RGB_TO_Y \
    RGB_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_24_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  12
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    RGB_YUV_LOAD_RGB_24 \
    BGR_TO_Y \
    BGR_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   rgb_32_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_32 \
    RGB_TO_Y \
    RGB_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

#define FUNC_NAME   bgr_32_to_yuy2_mmx
#define IN_TYPE     uint8_t
#define OUT_TYPE    uint8_t
#define IN_ADVANCE  16
#define OUT_ADVANCE 8
#define NUM_PIXELS  4
#define CONVERT     \
    LOAD_RGB_32 \
    BGR_TO_Y \
    BGR_TO_UV_2 \
    OUTPUT_YUY2

#define CLEANUP emms();

// #define INIT

#include "../csp_packed_packed.h"

/***************************************
 * -> YUV 420 P
 ***************************************/

#define FUNC_NAME      rgb_15_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_15 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_15 \
  BGR_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();


#include "../csp_packed_planar.h"


#define FUNC_NAME      bgr_15_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_15 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_15 \
  RGB_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      rgb_16_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_16 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_16 \
  BGR_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      bgr_16_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_16 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_16 \
  RGB_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      rgb_24_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  RGB_YUV_LOAD_RGB_24 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  RGB_YUV_LOAD_RGB_24 \
  RGB_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      bgr_24_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  RGB_YUV_LOAD_RGB_24 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  RGB_YUV_LOAD_RGB_24 \
  BGR_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"


#define FUNC_NAME      rgb_32_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_32 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_32 \
  RGB_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"


#define FUNC_NAME      bgr_32_to_yuv_420_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_32 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CONVERT_Y      \
  LOAD_RGB_32 \
  BGR_TO_Y \
  OUTPUT_PLANAR_Y

#define CHROMA_SUB     2
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"


/***************************************
 * -> YUV 422 P
 ***************************************/

#define FUNC_NAME      rgb_15_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_15 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();


#include "../csp_packed_planar.h"


#define FUNC_NAME      bgr_15_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_15 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      rgb_16_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_16 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      bgr_16_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     8
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_16 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      rgb_24_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  RGB_YUV_LOAD_RGB_24 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"

#define FUNC_NAME      bgr_24_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     12
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  RGB_YUV_LOAD_RGB_24 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"


#define FUNC_NAME      rgb_32_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_32 \
  RGB_TO_Y \
  RGB_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"


#define FUNC_NAME      bgr_32_to_yuv_422_p_mmx
#define IN_TYPE        uint8_t
#define OUT_TYPE       uint8_t
#define IN_ADVANCE     16
#define OUT_ADVANCE_Y  4
#define OUT_ADVANCE_UV 2
#define NUM_PIXELS     4
#define CONVERT_YUV    \
  LOAD_RGB_32 \
  BGR_TO_Y \
  BGR_TO_UV_2 \
  OUTPUT_PLANAR_Y \
  OUTPUT_PLANAR_UV2

#define CHROMA_SUB     1
#define INIT           int i_tmp;
#define CLEANUP        emms();

#include "../csp_packed_planar.h"



/***************************************
 * -> END
 ***************************************/



static void rgba_32_to_yuy2_mmx(gavl_video_convert_context_t * ctx)
  {

  }

static void rgba_32_to_yuv_420_p_mmx(gavl_video_convert_context_t * ctx)
  {

  }

static void rgba_32_to_yuv_422_p_mmx(gavl_video_convert_context_t * ctx)
  {

  }

#ifdef MMXEXT

#ifdef SCANLINE
void
gavl_init_rgb_yuv_scanline_funcs_mmxext(gavl_colorspace_function_table_t * tab,
                                        int width)
#else     
void gavl_init_rgb_yuv_funcs_mmxext(gavl_colorspace_function_table_t * tab,
                                    int width)
#endif

#else /* !MMXEXT */

#ifdef SCANLINE
void
gavl_init_rgb_yuv_scanline_funcs_mmx(gavl_colorspace_function_table_t * tab,
                                     int width)
#else     
void
gavl_init_rgb_yuv_funcs_mmx(gavl_colorspace_function_table_t * tab,
                            int width)
#endif
     
#endif /* MMXEXT */
  {
  if(width % 4)
    return;

  tab->rgb_15_to_yuy2 =      rgb_15_to_yuy2_mmx;
  tab->rgb_15_to_yuv_420_p = rgb_15_to_yuv_420_p_mmx;
  tab->rgb_15_to_yuv_422_p = rgb_15_to_yuv_422_p_mmx;

  tab->bgr_15_to_yuy2 =      bgr_15_to_yuy2_mmx;
  tab->bgr_15_to_yuv_420_p = bgr_15_to_yuv_420_p_mmx;
  tab->bgr_15_to_yuv_422_p = bgr_15_to_yuv_422_p_mmx;

  tab->rgb_16_to_yuy2 =      rgb_16_to_yuy2_mmx;
  tab->rgb_16_to_yuv_420_p = rgb_16_to_yuv_420_p_mmx;
  tab->rgb_16_to_yuv_422_p = rgb_16_to_yuv_422_p_mmx;

  tab->bgr_16_to_yuy2 =      bgr_16_to_yuy2_mmx;
  tab->bgr_16_to_yuv_420_p = bgr_16_to_yuv_420_p_mmx;
  tab->bgr_16_to_yuv_422_p = bgr_16_to_yuv_422_p_mmx;

  tab->rgb_24_to_yuy2 =      rgb_24_to_yuy2_mmx;
  tab->rgb_24_to_yuv_420_p = rgb_24_to_yuv_420_p_mmx;
  tab->rgb_24_to_yuv_422_p = rgb_24_to_yuv_422_p_mmx;

  tab->bgr_24_to_yuy2 =      bgr_24_to_yuy2_mmx;
  tab->bgr_24_to_yuv_420_p = bgr_24_to_yuv_420_p_mmx;
  tab->bgr_24_to_yuv_422_p = bgr_24_to_yuv_422_p_mmx;
  
  tab->rgb_32_to_yuy2 =      rgb_32_to_yuy2_mmx;
  tab->rgb_32_to_yuv_420_p = rgb_32_to_yuv_420_p_mmx;
  tab->rgb_32_to_yuv_422_p = rgb_32_to_yuv_422_p_mmx;

  tab->bgr_32_to_yuy2 =      bgr_32_to_yuy2_mmx;
  tab->bgr_32_to_yuv_420_p = bgr_32_to_yuv_420_p_mmx;
  tab->bgr_32_to_yuv_422_p = bgr_32_to_yuv_422_p_mmx;

  //  tab->rgba_32_to_yuy2 =      rgba_32_to_yuy2_mmx;
  //  tab->rgba_32_to_yuv_420_p = rgba_32_to_yuv_420_p_mmx;
  //  tab->rgba_32_to_yuv_422_p = rgba_32_to_yuv_422_p_mmx;

  }
