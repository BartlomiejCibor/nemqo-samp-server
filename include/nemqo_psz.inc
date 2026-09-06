// NEMQO PSZ Tribute - mechaniki Drift & Zabawa dla SA-MP 0.3.7-R2.
forward PSZ_SecondTick();
forward PSZ_AutoEvent();
forward PSZ_DestroyRamp(playerid, objectid);
forward Float:PSZ_AngleDifference(Float:a, Float:b);

stock PSZ_Init()
{
    SetTimer("PSZ_SecondTick", 1000, true);
    SetTimer("PSZ_AutoEvent", 900000, true);
    g_PSZHidePhase = PSZ_HIDE_IDLE;
    g_PSZCurrentEvent = PSZ_EVENT_NONE;
    PSZ_CreatePlayground();
    return 1;
}

stock PSZ_RunSelfTest()
{
    new failures = 0, actorCount = 0, objectCount = 0, playgroundVehicles = 0, pierElegyCount = 0;
    if (g_V41GroundVehiclesCreated != 171) failures++;
    if (g_V41AircraftCreated != 13) failures++;
    for (new i = 0; i < V41_AMBIENT_ACTORS; i++)
        if (g_V41Actors[i] != INVALID_ACTOR_ID) actorCount++;
    for (new i = 0; i < PSZ_PLAYGROUND_OBJECTS; i++)
        if (g_PSZPlaygroundObjects[i] != INVALID_OBJECT_ID) objectCount++;
    for (new i = 0; i < PSZ_PLAYGROUND_VEHICLES; i++)
        if (g_PSZPlaygroundVehicles[i] != INVALID_VEHICLE_ID) playgroundVehicles++;
    for (new i = 0; i < 3; i++)
        if (g_V41PierElegy[i] != INVALID_VEHICLE_ID && GetVehicleModel(g_V41PierElegy[i]) == 562) pierElegyCount++;
    if (actorCount != 24) failures++;
    if (objectCount != 32) failures++;
    if (playgroundVehicles != 20) failures++;
    if (pierElegyCount != 3) failures++;
    if (floatround(PSZ_AngleDifference(350.0, 10.0)) != 20) failures++;
    for (new i = 1; i < sizeof(g_PSZRankLimits); i++)
        if (g_PSZRankLimits[i] <= g_PSZRankLimits[i - 1]) failures++;

    if (failures == 0)
        printf("NEMQO PSZ self-test: PASS | fleet=306 aircraft=26 actors=%d objects=%d playground_vehicles=%d pier_elegy=%d angle_diff=%d",
            actorCount, objectCount, playgroundVehicles, pierElegyCount, floatround(PSZ_AngleDifference(350.0, 10.0)));
    else
        printf("NEMQO PSZ self-test: FAIL(%d) | actors=%d objects=%d playground_vehicles=%d pier_elegy=%d angle_diff=%d",
            failures, actorCount, objectCount, playgroundVehicles, pierElegyCount, floatround(PSZ_AngleDifference(350.0, 10.0)));
    return failures == 0;
}

stock PSZ_Reset(playerid)
{
    g_RememberSkin[playerid] = false;
    g_PSZRespect[playerid] = 0;
    g_PSZDriftTotal[playerid] = 0;
    g_PSZDriftBest[playerid] = 0;
    g_PSZHideWins[playerid] = 0;
    g_PSZEventWins[playerid] = 0;
    g_PSZDriftMode[playerid] = false;
    g_PSZDriftCombo[playerid] = 0;
    g_PSZDriftTicks[playerid] = 0;
    g_PSZTandemPartner[playerid] = INVALID_PLAYER_ID;
    g_PSZDriftTextVisible[playerid] = false;
    g_PSZRampObject[playerid] = INVALID_OBJECT_ID;
    g_PSZRampLastTick[playerid] = 0;
    g_PSZPanelEnabled[playerid] = true;
    g_PSZPanelVisible[playerid] = false;
    g_PSZHideJoined[playerid] = false;
    g_PSZHideSeeker[playerid] = false;
    g_PSZHideFound[playerid] = false;
    g_PSZHideOutsideTicks[playerid] = 0;
    g_PSZHideSnapshotValid[playerid] = false;
    return 1;
}

stock PSZ_Load(playerid)
{
    new query[128];
    format(query, sizeof(query), "SELECT * FROM accounts WHERE name='%s' LIMIT 1", g_Name[playerid]);
    new DBResult:result = db_query(g_DB, query);
    if (db_num_rows(result) > 0)
    {
        g_RememberSkin[playerid] = db_get_field_assoc_int(result, "remember_skin") != 0;
        g_PSZRespect[playerid] = db_get_field_assoc_int(result, "respect");
        g_PSZDriftTotal[playerid] = db_get_field_assoc_int(result, "drift_total");
        g_PSZDriftBest[playerid] = db_get_field_assoc_int(result, "drift_best");
        g_PSZHideWins[playerid] = db_get_field_assoc_int(result, "hide_wins");
        g_PSZEventWins[playerid] = db_get_field_assoc_int(result, "event_wins");
    }
    db_free_result(result);
    return 1;
}

stock PSZ_Save(playerid)
{
    if (!g_AccountExists[playerid]) return 0;
    new query[384];
    format(query, sizeof(query),
        "UPDATE accounts SET remember_skin=%d,respect=%d,drift_total=%d,drift_best=%d,hide_wins=%d,event_wins=%d WHERE name='%s'",
        g_RememberSkin[playerid], g_PSZRespect[playerid], g_PSZDriftTotal[playerid],
        g_PSZDriftBest[playerid], g_PSZHideWins[playerid], g_PSZEventWins[playerid], g_Name[playerid]);
    new DBResult:result = db_query(g_DB, query);
    db_free_result(result);
    return 1;
}

stock PSZ_CreatePlayerUI(playerid)
{
    g_PSZPanelBackground[playerid] = CreatePlayerTextDraw(playerid, 5.0, 112.0, "_");
    PlayerTextDrawAlignment(playerid, g_PSZPanelBackground[playerid], 1);
    PlayerTextDrawLetterSize(playerid, g_PSZPanelBackground[playerid], 0.0, 13.7);
    PlayerTextDrawUseBox(playerid, g_PSZPanelBackground[playerid], true);
    PlayerTextDrawBoxColor(playerid, g_PSZPanelBackground[playerid], 0x11182188);
    PlayerTextDrawTextSize(playerid, g_PSZPanelBackground[playerid], 155.0, 0.0);

    g_PSZPanelText[playerid] = CreatePlayerTextDraw(playerid, 12.0, 116.0, "~y~ZABAWY");
    PlayerTextDrawAlignment(playerid, g_PSZPanelText[playerid], 1);
    PlayerTextDrawBackgroundColor(playerid, g_PSZPanelText[playerid], 0x000000FF);
    PlayerTextDrawFont(playerid, g_PSZPanelText[playerid], 1);
    PlayerTextDrawLetterSize(playerid, g_PSZPanelText[playerid], 0.270, 1.15);
    PlayerTextDrawColor(playerid, g_PSZPanelText[playerid], COLOR_WHITE);
    PlayerTextDrawSetOutline(playerid, g_PSZPanelText[playerid], 1);
    PlayerTextDrawSetProportional(playerid, g_PSZPanelText[playerid], true);

    g_PSZDriftText[playerid] = CreatePlayerTextDraw(playerid, 320.0, 305.0, "~y~DRIFT COMBO");
    PlayerTextDrawAlignment(playerid, g_PSZDriftText[playerid], 2);
    PlayerTextDrawFont(playerid, g_PSZDriftText[playerid], 2);
    PlayerTextDrawLetterSize(playerid, g_PSZDriftText[playerid], 0.36, 1.55);
    PlayerTextDrawColor(playerid, g_PSZDriftText[playerid], COLOR_WHITE);
    PlayerTextDrawSetOutline(playerid, g_PSZDriftText[playerid], 1);
    return 1;
}

