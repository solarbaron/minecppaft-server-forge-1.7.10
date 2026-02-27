#pragma once
// WeatherManager — rain, thunder, and lightning cycle.
// Ported from mt.java (WorldServer) weather logic.
//
// Vanilla weather mechanics:
//   - Clear duration: 12000-180000 ticks (10min - 2.5hr)
//   - Rain duration:  12000-24000 ticks (10-20min)
//   - Thunder duration: 3600-15600 ticks (3-13min)
//   - Rain intensity ramps up/down over ~300 ticks
//   - Lightning strikes randomly during thunder
//
// Network: 0x2B Change Game State
//   reason 1 = Begin Raining, reason 2 = End Raining
//   reason 7 = Rain Level (0.0-1.0), reason 8 = Thunder Level (0.0-1.0)
//
// Lightning: 0x2C Spawn Global Entity (type 1 = lightning bolt)

#include <cstdint>
#include <random>
#include <chrono>
#include <vector>

namespace mc {

enum class WeatherState : uint8_t {
    CLEAR   = 0,
    RAIN    = 1,
    THUNDER = 2
};

// Lightning bolt to spawn
struct LightningBolt {
    int32_t entityId;
    double x, y, z;
};

class WeatherManager {
public:
    WeatherManager()
        : rng_(static_cast<unsigned>(
              std::chrono::steady_clock::now().time_since_epoch().count())) {
        clearDuration_ = randomClearDuration();
    }

    WeatherState state() const { return state_; }
    float rainStrength() const { return rainStrength_; }
    float thunderStrength() const { return thunderStrength_; }

    // Tick weather — returns true if state changed (needs packet broadcast)
    bool tick(int32_t* nextEntityId) {
        bool changed = false;
        pendingLightning_.clear();

        --clearDuration_;

        switch (state_) {
            case WeatherState::CLEAR:
                // Ramp down rain
                if (rainStrength_ > 0.0f) {
                    rainStrength_ -= 0.01f;
                    if (rainStrength_ < 0.0f) rainStrength_ = 0.0f;
                    changed = true;
                }

                // Transition to rain
                if (clearDuration_ <= 0) {
                    state_ = WeatherState::RAIN;
                    rainDuration_ = randomRainDuration();
                    changed = true;
                }
                break;

            case WeatherState::RAIN:
                // Ramp up rain
                if (rainStrength_ < 1.0f) {
                    rainStrength_ += 0.01f;
                    if (rainStrength_ > 1.0f) rainStrength_ = 1.0f;
                    changed = true;
                }

                --rainDuration_;

                // Chance to start thunder during rain
                if (thunderDuration_ <= 0 && (rng_() % 3000) == 0) {
                    state_ = WeatherState::THUNDER;
                    thunderDuration_ = randomThunderDuration();
                    changed = true;
                }

                // End rain
                if (rainDuration_ <= 0) {
                    state_ = WeatherState::CLEAR;
                    clearDuration_ = randomClearDuration();
                    changed = true;
                }
                break;

            case WeatherState::THUNDER:
                // Ramp up thunder
                if (thunderStrength_ < 1.0f) {
                    thunderStrength_ += 0.01f;
                    if (thunderStrength_ > 1.0f) thunderStrength_ = 1.0f;
                    changed = true;
                }

                // Keep rain at full
                if (rainStrength_ < 1.0f) {
                    rainStrength_ += 0.01f;
                    if (rainStrength_ > 1.0f) rainStrength_ = 1.0f;
                }

                --thunderDuration_;
                --rainDuration_;

                // Random lightning strikes
                if ((rng_() % 100000) == 0 && nextEntityId) {
                    // Random position around 0,0 (simplified — should be near players)
                    double lx = (static_cast<int>(rng_()) % 256) - 128.0;
                    double lz = (static_cast<int>(rng_()) % 256) - 128.0;
                    double ly = 64.0; // Simplified: ground level

                    LightningBolt bolt;
                    bolt.entityId = (*nextEntityId)++;
                    bolt.x = lx;
                    bolt.y = ly;
                    bolt.z = lz;
                    pendingLightning_.push_back(bolt);
                    changed = true;
                }

                // End thunder (rain continues)
                if (thunderDuration_ <= 0) {
                    state_ = WeatherState::RAIN;
                    thunderStrength_ = 0.0f;
                    changed = true;
                }

                // End rain entirely
                if (rainDuration_ <= 0) {
                    state_ = WeatherState::CLEAR;
                    clearDuration_ = randomClearDuration();
                    thunderStrength_ = 0.0f;
                    changed = true;
                }
                break;
        }

        return changed;
    }

    // Get pending lightning bolts from this tick
    const std::vector<LightningBolt>& pendingLightning() const {
        return pendingLightning_;
    }

    // Force weather state (for /weather command)
    void setWeather(WeatherState newState, int duration = -1) {
        state_ = newState;
        switch (newState) {
            case WeatherState::CLEAR:
                clearDuration_ = duration > 0 ? duration : randomClearDuration();
                rainStrength_ = 0.0f;
                thunderStrength_ = 0.0f;
                break;
            case WeatherState::RAIN:
                rainDuration_ = duration > 0 ? duration : randomRainDuration();
                rainStrength_ = 1.0f;
                thunderStrength_ = 0.0f;
                break;
            case WeatherState::THUNDER:
                rainDuration_ = duration > 0 ? duration : randomRainDuration();
                thunderDuration_ = duration > 0 ? duration : randomThunderDuration();
                rainStrength_ = 1.0f;
                thunderStrength_ = 1.0f;
                break;
        }
    }

private:
    WeatherState state_ = WeatherState::CLEAR;
    float rainStrength_ = 0.0f;     // 0.0 = clear, 1.0 = full rain
    float thunderStrength_ = 0.0f;  // 0.0 = no thunder, 1.0 = full thunder

    int clearDuration_ = 0;
    int rainDuration_ = 0;
    int thunderDuration_ = 0;

    std::mt19937 rng_;
    std::vector<LightningBolt> pendingLightning_;

    // Vanilla random durations — from mt.java
    int randomClearDuration() {
        return 12000 + (rng_() % 168001); // 12000-180000 ticks
    }

    int randomRainDuration() {
        return 12000 + (rng_() % 12001);  // 12000-24000 ticks
    }

    int randomThunderDuration() {
        return 3600 + (rng_() % 12001);   // 3600-15600 ticks
    }
};

} // namespace mc
