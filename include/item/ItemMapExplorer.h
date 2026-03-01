/**
 * ItemMapExplorer.h — Map exploration algorithm for in-game map items.
 *
 * Java references:
 *   - net.minecraft.item.ItemMap (196 lines)
 *   - net.minecraft.block.material.MapColor (62 lines)
 *
 * Map exploration mechanics:
 *   - Active only when held in main hand (isHeld = true parameter)
 *   - Scans columns with (column & 0xF) == (updateTick & 0xF) striding
 *   - Scan radius: 128 / blocksPerPixel (halved in Nether)
 *   - Per-column: samples NxN block area (N = 1<<scale)
 *   - Height sampling: chunk.getHeightValue + search down for non-air MapColor
 *   - Water depth counting: blocks below liquid surface
 *   - Color selection: most common MapColor via multiset voting
 *   - Brightness: 3 levels (0=dark, 1=normal, 2=bright)
 *     Overworld: based on height gradient + checkerboard dither
 *     Water: based on depth + checkerboard dither
 *   - Final color byte: mapColor.colorIndex * 4 + brightness
 *   - Nether: LCG hash (x+z*231871)^2*31287121+n*11 for dirt/stone pattern
 *   - Map scaling: onCreated with map_is_scaling NBT tag
 *
 * Thread safety: Called on entity tick thread.
 * JNI readiness: Pure POD computation, no complex C++ types.
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// MapColor — Block map color indices (64 base colors).
// Java: net.minecraft.block.material.MapColor
//
//   Each block has a MapColor with a colorIndex (0-63).
//   Map pixel = colorIndex * 4 + brightness (0-2).
//   Color 0 = airColor (transparent/unused).
//   Notable: waterColor = index 12
// ═══════════════════════════════════════════════════════════════════════════

struct MapColor {
    static constexpr int32_t AIR = 0;
    static constexpr int32_t GRASS = 1;
    static constexpr int32_t SAND = 2;
    static constexpr int32_t CLOTH = 3;
    static constexpr int32_t TNT = 4;
    static constexpr int32_t ICE = 5;
    static constexpr int32_t IRON = 6;
    static constexpr int32_t FOLIAGE = 7;
    static constexpr int32_t SNOW = 8;
    static constexpr int32_t CLAY = 9;
    static constexpr int32_t DIRT = 10;
    static constexpr int32_t STONE = 11;
    static constexpr int32_t WATER = 12;
    static constexpr int32_t WOOD = 13;
    static constexpr int32_t QUARTZ = 14;
    static constexpr int32_t ADOBE = 15;
    static constexpr int32_t MAGENTA = 16;
    static constexpr int32_t LIGHT_BLUE = 17;
    static constexpr int32_t YELLOW = 18;
    static constexpr int32_t LIME = 19;
    static constexpr int32_t PINK = 20;
    static constexpr int32_t GRAY = 21;
    static constexpr int32_t SILVER = 22;
    static constexpr int32_t CYAN = 23;
    static constexpr int32_t PURPLE = 24;
    static constexpr int32_t BLUE = 25;
    static constexpr int32_t BROWN = 26;
    static constexpr int32_t GREEN = 27;
    static constexpr int32_t RED = 28;
    static constexpr int32_t BLACK = 29;
    static constexpr int32_t GOLD = 30;
    static constexpr int32_t DIAMOND = 31;
    static constexpr int32_t LAPIS = 32;
    static constexpr int32_t EMERALD = 33;
    static constexpr int32_t OBSIDIAN = 34;
    static constexpr int32_t NETHERRACK = 35;

    // Java MapColor RGB values (base colors for brightness level 1)
    // These are the actual 32-bit ARGB values from the Java source
    static constexpr std::array<uint32_t, 36> BASE_COLORS = {{
        0x00000000, // 0  air (transparent)
        0xFF7FB238, // 1  grass
        0xFFF7E9A3, // 2  sand
        0xFFA7A7A7, // 3  cloth/wool
        0xFFFF0000, // 4  tnt/fire
        0xFFA0A0FF, // 5  ice
        0xFFA7A7A7, // 6  iron
        0xFF007C00, // 7  foliage
        0xFFFFFFFF, // 8  snow
        0xFFA4A8B8, // 9  clay
        0xFF976D4D, // 10 dirt
        0xFF707070, // 11 stone
        0xFF4040FF, // 12 water
        0xFF8F7748, // 13 wood
        0xFFFFFCF5, // 14 quartz
        0xFFD87F33, // 15 adobe/orange
        0xFFB24CD8, // 16 magenta
        0xFF6699D8, // 17 light blue
        0xFFE5E533, // 18 yellow
        0xFF7FCC19, // 19 lime
        0xFFF27FA5, // 20 pink
        0xFF4C4C4C, // 21 gray
        0xFF999999, // 22 silver
        0xFF4C7F99, // 23 cyan
        0xFF7F3FB2, // 24 purple
        0xFF334CB2, // 25 blue
        0xFF664C33, // 26 brown
        0xFF667F33, // 27 green
        0xFF993333, // 28 red
        0xFF191919, // 29 black
        0xFFFAEE4D, // 30 gold
        0xFF5CDBD5, // 31 diamond
        0xFF4A80FF, // 32 lapis
        0xFF00D93A, // 33 emerald
        0xFF815631, // 34 obsidian (podzol in 1.7)
        0xFF700200, // 35 netherrack
    }};

    // Get brightness-modified color (brightness 0=dark, 1=normal, 2=bright)
    // Java: brightness multipliers are 180/255, 220/255, 255/255 for 0,1,2
    // The actual modifier applied: color * (220 - brightness*40 + brightness*35) / 255
    // Simplified: level 0 = 180/255, level 1 = 220/255, level 2 = 255/255
    static uint32_t getBrightnessColor(int32_t colorIndex, int32_t brightness) {
        if (colorIndex <= 0 || colorIndex >= static_cast<int32_t>(BASE_COLORS.size()))
            return 0;
        uint32_t base = BASE_COLORS[colorIndex];
        int32_t multiplier;
        switch (brightness) {
            case 0: multiplier = 180; break;
            case 2: multiplier = 255; break;
            default: multiplier = 220; break;
        }
        uint8_t r = static_cast<uint8_t>(((base >> 16) & 0xFF) * multiplier / 255);
        uint8_t g = static_cast<uint8_t>(((base >> 8) & 0xFF) * multiplier / 255);
        uint8_t b = static_cast<uint8_t>((base & 0xFF) * multiplier / 255);
        return 0xFF000000u | (r << 16) | (g << 8) | b;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// ItemMapExplorer — Map exploration algorithm.
// Java: ItemMap.updateMapData(World, Entity, MapData) (196 lines)
//
// Algorithm overview per-tick:
//   1. Calculate scan radius: 128 / (1<<scale), halved for hasNoSky
//   2. Player pixel position: floor((posX - center) / scale) + 64
//   3. Column stride: only process column if (col & 0xF) == (tick & 0xF)
//   4. For each column, scan all rows in radius
//   5. Per pixel: sample NxN block heightmap area
//   6. Count MapColors via multiset, pick highest count
//   7. Calculate brightness from height gradient
//   8. Write color byte = colorIndex * 4 + brightness
//   9. Track dirty columns for MapInfo update
// ═══════════════════════════════════════════════════════════════════════════

class ItemMapExplorer {
public:
    // ─── Block color query interface ───
    // Caller provides these callbacks to query world state
    struct WorldQuery {
        // Get the MapColor index for block at (x, y, z) with metadata
        int32_t (*getBlockMapColor)(int32_t x, int32_t y, int32_t z, void* ctx);
        // Get height value at (x, z) from chunk
        int32_t (*getHeightValue)(int32_t x, int32_t z, void* ctx);
        // Check if block at (x, y, z) is liquid
        bool (*isLiquid)(int32_t x, int32_t y, int32_t z, void* ctx);
        // Check if chunk at block coords is loaded
        bool (*isChunkLoaded)(int32_t x, int32_t z, void* ctx);
        void* context;
    };

    // ═══════════════════════════════════════════════════════════════
    // updateMapData — Core map exploration algorithm.
    //
    // Parameters:
    //   mapColors: 128x128 color array to update
    //   entityX, entityZ: player position in world
    //   xCenter, zCenter: map center in world
    //   scale: 0-4
    //   hasNoSky: true for Nether
    //   updateTick: MapInfo.field_82569_d (incremented each call)
    //   query: world block query interface
    //
    // Returns: list of (column, minRow, maxRow) dirty regions
    // ═══════════════════════════════════════════════════════════════

    struct DirtyRegion {
        int32_t column;
        int32_t minRow;
        int32_t maxRow;
    };

    static std::vector<DirtyRegion> updateMapData(
        std::array<uint8_t, 16384>& mapColors,
        double entityX, double entityZ,
        int32_t xCenter, int32_t zCenter,
        int32_t scale,
        bool hasNoSky,
        int32_t updateTick,
        const WorldQuery& query
    ) {
        std::vector<DirtyRegion> dirtyRegions;

        int32_t blocksPerPixel = 1 << scale;
        int32_t pixelX = static_cast<int32_t>(
            std::floor(entityX - xCenter)) / blocksPerPixel + 64;
        int32_t pixelZ = static_cast<int32_t>(
            std::floor(entityZ - zCenter)) / blocksPerPixel + 64;

        int32_t scanRadius = 128 / blocksPerPixel;
        if (hasNoSky) {
            scanRadius /= 2;
        }

        for (int32_t col = pixelX - scanRadius + 1; col < pixelX + scanRadius; ++col) {
            // Column stride: only process if (col & 0xF) == (tick & 0xF)
            if ((col & 0xF) != (updateTick & 0xF)) continue;

            int32_t minDirty = 255;
            int32_t maxDirty = 0;
            double prevAvgHeight = 0.0;

            for (int32_t row = pixelZ - scanRadius - 1; row < pixelZ + scanRadius; ++row) {
                if (col < 0 || row < -1 || col >= 128 || row >= 128) continue;

                int32_t dx = col - pixelX;
                int32_t dz = row - pixelZ;
                bool isEdge = dx * dx + dz * dz > (scanRadius - 2) * (scanRadius - 2);

                // World block coordinates for this pixel
                int32_t worldX = (xCenter / blocksPerPixel + col - 64) * blocksPerPixel;
                int32_t worldZ = (zCenter / blocksPerPixel + row - 64) * blocksPerPixel;

                // Check chunk loaded
                if (!query.isChunkLoaded(worldX, worldZ, query.context)) continue;

                // Count MapColors via multiset voting
                std::unordered_map<int32_t, int32_t> colorCounts;
                int32_t waterDepthTotal = 0;
                double avgHeight = 0.0;

                if (hasNoSky) {
                    // Nether: LCG hash for dirt/stone pattern
                    // Java: n = worldX + worldZ * 231871
                    //   (n*n*31287121 + n*11) >> 20 & 1
                    int32_t n = worldX + worldZ * 231871;
                    n = n * n * 31287121 + n * 11;
                    if (((n >> 20) & 1) == 0) {
                        colorCounts[MapColor::DIRT] += 10;
                    } else {
                        colorCounts[MapColor::STONE] += 100;
                    }
                    avgHeight = 100.0;
                } else {
                    // Overworld: sample NxN block area
                    int32_t chunkLocalX = worldX & 0xF;
                    int32_t chunkLocalZ = worldZ & 0xF;

                    for (int32_t bx = 0; bx < blocksPerPixel; ++bx) {
                        for (int32_t bz = 0; bz < blocksPerPixel; ++bz) {
                            int32_t sampleX = worldX + bx;
                            int32_t sampleZ = worldZ + bz;

                            int32_t height = query.getHeightValue(
                                sampleX, sampleZ, query.context) + 1;

                            int32_t blockColor = MapColor::AIR;
                            if (height > 1) {
                                // Search down for non-air block
                                do {
                                    --height;
                                    blockColor = query.getBlockMapColor(
                                        sampleX, height, sampleZ, query.context);
                                } while (blockColor == MapColor::AIR && height > 0);

                                // Count water depth if liquid
                                if (height > 0 && query.isLiquid(
                                    sampleX, height, sampleZ, query.context))
                                {
                                    int32_t depthY = height - 1;
                                    while (depthY > 0 && query.isLiquid(
                                        sampleX, depthY, sampleZ, query.context))
                                    {
                                        --depthY;
                                        ++waterDepthTotal;
                                    }
                                }
                            }

                            avgHeight += static_cast<double>(height) /
                                static_cast<double>(blocksPerPixel * blocksPerPixel);
                            colorCounts[blockColor]++;
                        }
                    }
                }

                // Water depth average
                waterDepthTotal /= (blocksPerPixel * blocksPerPixel);

                // Find most common MapColor (highest count first)
                int32_t bestColor = MapColor::AIR;
                int32_t bestCount = 0;
                for (const auto& [color, count] : colorCounts) {
                    if (count > bestCount) {
                        bestCount = count;
                        bestColor = color;
                    }
                }

                // Calculate brightness (0=dark, 1=normal, 2=bright)
                // Java: d3 = (avgHeight - prevAvgHeight) * 4.0 / (scale+4)
                //            + ((col+row & 1) - 0.5) * 0.4
                double heightDiff = (avgHeight - prevAvgHeight) * 4.0 /
                                    (static_cast<double>(blocksPerPixel) + 4.0) +
                                    (static_cast<double>((col + row) & 1) - 0.5) * 0.4;

                int32_t brightness = 1;
                if (heightDiff > 0.6) brightness = 2;
                if (heightDiff < -0.6) brightness = 0;

                // Water special case
                if (bestColor == MapColor::WATER) {
                    double waterShade = static_cast<double>(waterDepthTotal) * 0.1 +
                                        static_cast<double>((col + row) & 1) * 0.2;
                    brightness = 1;
                    if (waterShade < 0.5) brightness = 2;
                    if (waterShade > 0.9) brightness = 0;
                }

                prevAvgHeight = avgHeight;

                // Skip first row (row == -1, used only for height comparison)
                if (row < 0) continue;

                // Skip if outside circular radius
                if (dx * dx + dz * dz >= scanRadius * scanRadius) continue;

                // Edge checkerboard dithering
                if (isEdge && ((col + row) & 1) == 0) continue;

                // Final color byte
                auto newColor = static_cast<uint8_t>(bestColor * 4 + brightness);
                uint8_t& existingColor = mapColors[col + row * 128];

                if (existingColor != newColor) {
                    existingColor = newColor;
                    if (minDirty > row) minDirty = row;
                    if (maxDirty < row) maxDirty = row;
                }
            }

            if (minDirty <= maxDirty) {
                dirtyRegions.push_back({col, minDirty, maxDirty});
            }
        }

        return dirtyRegions;
    }

    // ═══════════════════════════════════════════════════════════════
    // createNewMap — Initialize a new map centered on spawn.
    // Java: ItemMap.getMapData when mapData == null
    //
    // Default scale = 3 (8 blocks per pixel)
    // Center rounded to nearest (128 * (1<<scale)) grid
    // ═══════════════════════════════════════════════════════════════

    struct MapInitData {
        int32_t xCenter;
        int32_t zCenter;
        int8_t scale;
        int8_t dimension;
    };

    static MapInitData createNewMap(int32_t spawnX, int32_t spawnZ,
                                    int32_t dimensionId)
    {
        MapInitData data;
        data.scale = 3;
        int32_t gridSize = 128 * (1 << data.scale); // 128 * 8 = 1024
        data.xCenter = static_cast<int32_t>(
            std::round(static_cast<float>(spawnX) / static_cast<float>(gridSize))
        ) * gridSize;
        data.zCenter = static_cast<int32_t>(
            std::round(static_cast<float>(spawnZ) / static_cast<float>(gridSize))
        ) * gridSize;
        data.dimension = static_cast<int8_t>(dimensionId);
        return data;
    }

    // ═══════════════════════════════════════════════════════════════
    // scaleMap — Create a zoomed-out copy (scale + 1, max 4).
    // Java: ItemMap.onCreated with map_is_scaling tag
    // ═══════════════════════════════════════════════════════════════

    static MapInitData scaleMap(int32_t xCenter, int32_t zCenter,
                                int8_t currentScale, int8_t dimension)
    {
        MapInitData data;
        data.scale = static_cast<int8_t>(
            std::min(static_cast<int32_t>(currentScale) + 1, 4)
        );
        data.xCenter = xCenter;
        data.zCenter = zCenter;
        data.dimension = dimension;
        return data;
    }
};

} // namespace mccpp
