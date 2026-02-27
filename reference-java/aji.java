/*
 * Decompiled with CFR 0.152.
 */
import java.util.List;
import java.util.Random;

public class aji {
    public static final cw c = new cn("air");
    private abt a;
    protected String d;
    public static final ajm e = new ajm("stone", 1.0f, 1.0f);
    public static final ajm f = new ajm("wood", 1.0f, 1.0f);
    public static final ajm g = new ajm("gravel", 1.0f, 1.0f);
    public static final ajm h = new ajm("grass", 1.0f, 1.0f);
    public static final ajm i = new ajm("stone", 1.0f, 1.0f);
    public static final ajm j = new ajm("stone", 1.0f, 1.5f);
    public static final ajm k = new ajj("stone", 1.0f, 1.0f);
    public static final ajm l = new ajm("cloth", 1.0f, 1.0f);
    public static final ajm m = new ajm("sand", 1.0f, 1.0f);
    public static final ajm n = new ajm("snow", 1.0f, 1.0f);
    public static final ajm o = new ajk("ladder", 1.0f, 1.0f);
    public static final ajm p = new ajl("anvil", 0.3f, 1.0f);
    protected boolean q;
    protected int r;
    protected boolean s;
    protected int t;
    protected boolean u;
    protected float v;
    protected float w;
    protected boolean x = true;
    protected boolean y = true;
    protected boolean z;
    protected boolean A;
    protected double B;
    protected double C;
    protected double D;
    protected double E;
    protected double F;
    protected double G;
    public ajm H = e;
    public float I = 1.0f;
    protected final awt J;
    public float K = 0.6f;
    private String b;

    public static int b(aji aji2) {
        return c.b(aji2);
    }

    public static aji e(int n2) {
        return (aji)c.a(n2);
    }

    public static aji a(adb adb2) {
        return aji.e(adb.b(adb2));
    }

    public static aji b(String string) {
        if (c.b(string)) {
            return (aji)c.a(string);
        }
        try {
            return (aji)c.a(Integer.parseInt(string));
        }
        catch (NumberFormatException numberFormatException) {
            return null;
        }
    }

    public boolean j() {
        return this.q;
    }

    public int k() {
        return this.r;
    }

    public int m() {
        return this.t;
    }

    public boolean n() {
        return this.u;
    }

    public awt o() {
        return this.J;
    }

    public awv f(int n2) {
        return this.o().r();
    }

