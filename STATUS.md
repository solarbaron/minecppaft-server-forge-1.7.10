# MineCPPaft Server — Implementation Status

> **Protocol:** Minecraft 1.7.10 (Protocol Version 5)
> **Codebase:** 33 source files, 163 headers (~76K lines total)
> **Build:** C++17, CMake 3.20+, zlib dependency

---

## ✅ Working (Functional & Tested)

### Networking
- **TCP Listener** — accepts connections on port 25565
- **Connection** — per-client read/write threads, VarInt framing, condition-variable-based write queue
- **Handshake** — protocol version check, NextState routing (Status/Login)
- **Status** (Server List Ping) — responds with MOTD, player count, protocol info, ping/pong
- **Login** — offline-mode auth, MD5-based UUID generation, LoginSuccess packet
- **Play State Transition** — Login→Play state change, handler swap
- **Play Packets Handled:** KeepAlive (echo), ChatMessage (with `/` command dispatch), Player/PlayerPosition/PlayerLook/PlayerPosAndLook, ClientSettings, PluginMessage (silently consumed), PlayerAbilities, PlayerDigging (instant break), PlayerBlockPlace (block placement), CreativeInventory (C10, slot set + item drop), ClickWindow (C0E, modes 0-4 with cursor tracking), CloseWindow (drops cursor item)
- **Play Packets Silently Consumed:** HeldItemChange, Animation, EntityAction, ConfirmTransaction, UpdateSign, UseEntity, SteerVehicle, TabComplete, EnchantItem, ClientStatus

### Login Sequence (sent to client on join)
- S01 JoinGame (entity ID, gamemode, dimension, difficulty, max players, level type)
- S05 SpawnPosition
- S39 PlayerAbilities (flags, fly/walk speed)
- S21 ChunkData × 25 (5×5 grid around spawn, zlib compressed)
- S08 PlayerPosAndLook (sent AFTER chunks so terrain loads first)
- S06 UpdateHealth (health, food, saturation — uses Short for food per Java spec)
- S1F SetExperience (XP bar, level, totalXP — uses Short per Java spec)
- S00 KeepAlive (every 15 seconds via tick loop)
- **Dynamic Chunk Streaming** — S21 ChunkData sent/unloaded as player moves (VIEW_DISTANCE=7, 15×15 chunks)

### World
- **ChunkProviderFlat** — superflat generator (bedrock + 2 dirt + grass)
- **ChunkProviderServer** — chunk cache with shared_mutex, load/generate, unload queue
- **WorldServer** — tick loop (world time, day cycle), block get/set, spawn initialization (25×25 pre-gen)
- **Chunk** — 16 sections, biomes, heightMap, NBT serialize/deserialize
- **ChunkSection** — blockLSB/MSB, NibbleArray (metadata, blocklight, skylight), ref counting
- **RegionFile** — Anvil .mca read/write with zlib, sector management
- **GameRules** — string/bool/int rules, defaults

### Blocks & Items
- **Block Registry** — 171 blocks registered with IDs, materials, hardness, light values
- **Item Registry** — 311 items registered with IDs, stack sizes, durability
- **Materials** — Air, Rock, Wood, Cloth, Fire, Water, Lava, Plants, etc.

### Crafting
- **Shaped/Shapeless Recipes** — 90+ crafting recipes registered (all 5 tool tiers, 5 sword tiers, 4 armor sets, building blocks, utility items, redstone, food, storage blocks)
- **Smelting** — 22 smelting recipes with XP values
- **Stack sizes** — per-item max stack sizes (tools/weapons/armor=1, eggs/snowballs/ender pearls=16, default=64)

### Server
- **MinecraftServer** — main tick loop (20 TPS), "Can't keep up" detection, connection management
- **Keep Alive Timer** — sends every 300 ticks (15s) to prevent timeouts
- **WorldInfo** — level name, seed, game type, difficulty, weather, spawn position

---

## 🔶 Partially Implemented (Headers + Stubs)

