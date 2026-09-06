// Pure functions shared by production and deterministic self-tests. No accounts/natives with side effects.
#define V4_CELL_MAX (2147483647)
#define V4_GRANT_MONEY_MAX (1000000)
#define V4_GRANT_XP_MAX (100000)
#define V4_LEVEL_MAX (1000000)
#define V4_ODO_MAX_GAP (1500)
#define V4_ODO_MIN_SAMPLE (100)

stock V4_CalculateFuelCost(fuel)
{
    if (fuel >= 100) return 0;
    if (fuel < 0) fuel = 0;
    return ((100 - fuel + 1) / 2) * 13;
}

stock bool:V4_UsesFuelKind(kind)
{
    return kind == V4_VEHICLE_KIND_PUBLIC || kind == V4_VEHICLE_KIND_PRIVATE;
}

stock bool:V4_CanRefuelKind(kind, bool:isOwner)
{
    return kind == V4_VEHICLE_KIND_PUBLIC || (kind == V4_VEHICLE_KIND_PRIVATE && isOwner);
}

stock bool:V4_GrantPermission(bool:connected, bool:logged, bool:account, bool:npc, admin, bool:rcon)
{
    return connected && logged && account && !npc && (admin >= 1 || rcon);
}

stock bool:V4_GrantSpace(c)
{
    return c == ' ' || c == '\t';
}

stock bool:V4_ParseUInt(const text[], &index, maximum, &value)
{
    value = 0;
    while (V4_GrantSpace(text[index])) index++;
    if (text[index] < '0' || text[index] > '9') return false;
    while (text[index] >= '0' && text[index] <= '9')
    {
        new digit = text[index++] - '0';
        if (value > maximum / 10 || (value == maximum / 10 && digit > maximum % 10)) return false;
        value = value * 10 + digit;
    }
    return text[index] == 0 || V4_GrantSpace(text[index]);
}

stock bool:V4_ParseGrant(const text[], index, maxid, maximum, &target, &amount)
{
    if (!V4_ParseUInt(text, index, maxid, target)) return false;
    if (!V4_GrantSpace(text[index])) return false;
    if (!V4_ParseUInt(text, index, maximum, amount) || amount == 0) return false;
    while (V4_GrantSpace(text[index])) index++;
    return text[index] == 0;
}

stock bool:V4_CanGrantMoney(balance, amount)
{
    return amount > 0 && amount <= V4_GRANT_MONEY_MAX && balance <= V4_CELL_MAX - amount;
}

// Normalized XP and bounded amount mean at most 45 promotions per invocation.
// Invalid/corrupt account state fails closed rather than entering an unbounded while loop.
stock bool:V4_PlanXP(level, xp, amount, &nextLevel, &nextXP)
{
    if (level < 1 || level > V4_LEVEL_MAX || xp < 0 || xp >= level * 100) return false;
    if (amount <= 0 || amount > V4_GRANT_XP_MAX || xp > V4_CELL_MAX - amount) return false;
    nextLevel = level;
    nextXP = xp + amount;
    for (new steps = 0; steps < 64; steps++)
    {
        if (nextXP < nextLevel * 100) return true;
        if (nextLevel >= V4_LEVEL_MAX) return false;
        nextXP -= nextLevel * 100;
        nextLevel++;
    }
    return false;
}

stock bool:V4_OdoContinuity(bool:valid, vehicle, lastVehicle, driver, lastDriver,
    world, lastWorld, interior, lastInterior, elapsed)
{
    return valid && vehicle == lastVehicle && driver == lastDriver && world == lastWorld &&
        interior == lastInterior && elapsed > 0 && elapsed <= V4_ODO_MAX_GAP;
}

stock Float:V4_OdoDistance(Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2, elapsed)
{
    if (elapsed <= 0 || elapsed > V4_ODO_MAX_GAP) return 0.0;
    new Float:dx = x2-x1, Float:dy = y2-y1, Float:dz = z2-z1;
    // Reject NaN/infinity before sqrt; 140 m/s + 3 m sync tolerance (also allows aircraft).
    new Float:limit = 140.0 * float(elapsed) / 1000.0 + 3.0;
    if (!(floatabs(dx) <= limit && floatabs(dy) <= limit && floatabs(dz) <= limit)) return 0.0;
    new Float:distance = floatsqroot(dx*dx + dy*dy + dz*dz);
    if (!(distance <= limit)) return 0.0;
    return distance;
}

stock V4_OdoAccumulate(&meters, &Float:fraction, Float:distance)
{
    if (!(distance > 0.0 && distance <= 213.0)) return 0;
    new Float:total = fraction + distance;
    new whole = floatround(total, floatround_floor);
    if (meters >= V4_CELL_MAX - whole)
    {
        meters = V4_CELL_MAX;
        fraction = 0.0;
    }
    else
    {
        meters += whole;
        fraction = total - float(whole);
    }
    return 1;
}
