// NEMQO Freeroam RPG v4 - implementacja zgodna z SA-MP 0.3.7-R2

forward V4_SecondTick();
forward V4_Announcement();
forward V4_DestroyArenaObjects(playerid);

stock V4_CreateFuelMarkers()
{
    new created = 0;
    for (new station = 0; station < V4_FUEL_STATION_COUNT; station++)
    {
        g_V4FuelPickups[station] = CreatePickup(1239, 23,
            g_V4FuelStationPos[station][0], g_V4FuelStationPos[station][1], g_V4FuelStationPos[station][2]);
        g_V4FuelLabels[station] = Create3DTextLabel(
            "{FF3030}STACJA PALIW\n{FFFFFF}Zatankuj pojazd",
            COLOR_WHITE, g_V4FuelStationPos[station][0], g_V4FuelStationPos[station][1],
            g_V4FuelStationPos[station][2] + 1.2, 35.0, 0, 0);
        if (g_V4FuelPickups[station] != -1 && g_V4FuelLabels[station] != Text3D:INVALID_3DTEXT_ID) created++;
    }
    printf("NEMQO v4.1 fuel markers: %d/%d created", created, V4_FUEL_STATION_COUNT);
    return created == V4_FUEL_STATION_COUNT;
}

public V4_Init()
{
    if (!V4_OdoMigrate() || !V4_NeonMigrate())
    {
        print("FATAL: vehicle storage migration failed");
        SendRconCommand("exit");
        return 0;
    }
    if (!V4_CreateFuelMarkers())
    {
        print("FATAL: fuel station markers could not be created");
        SendRconCommand("exit");
        return 0;
    }
    SetTimer("V4_SecondTick", 1000, true);
    SetTimer("V4_Announcement", 120000, true);

    new DBResult:r;
    r = db_query(g_DB, "CREATE TABLE IF NOT EXISTS houses (id INTEGER PRIMARY KEY, owner TEXT NOT NULL DEFAULT '', price INTEGER NOT NULL)");
    db_free_result(r);
    r = db_query(g_DB, "CREATE TABLE IF NOT EXISTS businesses (id INTEGER PRIMARY KEY, owner TEXT NOT NULL DEFAULT '', price INTEGER NOT NULL, last_payout INTEGER NOT NULL DEFAULT 0)");
    db_free_result(r);
    r = db_query(g_DB, "CREATE TABLE IF NOT EXISTS reports (id INTEGER PRIMARY KEY AUTOINCREMENT, reporter TEXT NOT NULL, target TEXT NOT NULL, reason TEXT NOT NULL, created_at INTEGER NOT NULL, status INTEGER NOT NULL DEFAULT 0)");
    db_free_result(r);
    r = db_query(g_DB, "CREATE TABLE IF NOT EXISTS bans (name TEXT PRIMARY KEY COLLATE NOCASE, reason TEXT NOT NULL, admin TEXT NOT NULL, created_at INTEGER NOT NULL)");
    db_free_result(r);
    r = db_query(g_DB, "CREATE TABLE IF NOT EXISTS admin_logs (id INTEGER PRIMARY KEY AUTOINCREMENT, admin TEXT NOT NULL, target TEXT NOT NULL, action TEXT NOT NULL, created_at INTEGER NOT NULL)");
    db_free_result(r);

    for (new i = 1; i <= 5; i++)
    {
        new q[160];
        format(q, sizeof(q), "INSERT OR IGNORE INTO houses (id,owner,price) VALUES (%d,'',%d)", i, g_V4HousePrice[i]);
        r = db_query(g_DB, q);
        db_free_result(r);
        AddStaticPickup(1273, 1, g_V4HousePos[i][0], g_V4HousePos[i][1], g_V4HousePos[i][2]);
    }
    for (new i = 1; i <= 3; i++)
    {
        new q[180];
        format(q, sizeof(q), "INSERT OR IGNORE INTO businesses (id,owner,price,last_payout) VALUES (%d,'',%d,0)", i, g_V4BusinessPrice[i]);
        r = db_query(g_DB, q);
        db_free_result(r);
        AddStaticPickup(1272, 1, g_V4BusinessPos[i][0], g_V4BusinessPos[i][1], g_V4BusinessPos[i][2]);
    }
    return 1;
}

public V4_Reset(playerid)
{
    g_V4OdoSample[playerid][odoValid] = false;
    g_V4OdoFraction[playerid] = 0.0;
    g_V4OdoSaveTick[playerid] = GetTickCount();
    g_V4GrantUsed[playerid] = false;
    g_V4GrantTick[playerid] = 0;
    g_V4Admin[playerid] = 0;
    g_V4Bank[playerid] = 0;
    g_V4Job[playerid] = V4_JOB_NONE;
    g_V4JobXP[playerid] = 0;
    g_V4Wanted[playerid] = 0;
    g_V4RaceBest[playerid] = 0;
    g_V4OwnedModel[playerid] = 0;
    g_V4OwnedColor1[playerid] = 1;
    g_V4OwnedColor2[playerid] = 1;
    g_V4OwnedPaintjob[playerid] = -1;
    g_V4OwnedWheels[playerid] = 1080;
    g_V4OwnedNeon[playerid] = 0;
    g_V4Fuel[playerid] = 100;
    g_V4Odometer[playerid] = 0;
    g_V4House[playerid] = 0;
    g_V4Business[playerid] = 0;
    g_V4Achievements[playerid] = 0;
    g_V4DailyDay[playerid] = 0;
    g_V4DailyProgress[playerid] = 0;
    g_V4JailSeconds[playerid] = 0;
    g_V4Muted[playerid] = 0;
    g_V4WorkActive[playerid] = 0;
    g_V4RaceActive[playerid] = 0;
    g_V4RaceCP[playerid] = 0;
    g_V4RaceStart[playerid] = 0;
    g_V4OwnedVehicle[playerid] = INVALID_VEHICLE_ID;
    g_V4NeonObjects[playerid][0] = INVALID_OBJECT_ID;
    g_V4NeonObjects[playerid][1] = INVALID_OBJECT_ID;
    g_V4VehicleLocked[playerid] = false;
    g_V4Engine[playerid] = true;
    g_V4Lights[playerid] = false;
    g_V4Seatbelt[playerid] = false;
    g_V4Arena[playerid] = V4_ARENA_NONE;
    g_V4ArenaVehicle[playerid] = INVALID_VEHICLE_ID;
    g_V4FuelTick[playerid] = 0;
    g_V4FuelArea[playerid] = -1;
    for (new i = 0; i < 6; i++) g_V4ParkourObjects[playerid][i] = INVALID_OBJECT_ID;
    return 1;
}

public V4_Connect(playerid)
{
    new q[160];
    format(q, sizeof(q), "SELECT reason FROM bans WHERE name='%s' LIMIT 1", g_Name[playerid]);
    new DBResult:r = db_query(g_DB, q);
    if (db_num_rows(r) > 0)
    {
        new reason[80], msg[144];
        db_get_field_assoc(r, "reason", reason, sizeof(reason));
        format(msg, sizeof(msg), "Masz bana na tym serwerze. Powod: %s", reason);
        SendClientMessage(playerid, COLOR_RED, msg);
        SetTimerEx("KickDelayed", 500, false, "i", playerid);
    }
    db_free_result(r);
    return 1;
}

public V4_Disconnect(playerid)
{
    V4_DestroyNeon(playerid);
    if (g_V4OwnedVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        new vehicleid = g_V4OwnedVehicle[playerid];
        DestroyVehicle(vehicleid);
        g_V4VehicleOwner[vehicleid] = 0;
        g_V4OwnedVehicle[playerid] = INVALID_VEHICLE_ID;
    }
    V4_DestroyArenaObjects(playerid);
    return 1;
}

public V4_Load(playerid)
{
    new q[160];
    format(q, sizeof(q), "SELECT * FROM accounts WHERE name='%s' LIMIT 1", g_Name[playerid]);
    new DBResult:r = db_query(g_DB, q);
    if (db_num_rows(r) > 0)
    {
        g_V4Admin[playerid] = db_get_field_assoc_int(r, "admin");
        g_V4Bank[playerid] = db_get_field_assoc_int(r, "bank");
        g_V4Job[playerid] = db_get_field_assoc_int(r, "job");
        g_V4JobXP[playerid] = db_get_field_assoc_int(r, "job_xp");
        g_V4Wanted[playerid] = db_get_field_assoc_int(r, "wanted");
        g_V4RaceBest[playerid] = db_get_field_assoc_int(r, "race_best");
        g_V4OwnedModel[playerid] = db_get_field_assoc_int(r, "owned_model");
        g_V4OwnedColor1[playerid] = db_get_field_assoc_int(r, "owned_color1");
        g_V4OwnedColor2[playerid] = db_get_field_assoc_int(r, "owned_color2");
        g_V4OwnedPaintjob[playerid] = db_get_field_assoc_int(r, "owned_paintjob");
        g_V4OwnedWheels[playerid] = db_get_field_assoc_int(r, "owned_wheels");
        g_V4OwnedNeon[playerid] = db_get_field_assoc_int(r, "owned_neon");
        if (g_V4OwnedNeon[playerid] < 0 || g_V4OwnedNeon[playerid] > 6) g_V4OwnedNeon[playerid] = 0;
        g_V4Fuel[playerid] = db_get_field_assoc_int(r, "owned_fuel");
        g_V4Odometer[playerid] = db_get_field_assoc_int(r, "odometer");
        g_V4OdoFraction[playerid] = db_get_field_assoc_float(r, "odometer_fraction");
        if (g_V4Odometer[playerid] < 0) g_V4Odometer[playerid] = 0;
        if (!(g_V4OdoFraction[playerid] >= 0.0 && g_V4OdoFraction[playerid] < 1.0)) g_V4OdoFraction[playerid] = 0.0;
        g_V4OdoSample[playerid][odoValid] = false;
        g_V4House[playerid] = db_get_field_assoc_int(r, "house_id");
        g_V4Business[playerid] = db_get_field_assoc_int(r, "business_id");
        g_V4Achievements[playerid] = db_get_field_assoc_int(r, "achievements");
        g_V4DailyDay[playerid] = db_get_field_assoc_int(r, "daily_day");
        g_V4DailyProgress[playerid] = db_get_field_assoc_int(r, "daily_progress");
    }
    db_free_result(r);
    SetPlayerWantedLevel(playerid, g_V4Wanted[playerid]);
    return 1;
}

