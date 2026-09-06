// NEMQO v4.1 - natywna populacja miasta bez pluginow.
#define V41_AMBIENT_NPCS 6
#define V41_AMBIENT_ACTORS 24

forward V41_VerifyNPCMovement();

new g_V41Actors[V41_AMBIENT_ACTORS];

new const g_V41NPCNames[V41_AMBIENT_NPCS][16] =
{
    "Ped_LS_1", "Ped_LS_2", "Ped_SF_1", "Ped_SF_2", "Ped_LV_1", "Ped_LV_2"
};
new const g_V41NPCSkins[V41_AMBIENT_NPCS] = {7, 19, 21, 46, 67, 118};
new const Float:g_V41NPCSpawn[V41_AMBIENT_NPCS][4] =
{
    {2490.0, -1670.0, 13.34, 270.0},
    {1480.0, -1750.0, 13.20,  90.0},
    {-1985.0,  140.0, 27.69,  90.0},
    {-1700.0, 1300.0,  7.00,  90.0},
    {2030.0, 1340.0, 10.82,  90.0},
    {1680.0, 1760.0, 10.55,  90.0}
};

new const g_V41ActorSkins[V41_AMBIENT_ACTORS] =
{
    12, 14, 18, 23, 28, 30, 41, 44,
    56, 57, 60, 69, 76, 88, 98, 101,
    120, 138, 147, 170, 185, 188, 211, 223
};
new const Float:g_V41ActorPos[V41_AMBIENT_ACTORS][4] =
{
    {2493.0, -1688.0, 13.52, 180.0}, {1481.0, -1770.0, 18.80,   0.0},
    {1175.0, -1328.0, 14.00,  90.0}, {1833.0, -1843.0, 13.58, 180.0},
    {1688.0, -2334.0, 13.55,   0.0}, {2700.0, -1695.0,  9.20,  90.0},
    {2100.0, -1808.0, 13.55, 270.0}, {1545.0, -1672.0, 13.56, 180.0},

    {-1985.0,  137.0, 27.69, 270.0}, {-1590.0,  652.0,  7.19, 180.0},
    {-2655.0,  638.0, 14.45, 180.0}, {-1700.0, 1310.0,  7.19,  90.0},
    {-1912.0,  830.0, 35.17,   0.0}, {-1220.0,  -40.0, 14.15, 315.0},
    {-2025.0,   82.0, 28.00, 270.0}, {-1958.0,  260.0, 35.20,  90.0},

    {2027.0, 1008.0, 10.82,  90.0}, {2195.0, 1677.0, 12.37, 180.0},
    {2278.0, 2475.0, 10.82, 180.0}, {1600.0, 1825.0, 10.82,   0.0},
    {1685.0, 1750.0, 10.82, 270.0}, {2040.0, 1320.0, 10.82, 180.0},
    {1380.0, 1160.0, 10.82,   0.0}, {2577.0, 1039.0, 10.82, 180.0}
};

stock V41_CreateAmbientActors()
{
    new created = 0;
    for (new i = 0; i < V41_AMBIENT_ACTORS; i++)
    {
        g_V41Actors[i] = CreateActor(g_V41ActorSkins[i], g_V41ActorPos[i][0],
            g_V41ActorPos[i][1], g_V41ActorPos[i][2], g_V41ActorPos[i][3]);
        if (g_V41Actors[i] == INVALID_ACTOR_ID) continue;
        SetActorInvulnerable(g_V41Actors[i], true);
        switch (i % 4)
        {
            case 0: ApplyActorAnimation(g_V41Actors[i], "PED", "IDLE_CHAT", 4.1, 1, 0, 0, 0, 0);
            case 1: ApplyActorAnimation(g_V41Actors[i], "DEALER", "DEALER_IDLE", 4.1, 1, 0, 0, 0, 0);
            case 2: ApplyActorAnimation(g_V41Actors[i], "SMOKING", "M_smklean_loop", 4.1, 1, 0, 0, 0, 0);
            case 3: ApplyActorAnimation(g_V41Actors[i], "PED", "SEAT_idle", 4.1, 1, 0, 0, 0, 0);
        }
        created++;
    }
    printf("NEMQO v4.1 ambient actors: %d/%d created", created, V41_AMBIENT_ACTORS);
    return created;
}

