#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>
#include "../defs.h"

struct szb_t {
    int pos;    
    std::vector<unsigned char> data;
};

/*
#define GRANULARITY     256
#define GRANMASK        0xFFFFFF00
*/

//#define COORD_DEN       8.0f
#define COORD_DEN       32.0f
// 8  :: -4096 .. 4096
// 16 :: -2048 .. 2048
// 32 :: -1024 .. 1024

#define ensure_read(buf, s) \
    if (buf->pos + s > (int)buf->data.size()) { \
        return -1; \
    }

inline static void need_space(szb_t *buf, unsigned int space)
{
    int npos = buf->pos + space;
    if (npos > (int)buf->data.size()) {
        buf->data.resize(npos);
    }
}

szb_t *szb_create()
{
    szb_t *buf = new szb_t();
    buf->pos = 0;
    return buf;
}

void szb_free(szb_t *buf)
{
    delete buf;
}

void szb_write8(szb_t *buf, int v)
{
    assert(buf != NULL);
    need_space(buf, 1);
    buf->data[buf->pos] = v;
    buf->pos += 1;
}

void szb_write16(szb_t *buf, int v)
{
    assert(buf != NULL);
    need_space(buf, 2);
    *((unsigned short *)&buf->data[buf->pos]) = v;
    buf->pos += 2;
}

void szb_write32(szb_t *buf, int v)
{
    assert(buf != NULL);
    need_space(buf, 4);
    *((unsigned int *)&buf->data[buf->pos]) = v;
    buf->pos += 4;
}

void szb_writeAng8(szb_t *buf, float ang)
{
    assert(buf != NULL);
    need_space(buf, 1);
    buf->data[buf->pos] = (unsigned int)(ang * 256.0f/ 360.0f) & 0xFF;
    buf->pos += 1;
}

void szb_writeAng16(szb_t *buf, float ang)
{
    assert(buf != NULL);
    need_space(buf, 2);
    *((unsigned short *)&buf->data[buf->pos]) = (unsigned int)(ang * 65536.0f/ 360.0f) & 0xFFFF;
    buf->pos += 2;
}

void szb_writeCoord(szb_t *buf, float coord)
{
    assert(buf != NULL);
    need_space(buf, 2);
    *((signed short *)&buf->data[buf->pos]) = (signed int)(coord * COORD_DEN) & 0xFFFF;
    buf->pos += 2;
}

void szb_writePos(szb_t *buf, const float *xyz)
{
    assert(buf != NULL);
    assert(xyz != NULL);
    szb_writeCoord(buf, xyz[0]);
    szb_writeCoord(buf, xyz[1]);
    szb_writeCoord(buf, xyz[2]);
}

int szb_read8(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 1);
    signed char r = *((signed char *)&buf->data[buf->pos]);
    buf->pos += 1;
    return r;
}

int szb_read16(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 2);
    signed short r = *((signed short *)&buf->data[buf->pos]);
    buf->pos += 2;
    return r;
}

int szb_read32(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 4);
    int r = *((int *)&buf->data[buf->pos]);
    buf->pos += 4;
    return r;
}

float szb_readAng8(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 1);
    float r = float(*((unsigned char *)&buf->data[buf->pos])) * 360.0f / 256.0f;
    buf->pos += 1;
    return r;
}

float szb_readAng16(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 2);
    float r = float(*((unsigned short *)&buf->data[buf->pos])) * 360.0f / 65536.0f;
    buf->pos += 2;
    return r;
}

float szb_readCoord(szb_t *buf)
{
    assert(buf != NULL);
    ensure_read(buf, 2);
    float r = float(*((signed short *)&buf->data[buf->pos])) / COORD_DEN;
    buf->pos += 2;
    return r;
}

void szb_readPos(szb_t *buf, float *xyz)
{
    assert(buf != NULL);
    assert(xyz != NULL);
    xyz[0] = szb_readCoord(buf);
    xyz[1] = szb_readCoord(buf);
    xyz[2] = szb_readCoord(buf);
}

void szb_print(szb_t *buf)
{
    assert(buf != NULL);
    for (unsigned int i = 0; i < buf->data.size(); i++) {
        con_printf(" %02X", buf->data[i]);
    }
    con_printf("\n");
}

void szb_rewind(szb_t *buf)
{
    assert(buf != NULL);
    buf->pos = 0;
}

void szb_resize(szb_t *buf, int size)
{
    assert(buf != NULL);
    buf->data.resize(size);
    if (buf->pos > (int)buf->data.size()) {
        buf->pos = int(buf->data.size());
    }
}

void *szb_getPtr(szb_t *buf)
{
    assert(buf != NULL);
    return &buf->data[0];
}

int szb_getSize(szb_t *buf)
{
    assert(buf != NULL);
    return int(buf->data.size());
}

bool szb_eof(szb_t *buf)
{
    assert(buf != NULL);
    if (buf->pos >= (int)buf->data.size()) {
        return true;
    }
    return false;
}

void szb_test()
{
    szb_t *coisa;

    coisa = szb_create();
    szb_write32(coisa, 0x12345678);
    szb_writeAng8(coisa, 90.0f);
    szb_writeCoord(coisa, 1023.9f);
    szb_print(coisa);

    szb_rewind(coisa);
    int a = szb_read32(coisa);
    printf("a = %08X\n", a);
    float b = szb_readAng8(coisa);
    printf("b = %f\n", b);
    float c = szb_readCoord(coisa);
    printf("c = %f\n", c);

    szb_free(coisa);

}
