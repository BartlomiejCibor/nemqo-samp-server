#include <nemqo_v4_safety_core>

enum E_V4_ODO_SAMPLE
{
    bool:odoValid, odoVehicle, odoDriver, odoTick, odoWorld, odoInterior,
    Float:odoX, Float:odoY, Float:odoZ
};
new g_V4OdoSample[MAX_PLAYERS][E_V4_ODO_SAMPLE];
new Float:g_V4OdoFraction[MAX_PLAYERS];
new g_V4OdoSaveTick[MAX_PLAYERS];
new g_V4FleetOdoSample[MAX_VEHICLES][E_V4_ODO_SAMPLE];
new g_V4FleetOdometer[MAX_VEHICLES];
new Float:g_V4FleetOdoFraction[MAX_VEHICLES];
new bool:g_V4GrantUsed[MAX_PLAYERS];
new g_V4GrantTick[MAX_PLAYERS];

stock V4_VehicleTrackingInit()
{
    for (new vehicleid = 0; vehicleid < MAX_VEHICLES; vehicleid++)
    {
        g_V4VehicleLabel[vehicleid] = Text3D:INVALID_3DTEXT_ID;
        g_V4VehicleLabelWorld[vehicleid] = 0;
        g_V4VehicleLabelOwner[vehicleid] = -1;
        g_V4VehicleLabelDeciKm[vehicleid] = -1;
        g_V4VehicleLabelFuel[vehicleid] = -1;
        g_V4VehicleLabelKind[vehicleid] = V4_VEHICLE_KIND_NONE;
        g_V4VehicleKind[vehicleid] = V4_VEHICLE_KIND_NONE;
        g_V4PublicFuel[vehicleid] = 100;
        g_V4FleetOdometer[vehicleid] = 0;
        g_V4FleetOdoFraction[vehicleid] = 0.0;
        g_V4FleetOdoSample[vehicleid][odoValid] = false;
    }
    return 1;
}

stock V4_DestroyVehicleLabel(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    if (g_V4VehicleLabel[vehicleid] != Text3D:INVALID_3DTEXT_ID)
        Delete3DTextLabel(g_V4VehicleLabel[vehicleid]);
    g_V4VehicleLabel[vehicleid] = Text3D:INVALID_3DTEXT_ID;
    g_V4VehicleLabelWorld[vehicleid] = 0;
    g_V4VehicleLabelOwner[vehicleid] = -1;
    g_V4VehicleLabelDeciKm[vehicleid] = -1;
    g_V4VehicleLabelFuel[vehicleid] = -1;
    g_V4VehicleLabelKind[vehicleid] = V4_VEHICLE_KIND_NONE;
    return 1;
}

stock V4_GetVehicleFuel(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 100;
    if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PUBLIC) return g_V4PublicFuel[vehicleid];
    if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PRIVATE)
    {
        new owner = g_V4VehicleOwner[vehicleid] - 1;
        if (owner >= 0 && owner < MAX_PLAYERS && g_V4OwnedVehicle[owner] == vehicleid) return g_V4Fuel[owner];
    }
    return 100;
}

stock V4_SetVehicleFuel(vehicleid, fuel)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    if (fuel < 0) fuel = 0;
    if (fuel > 100) fuel = 100;
    if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PUBLIC)
    {
        g_V4PublicFuel[vehicleid] = fuel;
        return 1;
    }
    if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PRIVATE)
    {
        new owner = g_V4VehicleOwner[vehicleid] - 1;
        if (owner >= 0 && owner < MAX_PLAYERS && g_V4OwnedVehicle[owner] == vehicleid)
        {
            g_V4Fuel[owner] = fuel;
            return 1;
        }
    }
    return 0;
}

stock V4_IsFuelVehicle(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    return V4_UsesFuelKind(g_V4VehicleKind[vehicleid]);
}

stock V4_CreateVehicleLabel(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES || GetVehicleModel(vehicleid) == 0) return 0;
    V4_DestroyVehicleLabel(vehicleid);
    g_V4VehicleLabelWorld[vehicleid] = GetVehicleVirtualWorld(vehicleid);
    g_V4VehicleLabel[vehicleid] = Create3DTextLabel("Model GTA", 0xFFFFFFFF,
        0.0, 0.0, 0.0, 15.0, g_V4VehicleLabelWorld[vehicleid], 0);
    if (g_V4VehicleLabel[vehicleid] == Text3D:INVALID_3DTEXT_ID) return 0;
    Attach3DTextLabelToVehicle(g_V4VehicleLabel[vehicleid], vehicleid, 0.0, 0.0, 1.35);
    return 1;
}

