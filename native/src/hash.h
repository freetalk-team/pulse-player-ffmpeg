#pragma once

uint64_t murmurHash64(const void* key, size_t len, uint64_t seed)
{
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t* data = (const uint64_t*)key;
    const uint64_t* end = data + (len / 8);

    while (data != end) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const uint8_t* data2 = (const uint8_t*)data;

    switch (len & 7) {
        case 7: h ^= (uint64_t)data2[6] << 48;
        case 6: h ^= (uint64_t)data2[5] << 40;
        case 5: h ^= (uint64_t)data2[4] << 32;
        case 4: h ^= (uint64_t)data2[3] << 24;
        case 3: h ^= (uint64_t)data2[2] << 16;
        case 2: h ^= (uint64_t)data2[1] << 8;
        case 1: h ^= (uint64_t)data2[0];
                h *= m;
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

inline uint64_t murmurhash3_64_update(uint64_t hash, const uint8_t* data, size_t len) {
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    const uint64_t* data64 = reinterpret_cast<const uint64_t*>(data);
    const uint64_t* end = data64 + (len / 8);

    while (data64 != end) {
        uint64_t k = *data64++;
        k *= m;
        k ^= k >> r;
        k *= m;
        
        hash ^= k;
        hash *= m;
    }

    const uint8_t* data8 = reinterpret_cast<const uint8_t*>(data64);
    switch (len & 7) {
        case 7: hash ^= static_cast<uint64_t>(data8[6]) << 48; [[fallthrough]];
        case 6: hash ^= static_cast<uint64_t>(data8[5]) << 40; [[fallthrough]];
        case 5: hash ^= static_cast<uint64_t>(data8[4]) << 32; [[fallthrough]];
        case 4: hash ^= static_cast<uint64_t>(data8[3]) << 24; [[fallthrough]];
        case 3: hash ^= static_cast<uint64_t>(data8[2]) << 16; [[fallthrough]];
        case 2: hash ^= static_cast<uint64_t>(data8[1]) << 8;  [[fallthrough]];
        case 1: hash ^= static_cast<uint64_t>(data8[0]);
                hash *= m;
    };
    
    hash ^= hash >> r;
    hash *= m;
    hash ^= hash >> r;
    return hash;
}
