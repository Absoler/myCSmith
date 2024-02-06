#ifndef MY_SAFE_MATH_H
#define MY_SAFE_MATH_H

#include <math.h>

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int int64_t;
typedef unsigned long int uint64_t;

static int8_t(safe_unary_minus_func_int8_t_s)(int8_t si) {
  return -si;
}

static int8_t(safe_add_func_int8_t_s_s)(int8_t si1, int8_t si2) {
  return (si1 + si2);
}

static int8_t(safe_sub_func_int8_t_s_s)(int8_t si1, int8_t si2) {
  return (si1 - si2);
}

static int8_t(safe_mul_func_int8_t_s_s)(int8_t si1, int8_t si2) {
  return si1 * si2;
}

static int8_t(safe_mod_func_int8_t_s_s)(int8_t si1, int8_t si2) {
  return

      ((si2 == 0) || ((si1 == (-128)) && (si2 == (-1)))) ? ((si1)) :

                                                         (si1 % si2);
}

static int8_t(safe_div_func_int8_t_s_s)(int8_t si1, int8_t si2) {
  return

      ((si2 == 0) || ((si1 == (-128)) && (si2 == (-1)))) ? ((si1)) :

                                                         (si1 / si2);
}

static int8_t(safe_lshift_func_int8_t_s_s)(int8_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((127) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static int8_t(safe_lshift_func_int8_t_s_u)(int8_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32) ||
       (left > ((127) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static int8_t(safe_rshift_func_int8_t_s_s)(int8_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32))
          ? ((left))
          :

          (left >> ((int)right));
}

static int8_t(safe_rshift_func_int8_t_s_u)(int8_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32))
          ? ((left))
          :

          (left >> ((unsigned int)right));
}

static int16_t(safe_unary_minus_func_int16_t_s)(int16_t si) {
  return

      -si;
}

static int16_t(safe_add_func_int16_t_s_s)(int16_t si1, int16_t si2) {
  return (si1 + si2);
}

static int16_t(safe_sub_func_int16_t_s_s)(int16_t si1, int16_t si2) {
  return (si1 - si2);
}

static int16_t(safe_mul_func_int16_t_s_s)(int16_t si1, int16_t si2) {
  return si1 * si2;
}

static int16_t(safe_mod_func_int16_t_s_s)(int16_t si1, int16_t si2) {
  return

      ((si2 == 0) || ((si1 == (-32767 - 1)) && (si2 == (-1)))) ? ((si1)) :

                                                               (si1 % si2);
}

static int16_t(safe_div_func_int16_t_s_s)(int16_t si1, int16_t si2) {
  return

      ((si2 == 0) || ((si1 == (-32767 - 1)) && (si2 == (-1)))) ? ((si1)) :

                                                               (si1 / si2);
}

static int16_t(safe_lshift_func_int16_t_s_s)(int16_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((32767) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static int16_t(safe_lshift_func_int16_t_s_u)(int16_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32) ||
       (left > ((32767) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static int16_t(safe_rshift_func_int16_t_s_s)(int16_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32))
          ? ((left))
          :

          (left >> ((int)right));
}

static int16_t(safe_rshift_func_int16_t_s_u)(int16_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32))
          ? ((left))
          :

          (left >> ((unsigned int)right));
}

static int32_t(safe_unary_minus_func_int32_t_s)(int32_t si) {
  return

      (si == (-2147483647 - 1)) ? ((si)) :

                                -si;
}

static int32_t(safe_add_func_int32_t_s_s)(int32_t si1, int32_t si2) {
  return

      (((si1 > 0) && (si2 > 0) && (si1 > ((2147483647) - si2))) ||
       ((si1 < 0) && (si2 < 0) && (si1 < ((-2147483647 - 1) - si2))))
          ? ((si1))
          :

          (si1 + si2);
}

static int32_t(safe_sub_func_int32_t_s_s)(int32_t si1, int32_t si2) {
  return

      (((si1 ^ si2) & (((si1 ^ ((si1 ^ si2) & (~(2147483647)))) - si2) ^ si2)) <
       0)
          ? ((si1))
          :

          (si1 - si2);
}

static int32_t(safe_mul_func_int32_t_s_s)(int32_t si1, int32_t si2) {
  return

      (((si1 > 0) && (si2 > 0) && (si1 > ((2147483647) / si2))) ||
       ((si1 > 0) && (si2 <= 0) && (si2 < ((-2147483647 - 1) / si1))) ||
       ((si1 <= 0) && (si2 > 0) && (si1 < ((-2147483647 - 1) / si2))) ||
       ((si1 <= 0) && (si2 <= 0) && (si1 != 0) && (si2 < ((2147483647) / si1))))
          ? ((si1))
          :

          si1 * si2;
}

