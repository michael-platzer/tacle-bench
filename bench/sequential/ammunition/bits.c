/*

  FILE NAME:   bits.c

  TITLE:       Package for work with bits

  DESCRIPTION: This file implements functions of the package for work
      with bit strings.  A bit is given by address (start address) of
      byte from which counting bits starts and its displacement which
      is any non negative number of bit from the start address.  The
      most significant bit of the start address byte has number 0.
      The bit string is given by its first bit and its length in
      bits.

*/

#include "bits.h"

/* This function determines that given bit string contains only zero
   bits.  The function retruns TRUE if all bits of given bit string
   are zero or `bit_length' <= 0.  Value of `bit_displacement' must be
   non-negative and can be greater than CHAR_BIT. */

int
ammunition_is_zero_bit_string ( const void *start_byte, int bit_displacement,
                                int bit_length )
{
  const unsigned char *current_byte = ( unsigned const char * )start_byte;

  if ( bit_length <= 0 )
    return 1 /* TRUE */;
  current_byte += bit_displacement / CHAR_BIT;
  bit_displacement %= CHAR_BIT;
  if ( bit_length < CHAR_BIT - bit_displacement )
    return ( ( ( *current_byte << bit_displacement )
               & ( UCHAR_MAX << ( CHAR_BIT - bit_length ) ) )
             & UCHAR_MAX ) == 0;
  else
    if ( bit_displacement != 0 ) {
      if ( ( ( *current_byte << bit_displacement ) & UCHAR_MAX ) != 0 )
        return 0 /* FALSE */;
      current_byte += 1;
      bit_length -= CHAR_BIT - bit_displacement;
    }
  _Pragma( "loopbound min 0 max 7" )
  while ( bit_length >= CHAR_BIT ) {
    if ( *current_byte != 0 )
      return 0 /* FALSE */;
    current_byte++;
    bit_length -= CHAR_BIT;
  }
  if ( bit_length > 0 && ( *current_byte >> ( CHAR_BIT - bit_length ) ) != 0 )
    return 0 /* FALSE */;
  return 1 /* TRUE */;
}

/* This function sets up new value of all bits of given bit string.
   This function is analog of standard C function `memset'.  Value of
   `bit_displacement' must be non-negative and can be greater than
   CHAR_BIT. */

void
ammunition_bit_string_set ( void *start_byte, int bit_displacement, int bit,
                            int bit_length )
{
  unsigned char *current_byte = ( unsigned char * )start_byte;
  unsigned char filling_byte;
  int mask;

  if ( bit_length <= 0 )
    return ;
  bit = bit != 0; /* 1 or 0 */
  filling_byte = ( bit ? UCHAR_MAX : 0 );
  current_byte += bit_displacement / CHAR_BIT;
  bit_displacement %= CHAR_BIT;
  if ( bit_displacement != 0 ) {
    mask = UCHAR_MAX << ( CHAR_BIT - bit_displacement );
    if ( bit_length < CHAR_BIT - bit_displacement )
      mask |= UCHAR_MAX >> ( bit_displacement + bit_length );
    *current_byte = ( *current_byte & mask ) | ( filling_byte & ~mask );
    current_byte += 1;
    bit_length -= CHAR_BIT - bit_displacement;
  }
  _Pragma( "loopbound min 0 max 8" )
  while ( bit_length >= CHAR_BIT ) {
    *current_byte = filling_byte;
    current_byte++;
    bit_length -= CHAR_BIT;
  }
  if ( bit_length > 0 )
    *current_byte
      = ( *current_byte & ~( UCHAR_MAX << ( CHAR_BIT - bit_length ) ) )
        | ( filling_byte & ( UCHAR_MAX << ( CHAR_BIT - bit_length ) ) );
}

/* This function copys a bit string to another bit string.  This
   function is analog of standard C function `memcpy'.  Values of
   `to_bit_displacement' and `from_bit_displacement' must be
   non-negative and can be greater than CHAR_BIT.  The bit string must
   be non-overlapped. */

