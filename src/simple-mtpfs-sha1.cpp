/* ***** BEGIN LICENSE BLOCK *****
*   Copyright (C) 2012-2013, Peter Hatina <phatina@gmail.com>
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License as
*   published by the Free Software Foundation; either version 2 of
*   the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
* ***** END LICENSE BLOCK ***** */

#include <config.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "simple-mtpfs-sha1.h"


SHA1::SHA1()
{
    reset();
}

void SHA1::update(const std::string &s)
{
    std::istringstream is(s);
    update(is);
}

void SHA1::update(std::istream &is)
{
    std::string rest_of_buffer;
    read(is, rest_of_buffer, BLOCK_BYTES - m_buffer.size());
    m_buffer += rest_of_buffer;

    while (is) {
        uint32 block[BLOCK_INTS];
        buffer_to_block(m_buffer, block);
        transform(block);
        read(is, m_buffer, BLOCK_BYTES);
    }
}

std::string SHA1::final()
{
    uint64 total_bits = (m_transforms * BLOCK_BYTES + m_buffer.size()) * 8;
    m_buffer += static_cast<char>(0x80);
    unsigned int orig_size = m_buffer.size();
    while (m_buffer.size() < BLOCK_BYTES)
        m_buffer += static_cast<char>(0x00);

    uint32 block[BLOCK_INTS];
    buffer_to_block(m_buffer, block);

    if (orig_size > BLOCK_BYTES - 8) {
        transform(block);
        for (unsigned int i = 0; i < BLOCK_INTS - 2; ++i)
            block[i] = 0;
    }

    block[BLOCK_INTS - 1] = total_bits;
    block[BLOCK_INTS - 2] = (total_bits >> 32);
    transform(block);

    std::ostringstream result;
    for (unsigned int i = 0; i < DIGEST_INTS; ++i) {
        result << std::hex << std::setfill('0') << std::setw(8);
        result << (m_digest[i] & 0xffffffff);
    }

    reset();
    return result.str();
}

void SHA1::reset()
{
    m_digest[0] = 0x67452301;
    m_digest[1] = 0xefcdab89;
    m_digest[2] = 0x98badcfe;
    m_digest[3] = 0x10325476;
    m_digest[4] = 0xc3d2e1f0;

    m_transforms = 0;
    m_buffer = "";
}

void SHA1::transform(uint32 block[BLOCK_BYTES])
{
    uint32 a = m_digest[0];
    uint32 b = m_digest[1];
    uint32 c = m_digest[2];
    uint32 d = m_digest[3];
    uint32 e = m_digest[4];

    #define rol(value, bits) (((value) << (bits)) | (((value) & 0xffffffff) >> (32 - (bits))))
    #define blk(i) (block[i & 15] = rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i & 15], 1))

    #define R0(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y)        + block[i] + 0x5a827999 + rol(v,5); w = rol(w, 30);
    #define R1(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y)        + blk(i)   + 0x5a827999 + rol(v,5); w = rol(w, 30);
    #define R2(v,w,x,y,z,i) z += (w ^ x ^ y)                + blk(i)   + 0x6ed9eba1 + rol(v,5); w = rol(w, 30);
    #define R3(v,w,x,y,z,i) z += (((w | x) & y) | ( w & x)) + blk(i)   + 0x8f1bbcdc + rol(v,5); w = rol(w, 30);
    #define R4(v,w,x,y,z,i) z += (w ^ x ^ y)                + blk(i)   + 0xca62c1d6 + rol(v,5); w = rol(w, 30);

    R0(a, b, c, d, e,  0);
    R0(e, a, b, c, d,  1);
    R0(d, e, a, b, c,  2);
    R0(c, d, e, a, b,  3);
    R0(b, c, d, e, a,  4);
    R0(a, b, c, d, e,  5);
    R0(e, a, b, c, d,  6);
    R0(d, e, a, b, c,  7);
    R0(c, d, e, a, b,  8);
    R0(b, c, d, e, a,  9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    m_digest[0] += a;
    m_digest[1] += b;
    m_digest[2] += c;
    m_digest[3] += d;
    m_digest[4] += e;

    m_transforms++;
}

void SHA1::buffer_to_block(const std::string &buffer, uint32 block[BLOCK_BYTES])
{
    for (unsigned int i = 0; i < BLOCK_INTS; ++i) {
        block[i] = (buffer[4 * i + 3] & 0xff)
            | (buffer[4 * i + 2] & 0xff) << 8
            | (buffer[4 * i + 1] & 0xff) << 16
            | (buffer[4 * i + 0] & 0xff) << 24;
    }
}

void SHA1::read(std::istream &is, std::string &s, const int max)
{
    char sbuf[BLOCK_BYTES];
    is.read(sbuf, max);
    s.assign(sbuf, is.gcount());
}

std::string SHA1::sumString(const std::string &str)
{
    SHA1 sha1;
    sha1.update(str);
    return sha1.final();
}
