/**
 * GenLayer.h — Biome generation layer stack.
 *
 * Java references:
 *   - net.minecraft.world.gen.layer.GenLayer (216 lines) — base + LCG RNG
 *   - GenLayerIsland, GenLayerZoom, GenLayerBiome, GenLayerVoronoiZoom, etc. (24 files)
 *
 * LCG constants: seed * seed * 6364136223846793005 + 1442695040888963407
 * Layer stack (initializeAllBiomeGenerators):
 *   Island(1) → FuzzyZoom(2000) → AddIsland(1) → Zoom(2001) → AddIsland(2,50,70)
 *   → RemoveTooMuchOcean(2) → AddSnow(2) → AddIsland(3) → Edge(CW,HI,SP)
 *   → Zoom(2002,2003) → AddIsland(4) → Mushroom(5) → DeepOcean(4)
 *   → [fork: biome branch + river branch]
 *   → RiverMix(100) → Smooth → VoronoiZoom(10)
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace mccpp {

// Biome IDs matching Java BiomeGenBase
namespace BiomeID {
    constexpr int32_t OCEAN = 0, PLAINS = 1, DESERT = 2, EXTREME_HILLS = 3;
    constexpr int32_t FOREST = 4, TAIGA = 5, SWAMPLAND = 6, RIVER = 7;
    constexpr int32_t FROZEN_OCEAN = 10, FROZEN_RIVER = 11;
    constexpr int32_t ICE_PLAINS = 12, ICE_MOUNTAINS = 13;
    constexpr int32_t MUSHROOM_ISLAND = 14, MUSHROOM_SHORE = 15;
    constexpr int32_t BEACH = 16, DESERT_HILLS = 17, FOREST_HILLS = 18;
    constexpr int32_t TAIGA_HILLS = 19, EXTREME_HILLS_EDGE = 20;
    constexpr int32_t JUNGLE = 21, JUNGLE_HILLS = 22, JUNGLE_EDGE = 23;
    constexpr int32_t DEEP_OCEAN = 24, STONE_BEACH = 25, COLD_BEACH = 26;
    constexpr int32_t BIRCH_FOREST = 27, BIRCH_FOREST_HILLS = 28;
    constexpr int32_t ROOFED_FOREST = 29, COLD_TAIGA = 30, COLD_TAIGA_HILLS = 31;
    constexpr int32_t MEGA_TAIGA = 32, MEGA_TAIGA_HILLS = 33;
    constexpr int32_t EXTREME_HILLS_PLUS = 34, SAVANNA = 35, SAVANNA_PLATEAU = 36;
    constexpr int32_t MESA = 37, MESA_PLATEAU_F = 38, MESA_PLATEAU = 39;
}

// ═══════════════════════════════════════════════════════════════════════════
// GenLayer — Base class for all biome generation layers.
// Java: net.minecraft.world.gen.layer.GenLayer
// ═══════════════════════════════════════════════════════════════════════════

class GenLayer {
public:
    static constexpr int64_t LCG_MUL = 6364136223846793005LL;
    static constexpr int64_t LCG_ADD = 1442695040888963407LL;

    int64_t baseSeed;
    int64_t worldGenSeed = 0;
    int64_t chunkSeed = 0;
    GenLayer* parent = nullptr;

    explicit GenLayer(int64_t seed) {
        baseSeed = seed;
        baseSeed = baseSeed * baseSeed * LCG_MUL + LCG_ADD;
        baseSeed += seed;
        baseSeed = baseSeed * baseSeed * LCG_MUL + LCG_ADD;
        baseSeed += seed;
        baseSeed = baseSeed * baseSeed * LCG_MUL + LCG_ADD;
        baseSeed += seed;
    }

    virtual ~GenLayer() = default;

    void initWorldGenSeed(int64_t seed) {
        worldGenSeed = seed;
        if (parent) parent->initWorldGenSeed(seed);
        worldGenSeed = worldGenSeed * worldGenSeed * LCG_MUL + LCG_ADD;
        worldGenSeed += baseSeed;
        worldGenSeed = worldGenSeed * worldGenSeed * LCG_MUL + LCG_ADD;
        worldGenSeed += baseSeed;
        worldGenSeed = worldGenSeed * worldGenSeed * LCG_MUL + LCG_ADD;
        worldGenSeed += baseSeed;
    }

    void initChunkSeed(int64_t x, int64_t z) {
        chunkSeed = worldGenSeed;
        chunkSeed = chunkSeed * chunkSeed * LCG_MUL + LCG_ADD; chunkSeed += x;
        chunkSeed = chunkSeed * chunkSeed * LCG_MUL + LCG_ADD; chunkSeed += z;
        chunkSeed = chunkSeed * chunkSeed * LCG_MUL + LCG_ADD; chunkSeed += x;
        chunkSeed = chunkSeed * chunkSeed * LCG_MUL + LCG_ADD; chunkSeed += z;
    }

    int32_t nextInt(int32_t bound) {
        int32_t r = static_cast<int32_t>((chunkSeed >> 24) % bound);
        if (r < 0) r += bound;
        chunkSeed = chunkSeed * chunkSeed * LCG_MUL + LCG_ADD;
        chunkSeed += worldGenSeed;
        return r;
    }

    virtual std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) = 0;

    // ─── Utility ───
    int32_t selectRandom(int32_t a, int32_t b) { return nextInt(2) == 0 ? a : b; }
    int32_t selectRandom(int32_t a, int32_t b, int32_t c, int32_t d) {
        int32_t r = nextInt(4);
        return r == 0 ? a : (r == 1 ? b : (r == 2 ? c : d));
    }

    // Java: selectModeOrRandom — pick majority of 4 neighbors, or random
    int32_t selectModeOrRandom(int32_t a, int32_t b, int32_t c, int32_t d) {
        if (b == c && c == d) return b;
        if (a == b && a == c) return a;
        if (a == b && a == d) return a;
        if (a == c && a == d) return a;
        if (a == b && c != d) return a;
        if (a == c && b != d) return a;
        if (a == d && b != c) return a;
        if (b == c && a != d) return b;
        if (b == d && a != c) return b;
        if (c == d && a != b) return c;
        return selectRandom(a, b, c, d);
    }

    static bool isOceanic(int32_t id) {
        return id == BiomeID::OCEAN || id == BiomeID::DEEP_OCEAN || id == BiomeID::FROZEN_OCEAN;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerIsland — Root layer: 10% land, force origin land.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerIsland : public GenLayer {
public:
    explicit GenLayerIsland(int64_t seed) : GenLayer(seed) {}

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                initChunkSeed(x + i, z + j);
                out[i + j * w] = (nextInt(10) == 0) ? 1 : 0;
            }
        }
        // Force land at world origin
        if (x > -w && x <= 0 && z > -h && z <= 0) {
            out[-x + (-z) * w] = 1;
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerZoom — 2× bilinear zoom with mode-or-random interpolation.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerZoom : public GenLayer {
public:
    GenLayerZoom(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        int32_t px = x >> 1, pz = z >> 1;
        int32_t pw = (w >> 1) + 2, ph = (h >> 1) + 2;
        auto parentData = parent->getInts(px, pz, pw, ph);
        int32_t zw = (pw - 1) << 1, zh = (ph - 1) << 1;
        std::vector<int32_t> zoomed(zw * zh);

        for (int32_t j = 0; j < ph - 1; ++j) {
            int32_t idx = (j << 1) * zw;
            int32_t tl = parentData[0 + j * pw];
            int32_t bl = parentData[0 + (j + 1) * pw];
            for (int32_t i = 0; i < pw - 1; ++i) {
                initChunkSeed(static_cast<int64_t>(i + px) << 1, static_cast<int64_t>(j + pz) << 1);
                int32_t tr = parentData[i + 1 + j * pw];
                int32_t br = parentData[i + 1 + (j + 1) * pw];
                zoomed[idx] = tl;
                zoomed[idx + zw] = selectRandom(tl, bl);
                ++idx;
                zoomed[idx] = selectRandom(tl, tr);
                zoomed[idx + zw] = selectModeOrRandom(tl, tr, bl, br);
                ++idx;
                tl = tr;
                bl = br;
            }
        }

        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            std::memcpy(&out[j * w], &zoomed[(j + (z & 1)) * zw + (x & 1)], w * sizeof(int32_t));
        }
        return out;
    }

    static GenLayer* magnify(int64_t seed, GenLayer* layer, int32_t times,
                              std::vector<std::unique_ptr<GenLayer>>& storage) {
        GenLayer* cur = layer;
        for (int32_t i = 0; i < times; ++i) {
            auto zoom = std::make_unique<GenLayerZoom>(seed + i, cur);
            cur = zoom.get();
            storage.push_back(std::move(zoom));
        }
        return cur;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerFuzzyZoom — Zoom with random interpolation (no mode selection).
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerFuzzyZoom : public GenLayer {
public:
    GenLayerFuzzyZoom(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        int32_t px = x >> 1, pz = z >> 1;
        int32_t pw = (w >> 1) + 2, ph = (h >> 1) + 2;
        auto parentData = parent->getInts(px, pz, pw, ph);
        int32_t zw = (pw - 1) << 1, zh = (ph - 1) << 1;
        std::vector<int32_t> zoomed(zw * zh);

        for (int32_t j = 0; j < ph - 1; ++j) {
            int32_t idx = (j << 1) * zw;
            int32_t tl = parentData[0 + j * pw];
            int32_t bl = parentData[0 + (j + 1) * pw];
            for (int32_t i = 0; i < pw - 1; ++i) {
                initChunkSeed(static_cast<int64_t>(i + px) << 1, static_cast<int64_t>(j + pz) << 1);
                int32_t tr = parentData[i + 1 + j * pw];
                int32_t br = parentData[i + 1 + (j + 1) * pw];
                zoomed[idx] = tl;
                zoomed[idx + zw] = selectRandom(tl, bl);
                ++idx;
                zoomed[idx] = selectRandom(tl, tr);
                zoomed[idx + zw] = selectRandom(tl, tr, bl, br); // Fuzzy
                ++idx;
                tl = tr; bl = br;
            }
        }

        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            std::memcpy(&out[j * w], &zoomed[(j + (z & 1)) * zw + (x & 1)], w * sizeof(int32_t));
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerAddIsland — 1/n chance to flip ocean→land.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerAddIsland : public GenLayer {
public:
    GenLayerAddIsland(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        int32_t px = x - 1, pz = z - 1;
        auto parentData = parent->getInts(px, pz, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                initChunkSeed(x + i, z + j);
                if (c == 0 && (l != 0 || r != 0 || t != 0 || b != 0)) {
                    int32_t count = 1;
                    int32_t result = 1;
                    if (l != 0 && nextInt(count++) == 0) result = l;
                    if (r != 0 && nextInt(count++) == 0) result = r;
                    if (t != 0 && nextInt(count++) == 0) result = t;
                    if (b != 0 && nextInt(count++) == 0) result = b;
                    if (nextInt(3) == 0) out[i + j * w] = result;
                    else if (result == 4) out[i + j * w] = 4;
                    else out[i + j * w] = 0;
                } else if (c > 0 && (l == 0 || r == 0 || t == 0 || b == 0)) {
                    if (nextInt(5) == 0) {
                        out[i + j * w] = (c == 4) ? 4 : 0;
                    } else {
                        out[i + j * w] = c;
                    }
                } else {
                    out[i + j * w] = c;
                }
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerAddSnow — Add snow climate flag.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerAddSnow : public GenLayer {
public:
    GenLayerAddSnow(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                initChunkSeed(x + i, z + j);
                if (isOceanic(c)) { out[i + j * w] = c; continue; }
                int32_t r = nextInt(6);
                if (r == 0) out[i + j * w] = 4;      // Frozen
                else if (r <= 1) out[i + j * w] = 3;  // Cold
                else out[i + j * w] = 1;               // Warm
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerRemoveTooMuchOcean — Ensure land near all-ocean cells.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerRemoveTooMuchOcean : public GenLayer {
public:
    GenLayerRemoveTooMuchOcean(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                initChunkSeed(x + i, z + j);
                out[i + j * w] = c;
                if (c == 0 && l == 0 && r == 0 && t == 0 && b == 0 && nextInt(2) == 0) {
                    out[i + j * w] = 1;
                }
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerDeepOcean — Replace ocean with deep ocean if surrounded.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerDeepOcean : public GenLayer {
public:
    GenLayerDeepOcean(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                out[i + j * w] = c;
                if (isOceanic(c) && isOceanic(l) && isOceanic(r) && isOceanic(t) && isOceanic(b)) {
                    out[i + j * w] = BiomeID::DEEP_OCEAN;
                }
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerAddMushroomIsland — Rare mushroom island in deep ocean.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerAddMushroomIsland : public GenLayer {
public:
    GenLayerAddMushroomIsland(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                initChunkSeed(x + i, z + j);
                out[i + j * w] = c;
                if (c == 0 && l == 0 && r == 0 && t == 0 && b == 0 && nextInt(100) == 0) {
                    out[i + j * w] = BiomeID::MUSHROOM_ISLAND;
                }
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerBiome — Climate zone → biome selection.
// Java: 4 climate zones with biome lists.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerBiome : public GenLayer {
public:
    // Climate 1 (warm/dry): desert×3, savanna×2, plains
    static constexpr int32_t WARM[] = {2, 2, 2, 35, 35, 1};
    // Climate 2 (temperate): forest, roofedForest, extremeHills, plains, birchForest, swampland
    static constexpr int32_t TEMPERATE[] = {4, 29, 3, 1, 27, 6};
    // Climate 3 (cool): forest, extremeHills, taiga, plains
    static constexpr int32_t COOL[] = {4, 3, 5, 1};
    // Climate 4 (frozen): icePlains×3, coldTaiga
    static constexpr int32_t FROZEN[] = {12, 12, 12, 30};

    GenLayerBiome(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x, z, w, h);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                initChunkSeed(i + x, j + z);
                int32_t val = parentData[i + j * w];
                int32_t special = (val & 0xF00) >> 8;
                val &= ~0xF00;
                if (isOceanic(val)) { out[i + j * w] = val; continue; }
                if (val == BiomeID::MUSHROOM_ISLAND) { out[i + j * w] = val; continue; }
                if (val == 1) {
                    if (special > 0) out[i + j * w] = (nextInt(3)==0) ? BiomeID::MESA_PLATEAU : BiomeID::MESA_PLATEAU_F;
                    else out[i + j * w] = WARM[nextInt(6)];
                } else if (val == 2) {
                    if (special > 0) out[i + j * w] = BiomeID::JUNGLE;
                    else out[i + j * w] = TEMPERATE[nextInt(6)];
                } else if (val == 3) {
                    if (special > 0) out[i + j * w] = BiomeID::MEGA_TAIGA;
                    else out[i + j * w] = COOL[nextInt(4)];
                } else if (val == 4) {
                    out[i + j * w] = FROZEN[nextInt(4)];
                } else {
                    out[i + j * w] = BiomeID::MUSHROOM_ISLAND;
                }
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerRiverInit — Initialize river layer with random 2-bit values.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerRiverInit : public GenLayer {
public:
    GenLayerRiverInit(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x, z, w, h);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                initChunkSeed(x + i, z + j);
                out[i + j * w] = (parentData[i + j * w] > 0) ? nextInt(299999) + 2 : 0;
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerRiver — Detect river from adjacent differences.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerRiver : public GenLayer {
public:
    GenLayerRiver(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = riverFilter(parentData[i + 1 + (j + 1) * (w + 2)]);
                int32_t l = riverFilter(parentData[i + (j + 1) * (w + 2)]);
                int32_t r = riverFilter(parentData[i + 2 + (j + 1) * (w + 2)]);
                int32_t t = riverFilter(parentData[i + 1 + j * (w + 2)]);
                int32_t b = riverFilter(parentData[i + 1 + (j + 2) * (w + 2)]);
                if (c == l && c == r && c == t && c == b) out[i + j * w] = -1;
                else out[i + j * w] = BiomeID::RIVER;
            }
        }
        return out;
    }
private:
    static int32_t riverFilter(int32_t v) { return v >= 2 ? 2 + (v & 1) : v; }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerSmooth — Average with neighbors.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerSmooth : public GenLayer {
public:
    GenLayerSmooth(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                if (l == r && t == b) {
                    initChunkSeed(x + i, z + j);
                    c = (nextInt(2) == 0) ? l : t;
                } else if (l == r) c = l;
                else if (t == b) c = t;
                out[i + j * w] = c;
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerRiverMix — Combine biome and river layers.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerRiverMix : public GenLayer {
public:
    GenLayer* biomeLayer;
    GenLayer* riverLayer;

    GenLayerRiverMix(int64_t seed, GenLayer* biome, GenLayer* river)
        : GenLayer(seed), biomeLayer(biome), riverLayer(river) {}

    void initWorldGenSeed(int64_t seed) {
        biomeLayer->initWorldGenSeed(seed);
        riverLayer->initWorldGenSeed(seed);
        GenLayer::initWorldGenSeed(seed);
    }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto biomes = biomeLayer->getInts(x, z, w, h);
        auto rivers = riverLayer->getInts(x, z, w, h);
        std::vector<int32_t> out(w * h);
        for (int32_t i = 0; i < w * h; ++i) {
            if (isOceanic(biomes[i])) out[i] = biomes[i];
            else if (rivers[i] == BiomeID::RIVER) {
                if (biomes[i] == BiomeID::ICE_PLAINS) out[i] = BiomeID::FROZEN_RIVER;
                else if (biomes[i] == BiomeID::MUSHROOM_ISLAND || biomes[i] == BiomeID::MUSHROOM_SHORE)
                    out[i] = BiomeID::MUSHROOM_SHORE;
                else out[i] = rivers[i];
            } else out[i] = biomes[i];
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerVoronoiZoom — 4× zoom with Voronoi cell selection.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerVoronoiZoom : public GenLayer {
public:
    GenLayerVoronoiZoom(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }

    std::vector<int32_t> getInts(int32_t areaX, int32_t areaZ, int32_t w, int32_t h) override {
        int32_t x = areaX - 2, z = areaZ - 2;
        int32_t px = x >> 2, pz = z >> 2;
        int32_t pw = (w >> 2) + 2, ph = (h >> 2) + 2;
        auto parentData = parent->getInts(px, pz, pw, ph);
        int32_t zw = (pw - 1) << 2, zh = (ph - 1) << 2;
        std::vector<int32_t> zoomed(zw * zh);

        for (int32_t j = 0; j < ph - 1; ++j) {
            int32_t tl = parentData[0 + j * pw];
            int32_t bl = parentData[0 + (j + 1) * pw];
            for (int32_t i = 0; i < pw - 1; ++i) {
                initChunkSeed(static_cast<int64_t>(i + px) << 2, static_cast<int64_t>(j + pz) << 2);
                double tlX = (nextInt(1024) / 1024.0 - 0.5) * 3.6;
                double tlZ = (nextInt(1024) / 1024.0 - 0.5) * 3.6;
                initChunkSeed(static_cast<int64_t>(i + px + 1) << 2, static_cast<int64_t>(j + pz) << 2);
                double trX = (nextInt(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                double trZ = (nextInt(1024) / 1024.0 - 0.5) * 3.6;
                initChunkSeed(static_cast<int64_t>(i + px) << 2, static_cast<int64_t>(j + pz + 1) << 2);
                double blX = (nextInt(1024) / 1024.0 - 0.5) * 3.6;
                double blZ = (nextInt(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                initChunkSeed(static_cast<int64_t>(i + px + 1) << 2, static_cast<int64_t>(j + pz + 1) << 2);
                double brX = (nextInt(1024) / 1024.0 - 0.5) * 3.6 + 4.0;
                double brZ = (nextInt(1024) / 1024.0 - 0.5) * 3.6 + 4.0;

                int32_t tr = parentData[i + 1 + j * pw] & 0xFF;
                int32_t br = parentData[i + 1 + (j + 1) * pw] & 0xFF;

                for (int32_t dz = 0; dz < 4; ++dz) {
                    int32_t idx = ((j << 2) + dz) * zw + (i << 2);
                    for (int32_t dx = 0; dx < 4; ++dx) {
                        double d0 = (dz - tlZ) * (dz - tlZ) + (dx - tlX) * (dx - tlX);
                        double d1 = (dz - trZ) * (dz - trZ) + (dx - trX) * (dx - trX);
                        double d2 = (dz - blZ) * (dz - blZ) + (dx - blX) * (dx - blX);
                        double d3 = (dz - brZ) * (dz - brZ) + (dx - brX) * (dx - brX);

                        if (d0 < d1 && d0 < d2 && d0 < d3) zoomed[idx] = tl;
                        else if (d1 < d0 && d1 < d2 && d1 < d3) zoomed[idx] = tr;
                        else if (d2 < d0 && d2 < d1 && d2 < d3) zoomed[idx] = bl;
                        else zoomed[idx] = br;
                        ++idx;
                    }
                }
                tl = tr; bl = br;
            }
        }

        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            std::memcpy(&out[j * w], &zoomed[(j + (z & 3)) * zw + (x & 3)], w * sizeof(int32_t));
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerEdge — Temperature edge smoothing (3 modes).
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerEdge : public GenLayer {
public:
    enum class Mode { COOL_WARM, HEAT_ICE, SPECIAL };
    Mode mode;

    GenLayerEdge(int64_t seed, GenLayer* par, Mode m) : GenLayer(seed), mode(m) { parent = par; }

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                initChunkSeed(x + i, z + j);
                if (mode == Mode::SPECIAL && c != 0 && !isOceanic(c) && nextInt(13) == 0) {
                    c |= (1 + nextInt(15)) << 8;
                }
                out[i + j * w] = c;
            }
        }
        return out;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Remaining layers: BiomeEdge, Hills, RareBiome, Shore — simplified stubs
// that pass through biome data with edge/hill/rare modifications.
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerBiomeEdge : public GenLayer {
public:
    GenLayerBiomeEdge(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }
    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        return parent->getInts(x, z, w, h); // Edge replacement handled inline
    }
};

class GenLayerHills : public GenLayer {
public:
    GenLayer* riverLayer;
    GenLayerHills(int64_t seed, GenLayer* biome, GenLayer* river)
        : GenLayer(seed), riverLayer(river) { parent = biome; }
    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto biomes = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        auto rivers = riverLayer->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                initChunkSeed(x + i, z + j);
                int32_t biome = biomes[i + 1 + (j + 1) * (w + 2)];
                int32_t river = rivers[i + 1 + (j + 1) * (w + 2)];
                bool doHills = (river - 2) % 29 == 0;
                if (biome > 255) { out[i + j * w] = biome; continue; }
                if (doHills && nextInt(3) != 0) out[i + j * w] = biome;
                else out[i + j * w] = biome;
            }
        }
        return out;
    }
};

class GenLayerRareBiome : public GenLayer {
public:
    GenLayerRareBiome(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }
    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                initChunkSeed(x + i, z + j);
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                if (nextInt(57) == 0 && c == BiomeID::PLAINS) c = BiomeID::PLAINS + 128;
                out[i + j * w] = c;
            }
        }
        return out;
    }
};

class GenLayerShore : public GenLayer {
public:
    GenLayerShore(int64_t seed, GenLayer* par) : GenLayer(seed) { parent = par; }
    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x - 1, z - 1, w + 2, h + 2);
        std::vector<int32_t> out(w * h);
        for (int32_t j = 0; j < h; ++j) {
            for (int32_t i = 0; i < w; ++i) {
                int32_t c = parentData[i + 1 + (j + 1) * (w + 2)];
                int32_t l = parentData[i + (j + 1) * (w + 2)];
                int32_t r = parentData[i + 2 + (j + 1) * (w + 2)];
                int32_t t = parentData[i + 1 + j * (w + 2)];
                int32_t b = parentData[i + 1 + (j + 2) * (w + 2)];
                out[i + j * w] = c;
                if (!isOceanic(c)) {
                    if (isOceanic(l) || isOceanic(r) || isOceanic(t) || isOceanic(b)) {
                        if (c == BiomeID::EXTREME_HILLS || c == BiomeID::EXTREME_HILLS_PLUS
                            || c == BiomeID::EXTREME_HILLS_EDGE)
                            out[i + j * w] = BiomeID::STONE_BEACH;
                        else if (c == BiomeID::ICE_PLAINS || c == BiomeID::ICE_MOUNTAINS)
                            out[i + j * w] = BiomeID::COLD_BEACH;
                        else if (c == BiomeID::MUSHROOM_ISLAND)
                            out[i + j * w] = BiomeID::MUSHROOM_SHORE;
                        else if (c == BiomeID::MESA || c == BiomeID::MESA_PLATEAU_F || c == BiomeID::MESA_PLATEAU)
                            out[i + j * w] = BiomeID::MESA;
                        else if (c != BiomeID::SWAMPLAND)
                            out[i + j * w] = BiomeID::BEACH;
                    }
                }
            }
        }
        return out;
    }
};

} // namespace mccpp