static int32_t(safe_mod_func_int32_t_s_s)(int32_t si1, int32_t si2) {
  return

      ((si2 == 0) || ((si1 == (-2147483647 - 1)) && (si2 == (-1)))) ? ((si1)) :

                                                                    (si1 % si2);
}

static int32_t(safe_div_func_int32_t_s_s)(int32_t si1, int32_t si2) {
  return

      ((si2 == 0) || ((si1 == (-2147483647 - 1)) && (si2 == (-1)))) ? ((si1)) :

                                                                    (si1 / si2);
}

static int32_t(safe_lshift_func_int32_t_s_s)(int32_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((2147483647) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static int32_t(safe_lshift_func_int32_t_s_u)(int32_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32) ||
       (left > ((2147483647) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static int32_t(safe_rshift_func_int32_t_s_s)(int32_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32))
          ? ((left))
          :

          (left >> ((int)right));
}

static int32_t(safe_rshift_func_int32_t_s_u)(int32_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32))
          ? ((left))
          :

          (left >> ((unsigned int)right));
}

static int64_t(safe_unary_minus_func_int64_t_s)(int64_t si) {
  return

      (si == (-9223372036854775807L - 1)) ? ((si)) :

                                          -si;
}

static int64_t(safe_add_func_int64_t_s_s)(int64_t si1, int64_t si2) {
  return

      (((si1 > 0) && (si2 > 0) && (si1 > ((9223372036854775807L) - si2))) ||
       ((si1 < 0) && (si2 < 0) && (si1 < ((-9223372036854775807L - 1) - si2))))
          ? ((si1))
          :

          (si1 + si2);
}

static int64_t(safe_sub_func_int64_t_s_s)(int64_t si1, int64_t si2) {
  return

      (((si1 ^ si2) &
        (((si1 ^ ((si1 ^ si2) & (~(9223372036854775807L)))) - si2) ^ si2)) < 0)
          ? ((si1))
          :

          (si1 - si2);
}

static int64_t(safe_mul_func_int64_t_s_s)(int64_t si1, int64_t si2) {
  return

      (((si1 > 0) && (si2 > 0) && (si1 > ((9223372036854775807L) / si2))) ||
       ((si1 > 0) && (si2 <= 0) &&
        (si2 < ((-9223372036854775807L - 1) / si1))) ||
       ((si1 <= 0) && (si2 > 0) &&
        (si1 < ((-9223372036854775807L - 1) / si2))) ||
       ((si1 <= 0) && (si2 <= 0) && (si1 != 0) &&
        (si2 < ((9223372036854775807L) / si1))))
          ? ((si1))
          :

          si1 * si2;
}

static int64_t(safe_mod_func_int64_t_s_s)(int64_t si1, int64_t si2) {
  return

      ((si2 == 0) || ((si1 == (-9223372036854775807L - 1)) && (si2 == (-1))))
          ? ((si1))
          :

          (si1 % si2);
}

static int64_t(safe_div_func_int64_t_s_s)(int64_t si1, int64_t si2) {
  return

      ((si2 == 0) || ((si1 == (-9223372036854775807L - 1)) && (si2 == (-1))))
          ? ((si1))
          :

          (si1 / si2);
}

