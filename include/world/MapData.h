/**
 * MapData.h — In-game map item data, exploration tracking, and map markers.
 *
 * Java references:
 *   - net.minecraft.world.storage.MapData (169 lines)
 *   - net.minecraft.world.storage.MapData$MapInfo (84 lines)
 *   - net.minecraft.world.storage.MapData$MapCoord (24 lines)
 *
 * Map mechanics:
 *   - 128x128 pixel color array (16384 bytes)
 *   - Scale: 0-4 (blocks per pixel = 1<<scale, i.e., 1, 2, 4, 8, 16)
 *   - Dimension: 0 overworld, -1 nether, 1 end
 *   - Nether: pointer spins (worldTime/10, LCG hash for rotation)
 *   - Map markers: per-player and item frame positions
 *   - Dirty column tracking: field_76209_b[128] (min row), field_76210_c[128] (max row)
 *   - Update packet: type 0 = column data, type 1 = player positions, type 2 = scale info
 *   - Player location update interval: every 4 ticks (ticksUntilPlayerLocationMapUpdate)
 *   - Column scan: currentRandomNumber * 11 % 128 (one column per update tick)
 *
 * Thread safety: MapData accessed on main world thread; MapInfo per-player.
 * JNI readiness: Simple byte arrays and POD structs.
 */
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// MapCoord — Map marker icon (player dot or frame pointer).
// Java: MapData$MapCoord
//
//   iconSize: marker type (0=player, 1=frame, 6=off-edge)
//   centerX, centerZ: pixel position on 128x128 map (-128..127)
//   iconRotation: 0-15 (22.5° increments)
// ═══════════════════════════════════════════════════════════════════════════

struct MapCoord {
    int8_t iconSize = 0;
    int8_t centerX = 0;
    int8_t centerZ = 0;
    int8_t iconRotation = 0;

    MapCoord() = default;
    MapCoord(int8_t type, int8_t x, int8_t z, int8_t rot)
        : iconSize(type), centerX(x), centerZ(z), iconRotation(rot) {}
};

// ═══════════════════════════════════════════════════════════════════════════
// MapInfo — Per-player map update tracking.
// Java: MapData$MapInfo (84 lines)
//
//   Dirty column tracking:
//     field_76209_b[128]: min dirty row per column (init 0)
//     field_76210_c[128]: max dirty row per column (init 127)
//     After sending column, both set to -1 (clean)
//
//   Update packet format:
//     First send: [2, scale] (type 2 = scale info)
//     Every 4 ticks: player location packet [1, n*3 entries of icon data]
//     Column updates: [0, columnIndex, startRow, ...colors] (one per tick)
//       Column scan: (currentRandomNumber * 11) % 128
//
//   Player location comparison:
//     lastPlayerLocationOnMap cached and compared for change detection
//     Non-frame maps always resend; frame maps check for actual changes
// ═══════════════════════════════════════════════════════════════════════════

struct MapInfo {
    // ─── Dirty column tracking ───
    std::array<int32_t, 128> minDirtyRow;   // field_76209_b
    std::array<int32_t, 128> maxDirtyRow;   // field_76210_c

    int32_t currentRandomNumber = 0;
    int32_t ticksUntilPlayerLocationUpdate = 0;
    std::vector<uint8_t> lastPlayerLocationData;
    int32_t field_82569_d = 0; // used for something in 1.7.10
    bool hasReceivedScale = false; // field_82570_i