stock V4_UpdateVehicleLabel(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES || GetVehicleModel(vehicleid) == 0) return 0;
    new world = GetVehicleVirtualWorld(vehicleid);
    if (g_V4VehicleLabel[vehicleid] == Text3D:INVALID_3DTEXT_ID || g_V4VehicleLabelWorld[vehicleid] != world)
        if (!V4_CreateVehicleLabel(vehicleid)) return 0;

    new text[192], modelid = GetVehicleModel(vehicleid), kind = g_V4VehicleKind[vehicleid];
    new ownerToken = g_V4VehicleOwner[vehicleid], owner = ownerToken - 1, deciKm, fuel = V4_GetVehicleFuel(vehicleid);
    if (kind == V4_VEHICLE_KIND_PRIVATE && owner >= 0 && owner < MAX_PLAYERS && IsPlayerConnected(owner) && g_Logged[owner] &&
        g_AccountExists[owner] && g_V4OwnedVehicle[owner] == vehicleid)
    {
        deciKm = (g_V4Odometer[owner] / 100);
        if (g_V4VehicleLabelOwner[vehicleid] == ownerToken && g_V4VehicleLabelDeciKm[vehicleid] == deciKm &&
            g_V4VehicleLabelFuel[vehicleid] == fuel && g_V4VehicleLabelKind[vehicleid] == kind) return 1;
        new Float:km = (float(g_V4Odometer[owner]) + g_V4OdoFraction[owner]) / 1000.0;
        format(text, sizeof(text), "{FFFFFF}Model GTA: {FFD700}%d\n{FFFFFF}Wlasciciel: {55FF55}%s {FFD700}[ID %d]\n{FFFFFF}Przebieg: {55CCFF}%.1f km\n{FFFFFF}Paliwo: {FFCC55}%d%%",
            modelid, g_Name[owner], owner, km, fuel);
    }
    else if (kind == V4_VEHICLE_KIND_PUBLIC)
    {
        ownerToken = 0;
        deciKm = (g_V4FleetOdometer[vehicleid] / 100);
        if (g_V4VehicleLabelOwner[vehicleid] == ownerToken && g_V4VehicleLabelDeciKm[vehicleid] == deciKm &&
            g_V4VehicleLabelFuel[vehicleid] == fuel && g_V4VehicleLabelKind[vehicleid] == kind) return 1;
        new Float:km = (float(g_V4FleetOdometer[vehicleid]) + g_V4FleetOdoFraction[vehicleid]) / 1000.0;
        format(text, sizeof(text), "{FFFFFF}Model GTA: {FFD700}%d\n{FFFFFF}Przebieg: {55CCFF}%.1f km\n{FFFFFF}Paliwo: {FFCC55}%d%%",
            modelid, km, fuel);
    }
    else
    {
        ownerToken = 0;
        deciKm = (g_V4FleetOdometer[vehicleid] / 100);
        fuel = 100;
        if (g_V4VehicleLabelOwner[vehicleid] == ownerToken && g_V4VehicleLabelDeciKm[vehicleid] == deciKm &&
            g_V4VehicleLabelFuel[vehicleid] == fuel && g_V4VehicleLabelKind[vehicleid] == kind) return 1;
        format(text, sizeof(text), "{FFFFFF}Model GTA: {FFD700}%d", modelid);
    }
    Update3DTextLabelText(g_V4VehicleLabel[vehicleid], 0xFFFFFFFF, text);
    g_V4VehicleLabelOwner[vehicleid] = ownerToken;
    g_V4VehicleLabelDeciKm[vehicleid] = deciKm;
    g_V4VehicleLabelFuel[vehicleid] = fuel;
    g_V4VehicleLabelKind[vehicleid] = kind;
    return 1;
}

