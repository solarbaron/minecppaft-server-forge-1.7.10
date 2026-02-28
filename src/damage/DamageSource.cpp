/**
 * DamageSource.cpp — DamageSource factory implementation.
 *
 * Java reference: net.minecraft.util.DamageSource static fields and methods
 *
 * All 14 static damage sources match Java property chains exactly.
 * Factory methods create entity-bound damage sources per-event.
 */

#include "damage/DamageSource.h"

namespace mccpp {
namespace DamageSources {

// ═════════════════════════════════════════════════════════════════════════════
// 14 static damage sources
// ═════════════════════════════════════════════════════════════════════════════

DamageSource inFire() {
    DamageSource ds("inFire");
    ds.setFireDamage();
    return ds;
}

DamageSource onFire() {
    DamageSource ds("onFire");
    ds.setDamageBypassesArmor().setFireDamage();
    return ds;
}

DamageSource lava() {
    DamageSource ds("lava");
    ds.setFireDamage();
    return ds;
}

DamageSource inWall() {
    DamageSource ds("inWall");
    ds.setDamageBypassesArmor();
    return ds;
}

DamageSource drown() {
    DamageSource ds("drown");
    ds.setDamageBypassesArmor();
    return ds;
}

DamageSource starve() {
    DamageSource ds("starve");
    ds.setDamageBypassesArmor().setDamageIsAbsolute();
    return ds;
}

DamageSource cactus() {
    return DamageSource("cactus");
}

DamageSource fall() {
    DamageSource ds("fall");
    ds.setDamageBypassesArmor();
    return ds;
}

DamageSource outOfWorld() {
    DamageSource ds("outOfWorld");
    ds.setDamageBypassesArmor().setDamageAllowedInCreativeMode();
    return ds;
}

DamageSource generic() {
    DamageSource ds("generic");
    ds.setDamageBypassesArmor();
    return ds;
}

DamageSource magic() {
    DamageSource ds("magic");
    ds.setDamageBypassesArmor().setMagicDamage();
    return ds;
}

DamageSource wither() {
    DamageSource ds("wither");
    ds.setDamageBypassesArmor();
    return ds;
}

DamageSource anvil() {
    return DamageSource("anvil");
}

DamageSource fallingBlock() {
    return DamageSource("fallingBlock");
}

// ═════════════════════════════════════════════════════════════════════════════
// 7 factory methods for entity-sourced damage
// ═════════════════════════════════════════════════════════════════════════════

// Java: EntityDamageSource("mob", entity)
DamageSource causeMobDamage(int32_t mobEntityId) {
    DamageSource ds("mob");
    ds.sourceEntityId = mobEntityId;
    return ds;
}

// Java: EntityDamageSource("player", player)
DamageSource causePlayerDamage(int32_t playerEntityId) {
    DamageSource ds("player");
    ds.sourceEntityId = playerEntityId;
    return ds;
}

// Java: EntityDamageSourceIndirect("arrow", arrow, shooter).setProjectile()
DamageSource causeArrowDamage(int32_t arrowEntityId, int32_t shooterEntityId) {
    DamageSource ds("arrow");
    ds.sourceEntityId = arrowEntityId;
    ds.indirectEntityId = shooterEntityId;
    ds.setProjectile();
    return ds;
}

// Java: EntityDamageSourceIndirect("fireball"/"onFire", fireball, shooter).setFireDamage().setProjectile()
DamageSource causeFireballDamage(int32_t fireballEntityId, int32_t shooterEntityId) {
    if (shooterEntityId < 0) {
        // No shooter — uses "onFire" type
        DamageSource ds("onFire");
        ds.sourceEntityId = fireballEntityId;
        ds.indirectEntityId = fireballEntityId;
        ds.setFireDamage().setProjectile();
        return ds;
    }
    DamageSource ds("fireball");
    ds.sourceEntityId = fireballEntityId;
    ds.indirectEntityId = shooterEntityId;
    ds.setFireDamage().setProjectile();
    return ds;
}

// Java: EntityDamageSourceIndirect("thrown", projectile, thrower).setProjectile()
DamageSource causeThrownDamage(int32_t projectileEntityId, int32_t throwerEntityId) {
    DamageSource ds("thrown");
    ds.sourceEntityId = projectileEntityId;
    ds.indirectEntityId = throwerEntityId;
    ds.setProjectile();
    return ds;
}

// Java: EntityDamageSourceIndirect("indirectMagic", source, attacker).setDamageBypassesArmor().setMagicDamage()
DamageSource causeIndirectMagicDamage(int32_t sourceEntityId, int32_t attackerEntityId) {
    DamageSource ds("indirectMagic");
    ds.sourceEntityId = sourceEntityId;
    ds.indirectEntityId = attackerEntityId;
    ds.setDamageBypassesArmor().setMagicDamage();
    return ds;
}

// Java: EntityDamageSource("thorns", entity).setMagicDamage()
DamageSource causeThornsDamage(int32_t entityId) {
    DamageSource ds("thorns");
    ds.sourceEntityId = entityId;
    ds.setMagicDamage();
    return ds;
}

// Java: with placer -> EntityDamageSource("explosion.player", placer).setDifficultyScaled().setExplosion()
//       without   -> DamageSource("explosion").setDifficultyScaled().setExplosion()
DamageSource setExplosionSource(int32_t placerEntityId) {
    if (placerEntityId >= 0) {
        DamageSource ds("explosion.player");
        ds.sourceEntityId = placerEntityId;
        ds.setDifficultyScaled().setExplosion();
        return ds;
    }
    DamageSource ds("explosion");
    ds.setDifficultyScaled().setExplosion();
    return ds;
}

} // namespace DamageSources
} // namespace mccpp
