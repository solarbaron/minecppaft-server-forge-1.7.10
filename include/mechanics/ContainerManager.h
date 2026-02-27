#pragma once
// ContainerManager — tile entity containers for chests, hoppers, dispensers, droppers.
// Ported from apu.java (TileEntityChest), apt.java (TileEntityHopper),
// aps.java (TileEntityDispenser), apr.java (TileEntityDropper).
//
// Vanilla container types:
//   - Chest (54): 27 slots, combines into double chest with adjacent chest
//   - Trapped Chest (146): same as chest but emits redstone when opened
//   - Hopper (154): 5 slots, transfers items into containers below/beside
//   - Dispenser (23): 9 slots, shoots items/projectiles when powered
//   - Dropper (158): 9 slots, drops items as entities when powered
//
// Block IDs:
//   54 = Chest, 146 = Trapped Chest
//   154 = Hopper
//   23 = Dispenser, 158 = Dropper

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include <tuple>

namespace mc {

// Simplified item slot for containers
struct ContainerSlot {
    int16_t itemId = -1;
    int8_t count = 0;
    int16_t meta = 0;

    bool isEmpty() const { return itemId <= 0 || count <= 0; }
    void clear() { itemId = -1; count = 0; meta = 0; }
};

// Container types
enum class ContainerType : uint8_t {
    CHEST         = 0,  // "minecraft:chest" — 27 slots
    TRAPPED_CHEST = 0,  // Same window type as chest
    HOPPER        = 5,  // "minecraft:hopper" — 5 slots
    DISPENSER     = 3,  // "minecraft:dispenser" — 9 slots (3x3)
    DROPPER       = 3,  // Same window type as dispenser
};

// Base container tile entity
struct ContainerTileEntity {
    int x = 0, y = 0, z = 0;
    uint16_t blockId = 0;
    std::string customName;
    std::vector<ContainerSlot> slots;

    ContainerTileEntity() = default;

    ContainerTileEntity(int bx, int by, int bz, uint16_t bid, int slotCount)
        : x(bx), y(by), z(bz), blockId(bid), slots(slotCount) {}

    int size() const { return static_cast<int>(slots.size()); }

    ContainerSlot& getSlot(int idx) { return slots[idx]; }
    const ContainerSlot& getSlot(int idx) const { return slots[idx]; }

    // Try to add an item to the container, returns leftover count
    int addItem(int16_t itemId, int8_t count, int16_t meta) {
        // First try to stack with existing items
        for (auto& slot : slots) {
            if (slot.itemId == itemId && slot.meta == meta && slot.count < 64) {
                int space = 64 - slot.count;
                int toAdd = std::min(static_cast<int>(count), space);
                slot.count += toAdd;
                count -= toAdd;
                if (count <= 0) return 0;
            }
        }
        // Then try empty slots
        for (auto& slot : slots) {
            if (slot.isEmpty()) {
                int toAdd = std::min(static_cast<int>(count), 64);
                slot.itemId = itemId;
                slot.count = static_cast<int8_t>(toAdd);
                slot.meta = meta;
                count -= toAdd;
                if (count <= 0) return 0;
            }
        }
        return count; // Leftover
    }

    // Remove one item from first non-empty slot, returns removed item
    ContainerSlot removeOne() {
        for (auto& slot : slots) {
            if (!slot.isEmpty()) {
                ContainerSlot result = slot;
                result.count = 1;
                slot.count--;
                if (slot.count <= 0) slot.clear();
                return result;
            }
        }
        return {}; // Empty
    }

    // Remove one item from a random non-empty slot
    ContainerSlot removeRandom(std::mt19937& rng) {
        std::vector<int> nonEmpty;
        for (int i = 0; i < size(); ++i) {
            if (!slots[i].isEmpty()) nonEmpty.push_back(i);
        }
        if (nonEmpty.empty()) return {};
        int idx = nonEmpty[rng() % nonEmpty.size()];
        ContainerSlot result = slots[idx];
        result.count = 1;
        slots[idx].count--;
        if (slots[idx].count <= 0) slots[idx].clear();
        return result;
    }

    bool isEmpty() const {
        for (auto& s : slots) if (!s.isEmpty()) return false;
        return true;
    }

    // Get the window type string for opening
    std::string windowType() const {
        switch (blockId) {
            case 54: case 146: return "minecraft:chest";
            case 154: return "minecraft:hopper";
            case 23: case 158: return "minecraft:dispenser";
            default: return "minecraft:container";
        }
    }

    // Get slot count for window
    int windowSlotCount() const {
        return size();
    }
};

// Hash for tile entity positions
struct ContainerPosHash {
    size_t operator()(const std::tuple<int,int,int>& p) const {
        auto h1 = std::hash<int>{}(std::get<0>(p));
        auto h2 = std::hash<int>{}(std::get<1>(p));
        auto h3 = std::hash<int>{}(std::get<2>(p));
        return h1 ^ (h2 << 11) ^ (h3 << 22);
    }
};

// Manages all container tile entities in the world
class ContainerManager {
public:
    // Create a container at position
    ContainerTileEntity& createContainer(int x, int y, int z, uint16_t blockId) {
        auto key = std::make_tuple(x, y, z);
        int slotCount;
        switch (blockId) {
            case 54: case 146: slotCount = 27; break; // Chest
            case 154: slotCount = 5; break;            // Hopper
            case 23: case 158: slotCount = 9; break;   // Dispenser/Dropper
            default: slotCount = 27; break;
        }
        containers_[key] = ContainerTileEntity(x, y, z, blockId, slotCount);
        return containers_[key];
    }

