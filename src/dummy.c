#include <stdio.h>
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

struct ptr_s
{
  uint32_t low;
  uint32_t high;
};

#else

struct ptr_s
{
  uint32_t high;
  uint32_t low;
};

#endif

union ptr_u
{
  struct ptr_s p;
  uint64_t v;
};

int main(int argc, char** argv)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  printf("LITTLE_ENDIAN\n");
#else
   printf("BIG_ENDIAN\n");
#endif
   
   struct s_s
   {
     uint32_t low;
     uint32_t high;
   };

   union u_u
   {
     struct s_s s;
     uint64_t c;
     unsigned char d[8];
   };
   printf("sizeof(s_s): %ld\n", sizeof(struct s_s));
   printf("sizeof(u_u): %ld\n", sizeof(union u_u));

   union u_u u;

   u.s.low =  0xDDCCBBAA;
   u.s.high = 0xFFFFFFFF;
   for (int i = 0; i < 8; i++)
     printf("%d : 0x%x\n", i, u.d[i]);
   printf("0x%016lx\n", u.c);
  return 0;
}
#include <endian.h>
// uint64_t be64toh(uint64_t big_endian_64bits);
// uint64_t le64toh(uint64_t little_endian_64bits);