### Commands (12 commands, chat→command bridge ✅)
| Command | Status |
|---------|--------|
| `/stop` | ✅ Working via chat |
| `/say` | ✅ Working via chat |
| `/help` | ✅ Working via chat |
| `/gamemode` | ✅ Working — sends S2B + tracks gameMode_ server-side |
| `/time` | ✅ Working via chat (no actual time change yet) |
| `/give` | ✅ Working via chat (no actual item give yet) |
| `/tp` | ✅ Working via chat (no actual teleport yet) |
| `/gamerule` | ✅ Working via chat |
| `/difficulty` | ✅ Working via chat |
| `/seed` | ✅ Working via chat |
| `/list` | ✅ Working via chat |
| `/kill` | ✅ Working via chat (no actual kill yet) |

### Entity System
- **Entity base** — ID counter, UUID, position/rotation, bounding box
- **EntityLivingBase** — health, air supply, step height
- **EntityPlayer** — player dimensions, eye height, food stats, game mode
- **EntityPlayerMP** — network player, invulnerability ticks, onUpdate() (food exhaustion only)
- **EntityList** — entity type registry (164 lines)
- **InventoryPlayer** — basic slot structure (125 lines)
- ❌ No entity spawning/despawning in world
- ❌ No entity movement/physics/collision
- ❌ No entity-to-client sync (no S0C/S0E/S12/S14 packets)

### Scoreboard
- Headers + 270 lines of implementation
- Score objectives, teams, display slots
- ❌ Not wired to packets

### Statistics & Achievements
- 159 lines of stat/achievement registration
- ❌ Not tracked or sent to client

### Potions & Enchantments
- **Potions** — 200 lines, effect types registered
- **Enchantments** — 119 lines, enchantment types registered
- ❌ No gameplay effects applied

### AI / Pathfinding
- **EntityAI** — 214 lines, task system skeleton
- **PathFinder** — header-only (508 lines)
- ❌ No mob AI execution

### Redstone
- 451 lines, signal propagation framework
- ❌ Not connected to block updates

### Tile Entities
- 121 lines + 578-line header
- Furnace, Chest, Beacon, Brewing Stand, etc. declared
- ✅ **Chest** — in-memory per-position storage (27 slots), S2D OpenWindow + ClickWindow
- ✅ **Furnace** — in-memory smelting with Java-parity tick/canSmelt/smeltItem, S2D type 2, S31 progress
- ✅ **Ender Chest** — per-player 27-slot storage, block 130 interception
- ✅ **Signs** — per-position text storage, C12 UpdateSign handler, S33 broadcast
- ✅ **Brewing Stand** — per-position 4-slot storage, S2D type 5, S31 brew time
- ✅ **Dispenser** — per-position 9-slot storage, S2D type 3
- ✅ **Dropper** — per-position 9-slot storage, S2D type 6
- ✅ **Hopper** — per-position storage, S2D type 9, 5 slots
- ✅ **Beacon** — S2D type 7, S31 properties (power/effects)
- ✅ **Enchanting Table** — S2D type 4, bookshelf counting, S31 enchantment levels
- ✅ **Anvil** — S2D type 8, repair/rename GUI
- ❌ No tile entity ticking for brewing stand or hopper automation

### Inventory System
- Container hierarchy (679-line header)
- Basic inventory operations (250 lines)
- ❌ No server→client inventory sync (no window packets)
- ✅ Creative inventory action (C10) — set container slots + item drops in creative mode
- ✅ **ClickWindow (C0E)** — modes 0-4: normal click (left/right pickup/place/merge/swap), shift-click (hotbar↔main), number key swap, creative clone, drop from slot. Cursor item tracking + S32 ConfirmTransaction + full window resync
- ✅ CloseWindow (C0D) — drops cursor item on close + workbench grid items
- ✅ Player item drop (Q key) — status 3 (full stack) / 4 (single item), inventory decrement + S2F sync
- ✅ Food consumption — right-click food decrements stack count from hotbar + S2F sync
- ✅ **3×3 Crafting Table (Workbench)** — S2D OpenWindow type 1, 46-slot container, 3×3 CraftingGrid recipe matching + grid consumption, right-click block 58 interception
- ❌ No item pickup/drop in survival mode (only creative)