static int64_t(safe_lshift_func_int64_t_s_s)(int64_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((9223372036854775807L) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static int64_t(safe_lshift_func_int64_t_s_u)(int64_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32) ||
       (left > ((9223372036854775807L) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static int64_t(safe_rshift_func_int64_t_s_s)(int64_t left, int right) {
  return

      ((left < 0) || (((int)right) < 0) || (((int)right) >= 32))
          ? ((left))
          :

          (left >> ((int)right));
}

static int64_t(safe_rshift_func_int64_t_s_u)(int64_t left, unsigned int right) {
  return

      ((left < 0) || (((unsigned int)right) >= 32))
          ? ((left))
          :

          (left >> ((unsigned int)right));
}

static uint8_t(safe_unary_minus_func_uint8_t_u)(uint8_t ui) { return -ui; }

static uint8_t(safe_add_func_uint8_t_u_u)(uint8_t ui1, uint8_t ui2) {
  return ui1 + ui2;
}

static uint8_t(safe_sub_func_uint8_t_u_u)(uint8_t ui1, uint8_t ui2) {
  return ui1 - ui2;
}

static uint8_t(safe_mul_func_uint8_t_u_u)(uint8_t ui1, uint8_t ui2) {
  return ((unsigned int)ui1) * ((unsigned int)ui2);
}

static uint8_t(safe_mod_func_uint8_t_u_u)(uint8_t ui1, uint8_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 % ui2);
}

static uint8_t(safe_div_func_uint8_t_u_u)(uint8_t ui1, uint8_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 / ui2);
}

static uint8_t(safe_lshift_func_uint8_t_u_s)(uint8_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((255) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static uint8_t(safe_lshift_func_uint8_t_u_u)(uint8_t left, unsigned int right) {
  return

      ((((unsigned int)right) >= 32) ||
       (left > ((255) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static uint8_t(safe_rshift_func_uint8_t_u_s)(uint8_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32)) ? ((left)) :

                                                   (left >> ((int)right));
}

static uint8_t(safe_rshift_func_uint8_t_u_u)(uint8_t left, unsigned int right) {
  return

      (((unsigned int)right) >= 32) ? ((left)) :

                                    (left >> ((unsigned int)right));
}

static uint16_t(safe_unary_minus_func_uint16_t_u)(uint16_t ui) { return -ui; }

static uint16_t(safe_add_func_uint16_t_u_u)(uint16_t ui1, uint16_t ui2) {
  return ui1 + ui2;
}

static uint16_t(safe_sub_func_uint16_t_u_u)(uint16_t ui1, uint16_t ui2) {
  return ui1 - ui2;
}

static uint16_t(safe_mul_func_uint16_t_u_u)(uint16_t ui1, uint16_t ui2) {
  return ((unsigned int)ui1) * ((unsigned int)ui2);
}

static uint16_t(safe_mod_func_uint16_t_u_u)(uint16_t ui1, uint16_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 % ui2);
}

static uint16_t(safe_div_func_uint16_t_u_u)(uint16_t ui1, uint16_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 / ui2);
}

static uint16_t(safe_lshift_func_uint16_t_u_s)(uint16_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((65535) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static uint16_t(safe_lshift_func_uint16_t_u_u)(uint16_t left,
                                               unsigned int right) {
  return

      ((((unsigned int)right) >= 32) ||
       (left > ((65535) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static uint16_t(safe_rshift_func_uint16_t_u_s)(uint16_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32)) ? ((left)) :

                                                   (left >> ((int)right));
}

static uint16_t(safe_rshift_func_uint16_t_u_u)(uint16_t left,
                                               unsigned int right) {
  return

      (((unsigned int)right) >= 32) ? ((left)) :

                                    (left >> ((unsigned int)right));
}

static uint32_t(safe_unary_minus_func_uint32_t_u)(uint32_t ui) { return -ui; }

static uint32_t(safe_add_func_uint32_t_u_u)(uint32_t ui1, uint32_t ui2) {
  return ui1 + ui2;
}

static uint32_t(safe_sub_func_uint32_t_u_u)(uint32_t ui1, uint32_t ui2) {
  return ui1 - ui2;
}

static uint32_t(safe_mul_func_uint32_t_u_u)(uint32_t ui1, uint32_t ui2) {
  return ((unsigned int)ui1) * ((unsigned int)ui2);
}

static uint32_t(safe_mod_func_uint32_t_u_u)(uint32_t ui1, uint32_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 % ui2);
}

static uint32_t(safe_div_func_uint32_t_u_u)(uint32_t ui1, uint32_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 / ui2);
}

static uint32_t(safe_lshift_func_uint32_t_u_s)(uint32_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((4294967295U) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static uint32_t(safe_lshift_func_uint32_t_u_u)(uint32_t left,
                                               unsigned int right) {
  return

      ((((unsigned int)right) >= 32) ||
       (left > ((4294967295U) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static uint32_t(safe_rshift_func_uint32_t_u_s)(uint32_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32)) ? ((left)) :

                                                   (left >> ((int)right));
}

static uint32_t(safe_rshift_func_uint32_t_u_u)(uint32_t left,
                                               unsigned int right) {
  return

      (((unsigned int)right) >= 32) ? ((left)) :

                                    (left >> ((unsigned int)right));
}

static uint64_t(safe_unary_minus_func_uint64_t_u)(uint64_t ui) { return -ui; }

static uint64_t(safe_add_func_uint64_t_u_u)(uint64_t ui1, uint64_t ui2) {
  return ui1 + ui2;
}

static uint64_t(safe_sub_func_uint64_t_u_u)(uint64_t ui1, uint64_t ui2) {
  return ui1 - ui2;
}

static uint64_t(safe_mul_func_uint64_t_u_u)(uint64_t ui1, uint64_t ui2) {
  return ((unsigned long long)ui1) * ((unsigned long long)ui2);
}

static uint64_t(safe_mod_func_uint64_t_u_u)(uint64_t ui1, uint64_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 % ui2);
}

static uint64_t(safe_div_func_uint64_t_u_u)(uint64_t ui1, uint64_t ui2) {
  return

      (ui2 == 0) ? ((ui1)) :

                 (ui1 / ui2);
}

static uint64_t(safe_lshift_func_uint64_t_u_s)(uint64_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32) ||
       (left > ((18446744073709551615UL) >> ((int)right))))
          ? ((left))
          :

          (left << ((int)right));
}

static uint64_t(safe_lshift_func_uint64_t_u_u)(uint64_t left,
                                               unsigned int right) {
  return

      ((((unsigned int)right) >= 32) ||
       (left > ((18446744073709551615UL) >> ((unsigned int)right))))
          ? ((left))
          :

          (left << ((unsigned int)right));
}

static uint64_t(safe_rshift_func_uint64_t_u_s)(uint64_t left, int right) {
  return

      ((((int)right) < 0) || (((int)right) >= 32)) ? ((left)) :

                                                   (left >> ((int)right));
}

static uint64_t(safe_rshift_func_uint64_t_u_u)(uint64_t left,
                                               unsigned int right) {
  return

      (((unsigned int)right) >= 32) ? ((left)) :

                                    (left >> ((unsigned int)right));
}
static float(safe_add_func_float_f_f)(float sf1, float sf2) {
  return

      (fabsf((0.5f * sf1) + (0.5f * sf2)) >
       (0.5f * 3.40282346638528859811704183484516925e+38F))
          ? (sf1)
          :

          (sf1 + sf2);
}

static float(safe_sub_func_float_f_f)(float sf1, float sf2) {
  return

      (fabsf((0.5f * sf1) - (0.5f * sf2)) >
       (0.5f * 3.40282346638528859811704183484516925e+38F))
          ? (sf1)
          :

          (sf1 - sf2);
}

static float(safe_mul_func_float_f_f)(float sf1, float sf2) {
  return

      (fabsf((0x1.0p-100f * sf1) * (0x1.0p-28f * sf2)) >
       (0x1.0p-100f *
        (0x1.0p-28f * 3.40282346638528859811704183484516925e+38F)))
          ?

          (sf1)
          :

          (sf1 * sf2);
}

static float(safe_div_func_float_f_f)(float sf1, float sf2) {
  return

      ((fabsf(sf2) < 1.0f) &&
       (((sf2 == 0.0f) ||
         (fabsf((0x1.0p-49f * sf1) / (0x1.0p100f * sf2))) >
             (0x1.0p-100f *
              (0x1.0p-49f * 3.40282346638528859811704183484516925e+38F)))))
          ?

          (sf1)
          :

          (sf1 / sf2);
}

static double(safe_add_func_double_f_f)(double sf1, double sf2) {
  return

      (fabs((0.5 * sf1) + (0.5 * sf2)) >
       (0.5 * ((double)1.79769313486231570814527423731704357e+308L)))
          ? (sf1)
          :

          (sf1 + sf2);
}

static double(safe_sub_func_double_f_f)(double sf1, double sf2) {
  return

      (fabs((0.5 * sf1) - (0.5 * sf2)) >
       (0.5 * ((double)1.79769313486231570814527423731704357e+308L)))
          ? (sf1)
          :

          (sf1 - sf2);
}

static double(safe_mul_func_double_f_f)(double sf1, double sf2) {
  return

      (fabs((0x1.0p-100 * sf1) * (0x1.0p-924 * sf2)) >
       (0x1.0p-100 *
        (0x1.0p-924 * ((double)1.79769313486231570814527423731704357e+308L))))
          ?

          (sf1)
          :

          (sf1 * sf2);
}

static double(safe_div_func_double_f_f)(double sf1, double sf2) {
  return

      ((fabs(sf2) < 1.0) &&
       (((sf2 == 0.0) ||
         (fabs((0x1.0p-974 * sf1) / (0x1.0p100 * sf2))) >
             (0x1.0p-100 *
              (0x1.0p-974 *
               ((double)1.79769313486231570814527423731704357e+308L))))))
          ?

          (sf1)
          :

          (sf1 / sf2);
}
static int32_t(safe_convert_func_float_to_int32_t)(float sf1) {
  return

      ((sf1 <= (-2147483647 - 1)) || (sf1 >= (2147483647))) ? ((2147483647)) :

                                                            ((int32_t)(sf1));
}

#endif