void
ammunition_bit_string_copy ( void *to, int to_bit_displacement,
                             const void *from, int from_bit_displacement,
                             int bit_length )
{
  unsigned char *current_to_byte = ( unsigned char * )to;
  const unsigned char *current_from_byte = ( unsigned const char * )from;
  int byte;
  int mask;

  if ( bit_length <= 0 )
    return ;
  current_to_byte += to_bit_displacement / CHAR_BIT;
  to_bit_displacement %= CHAR_BIT;
  current_from_byte += from_bit_displacement / CHAR_BIT;
  from_bit_displacement %= CHAR_BIT;
  _Pragma( "loopbound min 0 max 7" )
  while ( 1 ) {
    byte = ( ( ( *current_from_byte << from_bit_displacement ) & UCHAR_MAX )
             | ( from_bit_displacement != 0
                 && bit_length > ( CHAR_BIT - from_bit_displacement )
                 ? current_from_byte [ 1 ] >> ( CHAR_BIT - from_bit_displacement )
                 : 0 ) );
    if ( bit_length <= CHAR_BIT )
      break;
    /* Shift is correct when to_bit_displacement == 0 because its
       value is less than word bit size. */
    *current_to_byte
      = ( *current_to_byte
          & ( UCHAR_MAX << ( CHAR_BIT - to_bit_displacement ) ) )
        | ( byte >> to_bit_displacement );
    if ( to_bit_displacement != 0 )
      current_to_byte [ 1 ]
        = ( current_to_byte [ 1 ] & ( UCHAR_MAX >> to_bit_displacement ) )
          | ( byte << ( CHAR_BIT - to_bit_displacement ) );
    bit_length -= CHAR_BIT;
    current_from_byte++;
    current_to_byte++;
  }
  /* Shift is correct when to_bit_displacement == 0 because its
    value is less than word bit size. */
  mask = ( ( UCHAR_MAX << ( CHAR_BIT - to_bit_displacement ) )
           | ( UCHAR_MAX >> ( to_bit_displacement + bit_length ) ) );
  *current_to_byte
    = ( *current_to_byte & mask ) | ( ( byte >> to_bit_displacement ) & ~mask );
  bit_length -= CHAR_BIT - to_bit_displacement;
  if ( bit_length > 0 )
    current_to_byte [ 1 ]
      = ( current_to_byte [ 1 ] & ( UCHAR_MAX >> bit_length ) )
        | ( ( byte << ( CHAR_BIT - to_bit_displacement ) )
            & ( UCHAR_MAX << ( CHAR_BIT - bit_length ) ) );
}

/* This function copys a bit string to another bit string.  Copying
   starts with the last bits of the bit strings.  This function is
   used by function `bit_string_move'.  Values of
   `to_bit_displacement' and `from_bit_displacement' must be
   non-negative and can be greater than CHAR_BIT.  The bit string must
   be non-overlapped. */

void ammunition_reverse_bit_string_copy ( void *to, int to_bit_displacement,
    const void *from, int from_bit_displacement,
    int bit_length )
{
  unsigned char *current_to_byte = ( unsigned char * )to;
  const unsigned char *current_from_byte = ( unsigned const char * )from;
  int byte;
  int mask;

  if ( bit_length <= 0 )
    return ;
  to_bit_displacement += bit_length - 1;
  current_to_byte += to_bit_displacement / CHAR_BIT; /* last byte */
  to_bit_displacement %= CHAR_BIT; /* last bit */
  from_bit_displacement += bit_length - 1;
  current_from_byte += from_bit_displacement / CHAR_BIT; /* last byte */
  from_bit_displacement %= CHAR_BIT; /* last bit */
  _Pragma( "loopbound min 0 max 7" )
  while ( 1 ) {
    /* Shift is correct when to_bit_displacement == 0 because its
       value is less than word bit size. */
    byte = ( ( *current_from_byte >> ( CHAR_BIT - 1 - from_bit_displacement ) )
             | ( ( from_bit_displacement != CHAR_BIT - 1
                   && bit_length > from_bit_displacement + 1
                   ? current_from_byte [  -1  ] << ( from_bit_displacement + 1 )
                   : 0 )
                 & UCHAR_MAX ) );
    if ( bit_length <= CHAR_BIT )
      break;
    /* Shift is correct when to_bit_displacement == 0 because its
       value is less than word bit size. */
    *current_to_byte
      = ( *current_to_byte & ( UCHAR_MAX >> ( to_bit_displacement + 1 ) ) )
        | ( byte << ( CHAR_BIT - 1 - to_bit_displacement ) );
    if ( to_bit_displacement != CHAR_BIT - 1 )
      current_to_byte [ -1 ]
        = ( current_to_byte [ -1 ]
            & ( UCHAR_MAX << ( CHAR_BIT - 1 - to_bit_displacement ) ) )
          | ( byte >> ( to_bit_displacement + 1 ) );
    bit_length -= CHAR_BIT;
    current_from_byte--;
    current_to_byte--;
  }
  /* Shift is correct when to_bit_displacement == 0 because its
    value is less than word bit size. */
  mask = ( ( UCHAR_MAX >> ( to_bit_displacement + 1 ) ) |
           ( UCHAR_MAX << ( CHAR_BIT - 1 - to_bit_displacement
                            + bit_length ) ) );
  *current_to_byte
    = ( *current_to_byte & mask )
      | ( ( byte << ( CHAR_BIT - 1 - to_bit_displacement ) ) & ~mask );
  bit_length -= to_bit_displacement + 1;
  if ( bit_length > 0 )
    current_to_byte [ -1 ]
      = ( current_to_byte [ -1 ] & ( UCHAR_MAX << bit_length ) )
        | ( byte >> ( to_bit_displacement + 1 )
            & ( UCHAR_MAX >> ( CHAR_BIT - bit_length ) ) );
}

