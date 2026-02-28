/**
 * DamageSource.h — Damage source and type system.
 *
 * Java references:
 *   - net.minecraft.util.DamageSource — Base damage source
 *   - net.minecraft.util.EntityDamageSource — Entity-caused damage
 *   - net.minecraft.util.EntityDamageSourceIndirect — Indirect (projectile) damage
 *
 * All 14 vanilla static damage sources plus 7 factory methods for
 * entity/projectile/explosion damage.
 *
 * Thread safety: Static damage types are read-only after init.
 *   Entity-bound sources are created per-event (no sharing needed).
 *
 * JNI readiness: String-based damageType for easy mapping.
 */
#pragma once

#include <cstdint>
#include <string>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// DamageSource — Describes the source and type of damage.
// Java reference: net.minecraft.util.DamageSource
//
// Properties:
//   - isUnblockable: bypasses armor absorption
//   - isDamageAllowedInCreativeMode: can kill creative players (void)
//   - damageIsAbsolute: bypasses enchantment/potion reduction
//   - hungerDamage: food exhaustion amount (default 0.3, 0.0 if bypasses armor)
//   - fireDamage: fire-based (fire resist potion blocks it)
//   - projectile: from projectile entity
//   - difficultyScaled: damage scales with difficulty
//   - magicDamage: magic-based (bypasses armor)
//   - explosion: from explosion
// ═══════════════════════════════════════════════════════════════════════════

struct DamageSource {
    std::string damageType;

    bool isUnblockable       = false;
    bool creativeAllowed     = false;
    bool damageIsAbsolute    = false;
    float hungerDamage       = 0.3f;
    bool fireDamage          = false;
    bool projectile          = false;
    bool difficultyScaled    = false;
    bool magicDamage         = false;
    bool explosion           = false;

    // Source entity ID (-1 = no entity source)
    int32_t sourceEntityId   = -1;
    // Indirect attacker entity ID (-1 = no indirect source)
    int32_t indirectEntityId = -1;

    DamageSource() = default;
    explicit DamageSource(const std::string& type) : damageType(type) {}

    // ─── Builder methods (match Java's fluent API) ───

    DamageSource& setDamageBypassesArmor() {
        isUnblockable = true;
        hungerDamage = 0.0f;
        return *this;
    }

    DamageSource& setDamageAllowedInCreativeMode() {
        creativeAllowed = true;
        return *this;
    }

    DamageSource& setDamageIsAbsolute() {
        damageIsAbsolute = true;
        hungerDamage = 0.0f;
        return *this;
    }

    DamageSource& setFireDamage() {
        fireDamage = true;
        return *this;
    }

    DamageSource& setProjectile() {
        projectile = true;
        return *this;
    }

    DamageSource& setDifficultyScaled() {
        difficultyScaled = true;
        return *this;
    }

    DamageSource& setMagicDamage() {
        magicDamage = true;
        return *this;
    }

    DamageSource& setExplosion() {
        explosion = true;
        return *this;
    }

    // ─── Queries ───

    bool canHarmInCreative() const { return creativeAllowed; }
    bool isDamageAbsolute() const { return damageIsAbsolute; }
    bool isFireDamage() const { return fireDamage; }
    bool isProjectile() const { return projectile; }
    bool isDifficultyScaled() const { return difficultyScaled; }
    bool isMagicDamage() const { return magicDamage; }
    bool isExplosion() const { return explosion; }
    float getHungerDamage() const { return hungerDamage; }
    const std::string& getDamageType() const { return damageType; }

    // Java: getDeathMessage key = "death.attack.<damageType>"
    std::string getDeathMessageKey() const {
        return "death.attack." + damageType;
    }

    bool hasSourceEntity() const { return sourceEntityId >= 0; }
    bool hasIndirectEntity() const { return indirectEntityId >= 0; }
};

// ═══════════════════════════════════════════════════════════════════════════
// DamageSources — Static damage source instances + factory methods.
// Java reference: net.minecraft.util.DamageSource static fields
// ═══════════════════════════════════════════════════════════════════════════

namespace DamageSources {

    // ─── 14 static damage sources ───

    // Java: DamageSource.inFire = new DamageSource("inFire").setFireDamage()
    DamageSource inFire();

    // Java: DamageSource.onFire = new DamageSource("onFire").setDamageBypassesArmor().setFireDamage()
    DamageSource onFire();

    // Java: DamageSource.lava = new DamageSource("lava").setFireDamage()
    DamageSource lava();

    // Java: DamageSource.inWall = new DamageSource("inWall").setDamageBypassesArmor()
    DamageSource inWall();

    // Java: DamageSource.drown = new DamageSource("drown").setDamageBypassesArmor()
    DamageSource drown();

    // Java: DamageSource.starve = new DamageSource("starve").setDamageBypassesArmor().setDamageIsAbsolute()
    DamageSource starve();

    // Java: DamageSource.cactus = new DamageSource("cactus")
    DamageSource cactus();

    // Java: DamageSource.fall = new DamageSource("fall").setDamageBypassesArmor()
    DamageSource fall();

    // Java: DamageSource.outOfWorld = new DamageSource("outOfWorld").setDamageBypassesArmor().setDamageAllowedInCreativeMode()
    DamageSource outOfWorld();

    // Java: DamageSource.generic = new DamageSource("generic").setDamageBypassesArmor()
    DamageSource generic();

    // Java: DamageSource.magic = new DamageSource("magic").setDamageBypassesArmor().setMagicDamage()
    DamageSource magic();

    // Java: DamageSource.wither = new DamageSource("wither").setDamageBypassesArmor()
    DamageSource wither();

    // Java: DamageSource.anvil = new DamageSource("anvil")
    DamageSource anvil();

    // Java: DamageSource.fallingBlock = new DamageSource("fallingBlock")
    DamageSource fallingBlock();

    // ─── 7 factory methods for entity-sourced damage ───

    // Java: DamageSource.causeMobDamage
    DamageSource causeMobDamage(int32_t mobEntityId);

    // Java: DamageSource.causePlayerDamage
    DamageSource causePlayerDamage(int32_t playerEntityId);

    // Java: DamageSource.causeArrowDamage
    DamageSource causeArrowDamage(int32_t arrowEntityId, int32_t shooterEntityId);

    // Java: DamageSource.causeFireballDamage
    DamageSource causeFireballDamage(int32_t fireballEntityId, int32_t shooterEntityId);

    // Java: DamageSource.causeThrownDamage
    DamageSource causeThrownDamage(int32_t projectileEntityId, int32_t throwerEntityId);

    // Java: DamageSource.causeIndirectMagicDamage
    DamageSource causeIndirectMagicDamage(int32_t sourceEntityId, int32_t attackerEntityId);

    // Java: DamageSource.causeThornsDamage
    DamageSource causeThornsDamage(int32_t entityId);

    // Java: DamageSource.setExplosionSource
    DamageSource setExplosionSource(int32_t placerEntityId = -1);

} // namespace DamageSources

} // namespace mccpp
