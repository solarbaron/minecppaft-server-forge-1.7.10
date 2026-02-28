/**
 * Weather.h — Weather system: rain, thunder, and lightning bolts.
 *
 * Java references:
 *   - net.minecraft.world.World.updateWeather — Rain/thunder timer and strength
 *   - net.minecraft.entity.effect.EntityLightningBolt — Lightning bolt entity
 *   - net.minecraft.entity.effect.EntityWeatherEffect — Base weather entity
 *
 * Rain/Thunder mechanics:
 *   - Timers count down each tick. When timer reaches 0, toggle state.
 *   - Raining→clear timer: rand(168000) + 12000
 *   - Clear→raining timer: rand(12000) + 12000 (rain) / rand(12000) + 3600 (thunder)
 *   - Raining→thundering timer: rand(12000) + 3600
 *   - Strength lerps ±0.01 per tick, clamped [0, 1]
 *
 * Lightning bolt:
 *   - State machine: lightningState starts at 2, decrements each tick
 *   - At state 2: play thunder + explode sounds
 *   - When state < 0: if boltLivingTime > 0 and random delay, repeat
 *   - While state >= 0: damage entities in 3-block AABB
 *   - Fire placement on Normal/Hard with doFireTick: 1 at pos + 4 random ±1
 *
 * Thread safety: Weather state updated on world tick thread.
 * JNI readiness: Simple data, predictable layout.
 */
#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <vector>

namespace mccpp {

// ═══════════════════════════════════════════════════════════════════════════
// WeatherState — Rain and thunder state management.
// Java reference: net.minecraft.world.World.updateWeather
// ═══════════════════════════════════════════════════════════════════════════

class WeatherState {
public:
    WeatherState() = default;

    // ─── State ───
    bool isRaining() const { return raining_; }
    bool isThundering() const { return thundering_; }
    float getRainStrength() const { return rainStrength_; }
    float getThunderStrength() const { return thunderStrength_; }
    float getPrevRainStrength() const { return prevRainStrength_; }
    float getPrevThunderStrength() const { return prevThunderStrength_; }

    void setRaining(bool v) { raining_ = v; }
    void setThundering(bool v) { thundering_ = v; }
    void setRainTime(int32_t t) { rainTime_ = t; }
    void setThunderTime(int32_t t) { thunderTime_ = t; }
    int32_t getRainTime() const { return rainTime_; }
    int32_t getThunderTime() const { return thunderTime_; }

    // Java: calculateInitialWeather
    void initializeWeather() {
        if (raining_) {
            rainStrength_ = 1.0f;
            if (thundering_) {
                thunderStrength_ = 1.0f;
            }
        }
    }

    // Java: updateWeather — called every tick
    // randFn returns random int in [0, bound)
    void update(std::function<int32_t(int32_t)> randFn, bool hasSky) {
        if (!hasSky) return;

        // ─── Thunder timer ───
        if (thunderTime_ <= 0) {
            if (thundering_) {
                // Thunder → clear: long delay
                thunderTime_ = randFn(12000) + 3600;
            } else {
                // Clear → thunder: very long delay
                thunderTime_ = randFn(168000) + 12000;
            }
        } else {
            --thunderTime_;
            if (thunderTime_ <= 0) {
                thundering_ = !thundering_;
            }
        }

        // Thunder strength lerp
        prevThunderStrength_ = thunderStrength_;
        if (thundering_) {
            thunderStrength_ = static_cast<float>(static_cast<double>(thunderStrength_) + 0.01);
        } else {
            thunderStrength_ = static_cast<float>(static_cast<double>(thunderStrength_) - 0.01);
        }
        thunderStrength_ = clamp(thunderStrength_, 0.0f, 1.0f);

        // ─── Rain timer ───
        if (rainTime_ <= 0) {
            if (raining_) {
                rainTime_ = randFn(12000) + 12000;
            } else {
                rainTime_ = randFn(168000) + 12000;
            }
        } else {
            --rainTime_;
            if (rainTime_ <= 0) {
                raining_ = !raining_;
            }
        }

        // Rain strength lerp
        prevRainStrength_ = rainStrength_;
        if (raining_) {
            rainStrength_ = static_cast<float>(static_cast<double>(rainStrength_) + 0.01);
        } else {
            rainStrength_ = static_cast<float>(static_cast<double>(rainStrength_) - 0.01);
        }
        rainStrength_ = clamp(rainStrength_, 0.0f, 1.0f);
    }

    // Interpolated rain strength for rendering
    float getRainStrength(float partialTicks) const {
        return prevRainStrength_ + (rainStrength_ - prevRainStrength_) * partialTicks;
    }

    float getThunderStrength(float partialTicks) const {
        return prevThunderStrength_ + (thunderStrength_ - prevThunderStrength_) * partialTicks;
    }

private:
    static float clamp(float v, float min, float max) {
        if (v < min) return min;
        if (v > max) return max;
        return v;
    }

    bool raining_ = false;
    bool thundering_ = false;
    int32_t rainTime_ = 0;
    int32_t thunderTime_ = 0;
    float rainStrength_ = 0.0f;
    float thunderStrength_ = 0.0f;
    float prevRainStrength_ = 0.0f;
    float prevThunderStrength_ = 0.0f;
};

// ═══════════════════════════════════════════════════════════════════════════
// LightningBolt — Lightning bolt entity state machine.
// Java reference: net.minecraft.entity.effect.EntityLightningBolt
// ═══════════════════════════════════════════════════════════════════════════

class LightningBolt {
public:
    LightningBolt(double x, double y, double z, uint64_t randomSeed)
        : posX_(x), posY_(y), posZ_(z), rng_(randomSeed) {
        lightningState_ = 2;
        boltVertex_ = nextLong();
        boltLivingTime_ = nextInt(3) + 1;
    }