stock PSZ_DestroyPlayerUI(playerid)
{
    PlayerTextDrawDestroy(playerid, g_PSZPanelBackground[playerid]);
    PlayerTextDrawDestroy(playerid, g_PSZPanelText[playerid]);
    PlayerTextDrawDestroy(playerid, g_PSZDriftText[playerid]);
    return 1;
}

stock PSZ_OnLogin(playerid)
{
    if (g_PSZPanelEnabled[playerid])
    {
        PlayerTextDrawShow(playerid, g_PSZPanelBackground[playerid]);
        PlayerTextDrawShow(playerid, g_PSZPanelText[playerid]);
        g_PSZPanelVisible[playerid] = true;
        PSZ_UpdatePanel(playerid);
    }
    return 1;
}

stock PSZ_CountMode(mode)
{
    new count = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || IsPlayerNPC(i) || !g_Logged[i]) continue;
        switch (mode)
        {
            case 0: if (g_PSZDriftMode[i]) count++;
            case 1: if (g_V4Arena[i] == V4_ARENA_DERBY) count++;
            case 2: if (g_V4RaceActive[i]) count++;
            case 3: if (g_V4Arena[i] == V4_ARENA_PAINTBALL || g_V4Arena[i] == V4_ARENA_SNIPER || g_V4Arena[i] == V4_ARENA_MINIGUN) count++;
            case 4: if (g_PSZHideJoined[i]) count++;
            case 5: if (g_InSumo[i]) count++;
            case 6: if (g_InDM[i]) count++;
            case 7: if (IsPlayerInRangeOfPoint(i, 250.0, 400.0, 2500.0, 18.0)) count++;
        }
    }
    return count;
}

stock PSZ_UpdatePanel(playerid)
{
    if (!g_PSZPanelEnabled[playerid] || !g_Logged[playerid]) return 0;
    new text[420];
    format(text, sizeof(text),
        "~y~ZABAWY ~w~[/zabawy]~n~~n~Drift: ~g~%d~w~/32~n~Derby: ~g~%d~w~/12~n~Race: ~g~%d~w~/16~n~Strzelnica: ~g~%d~w~/16~n~Chowany: ~g~%d~w~/16~n~Sumo: ~g~%d~w~/12~n~DM: ~g~%d~w~/16~n~Plac Zabaw: ~g~%d~w~/32",
        PSZ_CountMode(0), PSZ_CountMode(1), PSZ_CountMode(2), PSZ_CountMode(3),
        PSZ_CountMode(4), PSZ_CountMode(5), PSZ_CountMode(6), PSZ_CountMode(7));
    PlayerTextDrawSetString(playerid, g_PSZPanelText[playerid], text);
    return 1;
}

stock PSZ_GetRankIndex(playerid)
{
    new rank = 0;
    for (new i = 0; i < sizeof(g_PSZRankLimits); i++)
        if (g_PSZRespect[playerid] >= g_PSZRankLimits[i]) rank = i;
    return rank;
}

stock PSZ_AddRespect(playerid, amount)
{
    if (amount <= 0) return 0;
    g_PSZRespect[playerid] += amount;
    new message[96];
    format(message, sizeof(message), "+%d respektu | Razem: %d | Ranga: %s", amount,
        g_PSZRespect[playerid], g_PSZRankNames[PSZ_GetRankIndex(playerid)]);
    SendClientMessage(playerid, COLOR_YELLOW, message);
    return 1;
}

stock PSZ_DisableDrift(playerid)
{
    if (!g_PSZDriftMode[playerid]) return 0;
    PSZ_FinishDriftCombo(playerid);
    g_PSZDriftMode[playerid] = false;
    return 1;
}

