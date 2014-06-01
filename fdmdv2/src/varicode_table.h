//==========================================================================
// Name:            varicode_table.h
// Purpose:         Varicode look up table
// Created:         Nov 24, 2012
// Authors:         Clint Turner, KA7OEI,  Peter Martinez, G3PLX
// 
// License:
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.1,
//  as published by the Free Software Foundation.  This program is
//  distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//  License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//==========================================================================

/* The following table defines the PKS31 varicode.  There are 128 entries,
corresponding to ASCII characters 0-127 with two bytes for each entry.  The bits
for the varicode are to be shifted out MSB-first for both bytes, with the first byte
in the table being the first one to be sent.

More than one zero in sequence signifies the end of the character (i.e.
two zeroes are the intercharacter sequence, so at least two zeroes should always be
sent before the next character is sent.

This file is constructed with information from the article "PSK31 Fundamentals"
by Peter Martinez, G3PLX by Clint Turner, KA7OEI
*/
unsigned char const long_varicode_table[256] =	{
    0b10101010,
    0b11000000, // 0 NUL
    0b10110110,
    0b11000000, // 1 SOH
    0b10111011,
    0b01000000, // 2 STX
    0b11011101,
    0b11000000, // 3 ETX
    0b10111010,
    0b11000000, // 4 EOT
    0b11010111,
    0b11000000, // 5 ENQ
    0b10111011,
    0b11000000, // 6 ACK
    0b10111111,
    0b01000000, // 7 BEL
    0b10111111,
    0b11000000, // 8 BS
    0b11101111,
    0b00000000, // 9 HT
    0b11101000,
    0b00000000, // 10 LF
    0b11011011,
    0b11000000, // 11 VT
    0b10110111,
    0b01000000, // 12 FF
    0b11111000,
    0b00000000, // 13 CR
    0b11011101,
    0b01000000, // 14 SO
    0b11101010,
    0b11000000, // 15 SI
    0b10111101,
    0b11000000, // 16 DLE
    0b10111101,
    0b01000000, // 17 DC1
    0b11101011,
    0b01000000, // 18 DC2
    0b11101011,
    0b11000000, // 19 DC3
    0b11010110,
    0b11000000, // 20 DC4
    0b11011010,
    0b11000000, // 21 NAK
    0b11011011,
    0b01000000, // 22 SYN
    0b11010101,
    0b11000000, // 23 ETB
    0b11011110,
    0b11000000, // 24 CAN
    0b11011111,
    0b01000000, // 25 EM
    0b11101101,
    0b11000000, // 26 SUB
    0b11010101,
    0b01000000, // 27 ESC
    0b11010111,
    0b01000000, // 28 FS
    0b11101110,
    0b11000000, // 29 GS
    0b10111110,
    0b11000000, // 30 RS
    0b11011111,
    0b11000000, // 31 US
    0b10000000,
    0b00000000, // 32 SP
    0b11111111,
    0b10000000, // 33 !
    0b10101111,
    0b10000000, // 34 "
    0b11111010,
    0b10000000, // 35 #
    0b11101101,
    0b10000000, // 36 $
    0b10110101,
    0b01000000, // 37 %
    0b10101110,
    0b11000000, // 38 &
    0b10111111,
    0b10000000, // 39 '
    0b11111011,
    0b00000000, // 40 (
    0b11110111,
    0b00000000, // 41 )
    0b10110111,
    0b10000000, // 42 *
    0b11101111,
    0b10000000, // 43 +
    0b11101010,
    0b00000000, // 44 ,
    0b11010100,
    0b00000000, // 45 -
    0b10101110,
    0b00000000, // 46 .
    0b11010111,
    0b10000000, // 47 /
    0b10110111,
    0b00000000, // 48 0
    0b10111101,
    0b00000000, // 49 1
    0b11101101,
    0b00000000, // 50 2
    0b11111111,
    0b00000000, // 51 3
    0b10111011,
    0b10000000, // 52 4
    0b10101101,
    0b10000000, // 53 5
    0b10110101,
    0b10000000, // 54 6
    0b11010110,
    0b10000000, // 55 7
    0b11010101,
    0b10000000, // 56 8
    0b11011011,
    0b10000000, // 57 9
    0b11110101,
    0b00000000, // 58 :
    0b11011110,
    0b10000000, // 59 ;
    0b11110110,
    0b10000000, // 60 <
    0b10101010,
    0b00000000, // 61 =
    0b11101011,
    0b10000000, // 62 >
    0b10101011,
    0b11000000, // 63 ?
    0b10101111,
    0b01000000, // 64 @
    0b11111010,
    0b00000000, // 65 A
    0b11101011,
    0b00000000, // 66 B
    0b10101101,
    0b00000000, // 67 C
    0b10110101,
    0b00000000, // 68 D
    0b11101110,
    0b00000000, // 69 E
    0b11011011,
    0b00000000, // 70 F
    0b11111101,
    0b00000000, // 71 G
    0b10101010,
    0b10000000, // 72 H
    0b11111110,
    0b00000000, // 73 I
    0b11111110,
    0b10000000, // 74 J
    0b10111110,
    0b10000000, // 75 K
    0b11010111,
    0b00000000, // 76 L
    0b10111011,
    0b00000000, // 77 M
    0b11011101,
    0b00000000, // 78 N
    0b10101011,
    0b00000000, // 79 O
    0b11010101,
    0b00000000, // 80 P
    0b11101110,
    0b10000000, // 81 Q
    0b10101111,
    0b00000000, // 82 R
    0b11011110,
    0b00000000, // 83 S
    0b11011010,
    0b00000000, // 84 T
    0b10101011,
    0b10000000, // 85 U
    0b11011010,
    0b10000000, // 86 V
    0b10101110,
    0b10000000, // 87 W
    0b10111010,
    0b10000000, // 88 X
    0b10111101,
    0b10000000, // 89 Y
    0b10101011,
    0b01000000, // 90 Z
    0b11111011,
    0b10000000, // 91 [
    0b11110111,
    0b10000000, // 92 "\"
    0b11111101,
    0b10000000, // 93 ]
    0b10101111,
    0b11000000, // 94 ^
    0b10110110,
    0b10000000, // 95 _ (underline)
    0b10110111,
    0b11000000, // 96 `
    0b10110000,
    0b00000000, // 97 a
    0b10111110,
    0b00000000, // 98 b
    0b10111100,
    0b00000000, // 99 c
    0b10110100,
    0b00000000, // 100 d
    0b11000000,
    0b00000000, // 101 e
    0b11110100,
    0b00000000, // 102 f
    0b10110110,
    0b00000000, // 103 g
    0b10101100,
    0b00000000, // 104 h
    0b11010000,
    0b00000000, // 105 i
    0b11110101,
    0b10000000, // 106 j
    0b10111111,
    0b00000000, // 107 k
    0b11011000,
    0b00000000, // 108 l
    0b11101100,
    0b00000000, // 109 m
    0b11110000,
    0b00000000, // 110 n
    0b11100000,
    0b00000000, // 111 o
    0b11111100,
    0b00000000, // 112 p
    0b11011111,
    0b10000000, // 113 q
    0b10101000,
    0b00000000, // 114 r
    0b10111000,
    0b00000000, // 115 s
    0b10100000,
    0b00000000, // 116 t
    0b11011100,
    0b00000000, // 117 u
    0b11110110,
    0b00000000, // 118 v
    0b11010110,
    0b00000000, // 119 w
    0b11011111,
    0b00000000, // 120 x
    0b10111010,
    0b00000000, // 121 y
    0b11101010,
    0b10000000, // 122 z
    0b10101101,
    0b11000000, // 123 {
    0b11011101,
    0b10000000, // 124 |
    0b10101101,
    0b01000000, // 125 }
    0b10110101,
    0b11000000, // 126 ~
    0b11101101,
    0b01000000, // 127 (del)
};