    // ─── Position ───
    double getX() const { return posX_; }
    double getY() const { return posY_; }
    double getZ() const { return posZ_; }
    int64_t getBoltVertex() const { return boltVertex_; }

    bool isDead() const { return dead_; }
    bool isVisible() const { return lightningState_ >= 0; }

    // ─── Fire placement positions (generated at spawn) ───

    struct FirePos { int32_t x, y, z; };

    // Java: constructor fire placement (Normal/Hard + doFireTick)
    std::vector<FirePos> getInitialFirePositions() {
        std::vector<FirePos> fires;
        int32_t bx = floorD(posX_);
        int32_t by = floorD(posY_);
        int32_t bz = floorD(posZ_);

        // Fire at impact point
        fires.push_back({bx, by, bz});

        // 4 random fires ±1
        for (int32_t i = 0; i < 4; ++i) {
            int32_t fx = floorD(posX_) + nextInt(3) - 1;
            int32_t fy = floorD(posY_) + nextInt(3) - 1;
            int32_t fz = floorD(posZ_) + nextInt(3) - 1;
            fires.push_back({fx, fy, fz});
        }

        return fires;
    }

    // ─── Tick ───

    struct TickResult {
        bool playThunderSound = false;    // Play "ambient.weather.thunder" (10000vol, 0.8+rand*0.2 pitch)
        bool playExplodeSound = false;    // Play "random.explode" (2.0vol, 0.5+rand*0.2 pitch)
        bool damageEntities = false;      // Damage entities in 3-block AABB
        bool tryPlaceFire = false;        // Try to place fire at impact point
        float thunderPitch = 1.0f;
        float explodePitch = 0.7f;
    };

    TickResult onUpdate() {
        TickResult result;

        // Java: if lightningState == 2, play sounds
        if (lightningState_ == 2) {
            result.playThunderSound = true;
            result.playExplodeSound = true;
            result.thunderPitch = 0.8f + nextFloat() * 0.2f;
            result.explodePitch = 0.5f + nextFloat() * 0.2f;
        }

        --lightningState_;

        if (lightningState_ < 0) {
            if (boltLivingTime_ == 0) {
                dead_ = true;
            } else if (lightningState_ < -nextInt(10)) {
                --boltLivingTime_;
                lightningState_ = 1;
                boltVertex_ = nextLong();
                result.tryPlaceFire = true;
            }
        }

        // While visible, damage entities
        if (lightningState_ >= 0) {
            result.damageEntities = true;
        }

        return result;
    }

    // ─── Entity damage AABB ───
    // Java: 3-block radius, +6 height
    struct DamageAABB {
        double minX, minY, minZ;
        double maxX, maxY, maxZ;
    };

    DamageAABB getDamageAABB() const {
        constexpr double R = 3.0;
        return {
            posX_ - R, posY_ - R, posZ_ - R,
            posX_ + R, posY_ + 6.0 + R, posZ_ + R
        };
    }

    // ─── Sound parameters ───
    static constexpr float THUNDER_VOLUME = 10000.0f;
    static constexpr float EXPLODE_VOLUME = 2.0f;

private:
    static int32_t floorD(double d) {
        int32_t i = static_cast<int32_t>(d);
        return d < static_cast<double>(i) ? i - 1 : i;
    }

    int32_t nextInt(int32_t bound) {
        if (bound <= 0) return 0;
        rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<int32_t>((rng_ >> 33) % static_cast<uint64_t>(bound));
    }

    float nextFloat() {
        rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<float>((rng_ >> 33) & 0x7FFFFF) / static_cast<float>(0x800000);
    }

    int64_t nextLong() {
        rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<int64_t>(rng_);
    }

    double posX_, posY_, posZ_;
    int32_t lightningState_ = 2;
    int64_t boltVertex_ = 0;
    int32_t boltLivingTime_ = 1;
    bool dead_ = false;
    uint64_t rng_;
};

// ═══════════════════════════════════════════════════════════════════════════
// WeatherManager — Manages weather state + lightning bolt lifecycle.
// ═══════════════════════════════════════════════════════════════════════════

class WeatherManager {
public:
    WeatherManager() = default;

    WeatherState& getState() { return state_; }
    const WeatherState& getState() const { return state_; }

    // Add a lightning bolt at position
    void addLightningBolt(double x, double y, double z, uint64_t seed) {
        bolts_.emplace_back(x, y, z, seed);
    }

    // Tick all lightning bolts
    std::vector<LightningBolt::TickResult> tickBolts() {
        std::vector<LightningBolt::TickResult> results;
        for (auto& bolt : bolts_) {
            if (!bolt.isDead()) {
                results.push_back(bolt.onUpdate());
            }
        }
        // Remove dead bolts
        bolts_.erase(
            std::remove_if(bolts_.begin(), bolts_.end(),
                [](const LightningBolt& b) { return b.isDead(); }),
            bolts_.end());
        return results;
    }

    const std::vector<LightningBolt>& getBolts() const { return bolts_; }

    // Java: WorldServer tick lightning check
    // During thunderstorm, 1/100000 chance per chunk to strike
    struct LightningStrike {
        int32_t x, y, z;
    };

    static bool shouldStrikeLightning(int32_t rand100k) {
        return rand100k == 0; // 1/100000 chance
    }

private:
    WeatherState state_;
    std::vector<LightningBolt> bolts_;
};

} // namespace mccpp