stock Float:PSZ_NormalizeAngle(Float:angle)
{
    while (angle < 0.0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

stock Float:PSZ_AngleDifference(Float:a, Float:b)
{
    new Float:difference = PSZ_NormalizeAngle(a) - PSZ_NormalizeAngle(b);
    if (difference > 180.0) difference -= 360.0;
    if (difference < -180.0) difference += 360.0;
    if (difference < 0.0) difference = -difference;
    return difference;
}

stock PSZ_UpdateDrift(playerid, vehicleid, speed)
{
    if (!g_PSZDriftMode[playerid] || GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 0;
    if (!IsSafeTuningModel(GetVehicleModel(vehicleid)))
    {
        PSZ_FinishDriftCombo(playerid);
        return 0;
    }

    new Float:vx, Float:vy, Float:vz, Float:vehicleAngle;
    GetVehicleVelocity(vehicleid, vx, vy, vz);
    GetVehicleZAngle(vehicleid, vehicleAngle);
    new Float:horizontal = floatsqroot((vx * vx) + (vy * vy));
    if (horizontal < 0.08 || speed < 35 || floatabs(vz) > 0.18)
    {
        PSZ_FinishDriftCombo(playerid);
        return 0;
    }

    new Float:movementAngle = atan2(-vx, vy);
    new Float:driftAngle = PSZ_AngleDifference(vehicleAngle, movementAngle);
    if (driftAngle < 14.0 || driftAngle > 88.0)
    {
        PSZ_FinishDriftCombo(playerid);
        return 0;
    }

    g_PSZDriftTicks[playerid]++;
    new multiplier = 1 + (g_PSZDriftTicks[playerid] / 20);
    if (multiplier > 5) multiplier = 5;
    new points = floatround((float(speed) * driftAngle) / 35.0) * multiplier;

    new partner = g_PSZTandemPartner[playerid];
    if (partner != INVALID_PLAYER_ID && IsPlayerConnected(partner) && IsPlayerInAnyVehicle(partner)
        && GetPlayerVirtualWorld(partner) == GetPlayerVirtualWorld(playerid))
    {
        new Float:px, Float:py, Float:pz;
        GetPlayerPos(partner, px, py, pz);
        if (IsPlayerInRangeOfPoint(playerid, 18.0, px, py, pz)) points += points / 2;
    }

    g_PSZDriftCombo[playerid] += points;
    new text[128];
    format(text, sizeof(text), "~y~DRIFT ~w~%d pkt  ~g~x%d~n~~w~Kat: %.0f  Predkosc: %d km/h",
        g_PSZDriftCombo[playerid], multiplier, driftAngle, speed);
    PlayerTextDrawSetString(playerid, g_PSZDriftText[playerid], text);
    if (!g_PSZDriftTextVisible[playerid])
    {
        PlayerTextDrawShow(playerid, g_PSZDriftText[playerid]);
        g_PSZDriftTextVisible[playerid] = true;
    }
    return points;
}

stock PSZ_FinishDriftCombo(playerid)
{
    if (g_PSZDriftTicks[playerid] < 3)
    {
        g_PSZDriftCombo[playerid] = 0;
        g_PSZDriftTicks[playerid] = 0;
        if (g_PSZDriftTextVisible[playerid])
        {
            PlayerTextDrawHide(playerid, g_PSZDriftText[playerid]);
            g_PSZDriftTextVisible[playerid] = false;
        }
        return 0;
    }

    new score = g_PSZDriftCombo[playerid];
    g_PSZDriftTotal[playerid] += score;
    if (score > g_PSZDriftBest[playerid]) g_PSZDriftBest[playerid] = score;
    new respect = score / 1000;
    if (respect < 1 && score >= 250) respect = 1;
    if (respect > 20) respect = 20;
    if (respect > 0) PSZ_AddRespect(playerid, respect);

    new message[128];
    format(message, sizeof(message), "Drift zakonczony: %d pkt | Rekord: %d | Lacznie: %d",
        score, g_PSZDriftBest[playerid], g_PSZDriftTotal[playerid]);
    SendClientMessage(playerid, COLOR_GREEN, message);
    g_PSZDriftCombo[playerid] = 0;
    g_PSZDriftTicks[playerid] = 0;
    if (g_PSZDriftTextVisible[playerid])
    {
        PlayerTextDrawHide(playerid, g_PSZDriftText[playerid]);
        g_PSZDriftTextVisible[playerid] = false;
    }
    SaveAccount(playerid);
    return score;
}

stock PSZ_CreateRamp(playerid)
{
    if (!g_Logged[playerid] || GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
        return SendClientMessage(playerid, COLOR_YELLOW, "Rampa dziala tylko, gdy prowadzisz pojazd."), 0;

    new now = GetTickCount();
    if (now - g_PSZRampLastTick[playerid] < PSZ_RAMP_COOLDOWN_MS)
        return SendClientMessage(playerid, COLOR_GREY, "Rampa laduje sie. Odczekaj 5 sekund."), 0;

    if (g_PSZRampObject[playerid] != INVALID_OBJECT_ID)
        DestroyPlayerObject(playerid, g_PSZRampObject[playerid]);

    new vehicleid = GetPlayerVehicleID(playerid);
    new Float:x, Float:y, Float:z, Float:angle;
    GetVehiclePos(vehicleid, x, y, z);
    GetVehicleZAngle(vehicleid, angle);
    x += 8.5 * floatsin(-angle, degrees);
    y += 8.5 * floatcos(-angle, degrees);
    z -= 0.85;
    g_PSZRampObject[playerid] = CreatePlayerObject(playerid, 1632, x, y, z, 0.0, 0.0, angle);
    if (g_PSZRampObject[playerid] == INVALID_OBJECT_ID)
        return SendClientMessage(playerid, COLOR_RED, "Nie udalo sie utworzyc rampy."), 0;

    g_PSZRampLastTick[playerid] = now;
    SetTimerEx("PSZ_DestroyRamp", PSZ_RAMP_LIFETIME_MS, false, "ii", playerid, g_PSZRampObject[playerid]);
    GameTextForPlayer(playerid, "~y~RAMPA!", 1000, 3);
    return 1;
}

public PSZ_DestroyRamp(playerid, objectid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || !IsPlayerConnected(playerid)) return 1;
    if (g_PSZRampObject[playerid] == objectid)
    {
        DestroyPlayerObject(playerid, objectid);
        g_PSZRampObject[playerid] = INVALID_OBJECT_ID;
    }
    return 1;
}

stock PSZ_OnKeyStateChange(playerid, newkeys, oldkeys)
{
    if ((newkeys & KEY_CROUCH) && !(oldkeys & KEY_CROUCH) && GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
        return PSZ_CreateRamp(playerid);
    return 0;
}

stock PSZ_SaveHideSnapshot(playerid)
{
    GetPlayerPos(playerid, g_PSZHideReturnX[playerid], g_PSZHideReturnY[playerid], g_PSZHideReturnZ[playerid]);
    GetPlayerFacingAngle(playerid, g_PSZHideReturnA[playerid]);
    g_PSZHideReturnWorld[playerid] = GetPlayerVirtualWorld(playerid);
    g_PSZHideReturnInterior[playerid] = GetPlayerInterior(playerid);
    g_PSZHideReturnSkin[playerid] = GetPlayerSkin(playerid);
    g_PSZHideReturnColor[playerid] = GetPlayerColor(playerid);
    GetPlayerHealth(playerid, g_PSZHideReturnHealth[playerid]);
    GetPlayerArmour(playerid, g_PSZHideReturnArmour[playerid]);
    for (new slot = 0; slot < 13; slot++)
        GetPlayerWeaponData(playerid, slot, g_PSZHideReturnWeapon[playerid][slot], g_PSZHideReturnAmmo[playerid][slot]);
    g_PSZHideSnapshotValid[playerid] = true;
    return 1;
}

stock PSZ_RestoreHideSnapshot(playerid)
{
    if (!g_PSZHideSnapshotValid[playerid]) return 0;
    TogglePlayerControllable(playerid, true);
    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
    SetPlayerVirtualWorld(playerid, g_PSZHideReturnWorld[playerid]);
    SetPlayerInterior(playerid, g_PSZHideReturnInterior[playerid]);
    SetPlayerPos(playerid, g_PSZHideReturnX[playerid], g_PSZHideReturnY[playerid], g_PSZHideReturnZ[playerid]);
    SetPlayerFacingAngle(playerid, g_PSZHideReturnA[playerid]);
    SetPlayerSkin(playerid, g_PSZHideReturnSkin[playerid]);
    SetPlayerColor(playerid, g_PSZHideReturnColor[playerid]);
    SetPlayerHealth(playerid, g_PSZHideReturnHealth[playerid]);
    SetPlayerArmour(playerid, g_PSZHideReturnArmour[playerid]);
    ResetPlayerWeapons(playerid);
    for (new slot = 0; slot < 13; slot++)
        if (g_PSZHideReturnWeapon[playerid][slot] > 0 && g_PSZHideReturnAmmo[playerid][slot] > 0)
            GivePlayerWeapon(playerid, g_PSZHideReturnWeapon[playerid][slot], g_PSZHideReturnAmmo[playerid][slot]);
    SetCameraBehindPlayer(playerid);
    g_PSZHideSnapshotValid[playerid] = false;
    g_PSZHideOutsideTicks[playerid] = 0;
    return 1;
}

stock PSZ_StartHideSignups()
{
    if (g_PSZHidePhase != PSZ_HIDE_IDLE) return 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        g_PSZHideJoined[i] = false;
        g_PSZHideSeeker[i] = false;
        g_PSZHideFound[i] = false;
        g_PSZHideOutsideTicks[i] = 0;
        g_PSZHideSnapshotValid[i] = false;
    }
    g_PSZHidePhase = PSZ_HIDE_SIGNUP;
    g_PSZHideSeconds = 30;
    SendClientMessageToAll(COLOR_YELLOW, "[EVENT] Zapisy do Chowanego przez 30 sekund. Wpisz /chowany lub /event.");
    return 1;
}

stock PSZ_JoinHide(playerid)
{
    if (g_InDM[playerid] || g_InSumo[playerid] || V4_IsInArena(playerid) ||
        g_V4RaceActive[playerid] || g_V4WorkActive[playerid] || g_V4JailSeconds[playerid] > 0)
        return SendClientMessage(playerid, COLOR_RED, "Najpierw zakoncz aktywna arene, wyscig, prace lub wiezienie."), 0;
    PSZ_DisableDrift(playerid);
    if (g_PSZHidePhase == PSZ_HIDE_IDLE) PSZ_StartHideSignups();
    if (g_PSZHidePhase != PSZ_HIDE_SIGNUP)
        return SendClientMessage(playerid, COLOR_RED, "Runda Chowanego juz trwa. Poczekaj na kolejna."), 0;
    if (g_PSZHideJoined[playerid])
        return SendClientMessage(playerid, COLOR_GREY, "Jestes juz zapisany do Chowanego."), 0;
    g_PSZHideJoined[playerid] = true;
    g_PSZHideFound[playerid] = false;
    SendClientMessage(playerid, COLOR_GREEN, "Dolaczasz do Chowanego. Czekaj na start rundy.");
    return 1;
}

stock PSZ_CountHidePlayers(includeFound)
{
    new count = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !g_PSZHideJoined[i]) continue;
        if (!includeFound && g_PSZHideFound[i]) continue;
        count++;
    }
    return count;
}

stock PSZ_StartHideRound()
{
    new candidates[MAX_PLAYERS], count = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !g_PSZHideJoined[i] || IsPlayerNPC(i)) continue;
        if (g_InDM[i] || g_InSumo[i] || V4_IsInArena(i) || g_V4RaceActive[i] ||
            g_V4WorkActive[i] || g_V4JailSeconds[i] > 0)
        {
            g_PSZHideJoined[i] = false;
            SendClientMessage(i, COLOR_RED, "Usunieto z Chowanego: rozpoczeto inna aktywnosc podczas zapisow.");
            continue;
        }
        candidates[count++] = i;
    }

    if (count < 2)
    {
        SendClientMessageToAll(COLOR_RED, "[CHOWANY] Za malo graczy. Event anulowany.");
        g_PSZHidePhase = PSZ_HIDE_IDLE;
        if (g_PSZCurrentEvent == PSZ_EVENT_HIDE) g_PSZCurrentEvent = PSZ_EVENT_NONE;
        for (new i = 0; i < MAX_PLAYERS; i++) g_PSZHideJoined[i] = false;
        return 0;
    }

    new seeker = candidates[random(count)];
    g_PSZHideMap = random(PSZ_HIDE_MAPS);
    g_PSZHidePhase = PSZ_HIDE_HIDING;
    g_PSZHideSeconds = 20;

    for (new n = 0; n < count; n++)
    {
        new playerid = candidates[n];
        PSZ_SaveHideSnapshot(playerid);
        g_PSZHideOutsideTicks[playerid] = 0;
        if (V4_IsInArena(playerid)) V4_LeaveArena(playerid);
        g_InDM[playerid] = false;
        g_InSumo[playerid] = false;
        if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
        SetPlayerVirtualWorld(playerid, PSZ_WORLD_HIDE_BASE + g_PSZHideMap);
        SetPlayerInterior(playerid, 0);
        ResetPlayerWeapons(playerid);
        SetPlayerHealth(playerid, 100.0);
        g_PSZHideSeeker[playerid] = playerid == seeker;
        g_PSZHideFound[playerid] = false;
        if (playerid == seeker)
        {
            SetPlayerPos(playerid, g_PSZHideSeekerSpawn[g_PSZHideMap][0], g_PSZHideSeekerSpawn[g_PSZHideMap][1], g_PSZHideSeekerSpawn[g_PSZHideMap][2]);
            SetPlayerFacingAngle(playerid, g_PSZHideSeekerSpawn[g_PSZHideMap][3]);
            SetPlayerColor(playerid, COLOR_RED);
            TogglePlayerControllable(playerid, false);
            GameTextForPlayer(playerid, "~r~SZUKAJACY~n~~w~CZEKAJ 20 SEKUND", 5000, 3);
        }
        else
        {
            new spawn = random(PSZ_HIDE_POINTS);
            SetPlayerPos(playerid, g_PSZHideSpawns[g_PSZHideMap][spawn][0], g_PSZHideSpawns[g_PSZHideMap][spawn][1], g_PSZHideSpawns[g_PSZHideMap][spawn][2]);
            SetPlayerFacingAngle(playerid, g_PSZHideSpawns[g_PSZHideMap][spawn][3]);
            SetPlayerColor(playerid, COLOR_GREEN);
            TogglePlayerControllable(playerid, true);
            GameTextForPlayer(playerid, "~g~SCHOWAJ SIE!~n~~w~20 SEKUND", 5000, 3);
        }
    }
    SendClientMessageToAll(COLOR_YELLOW, "[CHOWANY] Runda rozpoczeta. Chowajacy maja 20 sekund.");
    return 1;
}

