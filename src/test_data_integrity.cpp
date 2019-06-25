/**
 * @file test_data_integrity.cpp
 *
 * @description
 * Test the MP's data integrity after basic allocate/free operations.
 *
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

    std::cout << ">>> Begining DATA INTEGRITY tests...\n\n";

    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Reference value.
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

        // Test each position.
        for (auto i(0); i < n_chunks; ++i)
        {
            passed = strcmp(oss.str().c_str(), vet[i]) == 0;
            if (not passed)
                break;
        }

        std::cout << ">>> Testing pool integrity after writing the entire pool... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Reference value.
        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Reference value.
        std::string reference_a = oss.str();
        std::string reference_b(reference_a);
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(reference_b.begin(), reference_b.end(), g);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], reference_a.c_str());
        }

        // Overwriting the even positions within the array of strings.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 == 0)
                strcpy(vet[i], reference_b.c_str());
        }

        // Test each position.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 == 0)
                passed = strcmp(reference_b.c_str(), vet[i]) == 0;
            else
                passed = strcmp(reference_a.c_str(), vet[i]) == 0;
            if (not passed)
                break;
        }

        std::cout << ">>> Testing pool integrity after interleaved writing... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Reference value.
        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        std::string reference_a(oss.str());

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], reference_a.c_str());
        }

        // Write a different value on odd locations.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 == 0)
                delete[] vet[i];
        }

        // Test each position against references.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 != 0)
            {
                passed = strcmp(reference_a.c_str(), vet[i]) == 0;
                if (not passed)
                    break;
            }
        }

        std::cout << ">>> Testing pool integrity after deleting even interleaved areas... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }
    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Reference value.
        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        std::string reference_a(oss.str());

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], reference_a.c_str());
        }

        // Write a different value on odd locations.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 != 0)
                delete[] vet[i];
        }

        // Test each position against references.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 == 0)
            {
                passed = strcmp(reference_a.c_str(), vet[i]) == 0;
                if (not passed)
                    break;
            }
        }

        std::cout << ">>> Testing pool integrity after deleting odd interleaved areas... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }
    {
        // Set up 7 chunks of data.
        // We decrease the area metainfo so we might suppport a single reserved area
        // with the size of the entire pool.
        mp::SLPool<BLOCK_SIZE> p(sizeof(byte) * chunk * n_chunks);
        std::cout << p << std::endl;

        bool passed(false);
        byte *vet[n_chunks]; // Array of chunk pointer to be allocated.

        // Define the chunk length so that it corresponds to two blocks of memory.
        size_t chunk_len(2 * BLOCK_SIZE - area_metainfo);

        // Reference value.
        // Fill up the cha array with "01234567890123...."
        std::ostringstream oss;
        auto j(0u);
        while (j < chunk_len - 1) // Remember we have to reserve one extra space for the '\0'.
            oss << (j++ % 10);

        // Reference value.
        std::string reference_a = oss.str();
        std::string reference_b(reference_a);
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(reference_b.begin(), reference_b.end(), g);

        // Fill up the MP.
        for (auto i(0); i < n_chunks; ++i)
        {
            vet[i] = new (p) byte[chunk_len];
            strcpy(vet[i], reference_a.c_str());
        }

        // Free memory of odd positions with the array.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 != 0)
                delete[] vet[i];
        }

        // Reallocate the odd locations.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 != 0)
            {
                vet[i] = new (p) byte[chunk_len];
                // new value
                strcpy(vet[i], reference_b.c_str());
            }
        }

        // Test each position against references.
        for (auto i(0); i < n_chunks; ++i)
        {
            if (i % 2 == 0)
                passed = strcmp(reference_a.c_str(), vet[i]) == 0;
            else
                passed = strcmp(reference_b.c_str(), vet[i]) == 0;
            if (not passed)
                break;
        }

        std::cout << ">>> Testing pool integrity after deleting and realocating interleaved areas... ";
        std::cout << (passed ? "\e[1;35mpassed!\e[0m" : "\e[1;31mfailed!\e[0m") << std::endl;
    }

    return EXIT_SUCCESS;
}