### Interactive Blocks
- ✅ **Doors** (64) — toggle open/close meta 0x04, both halves sync, open/close sounds
- ✅ **Trapdoors** (96) — toggle meta 0x04
- ✅ **Fence gates** (107) — toggle meta 0x04
- ✅ **Levers** (69) — toggle meta 0x08 with click sound
- ✅ **Buttons** (77/143) — momentary press meta 0x08
- ✅ **Note blocks** (25) — pitch 0-24, harp sound + note particle
- ✅ **Repeaters** (93/94) — delay cycle bits 2-3, 4 states
- ✅ **Comparators** (149/150) — subtract mode toggle bit 0x04
- ✅ **Cake** (92) — eat slices meta 0-6, +2 food per slice
- ✅ **Trapped Chest** (146) — opens like normal chest
- ❌ Pressure plates, daylight sensors, trip wires

### Random Block Ticks
- ✅ **Crop growth** — wheat (59), carrot (141), potato (142) meta 0→7
- ✅ **Sapling growth** — stage bit then oak tree generation
- ✅ **Grass spread** — grass block spreads to adjacent dirt
- ✅ **Farmland hydration** — 9×9 water check, decay without water
- ✅ **Melon/pumpkin stems** — grow, then place fruit adjacent
- ✅ **Leaf decay** — natural leaves without logs (4-block manhattan) decay

### Biomes & World Gen
- **BiomeRegistry** — 355 lines, biome types registered
- **NoiseGen** — 391 lines, Perlin/Simplex noise generators
- **GenLayer** — 745-line header, biome generation layers
- ~~❌ Only flat generator actually works; no terrain/caves/ores~~ ✅ OverworldGenerator with caves + ores

### Forge / JNI Bridge
- 440 lines, JVM initialization framework
- ❌ Not functional (no JVM loaded)

---

## ❌ Not Implemented (Critical Missing Features)

### Networking — Missing Packets
- ~~**Block changes** — S23 BlockChange, S22 MultiBlockChange~~
- **Entity packets** — ~~S0C SpawnPlayer~~, S0E SpawnObject, ~~S0F SpawnMob~~, ~~S12 EntityVelocity~~, S14 Entity, ~~S15 EntityRelMove~~, ~~S16 EntityLook~~, ~~S17 EntityLookRelMove~~, ~~S18 EntityTeleport~~, ~~S19 EntityHeadLook~~, ~~S1A EntityStatus~~, S1B AttachEntity, ~~S1C EntityMetadata~~, ~~S1D EntityEffect~~, ~~S1E RemoveEntityEffect~~
- **Inventory** — ~~S2D OpenWindow~~, ~~S2E CloseWindow~~, ~~S2F SetSlot~~, ~~S30 WindowItems~~, ~~S31 WindowProperty~~, ~~S32 ConfirmTransaction~~
- **Chat** — ✅ S02 ChatMessage (outbound, for command responses and chat broadcast)
- **Player Info** — S38 PlayerListItem (tab list) — packet implemented, not auto-sent
- **Sound/Particles** — ~~S28 Effect~~, ~~S29 SoundEffect~~, ~~S2A Particle~~
- **Explosions** — ✅ S27 Explosion — 5-phase createExplosion, TNT ignition
- **Signs** — ✅ S33 UpdateSign — C12 handler + per-position storage + broadcast
- **Weather** — ✅ S2B ChangeGameState — rain/thunder cycle with timer countdown, strength ramp, client sync on change and join
- **Time** — ✅ S03 TimeUpdate — auto-sent every 20 ticks (day/night cycle working)
- **Health** — ✅ S06 UpdateHealth — sent on login, respawn, and whenever health/food changes
- **Experience** — ✅ S1F SetExperience — sent on login (persisted XP), updated on mining/killing
- **Block changes** — ✅ S23 BlockChange — working, broadcast on break/place
- **Chunk streaming** — ✅ Dynamic S21 send/unload as player moves across chunk boundaries