stock PSZ_FindHider(playerid)
{
    if (!g_PSZHideJoined[playerid] || g_PSZHideSeeker[playerid] || g_PSZHideFound[playerid]) return 0;
    g_PSZHideFound[playerid] = true;
    SetPlayerColor(playerid, COLOR_GREY);
    TogglePlayerControllable(playerid, false);
    SetPlayerPos(playerid, g_PSZHideSeekerSpawn[g_PSZHideMap][0], g_PSZHideSeekerSpawn[g_PSZHideMap][1], g_PSZHideSeekerSpawn[g_PSZHideMap][2] + 3.0);
    SendClientMessage(playerid, COLOR_RED, "Zostales znaleziony. Czekasz do konca rundy.");
    if (PSZ_CountHidePlayers(false) <= 1) PSZ_FinishHideRound(true);
    return 1;
}

stock PSZ_FinishHideRound(seekerWon)
{
    if (g_PSZHidePhase == PSZ_HIDE_IDLE) return 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !g_PSZHideJoined[i]) continue;
        if (seekerWon && g_PSZHideSeeker[i])
        {
            PSZ_AddRespect(i, 150);
            g_PSZEventWins[i]++;
        }
        else if (!seekerWon && !g_PSZHideSeeker[i] && !g_PSZHideFound[i])
        {
            PSZ_AddRespect(i, 100);
            g_PSZHideWins[i]++;
            g_PSZEventWins[i]++;
        }
        if (!PSZ_RestoreHideSnapshot(i))
        {
            TogglePlayerControllable(i, true);
            SetPlayerColor(i, COLOR_WHITE);
            SetPlayerVirtualWorld(i, 0);
            SetPlayerInterior(i, 0);
            SetPlayerPos(i, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z);
            SetPlayerFacingAngle(i, NEMQO_START_A);
            SetCameraBehindPlayer(i);
        }
        g_PSZHideJoined[i] = false;
        g_PSZHideSeeker[i] = false;
        g_PSZHideFound[i] = false;
        SaveAccount(i);
    }
    SendClientMessageToAll(seekerWon ? COLOR_RED : COLOR_GREEN,
        seekerWon ? "[CHOWANY] Szukajacy znalazl wszystkich i wygrywa!" : "[CHOWANY] Czas minal. Nieznalezieni chowajacy wygrywaja!");
    g_PSZHidePhase = PSZ_HIDE_IDLE;
    g_PSZHideSeconds = 0;
    if (g_PSZCurrentEvent == PSZ_EVENT_HIDE) g_PSZCurrentEvent = PSZ_EVENT_NONE;
    return 1;
}