stock V4_VerifyVehicleLabels()
{
    new vehicles = 0, labels = 0, unclassified = 0, publicVehicles = 0, temporaryVehicles = 0, privateVehicles = 0;
    for (new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
    {
        if (GetVehicleModel(vehicleid) == 0) continue;
        vehicles++;
        if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_NONE) unclassified++;
        else if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PUBLIC) publicVehicles++;
        else if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_TEMP) temporaryVehicles++;
        else if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PRIVATE) privateVehicles++;
        if (V4_UpdateVehicleLabel(vehicleid)) labels++;
    }
    printf("NEMQO vehicle labels: %d/%d | public=%d temporary=%d private=%d unclassified=%d",
        labels, vehicles, publicVehicles, temporaryVehicles, privateVehicles, unclassified);
    return labels == vehicles && unclassified == 0;
}

stock V4_UpdateAllVehicleLabels()
{
    for (new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
        if (GetVehicleModel(vehicleid) != 0) V4_UpdateVehicleLabel(vehicleid);
    return 1;
}

stock V4_DestroyAllVehicleLabels()
{
    for (new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
        if (g_V4VehicleLabel[vehicleid] != Text3D:INVALID_3DTEXT_ID) V4_DestroyVehicleLabel(vehicleid);
    return 1;
}

stock V4_OdoResetVehicle(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    new owner = g_V4VehicleOwner[vehicleid] - 1;
    if (owner >= 0 && owner < MAX_PLAYERS) g_V4OdoSample[owner][odoValid] = false;
    g_V4FleetOdoSample[vehicleid][odoValid] = false;
    return 1;
}

stock V4_OdoResetDriver(playerid)
{
    for (new owner = 0; owner < MAX_PLAYERS; owner++)
        if (g_V4OdoSample[owner][odoDriver] == playerid) g_V4OdoSample[owner][odoValid] = false;
    for (new vehicleid = 1; vehicleid < MAX_VEHICLES; vehicleid++)
        if (g_V4FleetOdoSample[vehicleid][odoDriver] == playerid) g_V4FleetOdoSample[vehicleid][odoValid] = false;
    return 1;
}

// Must precede ALL gamemode includes/call sites. Includes island/PSZ teleports and vehicles.
stock V4_OdoSetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
{
    V4_OdoResetVehicle(vehicleid);
    return SetVehiclePos(vehicleid, x, y, z);
}
stock V4_LabelAddStaticVehicleEx(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2, respawn_delay, addsiren = 0)
{
    new vehicleid = AddStaticVehicleEx(modelid, x, y, z, angle, color1, color2, respawn_delay, addsiren);
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return INVALID_VEHICLE_ID;
    g_V4VehicleKind[vehicleid] = V4_VEHICLE_KIND_PUBLIC;
    g_V4PublicFuel[vehicleid] = 100;
    if (!V4_CreateVehicleLabel(vehicleid))
    {
        DestroyVehicle(vehicleid);
        return INVALID_VEHICLE_ID;
    }
    return vehicleid;
}
stock V4_LabelAddStaticVehicle(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2)
{
    new vehicleid = AddStaticVehicle(modelid, x, y, z, angle, color1, color2);
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return INVALID_VEHICLE_ID;
    g_V4VehicleKind[vehicleid] = V4_VEHICLE_KIND_PUBLIC;
    g_V4PublicFuel[vehicleid] = 100;
    if (!V4_CreateVehicleLabel(vehicleid))
    {
        DestroyVehicle(vehicleid);
        return INVALID_VEHICLE_ID;
    }
    return vehicleid;
}
stock V4_LabelCreateVehicle(modelid, Float:x, Float:y, Float:z, Float:angle, color1, color2, respawn_delay, addsiren = 0)
{
    new vehicleid = CreateVehicle(modelid, x, y, z, angle, color1, color2, respawn_delay, addsiren);
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return INVALID_VEHICLE_ID;
    g_V4VehicleKind[vehicleid] = V4_VEHICLE_KIND_TEMP;
    g_V4PublicFuel[vehicleid] = 100;
    if (!V4_CreateVehicleLabel(vehicleid))
    {
        DestroyVehicle(vehicleid);
        return INVALID_VEHICLE_ID;
    }
    return vehicleid;
}
stock V4_LabelSetVehicleVirtualWorld(vehicleid, worldid)
{
    new result = SetVehicleVirtualWorld(vehicleid, worldid);
    if (result && vehicleid >= 1 && vehicleid < MAX_VEHICLES && GetVehicleModel(vehicleid) != 0)
        V4_UpdateVehicleLabel(vehicleid);
    return result;
}
stock V4_OdoDestroyVehicle(vehicleid)
{
    V4_OdoResetVehicle(vehicleid);
    V4_DestroyVehicleLabel(vehicleid);
    if (vehicleid >= 1 && vehicleid < MAX_VEHICLES)
    {
        g_V4FleetOdometer[vehicleid] = 0;
        g_V4FleetOdoFraction[vehicleid] = 0.0;
        g_V4PublicFuel[vehicleid] = 100;
        g_V4VehicleKind[vehicleid] = V4_VEHICLE_KIND_NONE;
    }
    return DestroyVehicle(vehicleid);
}
#define SetVehiclePos V4_OdoSetVehiclePos
#define AddStaticVehicleEx V4_LabelAddStaticVehicleEx
#define AddStaticVehicle V4_LabelAddStaticVehicle
#define CreateVehicle V4_LabelCreateVehicle
#define SetVehicleVirtualWorld V4_LabelSetVehicleVirtualWorld
#define DestroyVehicle V4_OdoDestroyVehicle

stock V4_OnTrackedVehicleSpawn(vehicleid)
{
    V4_OdoResetVehicle(vehicleid);
    if (vehicleid >= 1 && vehicleid < MAX_VEHICLES && g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PUBLIC)
        g_V4PublicFuel[vehicleid] = 100;
    return V4_UpdateVehicleLabel(vehicleid);
}

stock V4_TrackFleetOdometer(playerid, vehicle)
{
    new now = GetTickCount(), elapsed = now - g_V4FleetOdoSample[vehicle][odoTick];
    new world = GetVehicleVirtualWorld(vehicle), interior = GetPlayerInterior(playerid);
    new bool:continuous = V4_OdoContinuity(g_V4FleetOdoSample[vehicle][odoValid], vehicle,
        g_V4FleetOdoSample[vehicle][odoVehicle], playerid, g_V4FleetOdoSample[vehicle][odoDriver],
        world, g_V4FleetOdoSample[vehicle][odoWorld], interior, g_V4FleetOdoSample[vehicle][odoInterior], elapsed);
    if (continuous && elapsed < V4_ODO_MIN_SAMPLE) return 1;
    new Float:x, Float:y, Float:z;
    if (!GetVehiclePos(vehicle, x, y, z))
    {
        g_V4FleetOdoSample[vehicle][odoValid] = false;
        return 1;
    }
    if (continuous)
        V4_OdoAccumulate(g_V4FleetOdometer[vehicle], g_V4FleetOdoFraction[vehicle], V4_OdoDistance(
            g_V4FleetOdoSample[vehicle][odoX], g_V4FleetOdoSample[vehicle][odoY], g_V4FleetOdoSample[vehicle][odoZ], x, y, z, elapsed));
    g_V4FleetOdoSample[vehicle][odoValid] = true;
    g_V4FleetOdoSample[vehicle][odoVehicle] = vehicle;
    g_V4FleetOdoSample[vehicle][odoDriver] = playerid;
    g_V4FleetOdoSample[vehicle][odoTick] = now;
    g_V4FleetOdoSample[vehicle][odoWorld] = world;
    g_V4FleetOdoSample[vehicle][odoInterior] = interior;
    g_V4FleetOdoSample[vehicle][odoX] = x;
    g_V4FleetOdoSample[vehicle][odoY] = y;
    g_V4FleetOdoSample[vehicle][odoZ] = z;
    return 1;
}

// Sample only incoming human driver sync (not a timer repeatedly reading stale positions).
public OnPlayerUpdate(playerid)
{
    if (!g_Logged[playerid] || IsPlayerNPC(playerid) || GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 1;
    new vehicle = GetPlayerVehicleID(playerid);
    if (vehicle < 1 || vehicle >= MAX_VEHICLES) return 1;
    new owner = g_V4VehicleOwner[vehicle] - 1;
    if (owner < 0 || owner >= MAX_PLAYERS || !g_Logged[owner] || !g_AccountExists[owner] ||
        g_V4OwnedVehicle[owner] != vehicle || IsPlayerNPC(owner)) return V4_TrackFleetOdometer(playerid, vehicle);
    new now = GetTickCount(), elapsed = now - g_V4OdoSample[owner][odoTick];
    new world = GetVehicleVirtualWorld(vehicle), interior = GetPlayerInterior(playerid);
    new bool:continuous = V4_OdoContinuity(g_V4OdoSample[owner][odoValid], vehicle,
        g_V4OdoSample[owner][odoVehicle], playerid, g_V4OdoSample[owner][odoDriver],
        world, g_V4OdoSample[owner][odoWorld], interior, g_V4OdoSample[owner][odoInterior], elapsed);
    if (continuous && elapsed < V4_ODO_MIN_SAMPLE) return 1;
    new Float:x, Float:y, Float:z;
    if (!GetVehiclePos(vehicle, x, y, z))
    {
        g_V4OdoSample[owner][odoValid] = false;
        return 1;
    }
    if (continuous)
        V4_OdoAccumulate(g_V4Odometer[owner], g_V4OdoFraction[owner], V4_OdoDistance(
            g_V4OdoSample[owner][odoX], g_V4OdoSample[owner][odoY], g_V4OdoSample[owner][odoZ], x, y, z, elapsed));
    // Rebase even on rejected jumps; never join a teleport/gap to the next movement.
    g_V4OdoSample[owner][odoValid] = true;
    g_V4OdoSample[owner][odoVehicle] = vehicle;
    g_V4OdoSample[owner][odoDriver] = playerid;
    g_V4OdoSample[owner][odoTick] = now;
    g_V4OdoSample[owner][odoWorld] = world;
    g_V4OdoSample[owner][odoInterior] = interior;
    g_V4OdoSample[owner][odoX] = x;
    g_V4OdoSample[owner][odoY] = y;
    g_V4OdoSample[owner][odoZ] = z;
    return 1;
}

stock bool:V4_SQLExec(query[])
{
    new DBResult:r = db_query(g_DB, query);
    if (r == DBResult:0) return false;
    db_free_result(r);
    return true;
}

stock bool:V4_OdoMigrate()
{
    new DBResult:r = db_query(g_DB, "PRAGMA table_info(accounts)");
    if (r == DBResult:0) return false;
    new bool:found = false, field[64];
    if (db_num_rows(r)) do
    {
        db_get_field_assoc(r, "name", field, sizeof(field));
        if (!strcmp(field, "odometer_fraction")) found = true;
    } while (db_next_row(r));
    db_free_result(r);
    if (found) return true;
    return V4_SQLExec("ALTER TABLE accounts ADD COLUMN odometer_fraction REAL NOT NULL DEFAULT 0");
}

stock bool:V4_NeonMigrate()
{
    new DBResult:r = db_query(g_DB, "PRAGMA table_info(accounts)");
    if (r == DBResult:0) return false;
    new bool:found = false, field[64];
    if (db_num_rows(r)) do
    {
        db_get_field_assoc(r, "name", field, sizeof(field));
        if (!strcmp(field, "owned_neon")) found = true;
    } while (db_next_row(r));
    db_free_result(r);
    if (found) return true;
    return V4_SQLExec("ALTER TABLE accounts ADD COLUMN owned_neon INTEGER NOT NULL DEFAULT 0");
}

stock bool:V4_GrantActor(playerid)
{
    return V4_GrantPermission(IsPlayerConnected(playerid) != 0, g_Logged[playerid],
        g_AccountExists[playerid], IsPlayerNPC(playerid) != 0, g_V4Admin[playerid], IsPlayerAdmin(playerid) != 0);
}

// Save changed balances and the audit in one transaction BEFORE applying effects to live state.
// Dedicated UPDATE avoids SaveAccount's unchecked multi-query writes and touches no admin rank.
stock bool:V4_CommitGrant(playerid, target, amount, bool:xpGrant, money, level, xp)
{
    if (!IsValidAccountName(g_Name[playerid]) || !IsValidAccountName(g_Name[target])) return false;
    if (!V4_SQLExec("BEGIN IMMEDIATE TRANSACTION")) return false;
    new query[512], action[192];
    format(query, sizeof(query), "UPDATE accounts SET money=%d,level=%d,xp=%d WHERE name='%s'",
        money, level, xp, g_Name[target]);
    new bool:ok = V4_SQLExec(query);
    if (ok)
    {
        new DBResult:r = db_query(g_DB, "SELECT changes() AS changed");
        ok = false;
        if (r != DBResult:0)
        {
            ok = db_get_field_assoc_int(r, "changed") == 1;
            db_free_result(r);
        }
    }
    format(action, sizeof(action), "%s amount=%d money=%d->%d level=%d->%d xp=%d->%d adminlevel=%d rcon=%d",
        xpGrant ? ("givexp") : ("givemoney"), amount, g_Money[target], money,
        g_Level[target], level, g_XP[target], xp, g_V4Admin[playerid], IsPlayerAdmin(playerid));
    if (ok)
    {
        format(query, sizeof(query), "INSERT INTO admin_logs (admin,target,action,created_at) VALUES ('%s','%s','%s',%d)",
            g_Name[playerid], g_Name[target], action, gettime());
        ok = V4_SQLExec(query);
    }
    if (ok) ok = V4_SQLExec("COMMIT");
    if (!ok)
    {
        V4_SQLExec("ROLLBACK");
        printf("[GRANT ERROR] actor=%s target=%s action=%s", g_Name[playerid], g_Name[target], action);
        return false;
    }
    printf("[GRANT] actor=%s target=%s action=%s", g_Name[playerid], g_Name[target], action);
    return true;
}

stock V4_GrantCommand(playerid, const cmdtext[])
{
    new offset, bool:xpGrant;
    if (!strcmp(cmdtext, "/givemoney", true, 10) && (cmdtext[10] == 0 || V4_GrantSpace(cmdtext[10]))) offset = 10;
    else if (!strcmp(cmdtext, "/givexp", true, 7) && (cmdtext[7] == 0 || V4_GrantSpace(cmdtext[7])))
    {
        offset = 7;
        xpGrant = true;
    }
    else return 0;
    if (!V4_GrantActor(playerid))
        return SendClientMessage(playerid, COLOR_RED, "Brak uprawnien: wymagany admin 1+ lub RCON oraz zalogowane konto gracza."), 1;
    new target, amount;
    if (!V4_ParseGrant(cmdtext, offset, MAX_PLAYERS - 1, xpGrant ? V4_GRANT_XP_MAX : V4_GRANT_MONEY_MAX, target, amount))
        return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /givemoney [id] [1-1000000] lub /givexp [id] [1-100000]. Tylko cyfry."), 1;
    if (!IsPlayerConnected(target) || IsPlayerNPC(target) || !g_Logged[target] || !g_AccountExists[target])
        return SendClientMessage(playerid, COLOR_RED, "Cel musi byc zalogowanym graczem, nie NPC."), 1;
    new now = GetTickCount();
    if (g_V4GrantUsed[playerid] && now - g_V4GrantTick[playerid] >= 0 && now - g_V4GrantTick[playerid] < 2000)
        return SendClientMessage(playerid, COLOR_RED, "Odczekaj 2 sekundy pomiedzy grantami."), 1;
    new money = g_Money[target], level = g_Level[target], xp = g_XP[target];
    if (xpGrant)
    {
        if (!V4_PlanXP(g_Level[target], g_XP[target], amount, level, xp))
            return SendClientMessage(playerid, COLOR_RED, "Grant XP odrzucony: limit lub nieprawidlowy stan level/XP konta."), 1;
    }
    else
    {
        if (!V4_CanGrantMoney(money, amount))
            return SendClientMessage(playerid, COLOR_RED, "Grant odrzucony: przekroczenie zakresu pieniedzy."), 1;
        money += amount;
    }
    if (!V4_CommitGrant(playerid, target, amount, xpGrant, money, level, xp))
        return SendClientMessage(playerid, COLOR_RED, "Blad zapisu bazy: grant NIE zostal przyznany."), 1;
    if (xpGrant) AddXP(target, amount);
    else AddMoney(target, amount);
    g_V4GrantUsed[playerid] = true;
    g_V4GrantTick[playerid] = now;
    new message[144];
    format(message, sizeof(message), "Admin %s przyznal %s (ID %d): +%d %s. Zapisano.",
        g_Name[playerid], g_Name[target], target, amount, xpGrant ? ("XP") : ("$"));
    SendClientMessage(playerid, COLOR_GREEN, message);
    if (target != playerid) SendClientMessage(target, COLOR_GREEN, message);
    return 1;
}