### Gameplay
- ~~**Block breaking/placing**~~ — ✅ PlayerDigging/PlayerBlockPlace handled; **creative instant-break** (no drops), **survival timed break** (status 0→2 with drop table), hardness-0 instant-break, unbreakable rejection
- ~~**Block drop tables**~~ — ✅ 60+ entries matching Java `getItemDropped` (stone→cobblestone, coal ore→coal, diamond ore→diamond, etc.)
- ~~**Material break sounds**~~ — ✅ 8 sound types (dig.stone/wood/grass/gravel/sand/cloth/snow/glass) mapped by block ID
- ~~**Material place sounds**~~ — ✅ Same 8 sound types used for block placement
- ~~**Survival inventory consumption**~~ — ✅ Block place decrements held stack in survival (creative exempt) + S2F sync
- ~~**Item drops**~~ — ✅ EntityItem spawn on block break with pickup
- ~~**Player-to-player visibility**~~ — ✅ S0C SpawnPlayer + S38 PlayerListItem + S13 DestroyEntities working
- ~~**Combat**~~ — ✅ damage (weapon damage table: swords/axes/picks/shovels × 5 materials), armor reduction (vanilla formula), knockback (S12), hurt animation (S1A), death + respawn
- ~~**Fall damage**~~ — ✅ Server-side: accumulate fallDistance, damage=ceil(dist-3), fall sounds, death message
- ~~**Drowning**~~ — ✅ Air supply (300 ticks = 15s), depletes in water, 2 damage when exhausted, death message
- ~~**Suffocation**~~ — ✅ 1 damage/tick when head inside opaque full-cube block (60+ block whitelist), death message
- ~~**Lava damage**~~ — ✅ 4 damage/tick in lava + set on fire 15s, death: "tried to swim in lava"
- ~~**Fire damage**~~ — ✅ 1 damage/tick in fire block + set on fire 8s, burning 1 dmg/sec, water extinguishes
- ~~**Void damage**~~ — ✅ 4 damage/tick below Y=-64 (applies even in creative), death: "fell out of the world"
- ~~**Health/hunger**~~ — ✅ Full FoodStats system: exhaustion→saturation→hunger drain, natural health regen (food≥18), starvation damage (food=0), movement exhaustion, food eating, NBT save/load
- **Movement validation** — server accepts all positions without anti-cheat; ✅ S18 EntityTeleport + S19 HeadLook broadcast to other players
- **Lighting** — no light propagation engine
- **Gravity/physics** — no server-side player physics (client handles its own)
- **Mob spawning** — ✅ Done (natural hostile spawn near players, S0F SpawnMob, despawn >600 ticks)
- ~~**Functional commands**~~ — ✅ Done (/stop, /gamemode, /time, /give, /tp, /kill, /difficulty, /seed, /list, /say, /gamerule, /weather, /help all with actual game effects)
- ~~**World saving**~~ — ✅ saveAllChunks on shutdown
- ~~**Player data persistence**~~ — ✅ save/load position and rotation via NBT to world/playerdata/<uuid>.dat
- ~~**Tab complete**~~ — ✅ Server-side completion for commands and player names (C14→S3A)
- ~~**Held item tracking**~~ — ✅ C09 HeldItemChange updates player's currentSlot (0-8 validated)
- ~~**Animation broadcast**~~ — ✅ C0A arm swing → S0B broadcast to other players
- ~~**Entity actions**~~ — ✅ C0B sneak/sprint start/stop → S1C metadata flags broadcast

### World Generation
- ~~**Terrain**~~ — ✅ OverworldGenerator with noise-based density + surface replacement
- ~~**Caves**~~ — ✅ MapGenCaves worm carving integrated
- ~~**Ores**~~ — ✅ All 11 vanilla ores (coal, iron, gold, diamond, redstone, lapis, emerald, dirt, gravel)
- **Structures** — no villages, temples, strongholds, dungeons
- ~~**Trees/vegetation**~~ — ✅ Oak tree generation via TreeGenerator (1/10 chance per chunk in plains)
- **Nether/End** — dimensions declared but not generated

---

## 📊 File Size Summary

