//
// Created by Mindaugas Vinkelis on 2017-01-10.
//
#include <gmock/gmock.h>
#include "BufferWriter.h"
#include "BufferReader.h"

using testing::Eq;
using testing::ContainerEq;

#include <list>
#include <bitset>


struct IntegralUnsignedTypes {
    uint32_t a;
    uint16_t b;
    uint8_t c;
    uint8_t d;
    uint64_t e;
};

TEST(BufferBitsOperations, WriteAndReadBits) {
    //setup data
    IntegralUnsignedTypes data;
    data.a = 485454;//bits 19
    data.b = 45978;//bits 16
    data.c = 0;//bits 1
    data.d = 36;//bits 6
    data.e = 479845648946;//bits 39

    constexpr size_t aBITS = 21;
    constexpr size_t bBITS = 16;
    constexpr size_t cBITS = 5;
    constexpr size_t dBITS = 7;
    constexpr size_t eBITS = 40;

    //create and write to buffer
    std::vector<uint8_t> buf;
    BufferWriter bw{buf};

    bw.writeBits(data.a, aBITS);
    bw.writeBits(data.b, bBITS);
    bw.writeBits(data.c, cBITS);
    bw.writeBits(data.d, dBITS);
    bw.writeBits(data.e, eBITS);
    bw.flush();
    auto bytesCount = ((aBITS + bBITS + cBITS + dBITS + eBITS) / 8) +1 ;
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(bytesCount));
    //read from buffer
    BufferReader br{buf};
    IntegralUnsignedTypes res;

    br.readBits(res.a, aBITS);
    br.readBits(res.b, bBITS);
    br.readBits(res.c, cBITS);
    br.readBits(res.d, dBITS);
    br.readBits(res.e, eBITS);

    EXPECT_THAT(res.a, Eq(data.a));
    EXPECT_THAT(res.b, Eq(data.b));
    EXPECT_THAT(res.c, Eq(data.c));
    EXPECT_THAT(res.d, Eq(data.d));
    EXPECT_THAT(res.e, Eq(data.e));

}

TEST(BufferBitsOperations, WhenFinishedFlushWriter) {

    std::vector<uint8_t> buf;
    BufferWriter bw{buf};

    bw.writeBits(3u, 2);
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(0));
    bw.flush();
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(1));

}

TEST(BufferBitsOperations, BufferSizeIsCountedPerByteNotPerBit) {
    //setup data

    //create and write to buffer
    std::vector<uint8_t> buf;
    BufferWriter bw{buf};

    bw.writeBits(7u,3);
    bw.flush();
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(1));

    //read from buffer
    BufferReader br{buf};
    unsigned tmp;
    EXPECT_THAT(br.readBits(tmp,4), Eq(true));
    EXPECT_THAT(br.readBits(tmp,2), Eq(true));
    EXPECT_THAT(br.readBits(tmp,2), Eq(true));
    EXPECT_THAT(br.readBits(tmp,2), Eq(false));

    //part of next byte
    BufferReader br1{buf};
    EXPECT_THAT(br1.readBits(tmp,2), Eq(true));
    EXPECT_THAT(br1.readBits(tmp,7), Eq(false));

    //bigger than byte
    BufferReader br2{buf};
    EXPECT_THAT(br2.readBits(tmp,9), Eq(false));
}

TEST(BufferBitsOperations, WhenAlignedFlushHasNoEffect) {
    //setup data

    //create and write to buffer
    std::vector<uint8_t> buf;
    BufferWriter bw{buf};

    bw.writeBits(3u, 2);
    bw.align();
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(1));
    bw.flush();
    EXPECT_THAT(std::distance(buf.begin(), buf.end()), Eq(1));
}

TEST(BufferBitsOperations, AlignMustWriteZerosBits) {
    //setup data

    //create and write to buffer
    std::vector<uint8_t> buf;
    BufferWriter bw{buf};

    //write 2 bits and align
    bw.writeBits(3u, 2);
    bw.align();

    unsigned char tmp;
    BufferReader br1{buf};
    br1.readBits(tmp,2);
    //read aligned bits
    EXPECT_THAT(br1.readBits(tmp,6), Eq(true));
    EXPECT_THAT(tmp, Eq(0));

    BufferReader br2{buf};
    //read 2 bits
    br2.readBits(tmp,2);
    EXPECT_THAT(br2.align(), Eq(true));
}