    public static void p() {
        c.a(0, "air", new aja().c("air"));
        c.a(1, "stone", new anv().c(1.5f).b(10.0f).a(i).c("stone").d("stone"));
        c.a(2, "grass", new alh().c(0.6f).a(h).c("grass").d("grass"));
        c.a(3, "dirt", new akl().c(0.5f).a(g).c("dirt").d("dirt"));
        aji aji2 = new aji(awt.e).c(2.0f).b(10.0f).a(i).c("stonebrick").a(abt.b).d("cobblestone");
        c.a(4, "cobblestone", aji2);
        aji aji3 = new aom().c(2.0f).b(5.0f).a(f).c("wood").d("planks");
        c.a(5, "planks", aji3);
        c.a(6, "sapling", new anj().c(0.0f).a(h).c("sapling").d("sapling"));
        c.a(7, "bedrock", new aji(awt.e).s().b(6000000.0f).a(i).c("bedrock").H().a(abt.b).d("bedrock"));
        c.a(8, "flowing_water", new akr(awt.h).c(100.0f).g(3).c("water").H().d("water_flow"));
        c.a(9, "water", new ant(awt.h).c(100.0f).g(3).c("water").H().d("water_still"));
        c.a(10, "flowing_lava", new akr(awt.i).c(100.0f).a(1.0f).c("lava").H().d("lava_flow"));
        c.a(11, "lava", new ant(awt.i).c(100.0f).a(1.0f).c("lava").H().d("lava_still"));
        c.a(12, "sand", new anh().c(0.5f).a(m).c("sand").d("sand"));
        c.a(13, "gravel", new ali().c(0.6f).a(g).c("gravel").d("gravel"));
        c.a(14, "gold_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreGold").d("gold_ore"));
        c.a(15, "iron_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreIron").d("iron_ore"));
        c.a(16, "coal_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreCoal").d("coal_ore"));
        c.a(17, "log", new amm().c("log").d("log"));
        c.a(18, "leaves", new aml().c("leaves").d("leaves"));
        c.a(19, "sponge", new anp().c(0.6f).a(h).c("sponge").d("sponge"));
        c.a(20, "glass", new alf(awt.s, false).c(0.3f).a(k).c("glass").d("glass"));
        c.a(21, "lapis_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreLapis").d("lapis_ore"));
        c.a(22, "lapis_block", new alz(awv.H).c(3.0f).b(5.0f).a(i).c("blockLapis").a(abt.b).d("lapis_block"));
        c.a(23, "dispenser", new akm().c(3.5f).a(i).c("dispenser").d("dispenser"));
        aji aji4 = new ani().a(i).c(0.8f).c("sandStone").d("sandstone");
        c.a(24, "sandstone", aji4);
        c.a(25, "noteblock", new amj().c(0.8f).c("musicBlock").d("noteblock"));
        c.a(26, "bed", new ajh().c(0.2f).c("bed").H().d("bed"));
        c.a(27, "golden_rail", new amt().c(0.7f).a(j).c("goldenRail").d("rail_golden"));
        c.a(28, "detector_rail", new aki().c(0.7f).a(j).c("detectorRail").d("rail_detector"));
        c.a(29, "sticky_piston", new app(true).c("pistonStickyBase"));
        c.a(30, "web", new aok().g(1).c(4.0f).c("web").d("web"));
        c.a(31, "tallgrass", new anz().c(0.0f).a(h).c("tallgrass"));
        c.a(32, "deadbush", new akh().c(0.0f).a(h).c("deadbush").d("deadbush"));
        c.a(33, "piston", new app(false).c("pistonBase"));
        c.a(34, "piston_head", new apq());
        c.a(35, "wool", new aka(awt.n).c(0.8f).a(l).c("cloth").d("wool_colored"));
        c.a(36, "piston_extension", new apr());
        c.a(37, "yellow_flower", new alc(0).c(0.0f).a(h).c("flower1").d("flower_dandelion"));
        c.a(38, "red_flower", new alc(1).c(0.0f).a(h).c("flower2").d("flower_rose"));
        c.a(39, "brown_mushroom", new amc().c(0.0f).a(h).a(0.125f).c("mushroom").d("mushroom_brown"));
        c.a(40, "red_mushroom", new amc().c(0.0f).a(h).c("mushroom").d("mushroom_red"));
        c.a(41, "gold_block", new alz(awv.F).c(3.0f).b(10.0f).a(j).c("blockGold").d("gold_block"));
        c.a(42, "iron_block", new alz(awv.h).c(5.0f).b(10.0f).a(j).c("blockIron").d("iron_block"));
        c.a(43, "double_stone_slab", new any(true).c(2.0f).b(10.0f).a(i).c("stoneSlab"));
        c.a(44, "stone_slab", new any(false).c(2.0f).b(10.0f).a(i).c("stoneSlab"));
        aji aji5 = new aji(awt.e).c(2.0f).b(10.0f).a(i).c("brick").a(abt.b).d("brick");
        c.a(45, "brick_block", aji5);
        c.a(46, "tnt", new aob().c(0.0f).a(h).c("tnt").d("tnt"));
        c.a(47, "bookshelf", new ajp().c(1.5f).a(f).c("bookshelf").d("bookshelf"));
        c.a(48, "mossy_cobblestone", new aji(awt.e).c(2.0f).b(10.0f).a(i).c("stoneMoss").a(abt.b).d("cobblestone_mossy"));
        c.a(49, "obsidian", new amk().c(50.0f).b(2000.0f).a(i).c("obsidian").d("obsidian"));
        c.a(50, "torch", new aoc().c(0.0f).a(0.9375f).a(f).c("torch").d("torch_on"));
        c.a(51, "fire", new alb().c(0.0f).a(1.0f).a(f).c("fire").H().d("fire"));
        c.a(52, "mob_spawner", new ama().c(5.0f).a(j).c("mobSpawner").H().d("mob_spawner"));
        c.a(53, "oak_stairs", new ans(aji3, 0).c("stairsWood"));
        c.a(54, "chest", new ajx(0).c(2.5f).a(f).c("chest"));
        c.a(55, "redstone_wire", new ana().c(0.0f).a(e).c("redstoneDust").H().d("redstone_dust"));
        c.a(56, "diamond_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreDiamond").d("diamond_ore"));
        c.a(57, "diamond_block", new alz(awv.G).c(5.0f).b(10.0f).a(j).c("blockDiamond").d("diamond_block"));
        c.a(58, "crafting_table", new ake().c(2.5f).a(f).c("workbench").d("crafting_table"));
        c.a(59, "wheat", new akf().c("crops").d("wheat"));
        aji aji6 = new aky().c(0.6f).a(g).c("farmland").d("farmland");
        c.a(60, "farmland", aji6);
        c.a(61, "furnace", new ale(false).c(3.5f).a(i).c("furnace").a(abt.c));
        c.a(62, "lit_furnace", new ale(true).c(3.5f).a(i).a(0.875f).c("furnace"));
        c.a(63, "standing_sign", new ank(apm.class, true).c(1.0f).a(f).c("sign").H());
        c.a(64, "wooden_door", new akn(awt.d).c(3.0f).a(f).c("doorWood").H().d("door_wood"));
        c.a(65, "ladder", new als().c(0.4f).a(o).c("ladder").d("ladder"));
        c.a(66, "rail", new amy().c(0.7f).a(j).c("rail").d("rail_normal"));
        c.a(67, "stone_stairs", new ans(aji2, 0).c("stairsStone"));
        c.a(68, "wall_sign", new ank(apm.class, false).c(1.0f).a(f).c("sign").H());
        c.a(69, "lever", new alv().c(0.5f).a(f).c("lever").d("lever"));
        c.a(70, "stone_pressure_plate", new amu("stone", awt.e, amv.b).c(0.5f).a(i).c("pressurePlate"));
        c.a(71, "iron_door", new akn(awt.f).c(5.0f).a(j).c("doorIron").H().d("door_iron"));
        c.a(72, "wooden_pressure_plate", new amu("planks_oak", awt.d, amv.a).c(0.5f).a(f).c("pressurePlate"));
        c.a(73, "redstone_ore", new amz(false).c(3.0f).b(5.0f).a(i).c("oreRedstone").a(abt.b).d("redstone_ore"));
        c.a(74, "lit_redstone_ore", new amz(true).a(0.625f).c(3.0f).b(5.0f).a(i).c("oreRedstone").d("redstone_ore"));
        c.a(75, "unlit_redstone_torch", new anc(false).c(0.0f).a(f).c("notGate").d("redstone_torch_off"));
        c.a(76, "redstone_torch", new anc(true).c(0.0f).a(0.5f).a(f).c("notGate").a(abt.d).d("redstone_torch_on"));
        c.a(77, "stone_button", new anx().c(0.5f).a(i).c("button"));
        c.a(78, "snow_layer", new ann().c(0.1f).a(n).c("snow").g(0).d("snow"));
        c.a(79, "ice", new alp().c(0.5f).g(3).a(k).c("ice").d("ice"));
        c.a(80, "snow", new anm().c(0.2f).a(n).c("snow").d("snow"));
        c.a(81, "cactus", new ajt().c(0.4f).a(l).c("cactus").d("cactus"));
        c.a(82, "clay", new ajy().c(0.6f).a(g).c("clay").d("clay"));
        c.a(83, "reeds", new ane().c(0.0f).a(h).c("reeds").H().d("reeds"));
        c.a(84, "jukebox", new alq().c(2.0f).b(10.0f).a(i).c("jukebox").d("jukebox"));
        c.a(85, "fence", new akz("planks_oak", awt.d).c(2.0f).b(5.0f).a(f).c("fence"));
        aji aji7 = new amw(false).c(1.0f).a(f).c("pumpkin").d("pumpkin");
        c.a(86, "pumpkin", aji7);
        c.a(87, "netherrack", new amg().c(0.4f).a(i).c("hellrock").d("netherrack"));
        c.a(88, "soul_sand", new ano().c(0.5f).a(m).c("hellsand").d("soul_sand"));
        c.a(89, "glowstone", new alg(awt.s).c(0.3f).a(k).a(1.0f).c("lightgem").d("glowstone"));
        c.a(90, "portal", new amp().c(-1.0f).a(k).a(0.75f).c("portal").d("portal"));
        c.a(91, "lit_pumpkin", new amw(true).c(1.0f).a(f).a(1.0f).c("litpumpkin").d("pumpkin"));
        c.a(92, "cake", new aju().c(0.5f).a(l).c("cake").H().d("cake"));
        c.a(93, "unpowered_repeater", new anf(false).c(0.0f).a(f).c("diode").H().d("repeater_off"));
        c.a(94, "powered_repeater", new anf(true).c(0.0f).a(0.625f).a(f).c("diode").H().d("repeater_on"));
        c.a(95, "stained_glass", new anq(awt.s).c(0.3f).a(k).c("stainedGlass").d("glass"));
        c.a(96, "trapdoor", new aoe(awt.d).c(3.0f).a(f).c("trapdoor").H().d("trapdoor"));
        c.a(97, "monster_egg", new amb().c(0.75f).c("monsterStoneEgg"));
        aji aji8 = new anw().c(1.5f).b(10.0f).a(i).c("stonebricksmooth").d("stonebrick");
        c.a(98, "stonebrick", aji8);
        c.a(99, "brown_mushroom_block", new alo(awt.d, 0).c(0.2f).a(f).c("mushroom").d("mushroom_block"));
        c.a(100, "red_mushroom_block", new alo(awt.d, 1).c(0.2f).a(f).c("mushroom").d("mushroom_block"));
        c.a(101, "iron_bars", new aoa("iron_bars", "iron_bars", awt.f, true).c(5.0f).b(10.0f).a(j).c("fenceIron"));
        c.a(102, "glass_pane", new aoa("glass", "glass_pane_top", awt.s, false).c(0.3f).a(k).c("thinGlass"));
        aji aji9 = new aly().c(1.0f).a(f).c("melon").d("melon");
        c.a(103, "melon_block", aji9);
        c.a(104, "pumpkin_stem", new anu(aji7).c(0.0f).a(f).c("pumpkinStem").d("pumpkin_stem"));
        c.a(105, "melon_stem", new anu(aji9).c(0.0f).a(f).c("pumpkinStem").d("melon_stem"));
        c.a(106, "vine", new aoh().c(0.2f).a(h).c("vine").d("vine"));
        c.a(107, "fence_gate", new ala().c(2.0f).b(5.0f).a(f).c("fenceGate"));
        c.a(108, "brick_stairs", new ans(aji5, 0).c("stairsBrick"));
        c.a(109, "stone_brick_stairs", new ans(aji8, 0).c("stairsStoneBrickSmooth"));
        c.a(110, "mycelium", new amd().c(0.6f).a(h).c("mycel").d("mycelium"));
        c.a(111, "waterlily", new aoj().c(0.0f).a(h).c("waterlily").d("waterlily"));
        aji aji10 = new aji(awt.e).c(2.0f).b(10.0f).a(i).c("netherBrick").a(abt.b).d("nether_brick");
        c.a(112, "nether_brick", aji10);
        c.a(113, "nether_brick_fence", new akz("nether_brick", awt.e).c(2.0f).b(10.0f).a(i).c("netherFence"));
        c.a(114, "nether_brick_stairs", new ans(aji10, 0).c("stairsNetherBrick"));
        c.a(115, "nether_wart", new amf().c("netherStalk").d("nether_wart"));
        c.a(116, "enchanting_table", new aks().c(5.0f).b(2000.0f).c("enchantmentTable").d("enchanting_table"));
        c.a(117, "brewing_stand", new ajq().c(0.5f).a(0.125f).c("brewingStand").d("brewing_stand"));
        c.a(118, "cauldron", new ajw().c(2.0f).c("cauldron").d("cauldron"));
        c.a(119, "end_portal", new akt(awt.E).c(-1.0f).b(6000000.0f));
        c.a(120, "end_portal_frame", new aku().a(k).a(0.125f).c(-1.0f).c("endPortalFrame").b(6000000.0f).a(abt.c).d("endframe"));
        c.a(121, "end_stone", new aji(awt.e).c(3.0f).b(15.0f).a(i).c("whiteStone").a(abt.b).d("end_stone"));
        c.a(122, "dragon_egg", new akp().c(3.0f).b(15.0f).a(i).a(0.125f).c("dragonEgg").d("dragon_egg"));
        c.a(123, "redstone_lamp", new anb(false).c(0.3f).a(k).c("redstoneLight").a(abt.d).d("redstone_lamp_off"));
        c.a(124, "lit_redstone_lamp", new anb(true).c(0.3f).a(k).c("redstoneLight").d("redstone_lamp_on"));
        c.a(125, "double_wooden_slab", new aoo(true).c(2.0f).b(5.0f).a(f).c("woodSlab"));
        c.a(126, "wooden_slab", new aoo(false).c(2.0f).b(5.0f).a(f).c("woodSlab"));
        c.a(127, "cocoa", new ajz().c(0.2f).b(5.0f).a(f).c("cocoa").d("cocoa"));
        c.a(128, "sandstone_stairs", new ans(aji4, 0).c("stairsSandStone"));
        c.a(129, "emerald_ore", new amn().c(3.0f).b(5.0f).a(i).c("oreEmerald").d("emerald_ore"));
        c.a(130, "ender_chest", new akv().c(22.5f).b(1000.0f).a(i).c("enderChest").a(0.5f));
        c.a(131, "tripwire_hook", new aog().c("tripWireSource").d("trip_wire_source"));
        c.a(132, "tripwire", new aof().c("tripWire").d("trip_wire"));
        c.a(133, "emerald_block", new alz(awv.I).c(5.0f).b(10.0f).a(j).c("blockEmerald").d("emerald_block"));
        c.a(134, "spruce_stairs", new ans(aji3, 1).c("stairsWoodSpruce"));
        c.a(135, "birch_stairs", new ans(aji3, 2).c("stairsWoodBirch"));
        c.a(136, "jungle_stairs", new ans(aji3, 3).c("stairsWoodJungle"));
        c.a(137, "command_block", new akb().s().b(6000000.0f).c("commandBlock").d("command_block"));
        c.a(138, "beacon", new ajg().c("beacon").a(1.0f).d("beacon"));
        c.a(139, "cobblestone_wall", new aoi(aji2).c("cobbleWall"));
        c.a(140, "flower_pot", new ald().c(0.0f).a(e).c("flowerPot").d("flower_pot"));
        c.a(141, "carrots", new ajv().c("carrots").d("carrots"));
        c.a(142, "potatoes", new amr().c("potatoes").d("potatoes"));
        c.a(143, "wooden_button", new aon().c(0.5f).a(f).c("button"));
        c.a(144, "skull", new anl().c(1.0f).a(i).c("skull").d("skull"));
        c.a(145, "anvil", new ajb().c(5.0f).a(p).b(2000.0f).c("anvil"));
        c.a(146, "trapped_chest", new ajx(1).c(2.5f).a(f).c("chestTrap"));
        c.a(147, "light_weighted_pressure_plate", new aol("gold_block", awt.f, 15).c(0.5f).a(f).c("weightedPlate_light"));
        c.a(148, "heavy_weighted_pressure_plate", new aol("iron_block", awt.f, 150).c(0.5f).a(f).c("weightedPlate_heavy"));
        c.a(149, "unpowered_comparator", new akc(false).c(0.0f).a(f).c("comparator").H().d("comparator_off"));
        c.a(150, "powered_comparator", new akc(true).c(0.0f).a(0.625f).a(f).c("comparator").H().d("comparator_on"));
        c.a(151, "daylight_detector", new akg().c(0.2f).a(f).c("daylightDetector").d("daylight_detector"));
        c.a(152, "redstone_block", new ams(awv.f).c(5.0f).b(10.0f).a(j).c("blockRedstone").d("redstone_block"));
        c.a(153, "quartz_ore", new amn().c(3.0f).b(5.0f).a(i).c("netherquartz").d("quartz_ore"));
        c.a(154, "hopper", new aln().c(3.0f).b(8.0f).a(f).c("hopper").d("hopper"));
        aji aji11 = new amx().a(i).c(0.8f).c("quartzBlock").d("quartz_block");
        c.a(155, "quartz_block", aji11);
        c.a(156, "quartz_stairs", new ans(aji11, 0).c("stairsQuartz"));
        c.a(157, "activator_rail", new amt().c(0.7f).a(j).c("activatorRail").d("rail_activator"));
        c.a(158, "dropper", new akq().c(3.5f).a(i).c("dropper").d("dropper"));
        c.a(159, "stained_hardened_clay", new aka(awt.e).c(1.25f).b(7.0f).a(i).c("clayHardenedStained").d("hardened_clay_stained"));
        c.a(160, "stained_glass_pane", new anr().c(0.3f).a(k).c("thinStainedGlass").d("glass"));
        c.a(161, "leaves2", new amh().c("leaves").d("leaves"));
        c.a(162, "log2", new ami().c("log").d("log"));
        c.a(163, "acacia_stairs", new ans(aji3, 4).c("stairsWoodAcacia"));
        c.a(164, "dark_oak_stairs", new ans(aji3, 5).c("stairsWoodDarkOak"));
        c.a(170, "hay_block", new alm().c(0.5f).a(h).c("hayBlock").a(abt.b).d("hay_block"));
        c.a(171, "carpet", new aop().c(0.1f).a(l).c("woolCarpet").g(0));
        c.a(172, "hardened_clay", new all().c(1.25f).b(7.0f).a(i).c("clayHardened").d("hardened_clay"));
        c.a(173, "coal_block", new aji(awt.e).c(5.0f).b(10.0f).a(i).c("blockCoal").a(abt.b).d("coal_block"));
        c.a(174, "packed_ice", new amo().c(0.5f).a(k).c("icePacked").d("ice_packed"));
        c.a(175, "double_plant", new ako());
        for (aji aji12 : c) {
            boolean bl2;
            if (aji12.J == awt.a) {
                aji12.u = false;
                continue;
            }
            boolean bl3 = false;
            boolean bl4 = aji12.b() == 10;
            boolean bl5 = aji12 instanceof alj;
            boolean bl6 = aji12 == aji6;
            boolean bl7 = aji12.s;
            boolean bl8 = bl2 = aji12.r == 0;
            if (bl4 || bl5 || bl6 || bl7 || bl2) {
                bl3 = true;
            }
            aji12.u = bl3;
        }
    }

    protected aji(awt awt2) {
        this.J = awt2;
        this.a(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        this.q = this.c();
        this.r = this.c() ? 255 : 0;
        this.s = !awt2.b();
    }

    protected aji a(ajm ajm2) {
        this.H = ajm2;
        return this;
    }

    protected aji g(int n2) {
        this.r = n2;
        return this;
    }

    protected aji a(float f2) {
        this.t = (int)(15.0f * f2);
        return this;
    }

    protected aji b(float f2) {
        this.w = f2 * 3.0f;
        return this;
    }

    public boolean r() {
        return this.J.k() && this.d() && !this.f();
    }

    public boolean d() {
        return true;
    }

    public boolean b(ahl ahl2, int n2, int n3, int n4) {
        return !this.J.c();
    }

    public int b() {
        return 0;
    }

    protected aji c(float f2) {
        this.v = f2;
        if (this.w < f2 * 5.0f) {
            this.w = f2 * 5.0f;
        }
        return this;
    }

    protected aji s() {
        this.c(-1.0f);
        return this;
    }

    public float f(ahb ahb2, int n2, int n3, int n4) {
        return this.v;
    }

    protected aji a(boolean bl2) {
        this.z = bl2;
        return this;
    }

    public boolean t() {
        return this.z;
    }

    public boolean u() {
        return this.A;
    }

    protected final void a(float f2, float f3, float f4, float f5, float f6, float f7) {
        this.B = f2;
        this.C = f3;
        this.D = f4;
        this.E = f5;
        this.F = f6;
        this.G = f7;
    }

    public boolean d(ahl ahl2, int n2, int n3, int n4, int n5) {
        return ahl2.a(n2, n3, n4).o().a();
    }

    public void a(ahb ahb2, int n2, int n3, int n4, azt azt2, List list, sa sa2) {
        azt azt3 = this.a(ahb2, n2, n3, n4);
        if (azt3 != null && azt2.b(azt3)) {
            list.add(azt3);
        }
    }

    public azt a(ahb ahb2, int n2, int n3, int n4) {
        return azt.a((double)n2 + this.B, (double)n3 + this.C, (double)n4 + this.D, (double)n2 + this.E, (double)n3 + this.F, (double)n4 + this.G);
    }

    public boolean c() {
        return true;
    }

    public boolean a(int n2, boolean bl2) {
        return this.v();
    }

    public boolean v() {
        return true;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, Random random) {
    }

    public void b(ahb ahb2, int n2, int n3, int n4, int n5) {
    }

    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2) {
    }

    public int a(ahb ahb2) {
        return 10;
    }

    public void b(ahb ahb2, int n2, int n3, int n4) {
    }

    public void a(ahb ahb2, int n2, int n3, int n4, aji aji2, int n5) {
    }

    public int a(Random random) {
        return 1;
    }

    public adb a(int n2, Random random, int n3) {
        return adb.a(this);
    }

    public float a(yz yz2, ahb ahb2, int n2, int n3, int n4) {
        float f2 = this.f(ahb2, n2, n3, n4);
        if (f2 < 0.0f) {
            return 0.0f;
        }
        if (!yz2.a(this)) {
            return yz2.a(this, false) / f2 / 100.0f;
        }
        return yz2.a(this, true) / f2 / 30.0f;
    }

    public final void b(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        this.a(ahb2, n2, n3, n4, n5, 1.0f, n6);
    }

    public void a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, int n6) {
        if (ahb2.E) {
            return;
        }
        int n7 = this.a(n6, ahb2.s);
        for (int i2 = 0; i2 < n7; ++i2) {
            adb adb2;
            if (ahb2.s.nextFloat() > f2 || (adb2 = this.a(n5, ahb2.s, n6)) == null) continue;
            this.a(ahb2, n2, n3, n4, new add(adb2, 1, this.a(n5)));
        }
    }

    protected void a(ahb ahb2, int n2, int n3, int n4, add add2) {
        if (ahb2.E || !ahb2.O().b("doTileDrops")) {
            return;
        }
        float f2 = 0.7f;
        double d2 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.5;
        double d3 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.5;
        double d4 = (double)(ahb2.s.nextFloat() * f2) + (double)(1.0f - f2) * 0.5;
        xk xk2 = new xk(ahb2, (double)n2 + d2, (double)n3 + d3, (double)n4 + d4, add2);
        xk2.b = 10;
        ahb2.d(xk2);
    }

    protected void c(ahb ahb2, int n2, int n3, int n4, int n5) {
        if (!ahb2.E) {
            while (n5 > 0) {
                int n6 = sq.a(n5);
                n5 -= n6;
                ahb2.d(new sq(ahb2, (double)n2 + 0.5, (double)n3 + 0.5, (double)n4 + 0.5, n6));
            }
        }
    }

    public int a(int n2) {
        return 0;
    }

    public float a(sa sa2) {
        return this.w / 5.0f;
    }

    public azu a(ahb ahb2, int n2, int n3, int n4, azw azw2, azw azw3) {
        this.a((ahl)ahb2, n2, n3, n4);
        azw2 = azw2.c(-n2, -n3, -n4);
        azw3 = azw3.c(-n2, -n3, -n4);
        azw azw4 = azw2.b(azw3, this.B);
        azw azw5 = azw2.b(azw3, this.E);
        azw azw6 = azw2.c(azw3, this.C);
        azw azw7 = azw2.c(azw3, this.F);
        azw azw8 = azw2.d(azw3, this.D);
        azw azw9 = azw2.d(azw3, this.G);
        if (!this.a(azw4)) {
            azw4 = null;
        }
        if (!this.a(azw5)) {
            azw5 = null;
        }
        if (!this.b(azw6)) {
            azw6 = null;
        }
        if (!this.b(azw7)) {
            azw7 = null;
        }
        if (!this.c(azw8)) {
            azw8 = null;
        }
        if (!this.c(azw9)) {
            azw9 = null;
        }
        azw azw10 = null;
        if (azw4 != null && (azw10 == null || azw2.e(azw4) < azw2.e(azw10))) {
            azw10 = azw4;
        }
        if (azw5 != null && (azw10 == null || azw2.e(azw5) < azw2.e(azw10))) {
            azw10 = azw5;
        }
        if (azw6 != null && (azw10 == null || azw2.e(azw6) < azw2.e(azw10))) {
            azw10 = azw6;
        }
        if (azw7 != null && (azw10 == null || azw2.e(azw7) < azw2.e(azw10))) {
            azw10 = azw7;
        }
        if (azw8 != null && (azw10 == null || azw2.e(azw8) < azw2.e(azw10))) {
            azw10 = azw8;
        }
        if (azw9 != null && (azw10 == null || azw2.e(azw9) < azw2.e(azw10))) {
            azw10 = azw9;
        }
        if (azw10 == null) {
            return null;
        }
        int n5 = -1;
        if (azw10 == azw4) {
            n5 = 4;
        }
        if (azw10 == azw5) {
            n5 = 5;
        }
        if (azw10 == azw6) {
            n5 = 0;
        }
        if (azw10 == azw7) {
            n5 = 1;
        }
        if (azw10 == azw8) {
            n5 = 2;
        }
        if (azw10 == azw9) {
            n5 = 3;
        }
        return new azu(n2, n3, n4, n5, azw10.c(n2, n3, n4));
    }

    private boolean a(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.b >= this.C && azw2.b <= this.F && azw2.c >= this.D && azw2.c <= this.G;
    }

    private boolean b(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.a >= this.B && azw2.a <= this.E && azw2.c >= this.D && azw2.c <= this.G;
    }

    private boolean c(azw azw2) {
        if (azw2 == null) {
            return false;
        }
        return azw2.a >= this.B && azw2.a <= this.E && azw2.b >= this.C && azw2.b <= this.F;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, agw agw2) {
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, add add2) {
        return this.d(ahb2, n2, n3, n4, n5);
    }

    public boolean d(ahb ahb2, int n2, int n3, int n4, int n5) {
        return this.c(ahb2, n2, n3, n4);
    }

    public boolean c(ahb ahb2, int n2, int n3, int n4) {
        return ahb2.a((int)n2, (int)n3, (int)n4).J.j();
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4, yz yz2, int n5, float f2, float f3, float f4) {
        return false;
    }

    public void b(ahb ahb2, int n2, int n3, int n4, sa sa2) {
    }

    public int a(ahb ahb2, int n2, int n3, int n4, int n5, float f2, float f3, float f4, int n6) {
        return n6;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, yz yz2) {
    }

    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2, azw azw2) {
    }

    public void a(ahl ahl2, int n2, int n3, int n4) {
    }

    public final double x() {
        return this.B;
    }

    public final double y() {
        return this.E;
    }

    public final double z() {
        return this.C;
    }

    public final double A() {
        return this.F;
    }

    public final double B() {
        return this.D;
    }

    public final double C() {
        return this.G;
    }

    public int b(ahl ahl2, int n2, int n3, int n4, int n5) {
        return 0;
    }

    public boolean f() {
        return false;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2) {
    }

    public int c(ahl ahl2, int n2, int n3, int n4, int n5) {
        return 0;
    }

    public void g() {
    }

    public void a(ahb ahb2, yz yz2, int n2, int n3, int n4, int n5) {
        yz2.a(pp.C[aji.b(this)], 1);
        yz2.a(0.025f);
        if (this.E() && afv.e(yz2)) {
            add add2 = this.j(n5);
            if (add2 != null) {
                this.a(ahb2, n2, n3, n4, add2);
            }
        } else {
            int n6 = afv.f(yz2);
            this.b(ahb2, n2, n3, n4, n5, n6);
        }
    }

    protected boolean E() {
        return this.d() && !this.A;
    }

    protected add j(int n2) {
        int n3 = 0;
        adb adb2 = adb.a(this);
        if (adb2 != null && adb2.n()) {
            n3 = n2;
        }
        return new add(adb2, 1, n3);
    }

    public int a(int n2, Random random) {
        return this.a(random);
    }

    public boolean j(ahb ahb2, int n2, int n3, int n4) {
        return true;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, sv sv2, add add2) {
    }

    public void e(ahb ahb2, int n2, int n3, int n4, int n5) {
    }

    public aji c(String string) {
        this.b = string;
        return this;
    }

    public String F() {
        return dd.a(this.a() + ".name");
    }

    public String a() {
        return "tile." + this.b;
    }

    public boolean a(ahb ahb2, int n2, int n3, int n4, int n5, int n6) {
        return false;
    }

    public boolean G() {
        return this.y;
    }

    protected aji H() {
        this.y = false;
        return this;
    }

    public int h() {
        return this.J.m();
    }

    public void a(ahb ahb2, int n2, int n3, int n4, sa sa2, float f2) {
    }

    public int k(ahb ahb2, int n2, int n3, int n4) {
        return this.a(ahb2.e(n2, n3, n4));
    }

    public aji a(abt abt2) {
        this.a = abt2;
        return this;
    }

    public void a(ahb ahb2, int n2, int n3, int n4, int n5, yz yz2) {
    }

    public void f(ahb ahb2, int n2, int n3, int n4, int n5) {
    }

    public void l(ahb ahb2, int n2, int n3, int n4) {
    }

    public boolean L() {
        return true;
    }

    public boolean a(agw agw2) {
        return true;
    }

    public boolean c(aji aji2) {
        return this == aji2;
    }

    public static boolean a(aji aji2, aji aji3) {
        if (aji2 == null || aji3 == null) {
            return false;
        }
        if (aji2 == aji3) {
            return true;
        }
        return aji2.c(aji3);
    }

    public boolean M() {
        return false;
    }

    public int g(ahb ahb2, int n2, int n3, int n4, int n5) {
        return 0;
    }

    protected aji d(String string) {
        this.d = string;
        return this;
    }
}