stock PSZ_LeaveHide(playerid, disconnecting)
{
    if (!g_PSZHideJoined[playerid]) return 0;
    new wasSeeker = g_PSZHideSeeker[playerid];
    new wasActive = g_PSZHidePhase >= PSZ_HIDE_HIDING;
    g_PSZHideJoined[playerid] = false;
    g_PSZHideSeeker[playerid] = false;
    g_PSZHideFound[playerid] = false;
    g_PSZHideOutsideTicks[playerid] = 0;
    if (!disconnecting && wasActive) PSZ_RestoreHideSnapshot(playerid);
    else if (disconnecting) g_PSZHideSnapshotValid[playerid] = false;
    else SendClientMessage(playerid, COLOR_GREY, "Opuszczasz zapisy do Chowanego.");
    if (wasSeeker && g_PSZHidePhase >= PSZ_HIDE_HIDING) PSZ_FinishHideRound(false);
    else if (g_PSZHidePhase >= PSZ_HIDE_HIDING && PSZ_CountHidePlayers(true) < 2) PSZ_FinishHideRound(true);
    return 1;
}

stock PSZ_GetEventName(eventid, output[], size)
{
    switch (eventid)
    {
        case PSZ_EVENT_HIDE: format(output, size, "Chowany");
        case PSZ_EVENT_DRIFT: format(output, size, "Drift Challenge");
        case PSZ_EVENT_DERBY: format(output, size, "Derby");
        case PSZ_EVENT_RACE: format(output, size, "Race");
        case PSZ_EVENT_SUMO: format(output, size, "Monster Sumo");
        default: format(output, size, "Brak");
    }
    return 1;
}

stock PSZ_StartEvent(eventid)
{
    g_PSZCurrentEvent = eventid;
    g_PSZEventSeconds = 120;
    new name[32], message[128];
    PSZ_GetEventName(eventid, name, sizeof(name));
    format(message, sizeof(message), "[AUTO EVENT] %s! Masz 120 sekund na dolaczenie: /event", name);
    SendClientMessageToAll(COLOR_YELLOW, message);
    if (eventid == PSZ_EVENT_HIDE && g_PSZHidePhase == PSZ_HIDE_IDLE) PSZ_StartHideSignups();
    return 1;
}

public PSZ_AutoEvent()
{
    if (g_PSZCurrentEvent != PSZ_EVENT_NONE || g_PSZHidePhase != PSZ_HIDE_IDLE) return 1;
    PSZ_StartEvent(1 + random(5));
    return 1;
}

stock PSZ_JoinCurrentEvent(playerid)
{
    if (g_PSZCurrentEvent != PSZ_EVENT_DRIFT) PSZ_DisableDrift(playerid);
    switch (g_PSZCurrentEvent)
    {
        case PSZ_EVENT_HIDE: return PSZ_JoinHide(playerid);
        case PSZ_EVENT_DRIFT: return ShowPlayerDialog(playerid, PSZ_DIALOG_DRIFT_ZONES, DIALOG_STYLE_LIST, "Drift Challenge", "Doki Los Santos\nLotnisko San Fierro\nParking Las Venturas\nMount Chilliad\nPlac Zabaw", "Jedz", "Anuluj"), 1;
        case PSZ_EVENT_DERBY: return V4_EnterArena(playerid, V4_ARENA_DERBY);
        case PSZ_EVENT_RACE: return V4_StartRace(playerid);
        case PSZ_EVENT_SUMO: return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sumo"), 1;
    }
    SendClientMessage(playerid, COLOR_GREY, "Aktualnie nie trwa automatyczny event.");
    return 0;
}

