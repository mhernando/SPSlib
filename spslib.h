//////////////////////////////////////////////////////////////////
// Copyright (C) 2023 Miguel Hernando
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// SPSlib Author: Miguel Hernando
//
//  CREDITS:
//      CRC USEFUL FUNCTIONS adapted from Lammert Bies code (libcrc)
//      https://github.com/lammertb/libcrc
//////////////////////////////////////////////////////////////////

#pragma once
#include <cstdint>
#include <cstddef>
//Multiplatform struct pack hack 
#ifndef _WIN32
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#include <iostream>
using std::cout, std::endl;

//endianess verification
// include something to verify that is LITTLE-ENDIAN

//SPS is intended to work only between little-endian systems. Otherwise int reconstruction and crc will not work 
namespace SPS
{
    typedef unsigned char uchar_t;
    static uint16_t crc16(const uchar_t * input_str, int num_bytes);
    template <typename T>
    union union2byte{
        uchar_t bytes[sizeof(T)];
        char sbytes[sizeof(T)];
        T var;
        union2byte(T v) :var(v) {}
        union2byte() = default;
    };
 
    #pragma pack(1)
    template <int _MAX_SIZE=254, uint16_t _HEADER=0x3AA3 >  
    struct Message
    {
        union {
            uchar_t data[_MAX_SIZE]; //header(2)+crc(2)+size(1)+id(1)+info(254)
            struct {
                uint16_t header;
                uint16_t crc;
                uchar_t size; //size = id + info
                uchar_t id;
                uchar_t info[254];
            }PACKED;
        };
        
        //commonly used constructors
        Message() :header{ _HEADER }, size{ 0 }{}
        Message(uchar_t cmd): header{ _HEADER }, size{ 1 }, id {cmd}{ write_crc(); }
        Message(uchar_t cmd, uchar_t value) : header{ _HEADER }, size{ 2 }, id{ cmd }{ info[0] = value; write_crc();}
        uchar_t& operator[](int ind) { return data[ind]; }
        static auto& none() { static Message _none; return _none; }
        void write_crc() { crc = crc16(data + FULLHEADER_SIZE, size); }
        bool check_crc() { return crc == crc16(data + FULLHEADER_SIZE, size);}
        uint16_t datagram_size() const { return size + FULLHEADER_SIZE;}

        //info generic writters and readers
        template <typename T, typename TT = T> //a trick to forze explicit type specification
        bool write(TT var) {
            if (size + sizeof(TT) >= _MAX_SIZE)return false;
            union2byte<TT> aux(var);
            
            uchar_t * p = info + size - 1;
            for (uint16_t i = 0; i < sizeof(TT); i++)p[i] = aux.bytes[i];
            size+=sizeof(TT);
            write_crc();
            return true;
        }
        
        template <typename T>
        auto read() { //if not possible returns the zero value
            if (size < sizeof(T)+index_reader)return T{};
            union2byte<T> aux;
            for (auto& a : aux.bytes)a = info[index_reader++];
            return aux.var;
        }
        
        template <typename T, typename TT = T>
        void read_array(TT* v, uchar_t n) {
            for (uchar_t i = 0; i < n; i++)v[i]=read<TT>();
        }

        template <typename T, typename TT = T>
        void write_array(TT* v, uchar_t n) {
            for (uchar_t i = 0; i < n; i++)write<TT>(v[i]);
        }

        void write_cstring(const char *s) {
            uint8_t index = 0;
            uchar_t* p = info + size - 1;
            while ((s[index] != 0) && (index+size < _MAX_SIZE)) {
                p[index] = s[index]; 
                index++;
            }
            p[index] = 0;
            size += index + 1;
            write_crc();
        }
        char * read_cstring(char* s, uchar_t max) {
            uint8_t index = 0;

            while ((index +1 < max) && (info[index_reader] != 0) && (index_reader<_MAX_SIZE))
                 s[index++]= info[index_reader++];
            s[index++] = 0;
            if (info[index_reader] == 0)index_reader++;
            return s;
        }
       
        //GENERIC MESSAGE READER
        class MsgReader{
            Message<_MAX_SIZE,_HEADER> mens;
            uchar_t index;
         public:
            MsgReader() :index(0) {}
            bool add_uchar(uchar_t data);
            auto getMessage() {
                index = 0;
                return mens;
            };
        };
        //GENERIC CircularBuffer
        template<int BSIZE=100>
        class CircularBuffer {
            Message<_MAX_SIZE, _HEADER> buffer[BSIZE];
            uchar_t init = 0;
            uchar_t end = 0;
        public:

            void push(const Message & m)
            {
                buffer[end] = m;
                if (++end >= BSIZE)end = 0;
            }
            void push_single(const Message & m, 
                bool (*f)(const Message&, const Message&)= 
                [](const Message& m1, const Message& m2) -> bool { return m1.id==m2.id; }) {
                //check if there is message that should be overriden
                uchar_t ind = init;
                while (ind != end) {
                    if (f(m, buffer[ind])) { buffer[ind] = m; return; }
                    if (++ind >= BSIZE)ind = 0;
                }
                push(m);
            }
            bool there_is_msg() { return init != end; }
            auto getMessage() {
                if (init == end)return Message::none();
                uchar_t ind = init++;
                if (init >= BSIZE)init = 0;
                return buffer[ind];
            }

        };
    private:
        uchar_t index_reader = 0;
        static constexpr uint16_t FULLHEADER_SIZE = 5;
        static_assert(_MAX_SIZE < 255, "Message info can't exceed 254 bytes");
    }PACKED;
    #pragma pack()

    template <int M, uint16_t H>
    inline bool Message<M,H>::MsgReader::add_uchar(uchar_t data)
    {
        mens[index++] = data;
        if ((index == 1)&&(data == (uchar_t)H)) return false; 
        if ((index == 2) && (mens.header == H)) return false; 
        if (index >2){
            if(index < FULLHEADER_SIZE+1)return false;  //header correctm the next 3 bytes are read
            if(index < mens.size + FULLHEADER_SIZE) return false;
            if(mens.check_crc()) return true; //message completed, check crc, otherwise reset
        }
        index = 0;
        return false;
    }
    inline void init_crc16_tab(uint16_t* crc_tab16) {
        uint16_t i;
        uint16_t j;
        uint16_t crc;
        uint16_t c;

        for (i = 0; i < 256; i++) {
            crc = 0;
            c = i;
            for (j = 0; j < 8; j++) {
                if ((crc ^ c) & 0x0001) crc = (crc >> 1) ^ 0xA001; //CRC_POLY_16
                else crc = crc >> 1;
                c = c >> 1;
            }
            crc_tab16[i] = crc;
        }

    }
    inline uint16_t crc16(const uchar_t* input_str, int num_bytes) {
        static bool  crc_tab16_init = false;
        static uint16_t  crc_tab16[256];
        uint16_t crc;
        uint16_t tmp;
        uint16_t short_c;
        const unsigned char* ptr;

        size_t a;
        if (!crc_tab16_init) init_crc16_tab(crc_tab16);
        crc_tab16_init = true;
        crc = 0x0000; //CRC_START_16
        ptr = input_str;
        if (ptr != nullptr) for (a = 0; a < (size_t)num_bytes; a++) {
            short_c = 0x00ff & (uint16_t)*ptr;
            tmp = crc ^ short_c;
            crc = (crc >> 8) ^ crc_tab16[tmp & 0xff];
            ptr++;
        }
        return crc;
    }
 };//SPS namespace