public V4_Save(playerid)
{
    if (!g_AccountExists[playerid]) return 0;
    new q[960];
    format(q, sizeof(q), "UPDATE accounts SET admin=%d,bank=%d,job=%d,job_xp=%d,wanted=%d,race_best=%d,owned_model=%d,owned_color1=%d,owned_color2=%d,owned_paintjob=%d,owned_wheels=%d,owned_neon=%d,owned_fuel=%d,odometer=%d,odometer_fraction=%.6f,house_id=%d,business_id=%d,achievements=%d,daily_day=%d,daily_progress=%d WHERE name='%s'",
        g_V4Admin[playerid], g_V4Bank[playerid], g_V4Job[playerid], g_V4JobXP[playerid],
        g_V4Wanted[playerid], g_V4RaceBest[playerid], g_V4OwnedModel[playerid],
        g_V4OwnedColor1[playerid], g_V4OwnedColor2[playerid], g_V4OwnedPaintjob[playerid],
        g_V4OwnedWheels[playerid], g_V4OwnedNeon[playerid], g_V4Fuel[playerid],
        g_V4Odometer[playerid], g_V4OdoFraction[playerid], g_V4House[playerid], g_V4Business[playerid],
        g_V4Achievements[playerid], g_V4DailyDay[playerid], g_V4DailyProgress[playerid], g_Name[playerid]);
    new DBResult:r = db_query(g_DB, q);
    db_free_result(r);
    return 1;
}

stock V4_Unlock(playerid, bit, const title[])
{
    if (g_V4Achievements[playerid] & bit) return 0;
    g_V4Achievements[playerid] |= bit;
    AddMoney(playerid, 1000);
    new msg[128];
    format(msg, sizeof(msg), "Osiagniecie: %s | nagroda $1000", title);
    SendClientMessage(playerid, COLOR_YELLOW, msg);
    V4_Save(playerid);
    return 1;
}

stock V4_ShowMenu(playerid)
{
    ShowPlayerDialog(playerid, V4_DIALOG_MENU, DIALOG_STYLE_LIST, "NEMQO v4 - Menu",
        "Wyscigi\nPrace\nMoj pojazd\nGPS\nAreny\nOsiagniecia\nZadanie dzienne\nDom\nFirma\nRadio\nAdministracja",
        "Wybierz", "Zamknij");
    return 1;
}

stock V4_ShowJobs(playerid)
{
    ShowPlayerDialog(playerid, V4_DIALOG_JOBS, DIALOG_STYLE_LIST, "Wybierz prace",
        "Taksowkarz\nKurier\nKierowca tira\nMechanik\nPolicjant\nRatownik\nPilot",
        "Wybierz", "Wroc");
    return 1;
}

stock V4_StartWork(playerid)
{
    if (g_V4Job[playerid] == V4_JOB_NONE)
    {
        SendClientMessage(playerid, COLOR_RED, "Najpierw wybierz prace: /jobs");
        return 0;
    }
    g_V4WorkActive[playerid] = 1;
    switch (g_V4Job[playerid])
    {
        case V4_JOB_TAXI: SetPlayerCheckpoint(playerid, 1804.8, -1865.2, 13.2, 6.0);
        case V4_JOB_COURIER: SetPlayerCheckpoint(playerid, -1912.4, 829.9, 35.2, 6.0);
        case V4_JOB_TRUCKER: SetPlayerCheckpoint(playerid, 1040.0, 1300.0, 10.6, 8.0);
        case V4_JOB_MECHANIC: SetPlayerCheckpoint(playerid, 2072.4, -2033.0, 13.4, 6.0);
        case V4_JOB_POLICE: SetPlayerCheckpoint(playerid, 1545.0, -1615.2, 13.2, 6.0);
        case V4_JOB_MEDIC: SetPlayerCheckpoint(playerid, 1178.4, -1338.2, 13.5, 6.0);
        case V4_JOB_PILOT: SetPlayerCheckpoint(playerid, 1880.0, -2490.0, 14.8, 12.0);
    }
    SendClientMessage(playerid, COLOR_GREEN, "Zlecenie rozpoczete. Jedz do czerwonego punktu.");
    return 1;
}

stock V4_StartRace(playerid)
{
    if (g_InDM[playerid] || g_InSumo[playerid] || V4_IsInArena(playerid) || g_PSZHideJoined[playerid])
    {
        SendClientMessage(playerid, COLOR_RED, "Najpierw opusc aktywna zabawe: /freeroam");
        return 0;
    }
    if (!IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_RED, "Do wyscigu potrzebujesz pojazdu.");
        return 0;
    }
    if (g_V4RaceActive[playerid]) return 0;
    g_V4RaceActive[playerid] = 1;
    g_V4RaceCP[playerid] = 0;
    g_V4RaceStart[playerid] = gettime();
    SetPlayerRaceCheckpoint(playerid, 0, g_V4RacePoints[0][0], g_V4RacePoints[0][1], g_V4RacePoints[0][2],
        g_V4RacePoints[1][0], g_V4RacePoints[1][1], g_V4RacePoints[1][2], 7.0);
    SendClientMessage(playerid, COLOR_YELLOW, "Wyscig LS Sprint rozpoczety. Ukoncz wszystkie checkpointy.");
    return 1;
}

stock V4_SpawnOwnedVehicle(playerid)
{
    g_V4OdoSample[playerid][odoValid] = false;
    if (g_V4OwnedModel[playerid] == 0)
    {
        SendClientMessage(playerid, COLOR_RED, "Nie posiadasz pojazdu. /buycar [400-611]");
        return 0;
    }
    if (g_V4OwnedVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        V4_DestroyNeon(playerid);
        new oldVehicle = g_V4OwnedVehicle[playerid];
        DestroyVehicle(oldVehicle);
        g_V4VehicleOwner[oldVehicle] = 0;
    }
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    g_V4OwnedVehicle[playerid] = CreateVehicle(g_V4OwnedModel[playerid], x + 4.0, y, z, a,
        g_V4OwnedColor1[playerid], g_V4OwnedColor2[playerid], -1);
    if (g_V4OwnedVehicle[playerid] == INVALID_VEHICLE_ID)
    {
        SendClientMessage(playerid, COLOR_RED, "Limit pojazdow osiagniety. Sprobuj ponownie pozniej.");
        return 0;
    }
    g_V4VehicleKind[g_V4OwnedVehicle[playerid]] = V4_VEHICLE_KIND_PRIVATE;
    g_V4VehicleOwner[g_V4OwnedVehicle[playerid]] = playerid + 1;
    V4_UpdateVehicleLabel(g_V4OwnedVehicle[playerid]);
    if (IsSafeTuningModel(g_V4OwnedModel[playerid]))
    {
        AddVehicleComponent(g_V4OwnedVehicle[playerid], g_V4OwnedWheels[playerid]);
        AddVehicleComponent(g_V4OwnedVehicle[playerid], 1087);
        AddVehicleComponent(g_V4OwnedVehicle[playerid], 1010);
        if (SupportsRandomPaintjob(g_V4OwnedModel[playerid]) && g_V4OwnedPaintjob[playerid] >= 0)
            ChangeVehiclePaintjob(g_V4OwnedVehicle[playerid], g_V4OwnedPaintjob[playerid]);
    }
    V4_ApplyNeon(playerid);
    PutPlayerInVehicle(playerid, g_V4OwnedVehicle[playerid], 0);
    SendClientMessage(playerid, COLOR_GREEN, "Twoj pojazd zostal podstawiony.");
    return 1;
}

stock V4_DestroyNeon(playerid)
{
    for (new i = 0; i < 2; i++)
    {
        if (g_V4NeonObjects[playerid][i] != INVALID_OBJECT_ID && IsValidObject(g_V4NeonObjects[playerid][i]))
            DestroyObject(g_V4NeonObjects[playerid][i]);
        g_V4NeonObjects[playerid][i] = INVALID_OBJECT_ID;
    }
    return 1;
}

stock V4_ApplyNeon(playerid)
{
    V4_DestroyNeon(playerid);
    if (g_V4OwnedNeon[playerid] < 1 || g_V4OwnedNeon[playerid] > 6) return 1;
    if (g_V4OwnedVehicle[playerid] < 1 || g_V4OwnedVehicle[playerid] >= MAX_VEHICLES) return 0;
    if (!IsSafeTuningModel(g_V4OwnedModel[playerid])) return 0;

    new model = 18646 + g_V4OwnedNeon[playerid];
    g_V4NeonObjects[playerid][0] = CreateObject(model, 0.0, 0.0, -1000.0, 0.0, 0.0, 0.0, 100.0);
    g_V4NeonObjects[playerid][1] = CreateObject(model, 0.0, 0.0, -1000.0, 0.0, 0.0, 0.0, 100.0);
    if (g_V4NeonObjects[playerid][0] == INVALID_OBJECT_ID || g_V4NeonObjects[playerid][1] == INVALID_OBJECT_ID)
    {
        V4_DestroyNeon(playerid);
        SendClientMessage(playerid, COLOR_RED, "Nie udalo sie utworzyc neonow: limit obiektow.");
        return 0;
    }
    AttachObjectToVehicle(g_V4NeonObjects[playerid][0], g_V4OwnedVehicle[playerid], -0.75, 0.0, -0.62, 0.0, 0.0, 0.0);
    AttachObjectToVehicle(g_V4NeonObjects[playerid][1], g_V4OwnedVehicle[playerid],  0.75, 0.0, -0.62, 0.0, 0.0, 0.0);
    return 1;
}

