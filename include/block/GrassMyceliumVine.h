/**
 * GrassMyceliumVine.h — Block growth mechanics for grass, mycelium, and vine.
 *
 * Java references:
 *   - net.minecraft.block.BlockGrass (88 lines)
 *   - net.minecraft.block.BlockMycelium (47 lines)
 *   - net.minecraft.block.BlockVine (289 lines)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * GRASS BLOCK (BlockGrass)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: grass, random tick enabled
 *
 * updateTick (random tick):
 *   DEATH: if lightValue(x, y+1, z) < 4 AND lightOpacity(above) > 2
 *     → replace with dirt
 *   SPREAD: if lightValue(x, y+1, z) >= 9
 *     → 4 attempts per tick
 *     → target position: x ± rand(3)-1, y ± rand(5)-3, z ± rand(3)-1
 *     → conditions for target:
 *       - Must be dirt (ID 3) with metadata 0
 *       - lightValue(targetX, targetY+1, targetZ) >= 4
 *       - lightOpacity(block above target) <= 2
 *     → if all conditions: replace dirt with grass
 *
 * Bone meal (IGrowable.fertilize):
 *   128 attempts, each with random walk of i/16 steps:
 *   - Walk: x ± rand(3)-1, y ± (rand(3)-1)*rand(3)/2, z ± rand(3)-1
 *   - Each walk step: must be on grass and not in solid block
 *   - If air at final position:
 *     - 7/8 chance: place tall grass (35:1) if canBlockStay
 *     - 1/8 chance: place biome-specific flower if canBlockStay
 *
 * Drop: dirt (not grass itself)
 *
 * ═══════════════════════════════════════════════════════════════════════
 * MYCELIUM (BlockMycelium)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Identical to grass for death/spread logic:
 *   DEATH: same conditions → dirt
 *   SPREAD: same 4-attempt algorithm → mycelium
 *   (only difference: spreads mycelium instead of grass)
 *
 * Drop: dirt
 *
 * ═══════════════════════════════════════════════════════════════════════
 * VINE (BlockVine)
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Material: vine, random tick enabled
 * No collision box (climbable but no collision)
 * No item drop (null), shears harvest gives vine item
 *
 * Metadata bitmask (4 faces):
 *   bit 0: south (+Z) — Direction index 0
 *   bit 1: west (-X) — Direction index 1
 *   bit 2: north (-Z) — Direction index 2
 *   bit 3: east (+X) — Direction index 3
 *
 * Support validation (func_150094_e):
 *   For each face bit set: needs either attached solid block OR
 *   vine above with same face bit set
 *   If no faces remain AND no solid block above → drop and remove
 *
 * Growth (updateTick — 25% chance per tick):
 *   1. Density check: count vines in 9×3×9 area (x±4, y±1, z±4)
 *      If 5+ found → stop growing (density limit)
 *   2. Choose random direction (0-5 = down,up,N,S,W,E)
 *   3. Growth cases:
 *
 *   GROW UP (direction 1, y < 255):
 *     - Target (x, y+1, z) must be air
 *     - If density limit: stop
 *     - Random subset of current face bits (rand(16) & meta)
 *     - Remove faces without adjacent solid block at y+1
 *     - If any faces remain: place vine at y+1
 *
 *   GROW SIDEWAYS (direction 2-5):
 *     - Convert to vine direction via facingToDirection[side]
 *     - If current vine doesn't have that face:
 *       If density limit: stop
 *       Check adjacent block in that direction:
 *       a) AIR: try wrapping around corners (4 sub-cases)
 *          - CW face + solid behind CW+direction → place CW
 *          - CCW face + solid behind CCW+direction → place CCW
 *          - CW face + air behind → wrap to opposite face
 *          - CCW face + air behind → wrap to opposite face
 *          - Solid block at direction+up → place flat (meta 0)
 *       b) SOLID+NORMAL: add face bit to current metadata
 *
 *   GROW DOWN (direction 0, y > 1):
 *     - Below must be air: random subset (rand(16) & meta)
 *       If any bits remain: place vine below
 *     - Below is vine: merge bits (OR existing with random subset)
 *
 * Placement side → metadata:
 *   side 2 → bit 0 (south face, +Z behind)
 *   side 3 → bit 2 (north face, -Z behind)
 *   side 4 → bit 3 (east face, +X behind)
 *   side 5 → bit 1 (west face, -X behind)
 *
 * Thread safety: Block updates on server thread.
 * JNI readiness: Simple constants and growth logic.
 */
#pragma once

