/**
 * WorldChunkManager.h — Biome query layer for world generation.
 *
 * Java reference: net.minecraft.world.biome.WorldChunkManager
 *
 * Provides biome IDs for terrain generation and decoration:
 *   - getBiomesForGeneration: 4×4 biome grid per chunk (for height blending)
 *   - getBiomeData: 16×16 per-block biomes (Voronoi-zoomed, for surface/snow)
 */
#pragma once

#include <cstdint>
#include <vector>
#include "worldgen/GenLayer.h"
#include "world/BiomeGenBase.h"

namespace mccpp {

class WorldChunkManager {
public:
    GenLayerStack layerStack;

    WorldChunkManager() = default;

    void init(int64_t worldSeed) {
        layerStack.initializeAllBiomeGenerators(worldSeed);
        BiomeGenRegistry::init();
    }

    // ── For height blending: 10×10 grid at 1:4 resolution ──
    // Java: WorldChunkManager.getBiomesForGeneration
    // Returns biome IDs for a region at 1:4 scale (each value covers 4×4 blocks)
    // Used during terrain density computation for biome height blending.
    // areaX, areaZ are in biome coords (blockX >> 2, blockZ >> 2).
    std::vector<int32_t> getBiomesForGeneration(int32_t areaX, int32_t areaZ,
                                                 int32_t w, int32_t h) {
        return layerStack.biomeLayer->getInts(areaX, areaZ, w, h);
    }

    // ── For per-block biomes: 16×16 per chunk ──
    // Java: WorldChunkManager.getBiomeGenAt (full resolution)
    // Uses voronoi zoom for block-level biome assignment.
    // blockX, blockZ are world block coordinates.
    std::vector<int32_t> getBiomeData(int32_t blockX, int32_t blockZ,
                                      int32_t w, int32_t h) {
        return layerStack.voronoiLayer->getInts(blockX, blockZ, w, h);
    }

    // ── Single-block biome query ──
    int32_t getBiomeAt(int32_t blockX, int32_t blockZ) {
        auto data = getBiomeData(blockX, blockZ, 1, 1);
        return data.empty() ? 0 : data[0];
    }

    // ── Get biome data object ──
    const BiomeGenBase* getBiomeGenAt(int32_t blockX, int32_t blockZ) {
        return BiomeGenRegistry::getBiome(getBiomeAt(blockX, blockZ));
    }
};

} // namespace mccpp