/* This function copys a bit string to another bit string with the aid
   of functions `bit_string_copy' and `reverse_bit_string_copy'.  This
   function is analog of standard C function `memmove'.  Values of
   `to_bit_displacement' and `from_bit_displacement' must be
   non-negative and can be greater than CHAR_BIT.  The bit string can
   be overlapped. */

void
ammunition_bit_string_move ( void *to, int to_bit_displacement,
                             const void *from, int from_bit_displacement,
                             int bit_length )
{
  unsigned char *current_to_byte = ( unsigned char * )to;
  const unsigned char *current_from_byte = ( unsigned const char * )from;

  if ( bit_length <= 0 )
    return ;
  current_to_byte += to_bit_displacement / CHAR_BIT;
  to_bit_displacement %= CHAR_BIT;
  current_from_byte += from_bit_displacement / CHAR_BIT;
  from_bit_displacement %= CHAR_BIT;
  if ( current_from_byte > current_to_byte
       || ( current_from_byte == current_to_byte
            && from_bit_displacement > to_bit_displacement ) )
    ammunition_bit_string_copy ( current_to_byte, to_bit_displacement,
                                 current_from_byte, from_bit_displacement,
                                 bit_length );
  else
    ammunition_reverse_bit_string_copy ( current_to_byte, to_bit_displacement,
                                         current_from_byte,
                                         from_bit_displacement, bit_length );
}

/* This function compares bit strings.  This function is analog of
   standard C function `memcmp'.  The function returns 0 if the bit
   strings are equal, 1 if the first bit string is greater than the
   second, -1 if the first bit string is less than the first.  Values
   of `bit_displacement1' and `bit_displacement2' must be non-negative
   and can be greater than CHAR_BIT. */

int
ammunition_bit_string_comparison ( const void *str1, int bit_displacement1,
                                   const void *str2, int bit_displacement2,
                                   int bit_length )
{
  const unsigned char *current_byte1 = ( unsigned const char * )str1;
  const unsigned char *current_byte2 = ( unsigned const char * )str2;
  int byte1;
  int byte2;
  int mask;

  if ( bit_length <= 0 )
    return 0;
  current_byte1 += bit_displacement1 / CHAR_BIT;
  bit_displacement1 %= CHAR_BIT;
  current_byte2 += bit_displacement2 / CHAR_BIT;
  bit_displacement2 %= CHAR_BIT;
  _Pragma( "loopbound min 0 max 7" )
  while ( 1 ) {
    byte1 = ( ( ( *current_byte1 << bit_displacement1 ) & UCHAR_MAX )
              | ( bit_displacement1 != 0
                  /* Shift is correct when to_bit_displacement == 0
                     because its value is less than word bit size. */
                  && bit_length > CHAR_BIT - bit_displacement1
                  ? current_byte1 [ 1 ] >> ( CHAR_BIT - bit_displacement1 )
                  : 0 ) );
    byte2 = ( ( ( *current_byte2 << bit_displacement2 ) & UCHAR_MAX )
              | ( bit_displacement2 != 0
                  && bit_length > CHAR_BIT - bit_displacement2
                  ? current_byte2 [ 1 ] >> ( CHAR_BIT - bit_displacement2 )
                  : 0 ) );
    if ( bit_length <= CHAR_BIT )
      break;
    if ( byte1 > byte2 )
      return 1;
    else
      if ( byte1 < byte2 )
        return -1;
    bit_length -= CHAR_BIT;
    current_byte2++;
    current_byte1++;
  }
  /* Shift is correct when to_bit_displacement == 0 because its value
    is less than word bit size. */
  mask = UCHAR_MAX << ( CHAR_BIT - bit_length );
  if ( ( byte1 & mask ) > ( byte2 & mask ) )
    return 1;
  else
    if ( ( byte1 & mask ) < ( byte2 & mask ) )
      return -1;
    else
      return 0;
}