public V4_OnVehicleDeath(vehicleid)
{
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    new owner = g_V4VehicleOwner[vehicleid] - 1;
    if (owner < 0 || owner >= MAX_PLAYERS || g_V4OwnedVehicle[owner] != vehicleid) return 0;
    if (g_Logged[owner] && g_AccountExists[owner]) V4_Save(owner);
    V4_DestroyNeon(owner);
    DestroyVehicle(vehicleid);
    g_V4VehicleOwner[vehicleid] = 0;
    g_V4OwnedVehicle[owner] = INVALID_VEHICLE_ID;
    return 1;
}

stock V4_ApplyVehicleLock(playerid)
{
    if (g_V4OwnedVehicle[playerid] == INVALID_VEHICLE_ID) return 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        SetVehicleParamsForPlayer(g_V4OwnedVehicle[playerid], i, 0,
            (g_V4VehicleLocked[playerid] && i != playerid) ? 1 : 0);
    }
    return 1;
}

stock V4_IsDrivingOwnedVehicle(playerid)
{
    return GetPlayerState(playerid) == PLAYER_STATE_DRIVER &&
        g_V4OwnedVehicle[playerid] != INVALID_VEHICLE_ID &&
        GetPlayerVehicleID(playerid) == g_V4OwnedVehicle[playerid];
}

stock V4_IsDrivingFuelVehicle(playerid)
{
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 0;
    new vehicleid = GetPlayerVehicleID(playerid);
    if (vehicleid < 1 || vehicleid >= MAX_VEHICLES) return 0;
    return V4_CanRefuelKind(g_V4VehicleKind[vehicleid],
        g_V4OwnedVehicle[playerid] == vehicleid && g_V4VehicleOwner[vehicleid] == playerid + 1);
}

stock V4_FindFuelStation(playerid, Float:range)
{
    for (new station = 0; station < sizeof(g_V4FuelStationPos); station++)
    {
        if (IsPlayerInRangeOfPoint(playerid, range,
            g_V4FuelStationPos[station][0], g_V4FuelStationPos[station][1], g_V4FuelStationPos[station][2])) return station;
    }
    return -1;
}

stock V4_IsNearFuelStation(playerid)
{
    return V4_FindFuelStation(playerid, 22.0) != -1;
}

stock V4_UpdateFuelArea(playerid)
{
    new station = -1;
    if (V4_IsDrivingFuelVehicle(playerid)) station = V4_FindFuelStation(playerid, 12.0);
    if (station == -1)
    {
        g_V4FuelArea[playerid] = -1;
        return 1;
    }
    if (station == g_V4FuelArea[playerid]) return 1;
    g_V4FuelArea[playerid] = station;
    V4_ShowRefuelDialog(playerid);
    return 1;
}

stock V4_ShowRefuelDialog(playerid)
{
    if (!V4_IsDrivingFuelVehicle(playerid))
        return SendClientMessage(playerid, COLOR_RED, "Musisz prowadzic pojazd publiczny albo swoje prywatne auto."), 0;
    if (!V4_IsNearFuelStation(playerid))
        return SendClientMessage(playerid, COLOR_RED, "Musisz zatrzymac sie przy znaku STACJA PALIW."), 0;

    new vehicleid = GetPlayerVehicleID(playerid), fuel = V4_GetVehicleFuel(vehicleid);
    new cost = V4_CalculateFuelCost(fuel);
    if (cost <= 0) return SendClientMessage(playerid, COLOR_YELLOW, "Bak jest juz pelny."), 0;

    new missing = 100 - fuel, text[192];
    format(text, sizeof(text), "Model GTA: %d\nPaliwo: %d%%\nBrakuje: %d%%\nKoszt: $%d\n\nZatankowac pojazd do 100%%?",
        GetVehicleModel(vehicleid), fuel, missing, cost);
    ShowPlayerDialog(playerid, V4_DIALOG_REFUEL, DIALOG_STYLE_MSGBOX, "Stacja paliw", text, "Tankuj", "Anuluj");
    return 1;
}

stock V4_ConfirmRefuel(playerid)
{
    if (!V4_IsDrivingFuelVehicle(playerid))
        return SendClientMessage(playerid, COLOR_RED, "Tankowanie przerwane: nie prowadzisz odpowiedniego pojazdu."), 0;
    if (!V4_IsNearFuelStation(playerid))
        return SendClientMessage(playerid, COLOR_RED, "Tankowanie przerwane: odjechales od dystrybutora."), 0;

    new vehicleid = GetPlayerVehicleID(playerid), fuel = V4_GetVehicleFuel(vehicleid);
    new cost = V4_CalculateFuelCost(fuel);
    if (cost <= 0) return SendClientMessage(playerid, COLOR_YELLOW, "Bak jest juz pelny."), 0;
    if (g_Money[playerid] < cost)
        return SendClientMessage(playerid, COLOR_RED, "Nie masz wystarczajaco pieniedzy na tankowanie."), 0;

    AddMoney(playerid, -cost);
    V4_SetVehicleFuel(vehicleid, 100);
    V4_Save(playerid);
    V4_UpdateVehicleLabel(vehicleid);
    new message[96];
    format(message, sizeof(message), "Bak zatankowany do 100%%. Zaplacono $%d.", cost);
    SendClientMessage(playerid, COLOR_GREEN, message);
    return 1;
}

stock V4_BuyHouse(playerid, houseid)
{
    if (houseid < 1 || houseid > 5) return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /buyhouse [1-5]"), 0;
    if (g_V4House[playerid] != 0) return SendClientMessage(playerid, COLOR_RED, "Masz juz dom."), 0;
    if (g_Money[playerid] < g_V4HousePrice[houseid]) return SendClientMessage(playerid, COLOR_RED, "Nie masz wystarczajaco pieniedzy."), 0;
    new q[160], owner[MAX_PLAYER_NAME + 1];
    format(q, sizeof(q), "SELECT owner FROM houses WHERE id=%d", houseid);
    new DBResult:r = db_query(g_DB, q);
    db_get_field_assoc(r, "owner", owner, sizeof(owner));
    db_free_result(r);
    if (strlen(owner) > 0) return SendClientMessage(playerid, COLOR_RED, "Ten dom ma juz wlasciciela."), 0;
    format(q, sizeof(q), "UPDATE houses SET owner='%s' WHERE id=%d", g_Name[playerid], houseid);
    r = db_query(g_DB, q); db_free_result(r);
    AddMoney(playerid, -g_V4HousePrice[houseid]);
    g_V4House[playerid] = houseid;
    V4_Unlock(playerid, V4_ACH_HOUSE, "Pierwszy dom");
    V4_Save(playerid);
    SendClientMessage(playerid, COLOR_GREEN, "Dom kupiony. Uzyj /home.");
    return 1;
}

stock V4_BuyBusiness(playerid, businessid)
{
    if (businessid < 1 || businessid > 3) return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /buybiz [1-3]"), 0;
    if (g_V4Business[playerid] != 0) return SendClientMessage(playerid, COLOR_RED, "Masz juz firme."), 0;
    if (g_Money[playerid] < g_V4BusinessPrice[businessid]) return SendClientMessage(playerid, COLOR_RED, "Nie masz wystarczajaco pieniedzy."), 0;
    new q[180], owner[MAX_PLAYER_NAME + 1];
    format(q, sizeof(q), "SELECT owner FROM businesses WHERE id=%d", businessid);
    new DBResult:r = db_query(g_DB, q);
    db_get_field_assoc(r, "owner", owner, sizeof(owner)); db_free_result(r);
    if (strlen(owner) > 0) return SendClientMessage(playerid, COLOR_RED, "Ta firma ma juz wlasciciela."), 0;
    format(q, sizeof(q), "UPDATE businesses SET owner='%s',last_payout=0 WHERE id=%d", g_Name[playerid], businessid);
    r = db_query(g_DB, q); db_free_result(r);
    AddMoney(playerid, -g_V4BusinessPrice[businessid]);
    g_V4Business[playerid] = businessid;
    V4_Save(playerid);
    SendClientMessage(playerid, COLOR_GREEN, "Firma kupiona. Premia: /bizbonus.");
    return 1;
}

public V4_DestroyArenaObjects(playerid)
{
    for (new i = 0; i < 6; i++)
    {
        if (g_V4ParkourObjects[playerid][i] != INVALID_OBJECT_ID)
        {
            DestroyPlayerObject(playerid, g_V4ParkourObjects[playerid][i]);
            g_V4ParkourObjects[playerid][i] = INVALID_OBJECT_ID;
        }
    }
    return 1;
}

public V4_LeaveArena(playerid)
{
    if (g_V4ArenaVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(g_V4ArenaVehicle[playerid]);
        g_V4ArenaVehicle[playerid] = INVALID_VEHICLE_ID;
    }
    V4_DestroyArenaObjects(playerid);
    g_V4Arena[playerid] = V4_ARENA_NONE;
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerInterior(playerid, 0);
    SetPlayerSkin(playerid, g_Skin[playerid]);
    SetPlayerPos(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z);
    SetPlayerFacingAngle(playerid, NEMQO_START_A);
    ResetPlayerWeapons(playerid);
    GivePlayerWeapon(playerid, 24, 70);
    GivePlayerWeapon(playerid, 29, 250);
    return 1;
}