public PSZ_SecondTick()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (IsPlayerConnected(playerid) && !IsPlayerNPC(playerid) && g_Logged[playerid]) PSZ_UpdatePanel(playerid);

    if (g_PSZCurrentEvent != PSZ_EVENT_NONE && g_PSZEventSeconds > 0)
    {
        g_PSZEventSeconds--;
        if (g_PSZEventSeconds == 0) g_PSZCurrentEvent = PSZ_EVENT_NONE;
    }

    if (g_PSZHidePhase == PSZ_HIDE_IDLE) return 1;
    if (g_PSZHideSeconds > 0) g_PSZHideSeconds--;

    if (g_PSZHidePhase == PSZ_HIDE_SIGNUP)
    {
        if (g_PSZHideSeconds == 10 || g_PSZHideSeconds == 5)
        {
            new message[96];
            format(message, sizeof(message), "[CHOWANY] Start za %d sekund. Zapis: /chowany", g_PSZHideSeconds);
            SendClientMessageToAll(COLOR_YELLOW, message);
        }
        if (g_PSZHideSeconds <= 0) PSZ_StartHideRound();
        return 1;
    }

    if (g_PSZHidePhase == PSZ_HIDE_HIDING && g_PSZHideSeconds <= 0)
    {
        g_PSZHidePhase = PSZ_HIDE_SEEKING;
        g_PSZHideSeconds = 180;
        for (new i = 0; i < MAX_PLAYERS; i++)
        {
            if (IsPlayerConnected(i) && g_PSZHideJoined[i] && g_PSZHideSeeker[i])
            {
                TogglePlayerControllable(i, true);
                GivePlayerWeapon(i, 3, 1);
                GameTextForPlayer(i, "~r~SZUKAJ!~n~~w~MASZ 180 SEKUND", 4000, 3);
            }
        }
        SendClientMessageToAll(COLOR_RED, "[CHOWANY] Szukajacy ruszyl. Dotkniecie gracza oznacza znalezienie.");
    }

    if (g_PSZHidePhase >= PSZ_HIDE_HIDING)
    {
        for (new i = 0; i < MAX_PLAYERS; i++)
        {
            if (!IsPlayerConnected(i) || !g_PSZHideJoined[i] || g_PSZHideFound[i]) continue;
            new outside = GetPlayerVirtualWorld(i) != PSZ_WORLD_HIDE_BASE + g_PSZHideMap ||
                GetPlayerInterior(i) != 0 ||
                !IsPlayerInRangeOfPoint(i, 160.0, g_PSZHideSeekerSpawn[g_PSZHideMap][0],
                    g_PSZHideSeekerSpawn[g_PSZHideMap][1], g_PSZHideSeekerSpawn[g_PSZHideMap][2]);
            if (outside)
            {
                g_PSZHideOutsideTicks[i]++;
                if (g_PSZHideOutsideTicks[i] == 1)
                    SendClientMessage(i, COLOR_YELLOW, "Wroc na obszar Chowanego. Eliminacja za 2 sekundy.");
                if (g_PSZHideOutsideTicks[i] >= 3)
                {
                    if (g_PSZHideSeeker[i])
                    {
                        SendClientMessageToAll(COLOR_RED, "[CHOWANY] Szukajacy opuscil mape. Chowajacy wygrywaja walkowerem.");
                        PSZ_FinishHideRound(false);
                        return 1;
                    }
                    PSZ_FindHider(i);
                    if (g_PSZHidePhase == PSZ_HIDE_IDLE) return 1;
                }
            }
            else g_PSZHideOutsideTicks[i] = 0;
        }
    }

    if (g_PSZHidePhase == PSZ_HIDE_SEEKING)
    {
        for (new seeker = 0; seeker < MAX_PLAYERS; seeker++)
        {
            if (!IsPlayerConnected(seeker) || !g_PSZHideJoined[seeker] || !g_PSZHideSeeker[seeker]) continue;
            new Float:x, Float:y, Float:z;
            GetPlayerPos(seeker, x, y, z);
            for (new hider = 0; hider < MAX_PLAYERS; hider++)
            {
                if (!IsPlayerConnected(hider) || !g_PSZHideJoined[hider] || g_PSZHideSeeker[hider] || g_PSZHideFound[hider]) continue;
                if (IsPlayerInRangeOfPoint(hider, 2.5, x, y, z)) PSZ_FindHider(hider);
            }
        }
        if (g_PSZHideSeconds <= 0) PSZ_FinishHideRound(false);
    }
    return 1;
}

stock PSZ_PlayAnimation(playerid, animation)
{
    switch (animation)
    {
        case 0: ApplyAnimation(playerid, "DANCING", "DAN_Down_A", 4.1, 1, 0, 0, 0, 0, 1);
        case 1: ApplyAnimation(playerid, "DANCING", "DAN_Left_A", 4.1, 1, 0, 0, 0, 0, 1);
        case 2: ApplyAnimation(playerid, "DANCING", "dnce_M_a", 4.1, 1, 0, 0, 0, 0, 1);
        case 3: ApplyAnimation(playerid, "DANCING", "dnce_M_b", 4.1, 1, 0, 0, 0, 0, 1);
        case 4: ApplyAnimation(playerid, "BEACH", "ParkSit_M_loop", 4.1, 1, 0, 0, 0, 0, 1);
        case 5: ApplyAnimation(playerid, "SMOKING", "M_smklean_loop", 4.1, 1, 0, 0, 0, 0, 1);
        case 6: ApplyAnimation(playerid, "GANGS", "leanIDLE", 4.1, 1, 0, 0, 0, 0, 1);
        case 7: SetPlayerSpecialAction(playerid, SPECIAL_ACTION_HANDSUP);
    }
    return 1;
}

