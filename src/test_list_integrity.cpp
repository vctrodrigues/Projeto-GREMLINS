/**
 * @file test_mp_list.cpp
 *
 * @description
 * Test the MP's integrity after basic allocate/free operations.
 *
 * 1) Test the allocation of a single area corresponding to the entire pool.
 * 2) Test the bad_alloc excepction (MP is full)
 * 3) Free an area between two free areas.
 * 4) Free an area between two reserved areas.
 * 5) Free an area between a reverved area (on the left) and a free area (on the right).
 * 6) Free an area between a free area (on the left) and a reserved area (on the right).
 * 7) Test if we get a single area after all memory has been freed.
 *
 * @autho	Selan R dos Santos, <selan.rds@gmail.com>
 * @date	2018, July 1st.
 */

#include <iostream>
#include <random>
#include <algorithm>
#include <string>
#include <sstream>

#include "../include/mempool_common.h"
#include "../include/SLPool.hpp"

using namespace mp;

int main()
{
    const short BLOCK_SIZE = 24;
    using byte = char;                        // 1 byte
    const short n_blocks(2);                  // How many blocks per chunk
    const short chunk(n_blocks * BLOCK_SIZE); // Each chunk ideally should have 1 or more blocks.
    const short area_metainfo(mp::SLPool<BLOCK_SIZE>::TAG_SZ + mp::SLPool<BLOCK_SIZE>::HEADER_SZ);

    // We need MP large enough to hold 7 separate chunks of memory.
    const short n_chunks(7); // 7 pieces of 32 bytes

    std::cout << ">>> Begining LIST INTEGRITY tests...\n\n";

    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        //std::cout << p << std::endl;

        auto passed(true);
        byte *temp;
        try
        {
            temp = new (p) byte[sizeof(byte) * chunk * n_chunks];
        }
        catch (const std::bad_alloc &e)
        {
            passed = false;
        }
        catch (const std::runtime_error &e)
        {
            passed = false;
        }
        std::cout << ">>> Allocating a single block with length equal to the entire pool size... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;

        delete[] temp;
    }

    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        //std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);
        //std::cout << ">> individual chunk length: " << chunk_len << std::endl;

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], oss.str().c_str());
        }
        //std::cout << p << std::endl;

        // Request an extra byte to invoke an exception (overflow).
        byte *temp(nullptr);
        try
        {
            temp = new (p) byte[chunk_len];
        }
        catch (std::runtime_error &e)
        {
            passed = true;
        }
        catch (std::bad_alloc &e)
        {
            passed = true;
        }

        std::cout << ">>> Testing pool overflow... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
        // \e[1m text in bold \e[0m
        // \e[1m turn on bold
        // \e[0m turn off bold
        // 32 -> green
        // 21 -> red
    }

    {
        // Set up 7 chunks of data.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks - area_metainfo);
        //std::cout << p << std::endl;

        byte *vet[n_chunks]; // Array of byte pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], oss.str().c_str());
            //std::cout << ">> vet[" << i << "] = " << vet[i] << std::endl;
        }

        /*
         * Freeing up target 'x'.
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         * | L | R | L | x | L | R | L |  ===>  | L | R |     L     | R | L |
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         *   0   1   2   3   4   5   6            0   1   2   3   4   5   6
         */

        // Set up initial condition.
        delete[] vet[0];
        delete[] vet[2];
        delete[] vet[4];
        delete[] vet[6];
        std::cout << p << std::endl;
        delete[] vet[3];
        //std::cout << p << std::endl;

        // Request a memory with size equal to the free blocks just combined.
        auto passed(true);
        try
        {
            // If the deletion went weel, we should be have a contiguous 6-block-length area.
            // Therefore, we should be able to successfully request back a single 6-block-length area.
            size_t new_size(6 * SLPool<BLOCK_SIZE>::BLK_SZ - SLPool<BLOCK_SIZE>::TAG_SZ - SLPool<BLOCK_SIZE>::HEADER_SZ);
            vet[3] = new (p) byte[new_size];
            // Fill up the char with "01234567890123...."
            std::ostringstream oss;
            auto j(0u);
            while (j < new_size - 1) // Remember we have to reserve one extra space for the '\0'.
                oss << (j++ % 10);
            strcpy(vet[3], oss.str().c_str());
        }
        catch (std::runtime_error &e)
        {
            passed = false;
        }
        catch (std::bad_alloc &e)
        {
            passed = false;
        }

        delete[] vet[1];
        delete[] vet[3];
        delete[] vet[5];

        std::cout << ">>> Testing pool merging 3 contiguous free areas: L R L => L L L... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks - area_metainfo);
        //std::cout << p << std::endl;

        byte *vet[n_chunks]; // Array of byte pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            // Fill up the char with "01234567890123...."
            strcpy(vet[i], oss.str().c_str());
            //std::cout << ">> vet[" << i << "] = " << vet[i] << std::endl;
        }

        /*
         * Teste #2
         *
         * Freeing up target 'x'.
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         * | R | L | R | x | R | L | R |  ===>  | R | L | R | L | R | R | L |
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         *   0   1   2   3   4   5   6            0   1   2   3   4   5   6
         */

        // Set up initial test condition.
        delete[] vet[1];
        delete[] vet[5];
        //std::cout << p << std::endl;
        delete[] vet[3];
        //std::cout << p << std::endl;

        auto passed(true);
        try
        {
            // If the deletion went well, we should be have a two free blocks at the middle.
            // Therefore, we should be able to successfully request back a single 2-block-length area.
            size_t new_size(2 * SLPool<BLOCK_SIZE>::BLK_SZ - SLPool<BLOCK_SIZE>::TAG_SZ - SLPool<BLOCK_SIZE>::HEADER_SZ);
            vet[1] = new (p) byte[new_size];
            vet[3] = new (p) byte[new_size];
            vet[5] = new (p) byte[new_size];
            // Fill up the char with "01234567890123...."
            std::ostringstream oss;
            auto j(0u);
            while (j < new_size - 1) // Remember we have to reserve one extra space for the '\0'.
                oss << (j++ % 10);
            strcpy(vet[1], oss.str().c_str());
            strcpy(vet[3], oss.str().c_str());
            strcpy(vet[5], oss.str().c_str());
        }
        catch (std::runtime_error &e)
        {
            passed = false;
        }
        catch (std::bad_alloc &e)
        {
            passed = false;
        }

        for (auto i(0); i < n_chunks; ++i)
            delete[] vet[i];

        std::cout << ">>> Testing pool after freeing area between 2 reserved areas:  R R R => R L R... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks - area_metainfo);

        byte *vet[n_chunks]; // Array of byte pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            // Fill up the char with "01234567890123...."
            strcpy(vet[i], oss.str().c_str());
            //std::cout << ">> vet[" << i << "] = " << vet[i] << std::endl;
        }

        /*
         * Teste #3
         *
         * Freeing up target 'x'.
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         * | R | L | R | x | L | R | L |  ===>  | R | L | R |   L   | R | L |
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         *   0   1   2   3   4   5   6            0   1   2   3   4   5   6
         */

        delete[] vet[1];
        delete[] vet[4];
        delete[] vet[6];
        //std::cout << p << std::endl;
        delete[] vet[3];
        //std::cout << p << std::endl;

        auto passed(true);
        try
        {
            // If the deletion went well, we should be have a two free blocks at the middle.
            // Therefore, we should be able to successfully request back a single 2-block-length area.
            size_t new_size(4 * SLPool<BLOCK_SIZE>::BLK_SZ - SLPool<BLOCK_SIZE>::TAG_SZ - SLPool<BLOCK_SIZE>::HEADER_SZ);
            vet[3] = new (p) byte[new_size];
            // Fill up the char with "01234567890123...."
            std::ostringstream oss;
            auto j(0u);
            while (j < new_size - 1) // Remember we have to reserve one extra space for the '\0'.
                oss << (j++ % 10);
            strcpy(vet[3], oss.str().c_str());
        }
        catch (std::runtime_error &e)
        {
            passed = false;
        }
        catch (std::bad_alloc &e)
        {
            passed = false;
        }

        std::cout << ">>> Testing pool after freeing area between a reserved and free areas:  R R L => R L L... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks - area_metainfo);

        byte *vet[n_chunks]; // Array of byte pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            // Fill up the char with "01234567890123...."
            strcpy(vet[i], oss.str().c_str());
            //std::cout << ">> vet[" << i << "] = " << vet[i] << std::endl;
        }

        /*
         * Teste #4
         *
         * Freeing up target 'x'.
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         * | L | R | L | x | R | L | R |  ===>  | L | R |   L   | R | L | R |
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         *   0   1   2   3   4   5   6            0   1   2   3   4   5   6
         */

        delete[] vet[0];
        delete[] vet[2];
        delete[] vet[5];
        //std::cout << p << std::endl;
        delete[] vet[3];
        //std::cout << p << std::endl;

        auto passed(true);
        try
        {
            // If the deletion went well, we should be have a two free blocks at the middle.
            // Therefore, we should be able to successfully request back a single 2-block-length area.
            size_t new_size(4 * SLPool<BLOCK_SIZE>::BLK_SZ - SLPool<BLOCK_SIZE>::TAG_SZ - SLPool<BLOCK_SIZE>::HEADER_SZ);
            vet[3] = new (p) byte[new_size];
            // Fill up the char with "01234567890123...."
            std::ostringstream oss;
            auto j(0u);
            while (j < new_size - 1) // Remember we have to reserve one extra space for the '\0'.
                oss << (j++ % 10);
            strcpy(vet[3], oss.str().c_str());
        }
        catch (std::runtime_error &e)
        {
            passed = false;
        }
        catch (std::bad_alloc &e)
        {
            passed = false;
        }

        std::cout << ">>> Testing pool after freeing area between a free and reserved areas:  L R R => L L R... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks - area_metainfo);

        byte *vet[n_chunks]; // Array of byte pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            // Fill up the char with "01234567890123...."
            strcpy(vet[i], oss.str().c_str());
            //std::cout << ">> vet[" << i << "] = " << vet[i] << std::endl;
        }

        /*
         * Teste #4
         *
         * Freeing up target 'x'.
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         * | R | R | R | R | R | R | R |  ===>  |             L             |
         * +---+---+---+---+---+---+---+        +---+---+---+---+---+---+---+
         *   0   1   2   3   4   5   6            0   1   2   3   4   5   6
         */

        for (auto i(0); i < n_chunks; i += 2)
        {
            delete[] vet[i];
            // Print pool
            //std::cout << p << std::endl;
        }
        for (auto i(1); i < n_chunks; i += 2)
        {
            delete[] vet[i];
            // Print pool
            //std::cout << p << std::endl;
        }
        auto passed(true);
        try
        {
            // If the deletion went well, we should be have a two free blocks at the middle.
            // Therefore, we should be able to successfully request back a single 2-block-length area.
            size_t new_size(14 * SLPool<BLOCK_SIZE>::BLK_SZ - SLPool<BLOCK_SIZE>::TAG_SZ - SLPool<BLOCK_SIZE>::HEADER_SZ);
            vet[3] = new (p) byte[new_size];
            // Fill up the char with "01234567890123...."
            std::ostringstream oss;
            auto j(0u);
            while (j < new_size - 1) // Remember we have to reserve one extra space for the '\0'.
                oss << (j++ % 10);
            strcpy(vet[3], oss.str().c_str());
        }
        catch (std::runtime_error &e)
        {
            passed = false;
        }
        catch (std::bad_alloc &e)
        {
            passed = false;
        }

        std::cout << ">>> Testing pool after freeing the entire pool: R R R R R R R => L... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    return EXIT_SUCCESS;
}