#include <cstdint>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// Grass Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace GrassBlockConstants {
    // ─── Block IDs ───
    static constexpr int32_t GRASS_ID = 2;
    static constexpr int32_t DIRT_ID = 3;
    static constexpr int32_t MYCELIUM_ID = 110;

    // ─── Death conditions ───
    // Java: lightValue < 4 AND lightOpacity > 2
    static constexpr int32_t DEATH_LIGHT_THRESHOLD = 4;
    static constexpr int32_t DEATH_OPACITY_THRESHOLD = 2;

    // ─── Spread conditions ───
    // Java: lightValue >= 9 at Y+1
    static constexpr int32_t SPREAD_LIGHT_THRESHOLD = 9;
    // Java: 4 attempts per random tick
    static constexpr int32_t SPREAD_ATTEMPTS = 4;

    // ─── Spread range ───
    // Java: x ± rand(3)-1, y ± rand(5)-3, z ± rand(3)-1
    static constexpr int32_t SPREAD_X_RANGE = 3;   // rand(3) = 0-2, minus 1 = -1 to +1
    static constexpr int32_t SPREAD_X_OFFSET = 1;
    static constexpr int32_t SPREAD_Y_RANGE = 5;   // rand(5) = 0-4, minus 3 = -3 to +1
    static constexpr int32_t SPREAD_Y_OFFSET = 3;
    static constexpr int32_t SPREAD_Z_RANGE = 3;
    static constexpr int32_t SPREAD_Z_OFFSET = 1;

    // ─── Target conditions ───
    // Must be dirt (ID 3) with metadata 0
    // lightValue at target Y+1 >= 4
    // lightOpacity of block above target <= 2
    static constexpr int32_t TARGET_DIRT_META = 0;
    static constexpr int32_t TARGET_LIGHT_MIN = 4;
    static constexpr int32_t TARGET_OPACITY_MAX = 2;
}

// ═══════════════════════════════════════════════════════════════════════════
// Bone Meal on Grass Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace GrassFertilize {
    // Java: 128 attempts
    static constexpr int32_t ATTEMPTS = 128;

    // Walk steps per attempt = i / 16
    static constexpr int32_t STEPS_DIVISOR = 16;

    // Walk deltas:
    // X: rand(3) - 1 = -1..+1
    // Y: (rand(3) - 1) * rand(3) / 2
    // Z: rand(3) - 1 = -1..+1

    // 7/8 = tall grass, 1/8 = biome flower
    static constexpr int32_t FLOWER_CHANCE = 8;  // rand(8) == 0

    // Tall grass metadata
    static constexpr int32_t TALLGRASS_BLOCK_ID = 31;
    static constexpr int32_t TALLGRASS_META = 1;   // tall grass (not fern)
}

// ═══════════════════════════════════════════════════════════════════════════
// Vine Block Constants
// ═══════════════════════════════════════════════════════════════════════════

namespace VineConstants {
    // ─── Block ID ───
    static constexpr int32_t VINE_ID = 106;

    // ─── Metadata face bitmask ───
    // Java: bit 0 = south, bit 1 = west, bit 2 = north, bit 3 = east
    static constexpr int32_t FACE_SOUTH = 1;   // bit 0, Direction index 0
    static constexpr int32_t FACE_WEST  = 2;   // bit 1, Direction index 1
    static constexpr int32_t FACE_NORTH = 4;   // bit 2, Direction index 2
    static constexpr int32_t FACE_EAST  = 8;   // bit 3, Direction index 3

    inline bool hasFace(int32_t meta, int32_t dirIndex) {
        return (meta & (1 << dirIndex)) != 0;
    }

    // ─── Growth probability ───
    // Java: rand.nextInt(4) == 0 → 25% per tick
    static constexpr int32_t GROWTH_CHANCE = 4;

    // ─── Density limit ───
    // Java: count >= 5 in 9×3×9 area → stop growing
    static constexpr int32_t DENSITY_LIMIT = 5;
    static constexpr int32_t DENSITY_X_RANGE = 4;  // x ± 4
    static constexpr int32_t DENSITY_Y_RANGE = 1;  // y ± 1
    static constexpr int32_t DENSITY_Z_RANGE = 4;  // z ± 4

    // ─── Max chain length ───
    static constexpr int32_t MAX_SCAN_DISTANCE = 42;

    // ─── Random face subset ───
    // Java: rand.nextInt(16) & currentMeta
    static constexpr int32_t FACE_RANDOM_MASK = 16;

    // ─── Placement side → metadata mapping ───
    // Java: side 2 → 1 (south), side 3 → 4 (north)
    //       side 4 → 8 (east),  side 5 → 2 (west)
    inline int32_t sideToMeta(int32_t side) {
        switch (side) {
            case 2: return FACE_SOUTH;
            case 3: return FACE_NORTH;
            case 4: return FACE_EAST;
            case 5: return FACE_WEST;
            default: return 0;
        }
    }

    // ─── Support check ───
    // Java: block.renderAsNormalBlock() && block.blockMaterial.blocksMovement()
    // "Can this block support a vine?"

    // ─── Direction offsets (same as regular Direction) ───
    // Index: 0=south(+Z), 1=west(-X), 2=north(-Z), 3=east(+X)
    static constexpr int32_t offsetX[] = {0, -1, 0, 1};
    static constexpr int32_t offsetZ[] = {1, 0, -1, 0};

    // ─── facingToDirection mapping ───
    // Java: Direction.facingToDirection = {-1, -1, 2, 0, 1, 3}
    // side 0(down)=-1, 1(up)=-1, 2(north)=2, 3(south)=0, 4(west)=1, 5(east)=3
    static constexpr int32_t facingToDirection[] = {-1, -1, 2, 0, 1, 3};

    // ─── Rotation helpers ───
    // CW: (dir + 1) & 3
    // CCW: (dir + 3) & 3
    // Opposite: (dir + 2) & 3
    inline int32_t rotateCW(int32_t dir) { return (dir + 1) & 3; }
    inline int32_t rotateCCW(int32_t dir) { return (dir + 3) & 3; }
    inline int32_t opposite(int32_t dir) { return (dir + 2) & 3; }

    // ─── Block bounds per face (1/16 = 0.0625) ───
    static constexpr float VINE_THICKNESS = 0.0625f;  // 1/16 block
}

} // namespace mccpp