    MapInfo() {
        for (int i = 0; i < 128; ++i) {
            minDirtyRow[i] = 0;
            maxDirtyRow[i] = 127;
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // getPlayersOnMap — Generate update packet data for this player.
    // Java: MapData$MapInfo.getPlayersOnMap(ItemStack)
    //
    // Returns: byte array for S34PacketMaps, or empty if no update
    //
    // Logic:
    //   1. First call: return [2, scale]
    //   2. Every 4 ticks: return player/frame marker positions
    //   3. Otherwise: send one dirty column (cycled via *11 % 128)
    // ═══════════════════════════════════════════════════════════════
    std::vector<uint8_t> getUpdatePacketData(
        int8_t scale,
        const std::array<uint8_t, 16384>& colors,
        const std::vector<MapCoord>& visibleMarkers,
        bool isOnItemFrame
    ) {
        // First send: scale info
        if (!hasReceivedScale) {
            hasReceivedScale = true;
            return {2, static_cast<uint8_t>(scale)};
        }

        // Player location update every 4 ticks
        if (--ticksUntilPlayerLocationUpdate < 0) {
            ticksUntilPlayerLocationUpdate = 4;

            std::vector<uint8_t> data;
            data.resize(visibleMarkers.size() * 3 + 1);
            data[0] = 1;
            for (size_t i = 0; i < visibleMarkers.size(); ++i) {
                const auto& coord = visibleMarkers[i];
                data[i * 3 + 1] = static_cast<uint8_t>(
                    (coord.iconSize << 4) | (coord.iconRotation & 0xF)
                );
                data[i * 3 + 2] = static_cast<uint8_t>(coord.centerX);
                data[i * 3 + 3] = static_cast<uint8_t>(coord.centerZ);
            }

            // Check if data changed (non-frame maps always resend)
            int shouldSkip = isOnItemFrame ? 0 : 1;
            if (lastPlayerLocationData.size() != data.size()) {
                shouldSkip = 0;
            } else if (shouldSkip != 0) {
                for (size_t i = 0; i < data.size(); ++i) {
                    if (data[i] != lastPlayerLocationData[i]) {
                        shouldSkip = 0;
                        break;
                    }
                }
            }

            if (shouldSkip == 0) {
                lastPlayerLocationData = data;
                return data;
            }
        }

        // Column scan: one dirty column per update
        for (int i = 0; i < 1; ++i) {
            int32_t col = (currentRandomNumber++ * 11) % 128;
            if (minDirtyRow[col] < 0) continue;

            int32_t height = maxDirtyRow[col] - minDirtyRow[col] + 1;
            int32_t startRow = minDirtyRow[col];

            std::vector<uint8_t> data;
            data.resize(height + 3);
            data[0] = 0;
            data[1] = static_cast<uint8_t>(col);
            data[2] = static_cast<uint8_t>(startRow);

            for (int32_t j = 0; j < height; ++j) {
                // Java: colors[(j + startRow) * 128 + col]
                data[j + 3] = colors[(j + startRow) * 128 + col];
            }

            maxDirtyRow[col] = -1;
            minDirtyRow[col] = -1;
            return data;
        }

        return {};
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// MapData — Complete map state for a single in-game map item.
// Java: MapData extends WorldSavedData (169 lines)
//
//   Core state:
//     xCenter, zCenter: world block coordinates of map center
//     dimension: 0=overworld, -1=nether, 1=end
//     scale: 0-4 (blocks per pixel = 1<<scale)
//     colors[16384]: 128x128 map color indices
//
//   NBT format:
//     dimension (byte), xCenter (int), zCenter (int), scale (byte)
//     width (short=128), height (short=128), colors (byte[16384])
//     Legacy: rescale from non-128x128 data (centered)
//
//   Player tracking:
//     playersVisibleOnMap: ordered map of name→MapCoord
//     playersHashMap: map of player→MapInfo
//     Per-player dirty tracking via MapInfo columns
//
//   Marker calculation (func_82567_a):
//     Position: (worldPos - center) / (1<<scale) * 2 + 0.5
//     Rotation: (yaw ± 8) * 16 / 360
//     In-bounds: -63..63 pixel range → normal marker (type 0 or 1)
//     Near edge: |pos| < 320 → edge marker (type 6) clamped
//     Far away: removed from playersVisibleOnMap
//     Nether: rotation = (worldTime/10)² * 34187121 + t*121 >> 15 & 0xF
// ═══════════════════════════════════════════════════════════════════════════

class MapData {
public:
    static constexpr int32_t MAP_SIZE = 128;
    static constexpr int32_t MAP_COLORS = MAP_SIZE * MAP_SIZE; // 16384
    static constexpr int32_t MAX_SCALE = 4;
    static constexpr int32_t MIN_SCALE = 0;
    static constexpr int32_t MARKER_RANGE = 63;
    static constexpr float MARKER_FAR_RANGE = 320.0f;

    // ─── Core map state ───
    int32_t xCenter = 0;
    int32_t zCenter = 0;
    int8_t dimension = 0;
    int8_t scale = 0; // 0-4, blocks per pixel = 1 << scale
    std::array<uint8_t, MAP_COLORS> colors = {};

    // ─── Player tracking ───
    // Ordered map preserving insertion order for marker display
    std::vector<std::pair<std::string, MapCoord>> playersVisibleOnMap;
    std::unordered_map<std::string, MapInfo> playerInfoMap;

    std::string mapName; // "map_N"
    bool dirty = false;

    MapData() = default;
    explicit MapData(const std::string& name) : mapName(name) {}

    // ═══════════════════════════════════════════════════════════════
    // NBT Read/Write
    // Java: MapData.readFromNBT / writeToNBT
    //
    // Read handles legacy non-128x128 maps by centering the data.
    // Scale is clamped to [0, 4].
    // ═══════════════════════════════════════════════════════════════

    struct NbtData {
        int8_t dimension;
        int32_t xCenter, zCenter;
        int8_t scale;
        int16_t width, height;
        std::vector<uint8_t> colorData;
    };

    void readFromNBT(const NbtData& nbt) {
        dimension = nbt.dimension;
        xCenter = nbt.xCenter;
        zCenter = nbt.zCenter;
        scale = nbt.scale;
        if (scale < 0) scale = 0;
        if (scale > 4) scale = 4;

        if (nbt.width == 128 && nbt.height == 128 &&
            nbt.colorData.size() == MAP_COLORS) {
            std::copy(nbt.colorData.begin(), nbt.colorData.end(), colors.begin());
        } else {
            // Legacy: center smaller/larger map data
            colors.fill(0);
            int32_t offsetX = (128 - nbt.width) / 2;
            int32_t offsetZ = (128 - nbt.height) / 2;
            for (int32_t z = 0; z < nbt.height; ++z) {
                int32_t destZ = z + offsetZ;
                if (destZ < 0 || destZ >= 128) continue;
                for (int32_t x = 0; x < nbt.width; ++x) {
                    int32_t destX = x + offsetX;
                    if (destX < 0 || destX >= 128) continue;
                    size_t srcIdx = static_cast<size_t>(x + z * nbt.width);
                    if (srcIdx < nbt.colorData.size()) {
                        colors[destX + destZ * 128] = nbt.colorData[srcIdx];
                    }
                }
            }
        }
    }

    NbtData writeToNBT() const {
        NbtData nbt;
        nbt.dimension = dimension;
        nbt.xCenter = xCenter;
        nbt.zCenter = zCenter;
        nbt.scale = scale;
        nbt.width = 128;
        nbt.height = 128;
        nbt.colorData.assign(colors.begin(), colors.end());
        return nbt;
    }

    // ═══════════════════════════════════════════════════════════════
    // updateMarker — Add/update a map marker for player or frame.
    // Java: MapData.func_82567_a(int type, World world, String name,
    //                            double worldX, double worldZ, double yaw)
    //
    // Marker positioning:
    //   pixelX = (worldX - xCenter) / (1<<scale)
    //   iconX = (byte)(pixelX * 2 + 0.5)
    //
    // In-bounds [-63..63]: normal marker
    // Near-edge [|pixel| < 320]: edge marker (type=6, clamped)
    // Far away: removed from map
    //
    // Nether rotation: LCG hash of (worldTime/10)
    //   rot = ((t*t*34187121 + t*121) >> 15) & 0xF
    // ═══════════════════════════════════════════════════════════════

    void updateMarker(
        int32_t markerType,
        const std::string& name,
        double worldX, double worldZ, double yaw,
        int64_t worldTime,
        bool isNether
    ) {
        int32_t blocksPerPixel = 1 << scale;
        float pixelX = static_cast<float>(worldX - xCenter) /
                        static_cast<float>(blocksPerPixel);
        float pixelZ = static_cast<float>(worldZ - zCenter) /
                        static_cast<float>(blocksPerPixel);
        auto iconX = static_cast<int8_t>(static_cast<double>(pixelX * 2.0f) + 0.5);
        auto iconZ = static_cast<int8_t>(static_cast<double>(pixelZ * 2.0f) + 0.5);

        int8_t rotation;

        if (pixelX >= static_cast<float>(-MARKER_RANGE) &&
            pixelZ >= static_cast<float>(-MARKER_RANGE) &&
            pixelX <= static_cast<float>(MARKER_RANGE) &&
            pixelZ <= static_cast<float>(MARKER_RANGE))
        {
            // In bounds — normal marker
            double adjustedYaw = yaw + (yaw < 0.0 ? -8.0 : 8.0);
            rotation = static_cast<int8_t>(adjustedYaw * 16.0 / 360.0);

            if (isNether) {
                // Nether dimension: pointer spins based on worldTime
                int32_t t = static_cast<int32_t>(worldTime / 10L);
                rotation = static_cast<int8_t>(
                    (t * t * 34187121 + t * 121 >> 15) & 0xF
                );
            }
        } else if (std::abs(pixelX) < MARKER_FAR_RANGE &&
                   std::abs(pixelZ) < MARKER_FAR_RANGE)
        {
            // Near edge — edge marker (type 6)
            markerType = 6;
            rotation = 0;

            if (pixelX <= static_cast<float>(-MARKER_RANGE)) {
                iconX = static_cast<int8_t>(
                    static_cast<double>(MARKER_RANGE * 2) + 2.5
                );
            }
            if (pixelZ <= static_cast<float>(-MARKER_RANGE)) {
                iconZ = static_cast<int8_t>(
                    static_cast<double>(MARKER_RANGE * 2) + 2.5
                );
            }
            if (pixelX >= static_cast<float>(MARKER_RANGE)) {
                iconX = static_cast<int8_t>(MARKER_RANGE * 2 + 1);
            }
            if (pixelZ >= static_cast<float>(MARKER_RANGE)) {
                iconZ = static_cast<int8_t>(MARKER_RANGE * 2 + 1);
            }
        } else {
            // Far away — remove marker
            removeMarker(name);
            return;
        }

        setMarker(name, MapCoord(
            static_cast<int8_t>(markerType), iconX, iconZ, rotation
        ));
    }

    // ═══════════════════════════════════════════════════════════════
    // setColumnDirty — Mark a column region as needing retransmission.
    // Java: MapData.setColumnDirty(int column, int minRow, int maxRow)
    //
    // Updates all MapInfo instances with the dirty range.
    // Expands existing dirty ranges (union of old and new).
    // ═══════════════════════════════════════════════════════════════

    void setColumnDirty(int32_t column, int32_t minRow, int32_t maxRow) {
        dirty = true;
        for (auto& [name, info] : playerInfoMap) {
            if (info.minDirtyRow[column] < 0 ||
                info.minDirtyRow[column] > minRow) {
                info.minDirtyRow[column] = minRow;
            }
            if (info.maxDirtyRow[column] < 0 ||
                info.maxDirtyRow[column] < maxRow) {
                info.maxDirtyRow[column] = maxRow;
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // getUpdateData — Get update packet data for a specific player.
    // Java: MapData.getUpdatePacketData / MapInfo.getPlayersOnMap
    // ═══════════════════════════════════════════════════════════════

    std::vector<uint8_t> getUpdateData(
        const std::string& playerName,
        bool isOnItemFrame
    ) {
        auto it = playerInfoMap.find(playerName);
        if (it == playerInfoMap.end()) return {};

        std::vector<MapCoord> markers;
        for (const auto& [name, coord] : playersVisibleOnMap) {
            markers.push_back(coord);
        }

        return it->second.getUpdatePacketData(
            scale, colors, markers, isOnItemFrame
        );
    }

    // ═══════════════════════════════════════════════════════════════
    // ensurePlayerTracked — Add a MapInfo for player if not present.
    // Java: MapData.func_82568_a(EntityPlayer)
    // ═══════════════════════════════════════════════════════════════

    MapInfo& ensurePlayerTracked(const std::string& playerName) {
        auto [it, inserted] = playerInfoMap.try_emplace(playerName);
        return it->second;
    }

private:
    void setMarker(const std::string& name, const MapCoord& coord) {
        // Ordered insertion preserving (LinkedHashMap behavior)
        for (auto& [n, c] : playersVisibleOnMap) {
            if (n == name) { c = coord; return; }
        }
        playersVisibleOnMap.emplace_back(name, coord);
    }

    void removeMarker(const std::string& name) {
        playersVisibleOnMap.erase(
            std::remove_if(playersVisibleOnMap.begin(),
                           playersVisibleOnMap.end(),
                           [&](const auto& p) { return p.first == name; }),
            playersVisibleOnMap.end()
        );
    }
};

} // namespace mccpp