| Module | .cpp Lines | .h Lines | Notes |
|--------|-----------|----------|-------|
| networking | 1,193 | ~3,000 | Most complete module |
| world | 1,148 | ~4,000 | Chunk I/O + flat gen working |
| block | 815 | ~6,000 | Registry only, no block logic |
| command | 443 | ~1,100 | 13 commands, fully functional with game effects |
| item | 426 | ~1,200 | Registry only |
| forge | 440 | ~1,000 | JNI bridge skeleton |
| redstone | 451 | ~800 | Framework only |
| entity | 489 | ~8,000 | Constructors + position only |
| crafting | 348 | ~300 | Basic recipes working |
| biome | 355 | ~300 | Registry only |
| worldgen | 391 | ~2,500 | Noise + layer headers |
| **Total** | **8,785** | **~67,700** | |

---

## 🎯 Recommended Next Priorities

1. ~~**Chat→Command bridge**~~ — ✅ Done
2. ~~**Block breaking/placing**~~ — ✅ Done
3. ~~**Player visibility**~~ — ✅ Done (S0C SpawnPlayer + S38 tab list + S13 cleanup on disconnect)
4. ~~**Time sync**~~ — ✅ Done (S03 every 20 ticks)
5. ~~**Health sync**~~ — ✅ Done (S06 on login)
6. ~~**Item drops**~~ — ✅ Done (EntityItem spawn on block break + S0E/S1C/S0D/S13 packets + pickup + physics)
7. ~~**Inventory sync**~~ — ✅ Done (S30 WindowItems on join + S2F SetSlot method)
8. ~~**Sound effects**~~ — ✅ Done (S29 SoundEffect for block break/place with broadcastSound)
9. ~~**World save**~~ — ✅ Done (saveAllChunks on shutdown via RegionFile Anvil format)
10. ~~**Terrain generation**~~ — ✅ Done (OverworldGenerator with 7 noise octaves + density field + surface replacement + bedrock)