stock V41_RefreshActorAnimations()
{
    for (new i = 0; i < V41_AMBIENT_ACTORS; i++)
    {
        if (g_V41Actors[i] == INVALID_ACTOR_ID) continue;
        switch (i % 4)
        {
            case 0: ApplyActorAnimation(g_V41Actors[i], "PED", "IDLE_CHAT", 4.1, 1, 0, 0, 0, 0);
            case 1: ApplyActorAnimation(g_V41Actors[i], "DEALER", "DEALER_IDLE", 4.1, 1, 0, 0, 0, 0);
            case 2: ApplyActorAnimation(g_V41Actors[i], "SMOKING", "M_smklean_loop", 4.1, 1, 0, 0, 0, 0);
            case 3: ApplyActorAnimation(g_V41Actors[i], "PED", "SEAT_idle", 4.1, 1, 0, 0, 0, 0);
        }
    }
    return 1;
}

stock V41_PreloadAnimations(playerid)
{
    ApplyAnimation(playerid, "PED", "IDLE_CHAT", 4.1, 0, 0, 0, 0, 1, 1);
    ApplyAnimation(playerid, "DEALER", "DEALER_IDLE", 4.1, 0, 0, 0, 0, 1, 1);
    ApplyAnimation(playerid, "SMOKING", "M_smklean_loop", 4.1, 0, 0, 0, 0, 1, 1);
    ApplyAnimation(playerid, "DANCING", "DAN_Down_A", 4.1, 0, 0, 0, 0, 1, 1);
    ClearAnimations(playerid, 1);
    V41_RefreshActorAnimations();
    return 1;
}

stock V41_StartAmbientNPCs()
{
    for (new i = 0; i < V41_AMBIENT_NPCS; i++) ConnectNPC(g_V41NPCNames[i], "ambient_walk");
    SetTimer("V41_VerifyNPCMovement", 6000, false);
    printf("NEMQO v4.1 ambient NPCs: requested %d walkers", V41_AMBIENT_NPCS);
    return 1;
}

public V41_VerifyNPCMovement()
{
    new connected = 0, moved = 0;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (!IsPlayerConnected(playerid) || !IsPlayerNPC(playerid)) continue;
        new index = V41_GetNPCIndex(playerid);
        if (index < 0) continue;
        connected++;
        if (!IsPlayerInRangeOfPoint(playerid, 0.50, g_V41NPCSpawn[index][0],
            g_V41NPCSpawn[index][1], g_V41NPCSpawn[index][2])) moved++;
    }
    if (connected == V41_AMBIENT_NPCS && moved == V41_AMBIENT_NPCS)
        printf("NEMQO v4.1 ambient NPC movement: PASS %d/%d", moved, connected);
    else
        printf("NEMQO v4.1 ambient NPC movement: FAIL moved=%d connected=%d expected=%d",
            moved, connected, V41_AMBIENT_NPCS);
    return 1;
}

stock V41_GetNPCIndex(playerid)
{
    if (!IsPlayerNPC(playerid)) return -1;
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof(name));
    for (new i = 0; i < V41_AMBIENT_NPCS; i++)
    {
        if (!strcmp(name, g_V41NPCNames[i], true)) return i;
    }
    return -1;
}

stock V41_PrepareNPC(playerid)
{
    new index = V41_GetNPCIndex(playerid);
    if (index < 0) return 0;
    SetSpawnInfo(playerid, 0, g_V41NPCSkins[index], g_V41NPCSpawn[index][0],
        g_V41NPCSpawn[index][1], g_V41NPCSpawn[index][2], g_V41NPCSpawn[index][3],
        0, 0, 0, 0, 0, 0);
    SpawnPlayer(playerid);
    return 1;
}

stock V41_SetupSpawnedNPC(playerid)
{
    new index = V41_GetNPCIndex(playerid);
    if (index < 0) return 0;
    SetPlayerSkin(playerid, g_V41NPCSkins[index]);
    SetPlayerColor(playerid, 0xB0B0B0AA);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, g_V41NPCSpawn[index][0], g_V41NPCSpawn[index][1], g_V41NPCSpawn[index][2]);
    SetPlayerFacingAngle(playerid, g_V41NPCSpawn[index][3]);
    ApplyAnimation(playerid, "PED", "WALK_civi", 4.1, 1, 1, 1, 1, 0, 1);
    return 1;
}