stock V4_EnterArena(playerid, arena)
{
    if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, false);
    PSZ_DisableDrift(playerid);
    g_InDM[playerid] = false;
    if (g_InSumo[playerid]) ExitSumo(playerid, false);
    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
    V4_LeaveArena(playerid);
    g_V4Arena[playerid] = arena;
    if (!V4_SpawnArena(playerid)) return 0;
    V4_Unlock(playerid, V4_ACH_ARENA, "Pierwsza dodatkowa arena");
    SendClientMessage(playerid, COLOR_YELLOW, "Arena aktywna. Powrot: /leavearena");
    return 1;
}

public V4_SpawnArena(playerid)
{
    new arena = g_V4Arena[playerid];
    if (arena == V4_ARENA_NONE) return 0;
    SetPlayerVirtualWorld(playerid, 30 + arena);
    SetPlayerInterior(playerid, 0);
    ResetPlayerWeapons(playerid);
    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, 100.0);
    switch (arena)
    {
        case V4_ARENA_DERBY:
        {
            SetPlayerPos(playerid, 1950.0, -1400.0, 51.0);
            g_V4ArenaVehicle[playerid] = CreateVehicle(504, 1950.0 + random(60), -1430.0 + random(60), 51.0, random(360), random(126), random(126), -1);
            if (g_V4ArenaVehicle[playerid] == INVALID_VEHICLE_ID)
            {
                SendClientMessage(playerid, COLOR_RED, "Nie mozna utworzyc auta Derby: limit pojazdow.");
                V4_LeaveArena(playerid);
                return 0;
            }
            SetVehicleVirtualWorld(g_V4ArenaVehicle[playerid], 31);
            PutPlayerInVehicle(playerid, g_V4ArenaVehicle[playerid], 0);
        }
        case V4_ARENA_PAINTBALL:
        {
            SetPlayerPos(playerid, -1360.0 + random(30), 940.0 + random(30), 1036.0);
            SetPlayerInterior(playerid, 15);
            GivePlayerWeapon(playerid, 29, 999);
        }
        case V4_ARENA_SNIPER:
        {
            SetPlayerPos(playerid, 1540.0 + random(50), -1350.0, 330.0);
            GivePlayerWeapon(playerid, 34, 100);
        }
        case V4_ARENA_MINIGUN:
        {
            SetPlayerPos(playerid, 1300.0 + random(50), 2100.0 + random(50), 11.0);
            GivePlayerWeapon(playerid, 38, 9999);
        }
        case V4_ARENA_ZOMBIE:
        {
            SetPlayerPos(playerid, -2100.0 + random(30), -2400.0 + random(30), 31.0);
            SetPlayerSkin(playerid, 162);
            GivePlayerWeapon(playerid, 25, 200);
            GivePlayerWeapon(playerid, 9, 1);
        }
        case V4_ARENA_STUNT:
        {
            SetPlayerPos(playerid, 1880.0, -2490.0, 14.8);
            g_V4ArenaVehicle[playerid] = CreateVehicle(522, 1880.0, -2490.0, 14.8, 90.0, 3, 8, -1);
            if (g_V4ArenaVehicle[playerid] == INVALID_VEHICLE_ID)
            {
                SendClientMessage(playerid, COLOR_RED, "Nie mozna utworzyc motocykla Stunt: limit pojazdow.");
                V4_LeaveArena(playerid);
                return 0;
            }
            SetVehicleVirtualWorld(g_V4ArenaVehicle[playerid], 36);
            PutPlayerInVehicle(playerid, g_V4ArenaVehicle[playerid], 0);
        }
        case V4_ARENA_PARKOUR:
        {
            SetPlayerPos(playerid, 2000.0, -1500.0, 302.0);
            for (new i = 0; i < 6; i++)
                g_V4ParkourObjects[playerid][i] = CreatePlayerObject(playerid, 19379, 2000.0 + (i * 9.0), -1500.0, 300.0 + (i * 2.0), 0.0, 90.0, 0.0);
        }
        case V4_ARENA_GANG:
        {
            SetPlayerPos(playerid, 2500.0 + random(30), -1670.0 + random(30), 13.3);
            SetPlayerSkin(playerid, random(2) ? 105 : 102);
            GivePlayerWeapon(playerid, 30, 500);
            GivePlayerWeapon(playerid, 24, 100);
        }
        case V4_ARENA_DUEL:
        {
            SetPlayerPos(playerid, 1350.0 + random(20), 2120.0 + random(20), 11.0);
            GivePlayerWeapon(playerid, 24, 100);
        }
    }
    SetCameraBehindPlayer(playerid);
    return 1;
}

public V4_IsInArena(playerid)
{
    return g_V4Arena[playerid] != V4_ARENA_NONE;
}

public V4_GetFuelPercent(playerid)
{
    if (IsPlayerInAnyVehicle(playerid)) return V4_GetVehicleFuel(GetPlayerVehicleID(playerid));
    return 100;
}

stock V4_ShowAchievements(playerid)
{
    new text[512];
    format(text, sizeof(text), "Pierwsza praca: %s\nPierwszy wyscig: %s\nWlasny pojazd: %s\nWlasny dom: %s\nPredkosc 200 km/h: %s\nDodatkowa arena: %s\nBogacz $100000: %s",
        (g_V4Achievements[playerid] & V4_ACH_FIRST_JOB) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_RACE) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_CAR) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_HOUSE) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_SPEED) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_ARENA) ? "TAK" : "NIE",
        (g_V4Achievements[playerid] & V4_ACH_RICH) ? "TAK" : "NIE");
    ShowPlayerDialog(playerid, V4_DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX, "Osiagniecia", text, "OK", "");
    return 1;
}

stock V4_GetDailyKey()
{
    new year, month, day;
    getdate(year, month, day);
    return year * 10000 + month * 100 + day;
}

stock V4_ShowDaily(playerid)
{
    new today = V4_GetDailyKey();
    if (g_V4DailyDay[playerid] != today)
    {
        g_V4DailyDay[playerid] = today;
        g_V4DailyProgress[playerid] = 0;
    }
    new text[160];
    format(text, sizeof(text), "Dzisiejsze zadanie: wykonaj 3 zlecenia pracy.\nPostep: %d/3\nNagroda: $5000 i 25 XP", g_V4DailyProgress[playerid]);
    ShowPlayerDialog(playerid, V4_DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX, "Zadanie dzienne", text, "OK", "");
    V4_Save(playerid);
    return 1;
}

stock V4_PlayerNearPlayer(playerid, targetid, Float:range)
{
    if (!IsPlayerConnected(targetid)) return 0;
    new Float:x, Float:y, Float:z;
    GetPlayerPos(targetid, x, y, z);
    return IsPlayerInRangeOfPoint(playerid, range, x, y, z);
}

stock V4_AdminLog(playerid, targetid, const action[])
{
    new targetName[MAX_PLAYER_NAME + 1] = "offline";
    if (IsPlayerConnected(targetid)) format(targetName, sizeof(targetName), "%s", g_Name[targetid]);
    new q[320];
    format(q, sizeof(q), "INSERT INTO admin_logs (admin,target,action,created_at) VALUES ('%s','%s','%s',%d)",
        g_Name[playerid], targetName, action, gettime());
    new DBResult:r = db_query(g_DB, q);
    db_free_result(r);
    return 1;
}