    // Remove container at position
    void removeContainer(int x, int y, int z) {
        containers_.erase(std::make_tuple(x, y, z));
    }

    // Get container at position (nullptr if none)
    ContainerTileEntity* getContainer(int x, int y, int z) {
        auto it = containers_.find(std::make_tuple(x, y, z));
        return it != containers_.end() ? &it->second : nullptr;
    }

    const ContainerTileEntity* getContainer(int x, int y, int z) const {
        auto it = containers_.find(std::make_tuple(x, y, z));
        return it != containers_.end() ? &it->second : nullptr;
    }

    // Check if position is a double chest (adjacent chest in same direction)
    bool isDoubleChest(int x, int y, int z,
                       std::function<uint16_t(int,int,int)> getBlock) const {
        uint16_t bid = getBlock(x, y, z);
        if (bid != 54 && bid != 146) return false;
        // Check 4 cardinal directions for matching chest
        return (getBlock(x-1,y,z) == bid || getBlock(x+1,y,z) == bid ||
                getBlock(x,y,z-1) == bid || getBlock(x,y,z+1) == bid);
    }

    // Get the paired chest container for a double chest
    ContainerTileEntity* getDoubleChestPair(int x, int y, int z,
                                             std::function<uint16_t(int,int,int)> getBlock) {
        uint16_t bid = getBlock(x, y, z);
        if (bid != 54 && bid != 146) return nullptr;
        static constexpr int dx[] = {-1,1,0,0};
        static constexpr int dz[] = {0,0,-1,1};
        for (int i = 0; i < 4; ++i) {
            if (getBlock(x+dx[i], y, z+dz[i]) == bid) {
                return getContainer(x+dx[i], y, z+dz[i]);
            }
        }
        return nullptr;
    }

    // Hopper tick: transfer items from hopper to container below
    struct HopperTransfer {
        int fromX, fromY, fromZ;
        int toX, toY, toZ;
        ContainerSlot item;
    };

    // Tick all hoppers — returns list of transfers made
    std::vector<HopperTransfer> tickHoppers(
            std::function<uint16_t(int,int,int)> getBlock,
            std::function<uint8_t(int,int,int)> getMeta) {
        std::vector<HopperTransfer> transfers;

        for (auto& [pos, container] : containers_) {
            if (container.blockId != 154) continue; // Only hoppers
            if (container.isEmpty()) continue;

            auto [hx, hy, hz] = pos;
            uint8_t meta = getMeta(hx, hy, hz);

            // Hopper output direction from metadata
            int outX = hx, outY = hy, outZ = hz;
            switch (meta & 0x7) {
                case 0: outY--; break; // Down
                case 2: outZ--; break; // North
                case 3: outZ++; break; // South
                case 4: outX--; break; // West
                case 5: outX++; break; // East
                default: outY--; break;
            }

            // Check if target is a container
            auto* target = getContainer(outX, outY, outZ);
            if (!target) continue;

            // Transfer one item
            ContainerSlot item = container.removeOne();
            if (!item.isEmpty()) {
                int leftover = target->addItem(item.itemId, item.count, item.meta);
                if (leftover > 0) {
                    // Put back if target is full
                    container.addItem(item.itemId, static_cast<int8_t>(leftover), item.meta);
                } else {
                    transfers.push_back({hx, hy, hz, outX, outY, outZ, item});
                }
            }
        }

        return transfers;
    }

    // Dispenser/dropper activation — returns item to spawn as entity
    ContainerSlot activateDispenser(int x, int y, int z) {
        auto* container = getContainer(x, y, z);
        if (!container || container->isEmpty()) return {};

        std::mt19937 rng(static_cast<unsigned>(x * 73856093 ^ y * 19349663 ^ z * 83492791));
        return container->removeRandom(rng);
    }

    // Count of trapped chests being viewed (for redstone signal)
    int getTrappedChestViewers(int x, int y, int z) const {
        auto it = viewerCounts_.find(std::make_tuple(x, y, z));
        return it != viewerCounts_.end() ? it->second : 0;
    }

    void addViewer(int x, int y, int z) {
        viewerCounts_[std::make_tuple(x, y, z)]++;
    }

    void removeViewer(int x, int y, int z) {
        auto key = std::make_tuple(x, y, z);
        auto it = viewerCounts_.find(key);
        if (it != viewerCounts_.end()) {
            it->second--;
            if (it->second <= 0) viewerCounts_.erase(it);
        }
    }

    // Get all container positions
    std::vector<std::tuple<int,int,int>> allPositions() const {
        std::vector<std::tuple<int,int,int>> result;
        for (auto& [pos, _] : containers_) result.push_back(pos);
        return result;
    }

private:
    std::unordered_map<std::tuple<int,int,int>, ContainerTileEntity, ContainerPosHash> containers_;
    std::unordered_map<std::tuple<int,int,int>, int, ContainerPosHash> viewerCounts_;
};

} // namespace mc