### New Priorities
11. **Caves + Ores** — ✅ Done (MapGenCaves worm carving + 11 vanilla ores via OreDistribution)
12. **Tree generation** — ✅ Done (TreeGenerator oak trees wired into OverworldGenerator)
13. **Player data persistence** — ✅ Done (savePlayerData/loadPlayerData with NBT Pos/Rotation)
14. **Combat system** — ✅ Done (UseEntity attack, damage, S12 knockback, S1A hurt/death, respawn via ClientStatus)
15. **Mob spawning** — ✅ Done (natural hostile spawn near players, S0F SpawnMob, despawn >600 ticks)
16. **Functional commands** — ✅ Done (/stop, /gamemode, /time, /give, /tp, /kill, /difficulty, /seed, /list, /say, /gamerule, /help)
17. **Tab completion + player actions** — ✅ Done (C14 tab complete, C09 held item, C0A animation, C0B sneak/sprint)
18. **Creative inventory** — ✅ Done (C10 handler with Java parity, gameMode_ tracking, readItemStack parser)
19. **Chunk streaming** — ✅ Done (dynamic S21 send/unload on player movement, VIEW_DISTANCE=7, sorted by distance)
20. **Experience sync** — ✅ Done (S1F SetExperience sent on login)
21. **Protocol audit** — ✅ Done (all 24 packet functions verified against Java references — field widths correct)
22. **Survival block breaking** — ✅ Done (creative/survival split, timed break, 60+ drop table entries, material sounds, exhaustion)
23. **Player item drop** — ✅ Done (Q=single item, Ctrl+Q=full stack, inventory decrement + S2F SetSlot sync)
24. **Food consumption** — ✅ Done (eating food decrements held item stack count + S2F sync)
25. **Survival block place** — ✅ Done (consumes held item in survival, material place sounds)
26. **Fall damage** — ✅ Done (accumulates fallDistance, damage=ceil(dist-3), fall sounds, death message, creative exempt)
27. **Drowning** — ✅ Done (air supply 300 ticks, depletes in water, 2 dmg at exhaustion, death: "drowned")
28. **Suffocation** — ✅ Done (1 dmg/tick in opaque full-cube blocks — fixed with explicit block whitelist)
29. **Fire/lava/void damage** — ✅ Done (lava 4/tick + 15s fire, fire block 1/tick + 8s fire, burning 1/sec, void 4/tick below Y=-64, water extinguishes)
30. **ClickWindow (C0E)** — ✅ Done (modes 0-4: left/right click, shift-click, number key swap, creative clone, drop. Cursor item tracking, S32 confirm, window resync)
31. **Block placement metadata** — ✅ Done (stairs orientation+upside-down, slabs upper/lower, logs axis, torches/ladders wall mount, furnace/chest/pumpkin yaw facing, pistons, buttons, repeaters, anvils)
32. **Entity equipment display** — ✅ Done (S04 EntityEquipment: held item + 4 armor slots broadcast on join + held item change via C09)
33. **Weapon damage + armor reduction** — ✅ Done (getWeaponDamage: 25 weapons with Java-parity damage, getTotalArmorValue: 20 armor pieces, applyArmorCalculations formula, attack exhaustion 0.3)
34. **2×2 crafting grid** — ✅ Done (InventoryCrafting/InventoryCraftResult IInventory, updateCraftingResult via CraftingManager, ingredient decrement on pickup, shift-click support, CloseWindow drops grid items)
35. **Item durability** — ✅ Done (getMaxDurability for 50+ items, damageHeldItem on combat/block break, damageArmor damage/4 per piece, sword 2x on block break, creative exempt)
36. **Item stack sizes + recipe expansion** — ✅ Done (getMaxStackSize per-item lookup: tools/weapons/armor=1, eggs/snowballs=16. Recipes expanded from 11 to 90+: all tools/weapons/armor/building/utility/redstone/food/storage)
37. **XP gain system** — ✅ Done (xpBarCap vanilla formula, addExperience with level-up, mining XP for 6 ore types, kill XP level*7 cap 100, death reset, NBT persist XpP/XpLevel/XpTotal, persisted XP on login)
38. **Weather system** — ✅ Done (updateWeather with Java-parity timer countdown/toggle, ±0.01/tick strength ramp, S2B broadcast: rain start/stop reasons 1/2, rain/thunder intensity reasons 7/8, weather sync on player join)
39. **/weather command** — ✅ Done (clear/rain/thunder modes + optional duration, setWeather server method, tab completion)
40. **3×3 Crafting Table** — ✅ Done (S2D OpenWindow type 1, 46-slot workbench container, CraftingGrid 3×3 recipe matching, grid consumption on craft, close drops grid items, block 58 activation intercept, sneak bypass)
41. **Item pickup to inventory** — ✅ Done (tryPickupItem calls addItemStackToInventory + S30 sync, inventory-full rejection keeps item on ground)
42. **Chest container** — ✅ Done (per-position in-memory storage, S2D OpenWindow type 0, 63-slot container, block 54 interception, left/right click, chest contents preserved, open/close sounds)
43. **Furnace container** — ✅ Done (FurnaceData with Java-parity tick/canSmelt/smeltItem/getItemBurnTime, 12+ fuels, S2D OpenWindow type 2, S31 progress bars, S30 39-slot, furnace ticking in main loop, block 61/62 interception, output-only slot 2)
44. **Ender chest** — ✅ Done (per-player 27-slot enderChestInventory_, reuses chest ClickWindow handler, block 130 interception, isEnderChest_ flag, open/close sounds)
45. **Entity relative movement** — ✅ Done (S15 EntityRelMove, S16 EntityLook, S17 EntityLookRelMove for byte-range deltas <4 blocks; S18 EntityTeleport for large deltas or forced sync every 400 ticks; S19 EntityHeadLook on rotation; last-sent position/rotation tracking)
46. **Chest close sound** — ✅ Done (plays random.chestclose on closeOpenWindow for chest/ender chest windows)
47. **Bed sleeping** — ✅ Done (block 26 right-click: night check time≥12541, S0A UseBed, setWorldTime(0), broadcastTimeUpdate, "Good morning!" chat; day: "You can only sleep at night")
48. **World time ticking** — ✅ Done (worldTime+1 per tick via WorldServer, S03 TimeUpdate broadcast every 20 ticks, tickCounter_ atomic for world age, getWorldTime/getWorldAge public methods)
49. **Explosion system** — ✅ Done (createExplosion 5-phase: 16-ray block destruction with explosion resistance, entity damage/knockback, block drops 1/power chance, S27 Explosion packet with per-player velocity, random.explode sound; TNT ignition with flint & steel item 259 on block 46)
50. **Sign text handling** — ✅ Done (C12 UpdateSign → setSignText per-position storage + S33 broadcast to all players; sendSignToPlayer for chunk load)
51. **Particle broadcast** — ✅ Done (broadcastParticle S2A to all connected players)
52. **Interactive blocks** — ✅ Done (wooden door 64 toggle meta 0x04 both halves, trapdoor 96, fence gate 107, lever 69 toggle meta 0x08, stone/wood button 77/143 press, note block 25 pitch 0-24 + note.harp sound + note particle)
53. **Enchanting table GUI** — ✅ Done (block 116, S2D type 4, Java-parity bookshelf counting in 3×3 ring, simplified enchantment level generation, S31 properties 0-2)
54. **S28 Effect (block break particles)** — ✅ Done (broadcastEffect S28 on all 3 block break paths, effectId 2001 with blockId data)
55. **New containers** — ✅ Done (brewing stand 117 S2D type 5, dispenser 23 type 3, dropper 158 type 6, hopper 154 type 9 — all with per-position storage)
56. **Repeater/comparator/anvil** — ✅ Done (repeater 93/94 delay cycle bits 2-3, comparator 149/150 subtract toggle bit 0x04, anvil 145 S2D type 8 repair GUI)
57. **Trapped chest + cake + beacon** — ✅ Done (trapped chest 146 like normal chest, cake 92 eat slices meta 0-6 +2 food, beacon 138 S2D type 7)
58. **Bucket mechanics** — ✅ Done (water 326 → place water source 9, lava 327 → place lava source 11, empty 325 → pick up water/lava sources, survival consumption)
59. **Item use mechanics** — ✅ Done (bonemeal 351:15 → S28 effect 2005 + grass/crop growth, hoe 290-294 → grass/dirt→farmland 60, flint+steel 259 → fire block 51 placement)
60. **Door item placement** — ✅ Done (item 324→block 64, item 330→block 71, 2-block tall with yaw-facing, upper/lower halves, survival consumption)
61. **Seed planting** — ✅ Done (wheat/carrot/potato/melon/pumpkin seeds on farmland only, item→block mapping for 16 items > 255)
62. **Random block tick** — ✅ Done (crop growth 0→7, sapling→tree, grass spread, farmland hydration/decay, melon/pumpkin fruit placement, leaf decay)
63. **Crop harvest drops** — ✅ Done (wheat→wheat+seeds at meta 7, carrot/potato metadata-aware drops, melon/pumpkin stem→seeds)
64. **Bed item placement** — ✅ Done (item 355→block 26, foot+head 2-block with yaw-facing, meta bit 8 for head part)
65. **Sand/gravel gravity** — ✅ Done (blocks 12/13/145 instant-fall through air/water/lava to solid surface, dig.sand landing sound)
66. **Multi-block break propagation** — ✅ Done (doors 64/71 remove both halves, beds 26 remove both halves, cactus 81/sugar cane 83 chain-break above with drops)
67. **Potion effect system** — ✅ Done (S1D EntityEffect, S1E RemoveEntityEffect, addPotionEffect, tickPotionEffects with regen/poison/wither/saturation per-tick effects)
68. **/effect command** — ✅ Done (/effect <player> <id> [sec] [amp], /effect <player> clear, 14th server command)
69. **Sand/gravel gravity in random tick** — ✅ Done (existing world blocks 12/13 fall through air/liquids during random ticks)
70. **Slab double-slab placement** — ✅ Done (stone slab 44→double 43, wood slab 126→double 125, sub-type from item damage bits 0-2)