stock PSZ_CreatePlayground()
{
    for (new i = 0; i < PSZ_PLAYGROUND_OBJECTS; i++) g_PSZPlaygroundObjects[i] = INVALID_OBJECT_ID;
    g_PSZPlaygroundObjects[0] = CreateObject(18753, 400.0, 2500.0, 15.0, 0.0, 0.0, 0.0, 500.0);
    g_PSZPlaygroundObjects[1] = CreateObject(18753, 510.0, 2500.0, 15.0, 0.0, 0.0, 0.0, 500.0);
    g_PSZPlaygroundObjects[2] = CreateObject(1632, 445.0, 2470.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[3] = CreateObject(1632, 445.0, 2485.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[4] = CreateObject(1632, 445.0, 2500.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[5] = CreateObject(1632, 445.0, 2515.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[6] = CreateObject(1632, 445.0, 2530.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[7] = CreateObject(1634, 535.0, 2470.0, 18.0, 0.0, 0.0, 270.0, 300.0);
    g_PSZPlaygroundObjects[8] = CreateObject(1634, 535.0, 2500.0, 18.0, 0.0, 0.0, 270.0, 300.0);
    g_PSZPlaygroundObjects[9] = CreateObject(1634, 535.0, 2530.0, 18.0, 0.0, 0.0, 270.0, 300.0);
    g_PSZPlaygroundObjects[10] = CreateObject(1655, 470.0, 2445.0, 18.0, 0.0, 0.0, 0.0, 300.0);
    g_PSZPlaygroundObjects[11] = CreateObject(1655, 490.0, 2445.0, 18.0, 0.0, 0.0, 0.0, 300.0);
    g_PSZPlaygroundObjects[12] = CreateObject(1655, 470.0, 2555.0, 18.0, 0.0, 0.0, 180.0, 300.0);
    g_PSZPlaygroundObjects[13] = CreateObject(1655, 490.0, 2555.0, 18.0, 0.0, 0.0, 180.0, 300.0);
    g_PSZPlaygroundObjects[14] = CreateObject(18800, 580.0, 2500.0, 35.0, 0.0, 90.0, 90.0, 500.0);
    g_PSZPlaygroundObjects[15] = CreateObject(18801, 620.0, 2500.0, 35.0, 0.0, 90.0, 90.0, 500.0);
    g_PSZPlaygroundObjects[16] = CreateObject(18772, 400.0, 2580.0, 33.0, 0.0, 65.0, 0.0, 500.0);
    g_PSZPlaygroundObjects[17] = CreateObject(18772, 510.0, 2580.0, 33.0, 0.0, 65.0, 180.0, 500.0);
    g_PSZPlaygroundObjects[18] = CreateObject(978, 360.0, 2450.0, 18.2, 0.0, 0.0, 0.0, 200.0);
    g_PSZPlaygroundObjects[19] = CreateObject(978, 370.0, 2450.0, 18.2, 0.0, 0.0, 0.0, 200.0);
    g_PSZPlaygroundObjects[20] = CreateObject(978, 380.0, 2450.0, 18.2, 0.0, 0.0, 0.0, 200.0);
    g_PSZPlaygroundObjects[21] = CreateObject(979, 360.0, 2550.0, 18.2, 0.0, 0.0, 180.0, 200.0);
    g_PSZPlaygroundObjects[22] = CreateObject(979, 370.0, 2550.0, 18.2, 0.0, 0.0, 180.0, 200.0);
    g_PSZPlaygroundObjects[23] = CreateObject(979, 380.0, 2550.0, 18.2, 0.0, 0.0, 180.0, 200.0);
    g_PSZPlaygroundObjects[24] = CreateObject(1633, 565.0, 2460.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[25] = CreateObject(1633, 565.0, 2540.0, 18.0, 0.0, 0.0, 90.0, 300.0);
    g_PSZPlaygroundObjects[26] = CreateObject(1632, 340.0, 2480.0, 18.0, 0.0, 0.0, 270.0, 300.0);
    g_PSZPlaygroundObjects[27] = CreateObject(1632, 340.0, 2520.0, 18.0, 0.0, 0.0, 270.0, 300.0);
    g_PSZPlaygroundObjects[28] = CreateObject(18753, 400.0, 2610.0, 15.0, 0.0, 0.0, 0.0, 500.0);
    g_PSZPlaygroundObjects[29] = CreateObject(18753, 510.0, 2610.0, 15.0, 0.0, 0.0, 0.0, 500.0);
    g_PSZPlaygroundObjects[30] = CreateObject(1634, 450.0, 2600.0, 18.0, 0.0, 0.0, 0.0, 300.0);
    g_PSZPlaygroundObjects[31] = CreateObject(1634, 490.0, 2600.0, 18.0, 0.0, 0.0, 0.0, 300.0);

    new const models[PSZ_PLAYGROUND_VEHICLES] = {562, 560, 559, 558, 565, 561, 562, 560, 559, 558, 565, 561, 562, 560, 559, 558, 565, 561, 562, 560};
    for (new i = 0; i < PSZ_PLAYGROUND_VEHICLES; i++)
    {
        new row = i / 10, column = i % 10;
        g_PSZPlaygroundVehicles[i] = AddStaticVehicleEx(models[i], 350.0 + (float(column) * 5.5),
            2470.0 + (float(row) * 60.0), 18.1, row ? 180.0 : 0.0,
            (i * 13) % 126, (i * 31) % 126, 300);
    }
    printf("NEMQO PSZ playground: 32 objects and 20 drift vehicles configured");
    return 1;
}

stock PSZ_TeleportPlayground(playerid)
{
    if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, false);
    if (V4_IsInArena(playerid)) V4_LeaveArena(playerid);
    if (g_InSumo[playerid]) ExitSumo(playerid, false);
    g_InDM[playerid] = false;
    if (IsPlayerInAnyVehicle(playerid))
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehicleid, 385.0, 2500.0, 19.0);
        SetVehicleZAngle(vehicleid, 90.0);
        LinkVehicleToInterior(vehicleid, 0);
        SetVehicleVirtualWorld(vehicleid, 0);
    }
    else SetPlayerPos(playerid, 385.0, 2500.0, 19.0);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerInterior(playerid, 0);
    SetCameraBehindPlayer(playerid);
    SendClientMessage(playerid, COLOR_YELLOW, "Plac Zabaw: rampy, loop, wallride, parking driftowozow. Klakson tworzy przenosna rampe.");
    return 1;
}

stock PSZ_ShowTopDrift(playerid)
{
    new DBResult:result = db_query(g_DB, "SELECT name,drift_best,respect FROM accounts ORDER BY drift_best DESC LIMIT 10");
    new text[700] = "Miejsce\tGracz\tRekord\tRespekt\n", name[MAX_PLAYER_NAME + 1], line[96], place = 1;
    if (db_num_rows(result) > 0)
    {
        do
        {
            db_get_field_assoc(result, "name", name, sizeof(name));
            format(line, sizeof(line), "%d\t%s\t%d\t%d\n", place++, name,
                db_get_field_assoc_int(result, "drift_best"), db_get_field_assoc_int(result, "respect"));
            strcat(text, line);
        }
        while (db_next_row(result));
    }
    db_free_result(result);
    ShowPlayerDialog(playerid, PSZ_DIALOG_EVENT, DIALOG_STYLE_TABLIST_HEADERS, "TOP 10 Drift", text, "OK", "");
    return 1;
}

stock PSZ_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused inputtext
    if (dialogid == PSZ_DIALOG_PLAY)
    {
        if (!response) return 1;
        if (listitem >= 1 && listitem <= 6) PSZ_DisableDrift(playerid);
        switch (listitem)
        {
            case 0: ShowPlayerDialog(playerid, PSZ_DIALOG_DRIFT_ZONES, DIALOG_STYLE_LIST, "Strefy driftu", "Doki Los Santos\nLotnisko San Fierro\nParking Las Venturas\nMount Chilliad\nPlac Zabaw", "Jedz", "Wroc");
            case 1: V4_EnterArena(playerid, V4_ARENA_DERBY);
            case 2: V4_StartRace(playerid);
            case 3: V4_EnterArena(playerid, V4_ARENA_PAINTBALL);
            case 4: PSZ_JoinHide(playerid);
            case 5: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sumo");
            case 6: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/dm");
            case 7: PSZ_TeleportPlayground(playerid);
        }
        return 1;
    }
    if (dialogid == PSZ_DIALOG_DRIFT_ZONES)
    {
        if (!response) return 1;
        if (listitem < 0 || listitem >= sizeof(g_PSZDriftZones)) return 1;
        if (!TeleportPlayer(playerid, g_PSZDriftZones[listitem][0], g_PSZDriftZones[listitem][1], g_PSZDriftZones[listitem][2], g_PSZDriftZones[listitem][3])) return 1;
        g_PSZDriftMode[playerid] = true;
        new message[96];
        format(message, sizeof(message), "Drift aktywny: %s. Zakonczenie: /drift off", g_PSZDriftZoneNames[listitem]);
        SendClientMessage(playerid, COLOR_GREEN, message);
        return 1;
    }
    if (dialogid == PSZ_DIALOG_ANIMS)
    {
        if (response && listitem >= 0 && listitem <= 7) PSZ_PlayAnimation(playerid, listitem);
        return 1;
    }
    if (dialogid == PSZ_DIALOG_EVENT) return 1;
    return 0;
}

stock PSZ_OnCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/zabawy", true))
    {
        ShowPlayerDialog(playerid, PSZ_DIALOG_PLAY, DIALOG_STYLE_LIST, "NEMQO - Zabawy",
            "Drift\nDerby\nRace\nStrzelnica\nChowany\nSumo\nDeathmatch\nPlac Zabaw", "Dolacz", "Zamknij");
        return 1;
    }
    if (!strcmp(cmdtext, "/panel", true))
    {
        g_PSZPanelEnabled[playerid] = !g_PSZPanelEnabled[playerid];
        if (g_PSZPanelEnabled[playerid])
        {
            PlayerTextDrawShow(playerid, g_PSZPanelBackground[playerid]);
            PlayerTextDrawShow(playerid, g_PSZPanelText[playerid]);
            g_PSZPanelVisible[playerid] = true;
            PSZ_UpdatePanel(playerid);
            SendClientMessage(playerid, COLOR_GREEN, "Panel Zabawy wlaczony.");
        }
        else
        {
            PlayerTextDrawHide(playerid, g_PSZPanelBackground[playerid]);
            PlayerTextDrawHide(playerid, g_PSZPanelText[playerid]);
            g_PSZPanelVisible[playerid] = false;
            SendClientMessage(playerid, COLOR_GREY, "Panel Zabawy wylaczony.");
        }
        return 1;
    }
    if (!strcmp(cmdtext, "/drift", true))
    {
        ShowPlayerDialog(playerid, PSZ_DIALOG_DRIFT_ZONES, DIALOG_STYLE_LIST, "Strefy driftu", "Doki Los Santos\nLotnisko San Fierro\nParking Las Venturas\nMount Chilliad\nPlac Zabaw", "Jedz", "Anuluj");
        return 1;
    }
    if (!strcmp(cmdtext, "/drift off", true))
    {
        PSZ_DisableDrift(playerid);
        SendClientMessage(playerid, COLOR_GREY, "Punktacja driftu wylaczona.");
        return 1;
    }
    if (!strcmp(cmdtext, "/driftstats", true))
    {
        new message[160];
        format(message, sizeof(message), "Drift: rekord %d | Lacznie %d | Respekt %d | Ranga %s",
            g_PSZDriftBest[playerid], g_PSZDriftTotal[playerid], g_PSZRespect[playerid], g_PSZRankNames[PSZ_GetRankIndex(playerid)]);
        SendClientMessage(playerid, COLOR_YELLOW, message);
        return 1;
    }
    if (!strcmp(cmdtext, "/topdrift", true)) return PSZ_ShowTopDrift(playerid);
    if (!strfind(cmdtext, "/tandem ", true))
    {
        new target = strval(cmdtext[8]);
        if (target < 0 || target >= MAX_PLAYERS || !IsPlayerConnected(target) || target == playerid)
            return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /tandem [ID gracza]"), 1;
        g_PSZTandemPartner[playerid] = target;
        g_PSZTandemPartner[target] = playerid;
        SendClientMessage(playerid, COLOR_GREEN, "Tandem polaczony. Drift blisko partnera daje +50% punktow.");
        SendClientMessage(target, COLOR_GREEN, "Gracz polaczyl z Toba Tandem Drift. Bliska jazda daje +50% punktow.");
        return 1;
    }
    if (!strcmp(cmdtext, "/tandemoff", true))
    {
        new partner = g_PSZTandemPartner[playerid];
        g_PSZTandemPartner[playerid] = INVALID_PLAYER_ID;
        if (partner != INVALID_PLAYER_ID && IsPlayerConnected(partner)) g_PSZTandemPartner[partner] = INVALID_PLAYER_ID;
        SendClientMessage(playerid, COLOR_GREY, "Tandem wylaczony.");
        return 1;
    }
    if (!strcmp(cmdtext, "/chowany", true)) return PSZ_JoinHide(playerid), 1;
    if (!strcmp(cmdtext, "/leavehide", true)) return PSZ_LeaveHide(playerid, false), 1;
    if (!strcmp(cmdtext, "/plac", true) || !strcmp(cmdtext, "/playground", true)) return PSZ_TeleportPlayground(playerid), 1;
    if (!strcmp(cmdtext, "/rampa", true)) return PSZ_CreateRamp(playerid), 1;
    if (!strcmp(cmdtext, "/anim", true))
    {
        ShowPlayerDialog(playerid, PSZ_DIALOG_ANIMS, DIALOG_STYLE_LIST, "Animacje", "Dance 1\nDance 2\nDance 3\nDance 4\nUsiadz\nPalenie\nOprzyj sie\nRece do gory", "Wybierz", "Anuluj");
        return 1;
    }
    if (!strcmp(cmdtext, "/dance", true)) return PSZ_PlayAnimation(playerid, random(4)), 1;
    if (!strcmp(cmdtext, "/stopanim", true)) return ClearAnimations(playerid, 1), 1;
    if (!strcmp(cmdtext, "/event", true)) return PSZ_JoinCurrentEvent(playerid), 1;
    if (!strcmp(cmdtext, "/respekt", true))
    {
        new message[96];
        format(message, sizeof(message), "Respekt: %d | Ranga: %s", g_PSZRespect[playerid], g_PSZRankNames[PSZ_GetRankIndex(playerid)]);
        SendClientMessage(playerid, COLOR_YELLOW, message);
        return 1;
    }
    if (!strcmp(cmdtext, "/rememberskin", true))
    {
        g_RememberSkin[playerid] = !g_RememberSkin[playerid];
        SaveAccount(playerid);
        SendClientMessage(playerid, g_RememberSkin[playerid] ? COLOR_GREEN : COLOR_GREY,
            g_RememberSkin[playerid] ? "Ostatni skin bedzie uzywany automatycznie po zalogowaniu." : "Po logowaniu ponownie zobaczysz wybor skinow.");
        return 1;
    }
    if (!strfind(cmdtext, "/startevent ", true))
    {
        if (g_V4Admin[playerid] < 1) return SendClientMessage(playerid, COLOR_RED, "Ta komenda wymaga administratora."), 1;
        if (!strcmp(cmdtext[12], "chowany", true)) PSZ_StartEvent(PSZ_EVENT_HIDE);
        else if (!strcmp(cmdtext[12], "drift", true)) PSZ_StartEvent(PSZ_EVENT_DRIFT);
        else if (!strcmp(cmdtext[12], "derby", true)) PSZ_StartEvent(PSZ_EVENT_DERBY);
        else if (!strcmp(cmdtext[12], "race", true)) PSZ_StartEvent(PSZ_EVENT_RACE);
        else if (!strcmp(cmdtext[12], "sumo", true)) PSZ_StartEvent(PSZ_EVENT_SUMO);
        else SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /startevent [chowany/drift/derby/race/sumo]");
        return 1;
    }
    return 0;
}

stock PSZ_OnDisconnect(playerid)
{
    PSZ_FinishDriftCombo(playerid);
    if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, true);
    if (g_PSZRampObject[playerid] != INVALID_OBJECT_ID) DestroyPlayerObject(playerid, g_PSZRampObject[playerid]);
    new partner = g_PSZTandemPartner[playerid];
    if (partner != INVALID_PLAYER_ID && IsPlayerConnected(partner)) g_PSZTandemPartner[partner] = INVALID_PLAYER_ID;
    PSZ_DestroyPlayerUI(playerid);
    return 1;
}
