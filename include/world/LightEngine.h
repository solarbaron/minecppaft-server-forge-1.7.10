#pragma once
// LightEngine — block light and sky light propagation.
// Ported from aqo.java (Chunk light computation) and related classes.
//
// Vanilla lighting:
//   - Block light: emitted by torches (14), glowstone (15), lava (15), etc.
//   - Sky light: propagated from top of world downward, attenuated by blocks
//   - Both stored as nibble arrays in chunk sections (4 bits per block, 0-15)
//   - Light spreads using BFS flood fill, decreasing by 1 per block
//   - Opaque blocks fully stop light, transparent blocks reduce by 1
//
// Block IDs and their light emission values — from aji.java (Block)

#include <cstdint>
#include <queue>
#include <array>
#include <functional>
#include <algorithm>

#include "world/Block.h" // getBlockLightEmission, getBlockOpacity

namespace mc {

struct LightPos {
    int x, y, z;
    uint8_t level;
};

class LightEngine {
public:
    using GetBlockFn = std::function<uint16_t(int,int,int)>;
    using GetLightFn = std::function<uint8_t(int,int,int)>;
    using SetLightFn = std::function<void(int,int,int,uint8_t)>;

    // Compute initial block light for a chunk column (called during generation)
    static void computeBlockLight(int cx, int cz,
                                   GetBlockFn getBlock,
                                   GetLightFn getLight,
                                   SetLightFn setLight) {
        // Phase 1: seed light sources
        std::queue<LightPos> queue;

        for (int bx = 0; bx < 16; ++bx) {
            for (int bz = 0; bz < 16; ++bz) {
                for (int by = 0; by < 256; ++by) {
                    int wx = cx * 16 + bx, wz = cz * 16 + bz;
                    uint16_t bid = getBlock(wx, by, wz);
                    uint8_t emission = getBlockLightEmission(bid);
                    if (emission > 0) {
                        setLight(wx, by, wz, emission);
                        queue.push({wx, by, wz, emission});
                    }
                }
            }
        }

        // Phase 2: BFS propagation
        propagate(queue, getBlock, getLight, setLight);
    }

    // Compute sky light for a chunk column
    static void computeSkyLight(int cx, int cz,
                                 GetBlockFn getBlock,
                                 GetLightFn getSkyLight,
                                 SetLightFn setSkyLight) {
        std::queue<LightPos> queue;

        // Phase 1: top-down propagation
        for (int bx = 0; bx < 16; ++bx) {
            for (int bz = 0; bz < 16; ++bz) {
                int wx = cx * 16 + bx, wz = cz * 16 + bz;
                uint8_t skyLevel = 15;

                for (int by = 255; by >= 0; --by) {
                    uint16_t bid = getBlock(wx, by, wz);
                    uint8_t opacity = getBlockOpacity(bid);

                    if (opacity > 0) {
                        skyLevel = static_cast<uint8_t>(
                            std::max(0, static_cast<int>(skyLevel) - opacity));
                    }

                    setSkyLight(wx, by, wz, skyLevel);

                    if (skyLevel > 1) {
                        queue.push({wx, by, wz, skyLevel});
                    }
                }
            }
        }

        // Phase 2: horizontal spread
        propagate(queue, getBlock, getSkyLight, setSkyLight);
    }

    // Update lighting when a block is placed/broken
    // Returns affected positions for client update
    static void updateBlockLight(int x, int y, int z,
                                  GetBlockFn getBlock,
                                  GetLightFn getLight,
                                  SetLightFn setLight) {
        uint16_t bid = getBlock(x, y, z);
        uint8_t emission = getBlockLightEmission(bid);

        // Recalculate at this position
        uint8_t maxNeighbor = 0;
        static constexpr int dx[] = {1,-1,0,0,0,0};
        static constexpr int dy[] = {0,0,1,-1,0,0};
        static constexpr int dz[] = {0,0,0,0,1,-1};

        for (int i = 0; i < 6; ++i) {
            uint8_t nl = getLight(x + dx[i], y + dy[i], z + dz[i]);
            if (nl > maxNeighbor) maxNeighbor = nl;
        }

        uint8_t opacity = getBlockOpacity(bid);
        uint8_t newLevel = std::max(emission,
            static_cast<uint8_t>(maxNeighbor > 0 ? std::max(0, maxNeighbor - 1 - opacity) : 0));

        setLight(x, y, z, newLevel);

        // Re-propagate from this point
        if (newLevel > 0) {
            std::queue<LightPos> queue;
            queue.push({x, y, z, newLevel});
            propagate(queue, getBlock, getLight, setLight);
        }

        // Also re-propagate darkness if we placed an opaque block
        if (opacity > 0) {
            // Simple: re-check neighbors
            for (int i = 0; i < 6; ++i) {
                int nx = x + dx[i], ny = y + dy[i], nz = z + dz[i];
                if (ny < 0 || ny > 255) continue;
                uint8_t currentLight = getLight(nx, ny, nz);
                if (currentLight > 0) {
                    std::queue<LightPos> q;
                    q.push({nx, ny, nz, currentLight});
                    propagate(q, getBlock, getLight, setLight);
                }
            }
        }
    }

private:
    // BFS light propagation
    static void propagate(std::queue<LightPos>& queue,
                           GetBlockFn getBlock,
                           GetLightFn getLight,
                           SetLightFn setLight) {
        static constexpr int dx[] = {1,-1,0,0,0,0};
        static constexpr int dy[] = {0,0,1,-1,0,0};
        static constexpr int dz[] = {0,0,0,0,1,-1};

        while (!queue.empty()) {
            auto [px, py, pz, level] = queue.front();
            queue.pop();

            if (level <= 1) continue;

            for (int i = 0; i < 6; ++i) {
                int nx = px + dx[i];
                int ny = py + dy[i];
                int nz = pz + dz[i];

                if (ny < 0 || ny > 255) continue;

                uint16_t nBid = getBlock(nx, ny, nz);
                uint8_t opacity = getBlockOpacity(nBid);
                uint8_t newLevel = static_cast<uint8_t>(
                    std::max(0, static_cast<int>(level) - 1 - opacity));

                if (newLevel > getLight(nx, ny, nz)) {
                    setLight(nx, ny, nz, newLevel);
                    queue.push({nx, ny, nz, newLevel});
                }
            }
        }
    }
};

} // namespace mc