char const short_varicode_table[] = {

    ' ' ,0b11000000,
    ',' ,0b01000000,
    '=' ,0b10000000, // (Start of message)
    '1' ,0b11110000,
    '2' ,0b01110000,
    '3' ,0b10110000,
    '4' ,0b11010000,
    '5' ,0b01010000,
    '6' ,0b10010000,
    '7' ,0b11100000,
    '8' ,0b01100000,
    '9' ,0b10100000,
    'a' ,0b11111100,
    'b' ,0b01111100,
    'c' ,0b10111100,
    'd' ,0b11011100,
    'e' ,0b01011100,
    'f' ,0b10011100,
    'g' ,0b11101100,
    'h' ,0b01101100,
    'i' ,0b10101100,
    'j' ,0b11110100,
    'k' ,0b01110100,
    'l' ,0b10110100,
    'm' ,0b11010100,
    'n' ,0b01010100,
    'o' ,0b10010100,
    'p' ,0b11100100,
    'q' ,0b01100100,
    'r' ,0b10100100,
    's' ,0b11111000,
    't' ,0b01111000,
    'u' ,0b10111000,
    'v' ,0b11011000,
    'w' ,0b01011000,
    'x' ,0b10011000,
    'y' ,0b11101000,
    'z' ,0b01101000,
    '0' ,0b10101000
};