public V4_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused inputtext
    if (!response) return 0;
    if (dialogid == V4_DIALOG_MENU)
    {
        switch (listitem)
        {
            case 0: V4_StartRace(playerid);
            case 1: V4_ShowJobs(playerid);
            case 2: ShowPlayerDialog(playerid, V4_DIALOG_VEHICLE, DIALOG_STYLE_LIST, "Moj pojazd", "Podstaw\nZaparkuj\nZamek\nSilnik\nSwiatla\nPasy\nNeony\nInformacje\nSprzedaj", "Wybierz", "Wroc");
            case 3: ShowPlayerDialog(playerid, V4_DIALOG_GPS, DIALOG_STYLE_LIST, "GPS", "Dom CJ\nMolo LS (start)\nRatusz LS\nLotnisko LS\nCentrum SF\nStrip LV\nSzpital LS\nKomisariat LS\nWyspa NEMQO", "Jedz", "Wroc");
            case 4: ShowPlayerDialog(playerid, V4_DIALOG_ARENAS, DIALOG_STYLE_LIST, "Areny", "Derby\nPaintball\nSniper\nMinigun\nZombie Survival\nStunt\nParkour\nGrove vs Ballas", "Wejdz", "Wroc");
            case 5: V4_ShowAchievements(playerid);
            case 6: V4_ShowDaily(playerid);
            case 7: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/house");
            case 8: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/business");
            case 9: ShowPlayerDialog(playerid, V4_DIALOG_RADIO, DIALOG_STYLE_LIST, "Radio", "Wylacz\nRadio Paradise\nSomaFM Groove Salad\nRadio Paradise Rock", "Wybierz", "Wroc");
            case 10: ShowPlayerDialog(playerid, V4_DIALOG_ADMIN, DIALOG_STYLE_MSGBOX, "Administracja", "Komendy: /reports /kick /ban /mute /jail /goto /gethere", "OK", "");
        }
        return 1;
    }
    if (dialogid == V4_DIALOG_JOBS)
    {
        g_V4Job[playerid] = listitem + 1;
        new msg[96];
        format(msg, sizeof(msg), "Wybrano prace: %s. Rozpocznij: /work", g_V4JobNames[g_V4Job[playerid]]);
        SendClientMessage(playerid, COLOR_GREEN, msg);
        V4_Save(playerid);
        return 1;
    }
    if (dialogid == V4_DIALOG_VEHICLE)
    {
        switch (listitem)
        {
            case 0: V4_SpawnOwnedVehicle(playerid);
            case 1:
            {
                if (g_V4OwnedVehicle[playerid] != INVALID_VEHICLE_ID && GetPlayerVehicleID(playerid) == g_V4OwnedVehicle[playerid])
                    SendClientMessage(playerid, COLOR_GREEN, "Pojazd zaparkowany. Stan paliwa i przebieg zapisano.");
                V4_Save(playerid);
            }
            case 2:
            {
                g_V4VehicleLocked[playerid] = !g_V4VehicleLocked[playerid];
                V4_ApplyVehicleLock(playerid);
                SendClientMessage(playerid, COLOR_YELLOW, g_V4VehicleLocked[playerid] ? "Pojazd zamkniety." : "Pojazd otwarty.");
            }
            case 3: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/engine");
            case 4: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/lights");
            case 5: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/seatbelt");
            case 6: ShowPlayerDialog(playerid, V4_DIALOG_NEON, DIALOG_STYLE_LIST, "Neony pod autem", "Wylaczone\nCzerwone\nNiebieskie\nZielone\nZolte\nRozowe\nBiale", "Wybierz", "Wroc");
            case 7:
            {
                new text[224];
                format(text, sizeof(text), "Model: %d\nPaliwo: %d%%\nPrzebieg: %d m\nPaintjob: %d\nFelgi: %d\nNeon: %s\nNitro 10x: TAK\nHydraulika: TAK\nZamek: %s",
                    g_V4OwnedModel[playerid], g_V4Fuel[playerid], g_V4Odometer[playerid],
                    g_V4OwnedPaintjob[playerid], g_V4OwnedWheels[playerid], g_V4NeonNames[g_V4OwnedNeon[playerid]],
                    g_V4VehicleLocked[playerid] ? "zamkniety" : "otwarty");
                ShowPlayerDialog(playerid, V4_DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX, "Moj pojazd", text, "OK", "");
            }
            case 8:
            {
                if (g_V4OwnedModel[playerid] != 0)
                {
                    AddMoney(playerid, 12500);
                    V4_DestroyNeon(playerid);
                    g_V4OwnedModel[playerid] = 0;
                    g_V4OwnedNeon[playerid] = 0;
                    if (g_V4OwnedVehicle[playerid] != INVALID_VEHICLE_ID)
                    {
                        new vehicleid = g_V4OwnedVehicle[playerid];
                        DestroyVehicle(vehicleid);
                        g_V4VehicleOwner[vehicleid] = 0;
                    }
                    g_V4OwnedVehicle[playerid] = INVALID_VEHICLE_ID;
                    V4_Save(playerid);
                    SendClientMessage(playerid, COLOR_GREEN, "Pojazd sprzedany za $12500.");
                }
            }
        }
        return 1;
    }
    if (dialogid == V4_DIALOG_NEON)
    {
        if (g_V4OwnedModel[playerid] == 0)
            return SendClientMessage(playerid, COLOR_RED, "Najpierw kup pojazd: /buycar [400-611]"), 1;
        if (listitem < 0 || listitem > 6) return 1;
        if (listitem > 0 && !IsSafeTuningModel(g_V4OwnedModel[playerid]))
            return SendClientMessage(playerid, COLOR_RED, "Neony sa dostepne tylko dla samochodow osobowych."), 1;
        g_V4OwnedNeon[playerid] = listitem;
        V4_ApplyNeon(playerid);
        V4_Save(playerid);
        new message[96];
        format(message, sizeof(message), "Neony: %s. Ustawienie zapisano.", g_V4NeonNames[listitem]);
        SendClientMessage(playerid, COLOR_GREEN, message);
        return 1;
    }
    if (dialogid == V4_DIALOG_REFUEL)
    {
        V4_ConfirmRefuel(playerid);
        return 1;
    }
    if (dialogid == V4_DIALOG_GPS)
    {
        switch (listitem)
        {
            case 0: TeleportPlayer(playerid, 2491.75, -1668.20, 13.34, 180.0);
            case 1: TeleportPlayer(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z, NEMQO_START_A);
            case 2: TeleportPlayer(playerid, 1481.12, -1769.22, 18.80, 0.0);
            case 3: TeleportPlayer(playerid, 1687.10, -2334.50, 13.55, 0.0);
            case 4: TeleportPlayer(playerid, -1985.75, 137.92, 27.69, 270.0);
            case 5: TeleportPlayer(playerid, 2027.30, 1008.60, 10.82, 90.0);
            case 6: TeleportPlayer(playerid, 1178.4, -1338.2, 13.5, 270.0);
            case 7: TeleportPlayer(playerid, 1545.0, -1615.2, 13.2, 180.0);
            case 8: Island_Teleport(playerid);
        }
        return 1;
    }
    if (dialogid == V4_DIALOG_ARENAS)
    {
        V4_EnterArena(playerid, listitem + 1);
        return 1;
    }
    if (dialogid == V4_DIALOG_RADIO)
    {
        if (listitem == 0) StopAudioStreamForPlayer(playerid);
        if (listitem == 1) PlayAudioStreamForPlayer(playerid, "https://stream.radioparadise.com/aac-320");
        if (listitem == 2) PlayAudioStreamForPlayer(playerid, "https://ice2.somafm.com/groovesalad-128-mp3");
        if (listitem == 3) PlayAudioStreamForPlayer(playerid, "https://stream.radioparadise.com/rock-320");
        return 1;
    }
    return 0;
}

