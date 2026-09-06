// Safe at startup or CallLocalFunction("V4_RunSafetySelfTest", ""): local values only.
forward V4_RunSafetySelfTest();
stock V4_SafetyAssert(bool:condition, const title[], &passed, &failed)
{
    if (condition) passed++;
    else { failed++; printf("[V4 SAFETY FAIL] %s", title); }
}
public V4_RunSafetySelfTest()
{
    new passed, failed, target, amount, level, xp, meters, publicVehicle = INVALID_VEHICLE_ID;
    new Float:fraction;
    V4_SafetyAssert(V4_ParseGrant(" 0 1", 0, 99, 1000000, target, amount) && target == 0 && amount == 1, "id zero", passed, failed);
    V4_SafetyAssert(V4_ParseGrant("\t99\t1000000  ", 0, 99, 1000000, target, amount) && amount == 1000000, "limits whitespace", passed, failed);
    static const bad[][] = {"", " ", "1", "1 ", "1 0", "-1 10", "+1 10", "1 -10", "1 +10", "1 1.0", "1 1e2", "1 0x10", "1 20x", "1 20 extra", "100 1", "1 1000001", "2147483648 1", "1 999999999999999999999999", "1 10\n"};
    for (new i = 0; i < sizeof(bad); i++)
        V4_SafetyAssert(!V4_ParseGrant(bad[i], 0, 99, 1000000, target, amount), bad[i], passed, failed);
    new index;
    V4_SafetyAssert(V4_ParseUInt("2147483647", index, V4_CELL_MAX, amount) && amount == V4_CELL_MAX, "cell max parser", passed, failed);
    V4_SafetyAssert(V4_CanGrantMoney(V4_CELL_MAX-1, 1), "money exact max", passed, failed);
    V4_SafetyAssert(!V4_CanGrantMoney(V4_CELL_MAX, 1), "money overflow", passed, failed);
    V4_SafetyAssert(!V4_CanGrantMoney(0, 1000001) && !V4_CanGrantMoney(0, 0), "money caps", passed, failed);
    // Entire permission truth table, including RCON without login and NPC RCON.
    for (new mask = 0; mask < 64; mask++)
    {
        new bool:c = (mask & 1) != 0, bool:l = (mask & 2) != 0, bool:a = (mask & 4) != 0;
        new bool:n = (mask & 8) != 0, admin = (mask & 16) ? 1 : 0, bool:r = (mask & 32) != 0;
        new bool:expected = (mask & 15) == 7 && (mask & 48) != 0;
        V4_SafetyAssert(V4_GrantPermission(c,l,a,n,admin,r) == expected, "permission matrix", passed, failed);
    }
    V4_SafetyAssert(V4_PlanXP(1,0,99,level,xp) && level == 1 && xp == 99, "XP below threshold", passed, failed);
    V4_SafetyAssert(V4_PlanXP(1,99,1,level,xp) && level == 2 && xp == 0, "XP promotion", passed, failed);
    V4_SafetyAssert(V4_PlanXP(1,0,100000,level,xp) && level == 45 && xp == 1000, "XP max grant bounded", passed, failed);
    V4_SafetyAssert(!V4_PlanXP(0,0,1,level,xp) && !V4_PlanXP(1,-1,1,level,xp), "XP corrupt state", passed, failed);
    V4_SafetyAssert(!V4_PlanXP(1,V4_CELL_MAX,1,level,xp), "XP overflow state", passed, failed);
    V4_SafetyAssert(!V4_PlanXP(1,0,100001,level,xp), "XP amount cap", passed, failed);
    V4_SafetyAssert(!V4_PlanXP(V4_LEVEL_MAX,99999999,1,level,xp), "XP level cap", passed, failed);
    V4_SafetyAssert(V4_CalculateFuelCost(100)==0 && V4_CalculateFuelCost(101)==0,"fuel full",passed,failed);
    V4_SafetyAssert(V4_CalculateFuelCost(99)==13 && V4_CalculateFuelCost(98)==13,"fuel first pair",passed,failed);
    V4_SafetyAssert(V4_CalculateFuelCost(81)==130 && V4_CalculateFuelCost(80)==130,"fuel pair rounding",passed,failed);
    V4_SafetyAssert(V4_CalculateFuelCost(0)==650 && V4_CalculateFuelCost(-5)==650,"fuel empty clamp",passed,failed);
    V4_SafetyAssert(V4_UsesFuelKind(V4_VEHICLE_KIND_PUBLIC), "public vehicle uses fuel", passed, failed);
    V4_SafetyAssert(V4_UsesFuelKind(V4_VEHICLE_KIND_PRIVATE), "private vehicle uses fuel", passed, failed);
    V4_SafetyAssert(!V4_UsesFuelKind(V4_VEHICLE_KIND_TEMP), "temporary vehicle ignores fuel", passed, failed);
    V4_SafetyAssert(V4_CanRefuelKind(V4_VEHICLE_KIND_PUBLIC, false), "public refuel allowed", passed, failed);
    V4_SafetyAssert(!V4_CanRefuelKind(V4_VEHICLE_KIND_PRIVATE, false) && V4_CanRefuelKind(V4_VEHICLE_KIND_PRIVATE, true), "private refuel owner only", passed, failed);
    for (new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
        if (GetVehicleModel(vehicleid) != 0 && g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PUBLIC) { publicVehicle = vehicleid; break; }
    new oldPublicFuel = (publicVehicle == INVALID_VEHICLE_ID) ? -1 : V4_GetVehicleFuel(publicVehicle);
    V4_SafetyAssert(publicVehicle != INVALID_VEHICLE_ID && oldPublicFuel == 100, "live public fuel initialized", passed, failed);
    new bool:setPublicFuel = publicVehicle != INVALID_VEHICLE_ID && V4_SetVehicleFuel(publicVehicle, 77);
    V4_SafetyAssert(setPublicFuel && V4_GetVehicleFuel(publicVehicle) == 77, "live public fuel mutation", passed, failed);
    new bool:restorePublicFuel = publicVehicle != INVALID_VEHICLE_ID && V4_SetVehicleFuel(publicVehicle, oldPublicFuel);
    V4_SafetyAssert(restorePublicFuel && V4_GetVehicleFuel(publicVehicle) == 100, "live public fuel restore", passed, failed);
    V4_SafetyAssert(V4_FUEL_STATION_COUNT == 17 && sizeof(g_V4FuelStationPos) == 17, "all fuel stations configured", passed, failed);
    V4_SafetyAssert(g_V4FuelStationPos[16][0] == 2113.7 && g_V4FuelStationPos[16][1] == 920.1, "fuel station table tail", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,3.0,4.0,12.0,1000) == 13.0, "3D distance", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,0.0,0.0,0.0,1000) == 0.0, "stationary", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,1000.0,0.0,0.0,1000) == 0.0, "teleport", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,1.0,0.0,0.0,1501) == 0.0, "sync gap", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,1.0,0.0,0.0,0) == 0.0, "zero interval", passed, failed);
    V4_SafetyAssert(V4_OdoDistance(0.0,0.0,0.0,1.0,0.0,0.0,-1) == 0.0, "negative interval", passed, failed);
    for (new i = 0; i < 100; i++) V4_OdoAccumulate(meters, fraction, 0.25);
    V4_SafetyAssert(meters == 25 && fraction == 0.0, "fraction preservation", passed, failed);
    V4_OdoAccumulate(meters, fraction, 0.75);
    V4_SafetyAssert(meters == 25 && fraction == 0.75, "fraction pending", passed, failed);
    meters = V4_CELL_MAX - 1;
    V4_OdoAccumulate(meters, fraction, 13.0);
    V4_SafetyAssert(meters == V4_CELL_MAX && fraction == 0.0, "odometer saturates", passed, failed);
    V4_SafetyAssert(V4_OdoContinuity(true,1,1,2,2,0,0,0,0,100), "continuous sample", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(false,1,1,2,2,0,0,0,0,100), "respawn reset", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(true,1,3,2,2,0,0,0,0,100), "vehicle change", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(true,1,1,2,3,0,0,0,0,100), "driver change", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(true,1,1,2,2,0,1,0,0,100), "world change", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(true,1,1,2,2,0,0,0,1,100), "interior change", passed, failed);
    V4_SafetyAssert(!V4_OdoContinuity(true,1,1,2,2,0,0,0,0,1501), "continuity gap", passed, failed);
    printf("[V4 SAFETY SELFTEST] passed=%d failed=%d", passed, failed);
    return failed == 0;
}
