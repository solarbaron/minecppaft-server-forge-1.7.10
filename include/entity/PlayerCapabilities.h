/**
 * PlayerCapabilities.h — Player ability flags and speed settings.
 *
 * Java reference: net.minecraft.entity.player.PlayerCapabilities
 *
 * Fields:
 *   disableDamage — Invulnerability (creative mode)
 *   isFlying      — Currently flying
 *   allowFlying   — Can toggle flight (creative/spectator)
 *   isCreativeMode — Creative mode (instant break, unlimited items)
 *   allowEdit     — Can place/break blocks (default: true)
 *   flySpeed      — Flying speed (default: 0.05)
 *   walkSpeed     — Walking speed (default: 0.1)
 *
 * NBT: Stored under compound tag "abilities" with keys:
 *   invulnerable, flying, mayfly, instabuild, mayBuild, flySpeed, walkSpeed
 *
 * Thread safety: Per-player, accessed from server thread only.
 *
 * JNI readiness: Simple struct with primitive fields.
 */
#pragma once

#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// PlayerCapabilities — Per-player ability state.
// Java reference: net.minecraft.entity.player.PlayerCapabilities
// ═══════════════════════════════════════════════════════════════════════════

struct PlayerCapabilities {
    // Java: public boolean disableDamage
    bool disableDamage = false;

    // Java: public boolean isFlying
    bool isFlying = false;

    // Java: public boolean allowFlying
    bool allowFlying = false;

    // Java: public boolean isCreativeMode
    bool isCreativeMode = false;

    // Java: public boolean allowEdit = true
    bool allowEdit = true;

    // Java: private float flySpeed = 0.05f
    float flySpeed = 0.05f;

    // Java: private float walkSpeed = 0.1f
    float walkSpeed = 0.1f;

    // ─── Getters (match Java) ───

    float getFlySpeed() const { return flySpeed; }
    float getWalkSpeed() const { return walkSpeed; }

    // ─── Game mode presets ───

    // Set capabilities for survival mode
    void setSurvival() {
        disableDamage = false;
        isFlying = false;
        allowFlying = false;
        isCreativeMode = false;
        allowEdit = true;
        flySpeed = 0.05f;
        walkSpeed = 0.1f;
    }

    // Set capabilities for creative mode
    void setCreative() {
        disableDamage = true;
        isFlying = false;   // Not flying by default, but allowed
        allowFlying = true;
        isCreativeMode = true;
        allowEdit = true;
        flySpeed = 0.05f;
        walkSpeed = 0.1f;
    }

    // Set capabilities for adventure mode
    void setAdventure() {
        disableDamage = false;
        isFlying = false;
        allowFlying = false;
        isCreativeMode = false;
        allowEdit = false;
        flySpeed = 0.05f;
        walkSpeed = 0.1f;
    }

    // ─── NBT field names (Java: writeCapabilitiesToNBT / readCapabilitiesFromNBT) ───

    static constexpr const char* NBT_TAG_NAME       = "abilities";
    static constexpr const char* NBT_INVULNERABLE   = "invulnerable";
    static constexpr const char* NBT_FLYING          = "flying";
    static constexpr const char* NBT_MAY_FLY         = "mayfly";
    static constexpr const char* NBT_INSTABUILD      = "instabuild";
    static constexpr const char* NBT_MAY_BUILD       = "mayBuild";
    static constexpr const char* NBT_FLY_SPEED       = "flySpeed";
    static constexpr const char* NBT_WALK_SPEED      = "walkSpeed";

    // ─── Protocol (0x39 Player Abilities packet) ───
    // Flags byte: bit 0=invulnerable, bit 1=flying, bit 2=allowFlying, bit 3=creative
    uint8_t getProtocolFlags() const {
        uint8_t flags = 0;
        if (disableDamage)  flags |= 0x01;  // Invulnerable
        if (isFlying)       flags |= 0x02;  // Flying
        if (allowFlying)    flags |= 0x04;  // Allow flying
        if (isCreativeMode) flags |= 0x08;  // Creative mode (instant break)
        return flags;
    }

    void setFromProtocolFlags(uint8_t flags) {
        disableDamage  = (flags & 0x01) != 0;
        isFlying       = (flags & 0x02) != 0;
        allowFlying    = (flags & 0x04) != 0;
        isCreativeMode = (flags & 0x08) != 0;
    }
};

} // namespace mccpp