public V4_OnPlayerCommandText(playerid, cmdtext[])
{
    if (V4_GrantCommand(playerid, cmdtext)) return 1;
    if (!strcmp(cmdtext, "/menu", true)) return V4_ShowMenu(playerid), 1;
    if (!strcmp(cmdtext, "/jobs", true)) return V4_ShowJobs(playerid), 1;
    if (!strcmp(cmdtext, "/work", true)) return V4_StartWork(playerid), 1;
    if (!strcmp(cmdtext, "/stopwork", true))
    {
        g_V4WorkActive[playerid] = 0;
        DisablePlayerCheckpoint(playerid);
        SendClientMessage(playerid, COLOR_YELLOW, "Zlecenie anulowane.");
        return 1;
    }
    if (!strcmp(cmdtext, "/race", true)) return V4_StartRace(playerid), 1;
    if (!strcmp(cmdtext, "/race quit", true))
    {
        g_V4RaceActive[playerid] = 0;
        DisablePlayerRaceCheckpoint(playerid);
        return 1;
    }
    if (!strcmp(cmdtext, "/v", true))
    {
        ShowPlayerDialog(playerid, V4_DIALOG_VEHICLE, DIALOG_STYLE_LIST, "Moj pojazd", "Podstaw\nZaparkuj\nZamek\nSilnik\nSwiatla\nPasy\nNeony\nInformacje\nSprzedaj", "Wybierz", "Zamknij");
        return 1;
    }
    if (!strcmp(cmdtext, "/neon", true))
    {
        ShowPlayerDialog(playerid, V4_DIALOG_NEON, DIALOG_STYLE_LIST, "Neony pod autem", "Wylaczone\nCzerwone\nNiebieskie\nZielone\nZolte\nRozowe\nBiale", "Wybierz", "Zamknij");
        return 1;
    }
    if (!strfind(cmdtext, "/buycar ", true))
    {
        new model = strval(cmdtext[8]);
        if (model < 400 || model > 611) return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /buycar [400-611]"), 1;
        if (g_V4OwnedModel[playerid] != 0) return SendClientMessage(playerid, COLOR_RED, "Masz juz pojazd."), 1;
        if (g_Money[playerid] < 25000) return SendClientMessage(playerid, COLOR_RED, "Pojazd kosztuje $25000."), 1;
        AddMoney(playerid, -25000);
        g_V4OwnedModel[playerid] = model;
        g_V4OwnedColor1[playerid] = random(126);
        g_V4OwnedColor2[playerid] = random(126);
        g_V4OwnedPaintjob[playerid] = -1;
        g_V4OwnedWheels[playerid] = 1080;
        g_V4OwnedNeon[playerid] = 0;
        g_V4Fuel[playerid] = 100;
        g_V4Odometer[playerid] = 0;
        g_V4OdoFraction[playerid] = 0.0;
        g_V4OdoSample[playerid][odoValid] = false;
        V4_Unlock(playerid, V4_ACH_CAR, "Pierwszy pojazd");
        V4_Save(playerid);
        V4_SpawnOwnedVehicle(playerid);
        return 1;
    }
    if (!strcmp(cmdtext, "/engine", true))
    {
        if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return SendClientMessage(playerid, COLOR_RED, "Musisz byc kierowca."), 1;
        new vehicleid = GetPlayerVehicleID(playerid);
        if (!g_V4Engine[playerid] && V4_IsFuelVehicle(vehicleid) && V4_GetVehicleFuel(vehicleid) <= 0)
            return SendClientMessage(playerid, COLOR_RED, "Nie uruchomisz silnika: bak jest pusty."), 1;
        g_V4Engine[playerid] = !g_V4Engine[playerid];
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
        SetVehicleParamsEx(vehicleid, g_V4Engine[playerid], lights, alarm, doors, bonnet, boot, objective);
        SendClientMessage(playerid, COLOR_YELLOW, g_V4Engine[playerid] ? "Silnik uruchomiony." : "Silnik zgaszony.");
        return 1;
    }
    if (!strcmp(cmdtext, "/lights", true))
    {
        if (!IsPlayerInAnyVehicle(playerid)) return 1;
        g_V4Lights[playerid] = !g_V4Lights[playerid];
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(GetPlayerVehicleID(playerid), engine, lights, alarm, doors, bonnet, boot, objective);
        SetVehicleParamsEx(GetPlayerVehicleID(playerid), engine, g_V4Lights[playerid], alarm, doors, bonnet, boot, objective);
        return 1;
    }
    if (!strcmp(cmdtext, "/seatbelt", true))
    {
        g_V4Seatbelt[playerid] = !g_V4Seatbelt[playerid];
        SendClientMessage(playerid, COLOR_YELLOW, g_V4Seatbelt[playerid] ? "Pasy zapiete." : "Pasy odpiete.");
        return 1;
    }
    if (!strcmp(cmdtext, "/refuel", true))
    {
        V4_ShowRefuelDialog(playerid);
        return 1;
    }
    if (!strfind(cmdtext, "/buyhouse ", true)) return V4_BuyHouse(playerid, strval(cmdtext[10])), 1;
    if (!strcmp(cmdtext, "/home", true))
    {
        if (g_V4House[playerid] == 0) return SendClientMessage(playerid, COLOR_RED, "Nie masz domu."), 1;
        new h = g_V4House[playerid];
        TeleportPlayer(playerid, g_V4HousePos[h][0], g_V4HousePos[h][1], g_V4HousePos[h][2], 0.0);
        return 1;
    }
    if (!strcmp(cmdtext, "/house", true))
    {
        new text[192];
        format(text, sizeof(text), "Twoj dom: %d\nKupno: /buyhouse [1-5]\nTeleport: /home\nSprzedaz: /sellhouse", g_V4House[playerid]);
        ShowPlayerDialog(playerid, V4_DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX, "Domy", text, "OK", "");
        return 1;
    }
    if (!strcmp(cmdtext, "/sellhouse", true))
    {
        if (g_V4House[playerid] == 0) return 1;
        new q[160];
        format(q, sizeof(q), "UPDATE houses SET owner='' WHERE id=%d", g_V4House[playerid]);
        new DBResult:r = db_query(g_DB, q); db_free_result(r);
        AddMoney(playerid, g_V4HousePrice[g_V4House[playerid]] / 2);
        g_V4House[playerid] = 0; V4_Save(playerid);
        return 1;
    }
    if (!strfind(cmdtext, "/buybiz ", true)) return V4_BuyBusiness(playerid, strval(cmdtext[8])), 1;
    if (!strcmp(cmdtext, "/business", true))
    {
        new text[192];
        format(text, sizeof(text), "Twoja firma: %d\nKupno: /buybiz [1-3]\nPremia: /bizbonus", g_V4Business[playerid]);
        ShowPlayerDialog(playerid, V4_DIALOG_ACHIEVEMENTS, DIALOG_STYLE_MSGBOX, "Firmy", text, "OK", "");
        return 1;
    }
    if (!strcmp(cmdtext, "/bizbonus", true))
    {
        if (g_V4Business[playerid] == 0) return SendClientMessage(playerid, COLOR_RED, "Nie masz firmy."), 1;
        new q[192];
        format(q, sizeof(q), "SELECT last_payout FROM businesses WHERE id=%d", g_V4Business[playerid]);
        new DBResult:r = db_query(g_DB, q);
        new lastPayout = db_get_field_assoc_int(r, "last_payout");
        db_free_result(r);
        new remaining = 3600 - (gettime() - lastPayout);
        if (remaining > 0)
        {
            new msg[96];
            format(msg, sizeof(msg), "Kolejna wyplata firmy za %d minut.", (remaining / 60) + 1);
            SendClientMessage(playerid, COLOR_YELLOW, msg);
            return 1;
        }
        format(q, sizeof(q), "UPDATE businesses SET last_payout=%d WHERE id=%d", gettime(), g_V4Business[playerid]);
        r = db_query(g_DB, q); db_free_result(r);
        AddMoney(playerid, 1500);
        V4_Save(playerid);
        SendClientMessage(playerid, COLOR_GREEN, "Firma wyplacila godzinny zysk: $1500.");
        return 1;
    }
    if (!strcmp(cmdtext, "/balance", true))
    {
        new msg[96];
        format(msg, sizeof(msg), "Gotowka: $%d | Konto bankowe: $%d", g_Money[playerid], g_V4Bank[playerid]);
        SendClientMessage(playerid, COLOR_YELLOW, msg);
        return 1;
    }
    if (!strfind(cmdtext, "/deposit ", true))
    {
        new amount = strval(cmdtext[9]);
        if (amount < 1 || amount > g_Money[playerid])
            return SendClientMessage(playerid, COLOR_RED, "Uzycie: /deposit [kwota posiadanej gotowki]"), 1;
        AddMoney(playerid, -amount);
        g_V4Bank[playerid] += amount;
        V4_Save(playerid);
        SendClientMessage(playerid, COLOR_GREEN, "Pieniadze wplacone do banku.");
        return 1;
    }
    if (!strfind(cmdtext, "/withdraw ", true))
    {
        new amount = strval(cmdtext[10]);
        if (amount < 1 || amount > g_V4Bank[playerid])
            return SendClientMessage(playerid, COLOR_RED, "Uzycie: /withdraw [kwota z konta]"), 1;
        g_V4Bank[playerid] -= amount;
        AddMoney(playerid, amount);
        V4_Save(playerid);
        SendClientMessage(playerid, COLOR_GREEN, "Pieniadze wyplacone z banku.");
        return 1;
    }
    if (!strfind(cmdtext, "/fix ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_MECHANIC)
            return SendClientMessage(playerid, COLOR_RED, "Musisz pracowac jako mechanik."), 1;
        new target = strval(cmdtext[5]);
        if (!V4_PlayerNearPlayer(playerid, target, 8.0) || !IsPlayerInAnyVehicle(target))
            return SendClientMessage(playerid, COLOR_RED, "Klient musi byc blisko i siedziec w pojezdzie."), 1;
        RepairVehicle(GetPlayerVehicleID(target));
        SetVehicleHealth(GetPlayerVehicleID(target), 1000.0);
        AddMoney(playerid, 500);
        AddMoney(target, -500);
        g_V4JobXP[playerid] += 5;
        SendClientMessage(playerid, COLOR_GREEN, "Naprawa wykonana: +$500 i +5 Job XP.");
        SendClientMessage(target, COLOR_YELLOW, "Mechanik naprawil pojazd. Koszt: $500.");
        V4_Save(playerid); V4_Save(target);
        return 1;
    }
    if (!strfind(cmdtext, "/medic ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_MEDIC)
            return SendClientMessage(playerid, COLOR_RED, "Musisz pracowac jako ratownik."), 1;
        new target = strval(cmdtext[7]);
        if (!V4_PlayerNearPlayer(playerid, target, 8.0))
            return SendClientMessage(playerid, COLOR_RED, "Pacjent nie jest w poblizu."), 1;
        SetPlayerHealth(target, 100.0);
        AddMoney(playerid, 400);
        AddMoney(target, -400);
        g_V4JobXP[playerid] += 5;
        SendClientMessage(playerid, COLOR_GREEN, "Pomoc udzielona: +$400 i +5 Job XP.");
        SendClientMessage(target, COLOR_YELLOW, "Ratownik odnowil zdrowie. Koszt: $400.");
        V4_Save(playerid); V4_Save(target);
        return 1;
    }
    if (!strcmp(cmdtext, "/wanted", true))
    {
        new msg[80]; format(msg, sizeof(msg), "Poziom poszukiwania: %d/6", g_V4Wanted[playerid]);
        SendClientMessage(playerid, COLOR_YELLOW, msg); return 1;
    }
    if (!strfind(cmdtext, "/ticket ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_POLICE) return SendClientMessage(playerid, COLOR_RED, "Musisz pracowac jako policjant."), 1;
        new target = strval(cmdtext[8]);
        if (!V4_PlayerNearPlayer(playerid, target, 10.0) || g_V4Wanted[target] < 1) return SendClientMessage(playerid, COLOR_RED, "Brak poszukiwanego gracza w poblizu."), 1;
        g_V4Wanted[target]--; SetPlayerWantedLevel(target, g_V4Wanted[target]); AddMoney(target, -500); AddMoney(playerid, 250);
        SendClientMessage(target, COLOR_YELLOW, "Otrzymales mandat $500."); V4_Save(target); V4_Save(playerid); return 1;
    }
    if (!strfind(cmdtext, "/arrest ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_POLICE) return SendClientMessage(playerid, COLOR_RED, "Musisz pracowac jako policjant."), 1;
        new target = strval(cmdtext[8]);
        if (!V4_PlayerNearPlayer(playerid, target, 10.0) || g_V4Wanted[target] < 3) return SendClientMessage(playerid, COLOR_RED, "Cel musi byc blisko i miec Wanted 3+."), 1;
        g_V4JailSeconds[target] = g_V4Wanted[target] * 30;
        AddMoney(playerid, g_V4Wanted[target] * 500);
        g_V4Wanted[target] = 0; SetPlayerWantedLevel(target, 0);
        SetPlayerPos(target, 264.6, 77.5, 1001.0); SetPlayerInterior(target, 6); SetPlayerVirtualWorld(target, 90);
        TogglePlayerControllable(target, true); V4_Save(target); return 1;
    }
    if (!strfind(cmdtext, "/cuff ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_POLICE) return 1;
        new target = strval(cmdtext[6]);
        if (V4_PlayerNearPlayer(playerid, target, 5.0)) TogglePlayerControllable(target, false);
        return 1;
    }
    if (!strfind(cmdtext, "/uncuff ", true))
    {
        if (g_V4Job[playerid] != V4_JOB_POLICE) return 1;
        new target = strval(cmdtext[8]); if (IsPlayerConnected(target)) TogglePlayerControllable(target, true); return 1;
    }
    if (!strcmp(cmdtext, "/radar", true))
    {
        if (g_V4Job[playerid] != V4_JOB_POLICE) return 1;
        new fastest = INVALID_PLAYER_ID, top = 0;
        for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i) && IsPlayerInAnyVehicle(i) && V4_PlayerNearPlayer(playerid, i, 100.0))
        {
            new Float:vx, Float:vy, Float:vz; GetVehicleVelocity(GetPlayerVehicleID(i), vx, vy, vz);
            new s = floatround(floatsqroot(vx*vx + vy*vy + vz*vz) * 180.0);
            if (s > top) { top = s; fastest = i; }
        }
        new msg[96]; format(msg, sizeof(msg), "Radar: gracz %d, predkosc %d km/h", fastest, top); SendClientMessage(playerid, COLOR_YELLOW, msg); return 1;
    }
    if (!strcmp(cmdtext, "/arena", true))
    {
        ShowPlayerDialog(playerid, V4_DIALOG_ARENAS, DIALOG_STYLE_LIST, "Areny", "Derby\nPaintball\nSniper\nMinigun\nZombie Survival\nStunt\nParkour\nGrove vs Ballas", "Wejdz", "Zamknij"); return 1;
    }
    if (!strcmp(cmdtext, "/derby", true)) return V4_EnterArena(playerid, V4_ARENA_DERBY), 1;
    if (!strcmp(cmdtext, "/paintball", true)) return V4_EnterArena(playerid, V4_ARENA_PAINTBALL), 1;
    if (!strcmp(cmdtext, "/sniper", true)) return V4_EnterArena(playerid, V4_ARENA_SNIPER), 1;
    if (!strcmp(cmdtext, "/minigun", true)) return V4_EnterArena(playerid, V4_ARENA_MINIGUN), 1;
    if (!strcmp(cmdtext, "/zombie", true)) return V4_EnterArena(playerid, V4_ARENA_ZOMBIE), 1;
    if (!strcmp(cmdtext, "/stunt", true)) return V4_EnterArena(playerid, V4_ARENA_STUNT), 1;
    if (!strcmp(cmdtext, "/parkour", true)) return V4_EnterArena(playerid, V4_ARENA_PARKOUR), 1;
    if (!strcmp(cmdtext, "/gang", true)) return V4_EnterArena(playerid, V4_ARENA_GANG), 1;
    if (!strcmp(cmdtext, "/leavearena", true)) return V4_LeaveArena(playerid), 1;
    if (!strfind(cmdtext, "/duel ", true))
    {
        new target = strval(cmdtext[6]);
        if (!IsPlayerConnected(target) || target == playerid) return 1;
        g_V4Arena[playerid] = V4_ARENA_DUEL; g_V4Arena[target] = V4_ARENA_DUEL;
        SetPlayerVirtualWorld(playerid, 100 + playerid); SetPlayerVirtualWorld(target, 100 + playerid);
        SetPlayerPos(playerid, 1340.0, 2110.0, 11.0); SetPlayerPos(target, 1390.0, 2110.0, 11.0);
        ResetPlayerWeapons(playerid); ResetPlayerWeapons(target); GivePlayerWeapon(playerid, 24, 100); GivePlayerWeapon(target, 24, 100); return 1;
    }
    if (!strcmp(cmdtext, "/achievements", true)) return V4_ShowAchievements(playerid), 1;
    if (!strcmp(cmdtext, "/daily", true)) return V4_ShowDaily(playerid), 1;
    if (!strcmp(cmdtext, "/gps", true))
    {
        ShowPlayerDialog(playerid, V4_DIALOG_GPS, DIALOG_STYLE_LIST, "GPS", "Dom CJ\nRatusz LS\nLotnisko LS\nCentrum SF\nStrip LV\nSzpital LS\nKomisariat LS", "Jedz", "Zamknij"); return 1;
    }
    if (!strcmp(cmdtext, "/radio", true))
    {
        ShowPlayerDialog(playerid, V4_DIALOG_RADIO, DIALOG_STYLE_LIST, "Radio", "Wylacz\nRadio Paradise\nSomaFM Groove Salad\nRadio Paradise Rock", "Wybierz", "Zamknij"); return 1;
    }
    if (!strfind(cmdtext, "/global ", true))
    {
        new msg[160]; format(msg, sizeof(msg), "[GLOBAL] %s: %s", g_Name[playerid], cmdtext[8]); SendClientMessageToAll(COLOR_WHITE, msg); return 1;
    }
    if (!strfind(cmdtext, "/pm ", true))
    {
        new target = strval(cmdtext[4]);
        new split = strfind(cmdtext, " ", false, 4);
        if (!IsPlayerConnected(target) || split == -1) return SendClientMessage(playerid, COLOR_RED, "Uzycie: /pm [id] [tekst]"), 1;
        new text[128], msg[180]; strmid(text, cmdtext, split + 1, strlen(cmdtext), sizeof(text));
        format(msg, sizeof(msg), "[PM od %s] %s", g_Name[playerid], text); SendClientMessage(target, COLOR_YELLOW, msg);
        format(msg, sizeof(msg), "[PM do %s] %s", g_Name[target], text); SendClientMessage(playerid, COLOR_YELLOW, msg); return 1;
    }
    if (!strfind(cmdtext, "/report ", true))
    {
        new target = strval(cmdtext[8]); new split = strfind(cmdtext, " ", false, 8);
        if (!IsPlayerConnected(target) || split == -1) return SendClientMessage(playerid, COLOR_RED, "Uzycie: /report [id] [powod]"), 1;
        new reason[96], q[384]; strmid(reason, cmdtext, split + 1, strlen(cmdtext), sizeof(reason));
        format(q, sizeof(q), "INSERT INTO reports (reporter,target,reason,created_at,status) VALUES ('%s','%s','%s',%d,0)", g_Name[playerid], g_Name[target], reason, gettime());
        new DBResult:r = db_query(g_DB, q); db_free_result(r); SendClientMessage(playerid, COLOR_GREEN, "Report zapisany dla administracji."); return 1;
    }
    if (!strcmp(cmdtext, "/reports", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new DBResult:r = db_query(g_DB, "SELECT id,reporter,target,reason FROM reports WHERE status=0 ORDER BY id DESC LIMIT 5");
        new text[768] = "";
        if (db_num_rows(r) == 0) format(text, sizeof(text), "Brak otwartych reportow.");
        else
        {
            do
            {
                new id = db_get_field_assoc_int(r, "id");
                new reporter[MAX_PLAYER_NAME + 1], target[MAX_PLAYER_NAME + 1], reason[96], line[180];
                db_get_field_assoc(r, "reporter", reporter, sizeof(reporter));
                db_get_field_assoc(r, "target", target, sizeof(target));
                db_get_field_assoc(r, "reason", reason, sizeof(reason));
                format(line, sizeof(line), "#%d %s -> %s: %s\n", id, reporter, target, reason);
                strcat(text, line, sizeof(text));
            }
            while (db_next_row(r));
        }
        db_free_result(r);
        ShowPlayerDialog(playerid, V4_DIALOG_ADMIN, DIALOG_STYLE_MSGBOX, "Ostatnie reporty", text, "OK", "");
        return 1;
    }
    if (!strcmp(cmdtext, "/adminhelp", true))
    {
        if (!V4_GrantActor(playerid)) return 1;
        ShowPlayerDialog(playerid, V4_DIALOG_ADMIN, DIALOG_STYLE_MSGBOX, "Administracja",
            "/givemoney [id] [1-1000000]\n/givexp [id] [1-100000]\n/reports - ostatnie zgloszenia\n/kick [id]\n/ban [id]\n/unban [nick]\n/mute [id]\n/jail [id]\n/goto [id]\n/gethere [id]\n/spec [id]\n/specoff",
            "OK", "");
        return 1;
    }
    if (!strfind(cmdtext, "/kick ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[6]);
        if (IsPlayerConnected(target)) { V4_AdminLog(playerid, target, "kick"); Kick(target); }
        return 1;
    }
    if (!strfind(cmdtext, "/mute ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[6]);
        if (IsPlayerConnected(target))
        {
            g_V4Muted[target] = !g_V4Muted[target];
            V4_AdminLog(playerid, target, "mute_toggle");
        }
        return 1;
    }
    if (!strfind(cmdtext, "/jail ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[6]);
        if (IsPlayerConnected(target))
        {
            V4_AdminLog(playerid, target, "jail_120");
            g_V4JailSeconds[target] = 120;
            SetPlayerPos(target, 264.6, 77.5, 1001.0);
            SetPlayerInterior(target, 6);
            SetPlayerVirtualWorld(target, 90);
        }
        return 1;
    }
    if (!strfind(cmdtext, "/goto ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[6]);
        if (IsPlayerConnected(target))
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(target, x, y, z);
            SetPlayerPos(playerid, x + 1.0, y, z);
            SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(target));
        }
        return 1;
    }
    if (!strfind(cmdtext, "/gethere ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[9]);
        if (IsPlayerConnected(target))
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(playerid, x, y, z);
            SetPlayerPos(target, x + 1.0, y, z);
            SetPlayerVirtualWorld(target, GetPlayerVirtualWorld(playerid));
        }
        return 1;
    }
    if (!strfind(cmdtext, "/ban ", true))
    {
        if (g_V4Admin[playerid] < 2) return 1;
        new target = strval(cmdtext[5]);
        if (!IsPlayerConnected(target)) return 1;
        new q[320];
        format(q, sizeof(q), "INSERT OR REPLACE INTO bans (name,reason,admin,created_at) VALUES ('%s','Zbanowany przez administratora','%s',%d)",
            g_Name[target], g_Name[playerid], gettime());
        new DBResult:r = db_query(g_DB, q); db_free_result(r);
        V4_AdminLog(playerid, target, "ban");
        Kick(target);
        return 1;
    }
    if (!strfind(cmdtext, "/unban ", true))
    {
        if (g_V4Admin[playerid] < 2) return 1;
        new name[MAX_PLAYER_NAME + 1], q[160];
        strmid(name, cmdtext, 7, strlen(cmdtext), sizeof(name));
        if (!IsValidAccountName(name))
            return SendClientMessage(playerid, COLOR_RED, "Nieprawidlowa nazwa gracza."), 1;
        format(q, sizeof(q), "DELETE FROM bans WHERE name='%s'", name);
        new DBResult:r = db_query(g_DB, q); db_free_result(r);
        SendClientMessage(playerid, COLOR_GREEN, "Ban konta usuniety.");
        return 1;
    }
    if (!strfind(cmdtext, "/spec ", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        new target = strval(cmdtext[6]);
        if (!IsPlayerConnected(target) || target == playerid) return 1;
        TogglePlayerSpectating(playerid, true);
        PlayerSpectatePlayer(playerid, target);
        return 1;
    }
    if (!strcmp(cmdtext, "/specoff", true))
    {
        if (g_V4Admin[playerid] < 1) return 1;
        TogglePlayerSpectating(playerid, false);
        SetSpawnInfo(playerid, 0, g_Skin[playerid], NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z, NEMQO_START_A, 0, 0, 0, 0, 0, 0);
        SpawnPlayer(playerid);
        return 1;
    }
    return 0;
}

public V4_OnPlayerDeath(playerid, killerid)
{
    if (killerid != INVALID_PLAYER_ID && killerid != playerid && IsPlayerConnected(killerid))
    {
        if (!g_InDM[killerid] && !g_InSumo[killerid] && !V4_IsInArena(killerid))
        {
            if (g_V4Wanted[killerid] < 6) g_V4Wanted[killerid]++;
            SetPlayerWantedLevel(killerid, g_V4Wanted[killerid]);
            SendClientMessage(killerid, COLOR_RED, "Morderstwo poza arena: wzrasta poziom Wanted.");
        }
    }
    return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
    if (g_V4WorkActive[playerid])
    {
        g_V4WorkActive[playerid] = 0;
        DisablePlayerCheckpoint(playerid);
        new reward = 800 + (g_V4Job[playerid] * 150);
        AddMoney(playerid, reward);
        g_V4JobXP[playerid] += 10;
        V4_Unlock(playerid, V4_ACH_FIRST_JOB, "Pierwsze zlecenie");
        new today = V4_GetDailyKey();
        if (g_V4DailyDay[playerid] != today) { g_V4DailyDay[playerid] = today; g_V4DailyProgress[playerid] = 0; }
        if (g_V4DailyProgress[playerid] < 3) g_V4DailyProgress[playerid]++;
        if (g_V4DailyProgress[playerid] == 3)
        {
            AddMoney(playerid, 5000); AddXP(playerid, 25); g_V4DailyProgress[playerid] = 4;
            SendClientMessage(playerid, COLOR_YELLOW, "Zadanie dzienne ukonczone: +$5000 i +25 XP.");
        }
        new msg[96]; format(msg, sizeof(msg), "Zlecenie wykonane: +$%d i +10 Job XP.", reward); SendClientMessage(playerid, COLOR_GREEN, msg);
        V4_Save(playerid);
        return 1;
    }
    return 1;
}

public OnPlayerEnterRaceCheckpoint(playerid)
{
    if (!g_V4RaceActive[playerid]) return 1;
    g_V4RaceCP[playerid]++;
    if (g_V4RaceCP[playerid] >= sizeof(g_V4RacePoints))
    {
        new raceTime = gettime() - g_V4RaceStart[playerid];
        g_V4RaceActive[playerid] = 0; DisablePlayerRaceCheckpoint(playerid); AddMoney(playerid, 3000); AddXP(playerid, 20);
        if (g_V4RaceBest[playerid] == 0 || raceTime < g_V4RaceBest[playerid]) g_V4RaceBest[playerid] = raceTime;
        V4_Unlock(playerid, V4_ACH_RACE, "Ukonczony wyscig"); V4_Save(playerid);
        new msg[128]; format(msg,sizeof(msg),"Meta! Czas %d s | rekord %d s | +$3000 +20 XP",raceTime,g_V4RaceBest[playerid]); SendClientMessage(playerid,COLOR_GREEN,msg); return 1;
    }
    new cp = g_V4RaceCP[playerid];
    new next = cp + 1; if (next >= sizeof(g_V4RacePoints)) next = cp;
    SetPlayerRaceCheckpoint(playerid, (cp == sizeof(g_V4RacePoints)-1) ? 1 : 0,
        g_V4RacePoints[cp][0],g_V4RacePoints[cp][1],g_V4RacePoints[cp][2],
        g_V4RacePoints[next][0],g_V4RacePoints[next][1],g_V4RacePoints[next][2],7.0);
    return 1;
}

public OnPlayerStateChange(playerid, newstate, oldstate)
{
    #pragma unused oldstate
    V4_OdoResetDriver(playerid);
    g_V4FuelTick[playerid] = 0;
    if (newstate == PLAYER_STATE_DRIVER)
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        new owner = g_V4VehicleOwner[vehicleid] - 1;
        if (owner >= 0 && owner != playerid && g_V4VehicleLocked[owner])
        {
            RemovePlayerFromVehicle(playerid);
            SendClientMessage(playerid, COLOR_RED, "Ten pojazd jest zamkniety.");
            return 1;
        }
        new engine,lights,alarm,doors,bonnet,boot,objective;
        GetVehicleParamsEx(vehicleid,engine,lights,alarm,doors,bonnet,boot,objective);
        new canStart = !V4_IsFuelVehicle(vehicleid) || V4_GetVehicleFuel(vehicleid) > 0;
        SetVehicleParamsEx(vehicleid,canStart,lights,alarm,doors,bonnet,boot,objective);
        g_V4Engine[playerid] = bool:canStart;
        if (!canStart) SendClientMessage(playerid, COLOR_RED, "Bak jest pusty. Zatankuj pojazd przy stacji.");
    }
    if (newstate == PLAYER_STATE_ONFOOT) g_V4Seatbelt[playerid] = false;
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if (g_V4Muted[playerid])
    {
        SendClientMessage(playerid, COLOR_RED, "Masz wyciszony czat.");
        return 0;
    }
    new msg[180]; format(msg, sizeof(msg), "%s mowi: %s", g_Name[playerid], text);
    new Float:x,Float:y,Float:z; GetPlayerPos(playerid,x,y,z);
    for (new i=0;i<MAX_PLAYERS;i++) if (IsPlayerConnected(i) && IsPlayerInRangeOfPoint(i,100.0,x,y,z)) SendClientMessage(i,COLOR_WHITE,msg);
    return 0;
}

public V4_SecondTick()
{
    static labelTick = 0;
    if (++labelTick >= 2)
    {
        labelTick = 0;
        V4_UpdateAllVehicleLabels();
    }
    for (new playerid=0;playerid<MAX_PLAYERS;playerid++)
    {
        if (!IsPlayerConnected(playerid) || !g_Logged[playerid]) continue;
        V4_UpdateFuelArea(playerid);
        if (g_V4JailSeconds[playerid] > 0)
        {
            g_V4JailSeconds[playerid]--;
            if (g_V4JailSeconds[playerid] == 0)
            {
                SetPlayerInterior(playerid,0); SetPlayerVirtualWorld(playerid,0); SetPlayerPos(playerid,1545.0,-1675.0,13.5);
                SendClientMessage(playerid,COLOR_GREEN,"Kara wiezienia zakonczona.");
            }
        }
        if (GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
        {
            new vehicleid = GetPlayerVehicleID(playerid);
            if (V4_IsFuelVehicle(vehicleid))
            {
                new Float:vx,Float:vy,Float:vz; GetVehicleVelocity(vehicleid,vx,vy,vz);
                new speed=floatround(floatsqroot(vx*vx+vy*vy+vz*vz)*180.0);
                g_V4FuelTick[playerid]++;
                if (g_V4FuelTick[playerid]>=20 && speed>2)
                {
                    g_V4FuelTick[playerid]=0;
                    new oldFuel = V4_GetVehicleFuel(vehicleid);
                    if (oldFuel > 0) V4_SetVehicleFuel(vehicleid, oldFuel - 1);
                    if (oldFuel > 0 && V4_GetVehicleFuel(vehicleid) <= 0)
                    {
                        g_V4Engine[playerid]=false;
                        new e,l,a,d,bn,bt,o; GetVehicleParamsEx(vehicleid,e,l,a,d,bn,bt,o); SetVehicleParamsEx(vehicleid,0,l,a,d,bn,bt,o);
                        SendClientMessage(playerid,COLOR_RED,"Brak paliwa. Uzyj /refuel przy stacji.");
                    }
                    new owner = g_V4VehicleOwner[vehicleid] - 1;
                    if (g_V4VehicleKind[vehicleid] == V4_VEHICLE_KIND_PRIVATE && owner >= 0 && owner < MAX_PLAYERS && g_Logged[owner]) V4_Save(owner);
                    V4_UpdateVehicleLabel(vehicleid);
                }
                if (speed>=200) V4_Unlock(playerid,V4_ACH_SPEED,"Predkosc 200 km/h");
            }
            else g_V4FuelTick[playerid] = 0;
        }
        if (g_Money[playerid]>=100000) V4_Unlock(playerid,V4_ACH_RICH,"Bogacz");
        if (GetTickCount() - g_V4OdoSaveTick[playerid] >= 30000)
        {
            V4_Save(playerid);
            g_V4OdoSaveTick[playerid] = GetTickCount();
        }
    }
    return 1;
}

public V4_Announcement()
{
    static index=0;
    static const messages[][]=
    {
        "[INFO] /menu otwiera glowne menu serwera.",
        "[INFO] Prace: /jobs, potem /work.",
        "[INFO] Wyscig: /race | Areny: /arena.",
        "[INFO] Wlasne auto: /buycar [model], potem /v.",
        "[INFO] Pomoc administracji: /report [id] [powod]."
    };
    SendClientMessageToAll(COLOR_YELLOW,messages[index]);
    index++; if (index>=sizeof(messages)) index=0;
    return 1;
}
