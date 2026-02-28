/**
 * WorldChunkManager.h — Biome provider for terrain generation.
 *
 * Java references:
 *   - net.minecraft.world.biome.WorldChunkManager — Biome provider
 *   - net.minecraft.world.biome.BiomeCache — Chunk-aligned biome cache
 *   - net.minecraft.world.gen.layer.GenLayer — Layer-based biome generation
 *   - net.minecraft.world.gen.layer.IntCache — Reusable int array cache
 *
 * Architecture:
 *   - GenLayer stack generates biome IDs from world seed
 *   - Two layers: genBiomes (quarter-res for density blending) and
 *     biomeIndexLayer (full-res for surface replacement)
 *   - BiomeCache: caches 16×16 chunk-aligned biome arrays, keyed by
 *     (chunkX, chunkZ) → long hash
 *   - biomesToSpawnIn: forest, plains, taiga, taigaHills, forestHills,
 *     jungle, jungleHills
 *
 * Thread safety: GenLayer evaluation is per-thread (no shared state).
 * JNI readiness: Simple data interfaces.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// GenLayer — Abstract biome generation layer.
// Java reference: net.minecraft.world.gen.layer.GenLayer
//
// Each layer transforms a 2D integer grid (biome IDs or intermediate values).
// Layers are chained: each layer's getInts() calls its parent's getInts().
// ═══════════════════════════════════════════════════════════════════════════

class GenLayer {
public:
    int64_t baseSeed;
    int64_t worldGenSeed;
    int64_t chunkSeed;

    explicit GenLayer(int64_t seed) : baseSeed(seed), worldGenSeed(0), chunkSeed(0) {}
    virtual ~GenLayer() = default;

    // Java: initWorldGenSeed — mix world seed with layer seed
    void initWorldGenSeed(int64_t worldSeed) {
        worldGenSeed = worldSeed;
        worldGenSeed = mixSeed(worldGenSeed, baseSeed);
        worldGenSeed = mixSeed(worldGenSeed, baseSeed);
        worldGenSeed = mixSeed(worldGenSeed, baseSeed);
    }

    // Java: initChunkSeed — per-position seed
    void initChunkSeed(int64_t x, int64_t z) {
        chunkSeed = worldGenSeed;
        chunkSeed = mixSeed(chunkSeed, x);
        chunkSeed = mixSeed(chunkSeed, z);
        chunkSeed = mixSeed(chunkSeed, x);
        chunkSeed = mixSeed(chunkSeed, z);
    }

    // Java: nextInt — bounded random from chunk seed
    int32_t nextInt(int32_t bound) {
        int32_t result = static_cast<int32_t>((chunkSeed >> 24) % static_cast<int64_t>(bound));
        if (result < 0) result += bound;
        chunkSeed = mixSeed(chunkSeed, worldGenSeed);
        return result;
    }

    // Get biome IDs for a rectangular area
    virtual std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) = 0;

private:
    static int64_t mixSeed(int64_t seed, int64_t salt) {
        seed *= seed * 6364136223846793005LL + 1442695040888963407LL;
        seed += salt;
        return seed;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerIsland — Base layer: 1/10 random chance of land (1), else ocean (0).
// Java reference: net.minecraft.world.gen.layer.GenLayerIsland
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerIsland : public GenLayer {
public:
    explicit GenLayerIsland(int64_t seed) : GenLayer(seed) {}

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        std::vector<int32_t> result(w * h);
        for (int32_t iz = 0; iz < h; ++iz) {
            for (int32_t ix = 0; ix < w; ++ix) {
                initChunkSeed(static_cast<int64_t>(x + ix), static_cast<int64_t>(z + iz));
                result[ix + iz * w] = (nextInt(10) == 0) ? 1 : 0;
            }
        }
        // Force center to land
        if (x > -w && x <= 0 && z > -h && z <= 0) {
            result[-x + (-z) * w] = 1;
        }
        return result;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerBiome — Assigns biome IDs based on climate zones.
// Simplified version — assigns from the exploration biome list.
// Java reference: net.minecraft.world.gen.layer.GenLayerBiome
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerBiome : public GenLayer {
public:
    GenLayer* parent;

    GenLayerBiome(int64_t seed, GenLayer* p) : GenLayer(seed), parent(p) {}

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        auto parentData = parent->getInts(x, z, w, h);
        std::vector<int32_t> result(w * h);

        // Biome assignment by climate
        // Warm: desert(2), savanna(35), plains(1)
        // Medium: forest(4), roofedForest(29), extremeHills(3), plains(1)
        // Cold: icePlains(12), coldTaiga(30), extremeHills(3)
        // Ocean: ocean(0)
        constexpr int32_t warmBiomes[] = {2, 35, 1, 35, 1, 2};
        constexpr int32_t medBiomes[] = {4, 29, 3, 1, 27, 4};
        constexpr int32_t coldBiomes[] = {12, 30, 3, 5, 12, 30};

        for (int32_t iz = 0; iz < h; ++iz) {
            for (int32_t ix = 0; ix < w; ++ix) {
                initChunkSeed(static_cast<int64_t>(x + ix), static_cast<int64_t>(z + iz));
                int32_t val = parentData[ix + iz * w];

                if (val == 0) {
                    result[ix + iz * w] = 0;  // ocean
                } else {
                    int32_t climate = nextInt(6);
                    int32_t temp = nextInt(3);
                    if (temp == 0) {
                        result[ix + iz * w] = warmBiomes[climate];
                    } else if (temp == 1) {
                        result[ix + iz * w] = medBiomes[climate];
                    } else {
                        result[ix + iz * w] = coldBiomes[climate];
                    }
                }
            }
        }
        return result;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// GenLayerZoom — 2× zoom with interpolation.
// Java reference: net.minecraft.world.gen.layer.GenLayerZoom
// ═══════════════════════════════════════════════════════════════════════════

class GenLayerZoom : public GenLayer {
public:
    GenLayer* parent;

    GenLayerZoom(int64_t seed, GenLayer* p) : GenLayer(seed), parent(p) {}

    std::vector<int32_t> getInts(int32_t x, int32_t z, int32_t w, int32_t h) override {
        int32_t px = x >> 1;
        int32_t pz = z >> 1;
        int32_t pw = (w >> 1) + 2;
        int32_t ph = (h >> 1) + 2;
        auto parentData = parent->getInts(px, pz, pw, ph);

        int32_t zoomedW = (pw - 1) * 2;
        int32_t zoomedH = (ph - 1) * 2;
        std::vector<int32_t> zoomed(zoomedW * zoomedH);

        for (int32_t iz = 0; iz < ph - 1; ++iz) {
            for (int32_t ix = 0; ix < pw - 1; ++ix) {
                initChunkSeed(static_cast<int64_t>((px + ix) * 2),
                             static_cast<int64_t>((pz + iz) * 2));
                int32_t tl = parentData[ix + iz * pw];
                int32_t tr = parentData[ix + 1 + iz * pw];
                int32_t bl = parentData[ix + (iz + 1) * pw];
                int32_t br = parentData[ix + 1 + (iz + 1) * pw];

                int32_t ox = ix * 2;
                int32_t oz = iz * 2;
                zoomed[ox + oz * zoomedW] = tl;
                zoomed[ox + (oz + 1) * zoomedW] = (nextInt(2) == 0) ? tl : bl;
                zoomed[ox + 1 + oz * zoomedW] = (nextInt(2) == 0) ? tl : tr;
                zoomed[ox + 1 + (oz + 1) * zoomedW] = selectRandom(tl, tr, bl, br);
            }
        }

        // Extract requested region
        int32_t offsetX = x & 1;
        int32_t offsetZ = z & 1;
        std::vector<int32_t> result(w * h);
        for (int32_t iz = 0; iz < h; ++iz) {
            for (int32_t ix = 0; ix < w; ++ix) {
                int32_t si = (ix + offsetX) + (iz + offsetZ) * zoomedW;
                if (si >= 0 && si < static_cast<int32_t>(zoomed.size())) {
                    result[ix + iz * w] = zoomed[si];
                }
            }
        }
        return result;
    }

private:
    int32_t selectRandom(int32_t a, int32_t b, int32_t c, int32_t d) {
        if (b == c && c == d) return b;
        if (a == b && a == c) return a;
        if (a == b && a == d) return a;
        if (a == c && a == d) return a;
        if (a == b) return a;
        if (a == c) return a;
        if (a == d) return a;
        if (b == c) return b;
        if (b == d) return b;
        if (c == d) return c;
        int32_t r = nextInt(4);
        if (r == 0) return a;
        if (r == 1) return b;
        if (r == 2) return c;
        return d;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// BiomeCache — Chunk-aligned biome lookup cache.
// Java reference: net.minecraft.world.biome.BiomeCache
// ═══════════════════════════════════════════════════════════════════════════

class BiomeCache {
public:
    struct CacheEntry {
        std::array<int32_t, 256> biomes{};  // 16×16
        int64_t lastAccess = 0;
    };

    // Get biome for a single coordinate
    int32_t getBiomeAt(int32_t x, int32_t z,
                         std::function<std::vector<int32_t>(int32_t, int32_t, int32_t, int32_t)> genFn) {
        int32_t cx = x >> 4;
        int32_t cz = z >> 4;
        int64_t key = packKey(cx, cz);

        auto it = cache_.find(key);
        if (it == cache_.end()) {
            auto ids = genFn(cx * 16, cz * 16, 16, 16);
            CacheEntry entry;
            for (int32_t i = 0; i < 256 && i < static_cast<int32_t>(ids.size()); ++i) {
                entry.biomes[i] = ids[i];
            }
            entry.lastAccess = ++accessCounter_;
            cache_[key] = entry;
            it = cache_.find(key);
        } else {
            it->second.lastAccess = ++accessCounter_;
        }

        int32_t lx = x & 0xF;
        int32_t lz = z & 0xF;
        return it->second.biomes[lz * 16 + lx];
    }

    // Get cached 16×16 biomes for a chunk
    const std::array<int32_t, 256>& getCachedBiomes(int32_t x, int32_t z,
            std::function<std::vector<int32_t>(int32_t, int32_t, int32_t, int32_t)> genFn) {
        int32_t cx = x >> 4;
        int32_t cz = z >> 4;
        int64_t key = packKey(cx, cz);

        auto it = cache_.find(key);
        if (it == cache_.end()) {
            auto ids = genFn(cx * 16, cz * 16, 16, 16);
            CacheEntry entry;
            for (int32_t i = 0; i < 256 && i < static_cast<int32_t>(ids.size()); ++i) {
                entry.biomes[i] = ids[i];
            }
            entry.lastAccess = ++accessCounter_;
            cache_[key] = entry;
            it = cache_.find(key);
        } else {
            it->second.lastAccess = ++accessCounter_;
        }
        return it->second.biomes;
    }

    // Remove entries older than 30 seconds (simplified: keep last 256 entries)
    void cleanup() {
        if (cache_.size() <= 256) return;
        int64_t threshold = accessCounter_ - 256;
        for (auto it = cache_.begin(); it != cache_.end(); ) {
            if (it->second.lastAccess < threshold) {
                it = cache_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    std::unordered_map<int64_t, CacheEntry> cache_;
    int64_t accessCounter_ = 0;

    static int64_t packKey(int32_t cx, int32_t cz) {
        return (static_cast<int64_t>(cx) & 0xFFFFFFFFL) |
               ((static_cast<int64_t>(cz) & 0xFFFFFFFFL) << 32);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// WorldChunkManager — Biome provider.
// Java reference: net.minecraft.world.biome.WorldChunkManager
// ═══════════════════════════════════════════════════════════════════════════

class WorldChunkManager {
public:
    WorldChunkManager() {
        // Default spawn biomes
        biomesToSpawnIn_ = {4, 1, 5, 19, 18, 21, 22};  // forest, plains, taiga, etc.
    }

    // Initialize with world seed
    void init(int64_t worldSeed) {
        worldSeed_ = worldSeed;

        // Build GenLayer stack: Island → Biome → Zoom × 4 (quarter-res)
        // Then full-res = quarter-res → Zoom × 2
        islandLayer_ = std::make_unique<GenLayerIsland>(1L);
        biomeLayer_ = std::make_unique<GenLayerBiome>(200L, islandLayer_.get());
        zoomLayers_.clear();

        // 4 zoom levels for quarter-resolution (genBiomes)
        GenLayer* current = biomeLayer_.get();
        for (int32_t i = 0; i < 4; ++i) {
            auto zoom = std::make_unique<GenLayerZoom>(1000L + i, current);
            current = zoom.get();
            zoomLayers_.push_back(std::move(zoom));
        }
        genBiomes_ = current;

        // 2 more zoom levels for full-resolution (biomeIndexLayer)
        for (int32_t i = 0; i < 2; ++i) {
            auto zoom = std::make_unique<GenLayerZoom>(2000L + i, current);
            current = zoom.get();
            zoomLayers_.push_back(std::move(zoom));
        }
        biomeIndexLayer_ = current;

        // Initialize world gen seeds
        islandLayer_->initWorldGenSeed(worldSeed);
        biomeLayer_->initWorldGenSeed(worldSeed);
        for (auto& z : zoomLayers_) z->initWorldGenSeed(worldSeed);
    }

    // Java: getBiomeGenAt (single point)
    int32_t getBiomeAt(int32_t x, int32_t z) {
        return cache_.getBiomeAt(x, z, [this](int32_t bx, int32_t bz, int32_t w, int32_t h) {
            return biomeIndexLayer_->getInts(bx, bz, w, h);
        });
    }

    // Java: getBiomesForGeneration — quarter resolution for density blending
    std::vector<int32_t> getBiomesForGeneration(int32_t x, int32_t z, int32_t w, int32_t h) {
        return genBiomes_->getInts(x, z, w, h);
    }

    // Java: loadBlockGeneratorData — full resolution with cache
    std::vector<int32_t> loadBlockGeneratorData(int32_t x, int32_t z, int32_t w, int32_t h) {
        if (w == 16 && h == 16 && (x & 0xF) == 0 && (z & 0xF) == 0) {
            auto& cached = cache_.getCachedBiomes(x, z,
                [this](int32_t bx, int32_t bz, int32_t cw, int32_t ch) {
                    return biomeIndexLayer_->getInts(bx, bz, cw, ch);
                });
            return std::vector<int32_t>(cached.begin(), cached.end());
        }
        return biomeIndexLayer_->getInts(x, z, w, h);
    }

    // Java: getRainfall
    std::vector<float> getRainfall(int32_t x, int32_t z, int32_t w, int32_t h) {
        auto biomeIds = biomeIndexLayer_->getInts(x, z, w, h);
        std::vector<float> rainfall(w * h);
        for (int32_t i = 0; i < w * h; ++i) {
            // Simplified: use biome ID to get rainfall
            // In full implementation this would query BiomeGenBase.getIntRainfall()
            float r = getRainfallForBiome(biomeIds[i]);
            if (r > 1.0f) r = 1.0f;
            rainfall[i] = r;
        }
        return rainfall;
    }

    // Java: areBiomesViable — check if area contains only allowed biomes
    bool areBiomesViable(int32_t x, int32_t z, int32_t radius,
                           const std::vector<int32_t>& allowed) {
        int32_t x1 = (x - radius) >> 2;
        int32_t z1 = (z - radius) >> 2;
        int32_t x2 = (x + radius) >> 2;
        int32_t z2 = (z + radius) >> 2;
        int32_t w = x2 - x1 + 1;
        int32_t h = z2 - z1 + 1;
        auto ids = genBiomes_->getInts(x1, z1, w, h);
        for (int32_t i = 0; i < w * h; ++i) {
            bool found = false;
            for (int32_t a : allowed) {
                if (ids[i] == a) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    // Java: findBiomePosition — find random matching biome in radius
    struct BiomePosition {
        int32_t x, z;
        bool found;
    };

    BiomePosition findBiomePosition(int32_t x, int32_t z, int32_t radius,
                                       const std::vector<int32_t>& allowed, int64_t rngSeed) {
        int32_t x1 = (x - radius) >> 2;
        int32_t z1 = (z - radius) >> 2;
        int32_t x2 = (x + radius) >> 2;
        int32_t z2 = (z + radius) >> 2;
        int32_t w = x2 - x1 + 1;
        int32_t h = z2 - z1 + 1;
        auto ids = genBiomes_->getInts(x1, z1, w, h);

        BiomePosition result{0, 0, false};
        int32_t count = 0;

        // Simple LCG for random selection
        auto nextIntRng = [&rngSeed](int32_t bound) -> int32_t {
            rngSeed = (rngSeed * 0x5DEECE66DALL + 0xBLL) & ((1LL << 48) - 1);
            return static_cast<int32_t>((rngSeed >> 17) % bound);
        };

        for (int32_t i = 0; i < w * h; ++i) {
            int32_t bx = (x1 + i % w) << 2;
            int32_t bz = (z1 + i / w) << 2;
            bool match = false;
            for (int32_t a : allowed) {
                if (ids[i] == a) { match = true; break; }
            }
            if (!match) continue;
            if (!result.found || nextIntRng(count + 1) == 0) {
                result = {bx, bz, true};
            }
            ++count;
        }
        return result;
    }

    const std::vector<int32_t>& getBiomesToSpawnIn() const { return biomesToSpawnIn_; }

    void cleanupCache() { cache_.cleanup(); }

private:
    int64_t worldSeed_ = 0;
    std::vector<int32_t> biomesToSpawnIn_;

    // GenLayer stack
    std::unique_ptr<GenLayerIsland> islandLayer_;
    std::unique_ptr<GenLayerBiome> biomeLayer_;
    std::vector<std::unique_ptr<GenLayerZoom>> zoomLayers_;
    GenLayer* genBiomes_ = nullptr;
    GenLayer* biomeIndexLayer_ = nullptr;

    BiomeCache cache_;

    // Simplified rainfall lookup
    static float getRainfallForBiome(int32_t biomeId) {
        // Key biomes with known rainfall values
        switch (biomeId) {
            case 0: return 0.5f;   // ocean
            case 1: return 0.4f;   // plains
            case 2: return 0.0f;   // desert
            case 3: return 0.3f;   // extreme hills
            case 4: return 0.8f;   // forest
            case 5: return 0.8f;   // taiga
            case 6: return 0.9f;   // swampland
            case 7: return 0.5f;   // river
            case 12: return 0.5f;  // ice plains
            case 14: return 1.0f;  // mushroom island
            case 21: return 0.9f;  // jungle
            case 30: return 0.4f;  // cold taiga
            case 35: return 0.0f;  // savanna
            case 37: return 0.0f;  // mesa
            default: return 0.5f;
        }
    }
};

} // namespace mccpp
