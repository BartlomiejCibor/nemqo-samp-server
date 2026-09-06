#include <a_samp>

#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_GREEN         0x40FF40FF
#define COLOR_RED           0xFF4040FF
#define COLOR_YELLOW        0xFFD740FF
#define COLOR_BLUE          0x40A0FFFF
#define COLOR_GREY          0xB0B0B0FF

#define DIALOG_LOGIN        1000
#define DIALOG_REGISTER     1001
#define DIALOG_HELP         1002
#define DIALOG_STATS        1003
#define DIALOG_SKIN         1004

#define START_MONEY         5000
#define KILL_REWARD         500
#define KILL_XP             25
#define BONUS_MONEY         1000
#define BONUS_XP            5
#define BONUS_COOLDOWN      300
#define SUMO_WORLD          20
#define SUMO_X              2000.0
#define SUMO_Y             -1400.0
#define SUMO_Z               200.0
#define SUMO_FALL_Z          194.0

// Duze molo Santa Maria Beach: wspolny spawn i punkt powrotu Freeroam.
#define NEMQO_START_X        370.7500
#define NEMQO_START_Y      -2032.5000
#define NEMQO_START_Z          7.8300
#define NEMQO_START_A          0.0000

new DB:g_DB;
new bool:g_Logged[MAX_PLAYERS];
new bool:g_AccountExists[MAX_PLAYERS];
new bool:g_InDM[MAX_PLAYERS];
new bool:g_InSumo[MAX_PLAYERS];
new bool:g_SpeedVisible[MAX_PLAYERS];
new g_LoginAttempts[MAX_PLAYERS];
new g_Name[MAX_PLAYERS][MAX_PLAYER_NAME + 1];
new g_PasswordHash[MAX_PLAYERS][65];
new g_Salt[MAX_PLAYERS][33];
new g_Money[MAX_PLAYERS];
new g_Level[MAX_PLAYERS];
new g_XP[MAX_PLAYERS];
new g_Kills[MAX_PLAYERS];
new g_Deaths[MAX_PLAYERS];
new g_LastBonus[MAX_PLAYERS];
new g_Skin[MAX_PLAYERS];
new g_PersonalVehicle[MAX_PLAYERS];
new PlayerText:g_SpeedText[MAX_PLAYERS];
new PlayerText:g_FuelText[MAX_PLAYERS];
new PlayerText:g_VehicleText[MAX_PLAYERS];
new g_SumoPlatform[MAX_PLAYERS];
new bool:g_SelectingSkin[MAX_PLAYERS];
new g_SelectedSkin[MAX_PLAYERS];

new const g_SkinChoices[] =
{
    0, 7, 14, 19, 20, 21, 22, 23, 28, 29,
    46, 47, 48, 56, 57, 58, 93, 105, 106, 107,
    120, 121, 122, 170, 188, 192, 193, 195, 211, 230
};

new const g_TuningWheels[] =
{
    1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080,
    1081, 1082, 1083, 1084, 1085, 1096, 1097, 1098
};

forward KickDelayed(playerid);
forward UpdateSpeedometers();
forward bool:IsValidAccountName(const name[]);
forward bool:IsSafeTuningModel(model);
forward bool:SupportsRandomPaintjob(model);
forward V4_Init();
forward V4_Reset(playerid);
forward V4_Connect(playerid);
forward V4_Disconnect(playerid);
forward V4_Load(playerid);
forward V4_Save(playerid);
forward V4_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
forward V4_IsInArena(playerid);
forward V4_SpawnArena(playerid);
forward V4_OnPlayerDeath(playerid, killerid);
forward V4_GetFuelPercent(playerid);
forward V4_LeaveArena(playerid);
forward V4_OnPlayerCommandText(playerid, cmdtext[]);
forward V4_OnVehicleDeath(vehicleid);
forward PSZ_Init();
forward PSZ_RunSelfTest();
forward PSZ_Reset(playerid);
forward PSZ_Load(playerid);
forward PSZ_Save(playerid);
forward PSZ_CreatePlayerUI(playerid);
forward PSZ_OnLogin(playerid);
forward PSZ_OnDisconnect(playerid);
forward PSZ_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
forward PSZ_OnCommandText(playerid, cmdtext[]);
forward PSZ_OnKeyStateChange(playerid, newkeys, oldkeys);
forward PSZ_UpdateDrift(playerid, vehicleid, speed);
forward PSZ_FinishDriftCombo(playerid);
forward PSZ_DisableDrift(playerid);
forward PSZ_GetRankIndex(playerid);
forward PSZ_LeaveHide(playerid, disconnecting);
forward Island_Init();
forward Island_RunSelfTest();
forward Island_OnCommandText(playerid, cmdtext[]);
forward Island_Destroy();

#include <nemqo_v4_data>
#include <nemqo_psz_data>
#include <nemqo_v4_safety>
#include <nemqo_v4_safety_tests>
#include <nemqo_v4_storage_tests>
#include <nemqo_v41_fleet>
#include <nemqo_v41_ambient>

main()
{
    print("-------------------------------------------");
    print(" NEMQO Drift & Zabawa | PSZ Tribute");
    print("-------------------------------------------");
}

public OnGameModeInit()
{
    V4_VehicleTrackingInit();
    SetGameModeText("Drift & Zabawa + RPG");
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_GLOBAL);
    ShowNameTags(1);
    UsePlayerPedAnims();
    EnableStuntBonusForAll(1);
    SetWorldTime(14);
    SetWeather(2);
    SetTimer("UpdateSpeedometers", 250, true);

    // Wszystkie bezpieczne postacie GTA SA. Skin 74 jest wadliwy w SA-MP 0.3.7.
    for (new skin = 0; skin <= 311; skin++)
    {
        if (skin == 74) continue;
        AddPlayerClass(skin, 2491.75, -1668.20, 13.34, 180.0, 0, 0, 0, 0, 0, 0);
    }

    // Los Santos - centrum, Grove Street, lotnisko i dzielnice.
    AddStaticVehicleEx(411, 1488.0, -1737.0, 13.2, 90.0, 1, 1, 300);
    AddStaticVehicleEx(451, 1488.0, -1742.0, 13.2, 90.0, 3, 3, 300);
    AddStaticVehicleEx(560, 1488.0, -1747.0, 13.2, 90.0, 6, 6, 300);
    AddStaticVehicleEx(522, 1488.0, -1752.0, 13.1, 90.0, 0, 0, 300);
    AddStaticVehicleEx(487, 1534.0, -1753.0, 13.6, 0.0, 1, 3, 300);
    AddStaticVehicleEx(432, 1545.0, -1675.0, 13.4, 90.0, 43, 0, 300);
    AddStaticVehicleEx(470, 1545.0, -1669.0, 13.4, 90.0, 43, 0, 300);
    AddStaticVehicleEx(541, 1405.0, -1795.0, 13.2, 0.0, 22, 22, 300);
    AddStaticVehicleEx(415, 1410.0, -1795.0, 13.2, 0.0, 25, 1, 300);
    AddStaticVehicleEx(601, 1530.0, -1645.0, 13.5, 180.0, 1, 1, 300);
    AddStaticVehicleEx(560, 2130.5, -1150.4, 23.6, 90.0, 6, 6, 300);
    AddStaticVehicleEx(562, 1945.2, -1135.8, 25.4, 90.0, 35, 35, 300);
    AddStaticVehicleEx(522, 2495.3, -1683.4, 13.1, 0.0, 3, 8, 300);
    AddStaticVehicleEx(579, 2501.0, -1683.2, 13.4, 0.0, 1, 1, 300);
    AddStaticVehicleEx(463, 2489.8, -1683.5, 13.0, 0.0, 79, 79, 300);
    AddStaticVehicleEx(411, 1177.8, -1325.0, 13.6, 270.0, 116, 1, 300);
    AddStaticVehicleEx(468, 1183.0, -1325.0, 13.2, 270.0, 46, 46, 300);
    AddStaticVehicleEx(489, 1098.4, -1772.8, 13.5, 90.0, 14, 123, 300);
    AddStaticVehicleEx(525, 1640.0, -1521.0, 13.5, 180.0, 1, 1, 300);
    AddStaticVehicleEx(487, 1990.0, -2286.0, 14.0, 90.0, 29, 42, 300);

    // San Fierro - centrum, port, lotnisko i polnoc miasta.
    AddStaticVehicleEx(411, -1958.3, 258.7, 35.2, 90.0, 64, 64, 300);
    AddStaticVehicleEx(560, -1659.3, 1210.2, 7.0, 220.0, 6, 6, 300);
    AddStaticVehicleEx(522, -1945.0, 273.5, 35.0, 180.0, 3, 8, 300);
    AddStaticVehicleEx(451, -2265.0, 535.0, 35.0, 180.0, 16, 16, 300);
    AddStaticVehicleEx(562, -2410.0, 741.0, 35.0, 180.0, 35, 35, 300);
    AddStaticVehicleEx(579, -2688.0, 636.0, 14.2, 180.0, 1, 1, 300);
    AddStaticVehicleEx(487, -2227.0, 2326.0, 7.5, 180.0, 54, 29, 300);
    AddStaticVehicleEx(593, -1260.0, -80.0, 14.1, 315.0, 1, 1, 300);
    AddStaticVehicleEx(417, -1224.0, -10.0, 14.2, 45.0, 1, 1, 300);
    AddStaticVehicleEx(429, -1682.0, 1313.0, 7.0, 135.0, 13, 13, 300);
    AddStaticVehicleEx(541, -1565.0, 677.0, 7.0, 90.0, 22, 22, 300);
    AddStaticVehicleEx(463, -2620.0, 1376.0, 7.0, 180.0, 79, 79, 300);

    // Las Venturas - Strip, kasyna, lotnisko i obrzeza.
    AddStaticVehicleEx(451, 2040.0520, 1319.2799, 10.3779, 183.2439, 16, 16, 300);
    AddStaticVehicleEx(429, 2040.5247, 1359.2783, 10.3516, 177.1306, 13, 13, 300);
    AddStaticVehicleEx(421, 2110.4102, 1398.3672, 10.7552, 359.5964, 13, 13, 300);
    AddStaticVehicleEx(411, 2074.9624, 1479.2120, 10.3990, 359.6861, 64, 64, 300);
    AddStaticVehicleEx(477, 2075.6038, 1666.9750, 10.4252, 359.7507, 94, 94, 300);
    AddStaticVehicleEx(541, 2119.5845, 1938.5969, 10.2967, 181.9064, 22, 22, 300);
    AddStaticVehicleEx(402, 1944.1003, 1344.7717, 8.9411, 0.8168, 30, 30, 300);
    AddStaticVehicleEx(415, 1685.4872, 1751.9667, 10.5990, 268.1183, 25, 1, 300);
    AddStaticVehicleEx(487, 1614.7153, 1548.7513, 11.2749, 347.1516, 58, 8, 300);
    AddStaticVehicleEx(476, 1665.0, 1220.0, 11.7, 270.0, 7, 6, 300);
    AddStaticVehicleEx(522, 1430.2354, 1999.0144, 10.3896, 352.0951, 6, 25, 300);
    AddStaticVehicleEx(598, 2277.6846, 2477.1096, 10.5652, 180.1090, 0, 1, 300);
    AddStaticVehicleEx(523, 2294.7305, 2441.2651, 10.3860, 9.3764, 0, 0, 300);
    AddStaticVehicleEx(437, 2577.2354, 1038.8063, 10.4777, 181.7069, 35, 1, 300);
    AddStaticVehicleEx(400, 1380.8304, 1159.1782, 10.9128, 355.7117, 123, 1, 300);

    // Los Santos - ruch uliczny, parkingi, sluzby i przemysl.
    AddStaticVehicleEx(420, 1804.8, -1865.2, 13.2, 90.0, 6, 1, 300);
    AddStaticVehicleEx(438, 1777.2, -1895.5, 13.2, 0.0, 6, 1, 300);
    AddStaticVehicleEx(431, 1802.5, -1915.5, 13.5, 0.0, 71, 59, 300);
    AddStaticVehicleEx(596, 1545.0, -1615.2, 13.2, 180.0, 0, 1, 300);
    AddStaticVehicleEx(596, 1552.0, -1615.2, 13.2, 180.0, 0, 1, 300);
    AddStaticVehicleEx(416, 1178.4, -1338.2, 13.5, 270.0, 1, 3, 300);
    AddStaticVehicleEx(407, 1750.0, -1455.0, 13.7, 270.0, 3, 1, 300);
    AddStaticVehicleEx(525, 2072.4, -2033.0, 13.4, 180.0, 1, 1, 300);
    AddStaticVehicleEx(408, 2180.0, -1990.0, 13.8, 90.0, 26, 26, 300);
    AddStaticVehicleEx(482, 2100.0, -1780.0, 13.5, 180.0, 10, 10, 300);
    AddStaticVehicleEx(499, 2175.0, -1805.0, 13.4, 0.0, 61, 61, 300);
    AddStaticVehicleEx(574, 1640.0, -1860.0, 13.1, 0.0, 26, 26, 300);
    AddStaticVehicleEx(567, 2508.0, -1670.5, 13.3, 90.0, 88, 64, 300);
    AddStaticVehicleEx(536, 2478.0, -1653.0, 13.2, 180.0, 12, 1, 300);
    AddStaticVehicleEx(462, 2105.0, -1806.0, 13.0, 90.0, 3, 3, 300);
    AddStaticVehicleEx(485, 1680.0, -2320.0, 13.1, 0.0, 1, 73, 300);

    // San Fierro - centrum, sluzby, port i dzielnice przemyslowe.
    AddStaticVehicleEx(420, -1980.0, 139.0, 27.3, 90.0, 6, 1, 300);
    AddStaticVehicleEx(597, -1590.0, 650.0, 7.0, 180.0, 0, 1, 300);
    AddStaticVehicleEx(597, -1600.0, 650.0, 7.0, 180.0, 0, 1, 300);
    AddStaticVehicleEx(416, -2655.0, 635.0, 14.6, 180.0, 1, 3, 300);
    AddStaticVehicleEx(407, -2025.0, 79.0, 28.0, 270.0, 3, 1, 300);
    AddStaticVehicleEx(431, -1986.0, 105.0, 27.7, 90.0, 71, 59, 300);
    AddStaticVehicleEx(525, -2105.0, -25.0, 35.2, 90.0, 1, 1, 300);
    AddStaticVehicleEx(455, -1700.0, 40.0, 3.6, 180.0, 84, 31, 300);
    AddStaticVehicleEx(482, -1750.0, 950.0, 24.8, 90.0, 10, 10, 300);
    AddStaticVehicleEx(413, -1800.0, -25.0, 15.2, 270.0, 88, 1, 300);
    AddStaticVehicleEx(518, -2450.0, 135.0, 35.0, 90.0, 21, 1, 300);
    AddStaticVehicleEx(468, -2120.0, 920.0, 79.5, 180.0, 46, 46, 300);
    AddStaticVehicleEx(574, -1900.0, 840.0, 35.0, 0.0, 26, 26, 300);
    AddStaticVehicleEx(485, -1355.0, -145.0, 14.0, 315.0, 1, 73, 300);

    // Las Venturas - Strip, kasyna, sluzby i zaplecze lotniska.
    AddStaticVehicleEx(420, 2035.0, 1005.0, 10.5, 90.0, 6, 1, 300);
    AddStaticVehicleEx(420, 2140.0, 1440.0, 10.5, 180.0, 6, 1, 300);
    AddStaticVehicleEx(409, 2190.0, 1675.0, 10.6, 0.0, 1, 1, 300);
    AddStaticVehicleEx(409, 2020.0, 1915.0, 12.0, 180.0, 1, 1, 300);
    AddStaticVehicleEx(598, 2265.0, 2477.0, 10.5, 180.0, 0, 1, 300);
    AddStaticVehicleEx(416, 1600.0, 1825.0, 10.9, 0.0, 1, 3, 300);
    AddStaticVehicleEx(407, 1755.0, 2070.0, 10.9, 180.0, 3, 1, 300);
    AddStaticVehicleEx(431, 1710.0, 1525.0, 10.8, 90.0, 71, 59, 300);
    AddStaticVehicleEx(525, 1040.0, 1300.0, 10.6, 0.0, 1, 1, 300);
    AddStaticVehicleEx(482, 1100.0, 1700.0, 10.8, 90.0, 10, 10, 300);
    AddStaticVehicleEx(579, 1700.0, 1275.0, 10.9, 180.0, 1, 1, 300);
    AddStaticVehicleEx(468, 1680.0, 1300.0, 10.4, 180.0, 46, 46, 300);
    AddStaticVehicleEx(574, 2200.0, 1250.0, 10.5, 0.0, 26, 26, 300);
    AddStaticVehicleEx(485, 1325.0, 1350.0, 10.4, 270.0, 1, 73, 300);

    // Samoloty na lotniskach Los Santos, San Fierro i Las Venturas.
    AddStaticVehicleEx(519, 1990.0, -2310.0, 14.8, 90.0, 1, 1, 600);
    AddStaticVehicleEx(511, 1935.0, -2310.0, 14.5, 90.0, 1, 1, 600);
    AddStaticVehicleEx(593, 1885.0, -2310.0, 14.2, 90.0, 1, 1, 600);
    AddStaticVehicleEx(512, 1840.0, -2310.0, 14.0, 90.0, 39, 39, 600);
    AddStaticVehicleEx(553, 1780.0, -2310.0, 15.5, 90.0, 1, 1, 600);
    AddStaticVehicleEx(519, -1300.0, -115.0, 15.0, 315.0, 1, 1, 600);
    AddStaticVehicleEx(511, -1340.0, -150.0, 14.8, 315.0, 1, 1, 600);
    AddStaticVehicleEx(476, -1380.0, -185.0, 15.0, 315.0, 7, 6, 600);
    AddStaticVehicleEx(519, 1660.0, 1200.0, 12.0, 270.0, 1, 1, 600);
    AddStaticVehicleEx(511, 1715.0, 1200.0, 11.8, 270.0, 1, 1, 600);
    AddStaticVehicleEx(593, 1765.0, 1200.0, 11.2, 270.0, 1, 1, 600);
    V41_CreateExpandedFleet();

    g_DB = db_open("accounts.db");
    if (g_DB == DB:0)
    {
        print("FATAL: Nie mozna otworzyc bazy accounts.db");
        SendRconCommand("exit");
        return 0;
    }

    new DBResult:result = db_query(g_DB, "CREATE TABLE IF NOT EXISTS accounts (name TEXT PRIMARY KEY COLLATE NOCASE,password_hash TEXT NOT NULL,salt TEXT NOT NULL,money INTEGER NOT NULL DEFAULT 5000,level INTEGER NOT NULL DEFAULT 1,xp INTEGER NOT NULL DEFAULT 0,kills INTEGER NOT NULL DEFAULT 0,deaths INTEGER NOT NULL DEFAULT 0,last_bonus INTEGER NOT NULL DEFAULT 0,skin INTEGER NOT NULL DEFAULT 0,admin INTEGER NOT NULL DEFAULT 0,bank INTEGER NOT NULL DEFAULT 0,job INTEGER NOT NULL DEFAULT 0,job_xp INTEGER NOT NULL DEFAULT 0,wanted INTEGER NOT NULL DEFAULT 0,race_best INTEGER NOT NULL DEFAULT 0,owned_model INTEGER NOT NULL DEFAULT 0,owned_color1 INTEGER NOT NULL DEFAULT 1,owned_color2 INTEGER NOT NULL DEFAULT 1,owned_paintjob INTEGER NOT NULL DEFAULT -1,owned_wheels INTEGER NOT NULL DEFAULT 1080,owned_neon INTEGER NOT NULL DEFAULT 0,owned_fuel INTEGER NOT NULL DEFAULT 100,odometer INTEGER NOT NULL DEFAULT 0,house_id INTEGER NOT NULL DEFAULT 0,business_id INTEGER NOT NULL DEFAULT 0,achievements INTEGER NOT NULL DEFAULT 0,daily_day INTEGER NOT NULL DEFAULT 0,daily_progress INTEGER NOT NULL DEFAULT 0,remember_skin INTEGER NOT NULL DEFAULT 0,respect INTEGER NOT NULL DEFAULT 0,drift_total INTEGER NOT NULL DEFAULT 0,drift_best INTEGER NOT NULL DEFAULT 0,hide_wins INTEGER NOT NULL DEFAULT 0,event_wins INTEGER NOT NULL DEFAULT 0)");
    db_free_result(result);
    V4_Init();
    PSZ_Init();
    Island_Init();
    V41_CreateAmbientActors();
    PSZ_RunSelfTest();
    if (!V4_VerifyVehicleLabels() || !V4_RunSafetySelfTest() || !V4_RunStorageSelfTest())
    {
        print("FATAL: V4 safety/storage verification failed");
        SendRconCommand("exit");
        return 0;
    }
    Island_RunSelfTest();
    V41_StartAmbientNPCs();
    return 1;
}

public OnGameModeExit()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (IsPlayerConnected(playerid) && g_Logged[playerid]) SaveAccount(playerid);
    }
    Island_Destroy();
    V4_DestroyAllVehicleLabels();
    db_close(g_DB);
    return 1;
}

public OnPlayerConnect(playerid)
{
    ResetPlayerData(playerid);
    GetPlayerName(playerid, g_Name[playerid], MAX_PLAYER_NAME + 1);

    if (IsPlayerNPC(playerid))
    {
        if (!V41_PrepareNPC(playerid)) Kick(playerid);
        return 1;
    }

    CreateVehicleHUD(playerid);
    PSZ_CreatePlayerUI(playerid);

    if (!IsValidAccountName(g_Name[playerid]))
    {
        SendClientMessage(playerid, COLOR_RED, "Nazwa moze zawierac tylko litery, cyfry, _, [ oraz ].");
        SetTimerEx("KickDelayed", 500, false, "i", playerid);
        return 1;
    }

    TogglePlayerSpectating(playerid, true);
    V4_Connect(playerid);

    new query[128];
    format(query, sizeof(query), "SELECT * FROM accounts WHERE name='%s' LIMIT 1", g_Name[playerid]);
    new DBResult:result = db_query(g_DB, query);

    if (db_num_rows(result) > 0)
    {
        g_AccountExists[playerid] = true;
        db_get_field_assoc(result, "password_hash", g_PasswordHash[playerid], 65);
        db_get_field_assoc(result, "salt", g_Salt[playerid], 33);
        ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD,
            "Logowanie", "Konto istnieje. Podaj haslo:", "Zaloguj", "Wyjdz");
    }
    else
    {
        g_AccountExists[playerid] = false;
        ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD,
            "Rejestracja", "Nowe konto. Ustaw haslo (minimum 6 znakow):", "Utworz", "Wyjdz");
    }
    db_free_result(result);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    #pragma unused reason
    V4_OdoResetDriver(playerid);
    if (IsPlayerNPC(playerid))
    {
        ResetPlayerData(playerid);
        return 1;
    }
    PSZ_OnDisconnect(playerid);
    if (g_Logged[playerid]) SaveAccount(playerid);
    V4_Disconnect(playerid);
    if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(g_PersonalVehicle[playerid]);
    }
    if (g_SumoPlatform[playerid] != INVALID_OBJECT_ID)
    {
        DestroyPlayerObject(playerid, g_SumoPlatform[playerid]);
    }
    DestroyVehicleHUD(playerid);
    ResetPlayerData(playerid);
    return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
    if (!g_SelectingSkin[playerid]) return 1;

    // AddPlayerClass pomija wadliwy skin 74, dlatego od klasy 74 przesuwamy ID o 1.
    g_SelectedSkin[playerid] = (classid < 74) ? classid : classid + 1;
    SetPlayerPos(playerid, 2491.75, -1668.20, 13.34);
    SetPlayerFacingAngle(playerid, 180.0);
    SetPlayerCameraPos(playerid, 2491.75, -1673.50, 14.20);
    SetPlayerCameraLookAt(playerid, 2491.75, -1668.20, 13.80);

    new text[96];
    format(text, sizeof(text), "~w~POSTAC ~y~%d~n~~g~STRZALKI: ZMIANA  ~w~SPAWN: WYBIERZ", g_SelectedSkin[playerid]);
    GameTextForPlayer(playerid, text, 5000, 3);
    return 1;
}

public OnPlayerRequestSpawn(playerid)
{
    if (!g_SelectingSkin[playerid]) return 1;
    g_Skin[playerid] = g_SelectedSkin[playerid];
    g_SelectingSkin[playerid] = false;
    new message[160];
    format(message, sizeof(message), "Wybrano skin ID %d.\nCzy serwer ma automatycznie uzywac go przy kolejnych logowaniach?", g_Skin[playerid]);
    ShowPlayerDialog(playerid, DIALOG_REMEMBER_SKIN, DIALOG_STYLE_MSGBOX,
        "Pamietanie ostatniego skina", message, "Zapamietaj", "Wybieraj");
    return 0;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_REMEMBER_SKIN)
    {
        g_RememberSkin[playerid] = response != 0;
        FinishLogin(playerid);
        SaveAccount(playerid);
        SendClientMessage(playerid, g_RememberSkin[playerid] ? COLOR_GREEN : COLOR_GREY,
            g_RememberSkin[playerid] ? "Skin zapisany i bedzie wybierany automatycznie." : "Skin zapisany. Selektor pojawi sie przy nastepnym logowaniu.");
        return 1;
    }
    if (PSZ_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (V4_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (dialogid == DIALOG_LOGIN)
    {
        if (!response)
        {
            SetTimerEx("KickDelayed", 200, false, "i", playerid);
            return 1;
        }

        new hash[65];
        SHA256_PassHash(inputtext, g_Salt[playerid], hash, sizeof(hash));
        if (strcmp(hash, g_PasswordHash[playerid], true) == 0)
        {
            LoadAccount(playerid);
            if (g_RememberSkin[playerid] && g_Skin[playerid] >= 0 && g_Skin[playerid] <= 311 && g_Skin[playerid] != 74)
            {
                FinishLogin(playerid);
                SendClientMessage(playerid, COLOR_GREEN, "Uzyto zapamietanego skina. Zmiana: /skinselect lub /rememberskin.");
            }
            else ShowSkinSelection(playerid);
        }
        else
        {
            g_LoginAttempts[playerid]++;
            if (g_LoginAttempts[playerid] >= 3)
            {
                SendClientMessage(playerid, COLOR_RED, "Trzy bledne proby logowania.");
                SetTimerEx("KickDelayed", 500, false, "i", playerid);
            }
            else
            {
                new message[96];
                format(message, sizeof(message), "Bledne haslo. Pozostalo prob: %d", 3 - g_LoginAttempts[playerid]);
                ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD,
                    "Logowanie", message, "Zaloguj", "Wyjdz");
            }
        }
        return 1;
    }

    if (dialogid == DIALOG_REGISTER)
    {
        if (!response)
        {
            SetTimerEx("KickDelayed", 200, false, "i", playerid);
            return 1;
        }
        if (strlen(inputtext) < 6 || strlen(inputtext) > 32)
        {
            ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD,
                "Rejestracja", "Haslo musi miec od 6 do 32 znakow.", "Utworz", "Wyjdz");
            return 1;
        }

        format(g_Salt[playerid], 33, "%08x%08x%08x%08x",
            random(2147483647), random(2147483647), random(2147483647), random(2147483647));
        SHA256_PassHash(inputtext, g_Salt[playerid], g_PasswordHash[playerid], 65);

        new query[512];
        format(query, sizeof(query),
            "INSERT INTO accounts (name,password_hash,salt,money,level,xp,kills,deaths,last_bonus) VALUES ('%s','%s','%s',%d,1,0,0,0,0)",
            g_Name[playerid], g_PasswordHash[playerid], g_Salt[playerid], START_MONEY);
        new DBResult:result = db_query(g_DB, query);
        db_free_result(result);

        g_AccountExists[playerid] = true;
        g_Money[playerid] = START_MONEY;
        g_Level[playerid] = 1;
        g_Skin[playerid] = 0;
        g_RememberSkin[playerid] = false;
        SendClientMessage(playerid, COLOR_GREEN, "Konto zostalo utworzone. Otrzymujesz $5000 na start.");
        ShowSkinSelection(playerid);
        return 1;
    }

    if (dialogid == DIALOG_SKIN)
    {
        if (!response || listitem < 0 || listitem >= sizeof(g_SkinChoices))
        {
            ShowSkinSelection(playerid);
            return 1;
        }
        g_Skin[playerid] = g_SkinChoices[listitem];
        FinishLogin(playerid);
        SaveAccount(playerid);
        new message[96];
        format(message, sizeof(message), "Wybrano skin ID %d. Pozniej zmienisz go komenda /skin.", g_Skin[playerid]);
        SendClientMessage(playerid, COLOR_GREEN, message);
        return 1;
    }
    return 0;
}

public OnPlayerSpawn(playerid)
{
    if (IsPlayerNPC(playerid)) return V41_SetupSpawnedNPC(playerid);
    if (!g_Logged[playerid]) return 1;

    V41_PreloadAnimations(playerid);

    SetPlayerSkin(playerid, g_Skin[playerid]);
    ResetPlayerWeapons(playerid);
    SetPlayerHealth(playerid, 100.0);

    if (g_InDM[playerid])
    {
        SpawnInDMArena(playerid);
    }
    else if (g_InSumo[playerid])
    {
        SpawnInSumoArena(playerid);
    }
    else if (V4_IsInArena(playerid))
    {
        V4_SpawnArena(playerid);
    }
    else
    {
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerInterior(playerid, 0);
        GivePlayerWeapon(playerid, 24, 70);
        GivePlayerWeapon(playerid, 29, 250);
        SendClientMessage(playerid, COLOR_BLUE, "Freeroam: /help | Arena: /dm");
    }
    return 1;
}

public OnVehicleSpawn(vehicleid)
{
    V4_OnTrackedVehicleSpawn(vehicleid);
    V41_OnVehicleSpawn(vehicleid);
    return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    #pragma unused killerid
    V4_OdoResetVehicle(vehicleid);
    V4_OnVehicleDeath(vehicleid);
    return 1;
}

public OnPlayerExitVehicle(playerid, vehicleid)
{
    V4_OdoResetDriver(playerid);
    #pragma unused vehicleid
    if (g_InSumo[playerid]) ExitSumo(playerid, true);
    return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
    #pragma unused reason
    if (IsPlayerNPC(playerid)) return 1;
    if (!g_Logged[playerid]) return 1;

    g_Deaths[playerid]++;
    V4_OnPlayerDeath(playerid, killerid);

    if (killerid != INVALID_PLAYER_ID && killerid != playerid && g_Logged[killerid])
    {
        g_Kills[killerid]++;
        if (g_InDM[playerid] && g_InDM[killerid])
        {
            AddMoney(killerid, KILL_REWARD);
            AddXP(killerid, KILL_XP);
            SendClientMessage(killerid, COLOR_GREEN, "+$500 i +25 XP za zabojstwo na arenie.");
        }
        SaveAccount(killerid);
    }
    SaveAccount(playerid);
    return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    if (PSZ_OnKeyStateChange(playerid, newkeys, oldkeys)) return 1;
    if ((newkeys & KEY_SUBMISSION) && !(oldkeys & KEY_SUBMISSION))
    {
        RepairPlayerVehicle(playerid);
    }
    return 1;
}

public OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
    #pragma unused playerid
    #pragma unused fX
    #pragma unused fY
    #pragma unused fZ

    if (hittype != BULLET_HIT_TYPE_VEHICLE || GetVehicleModel(hitid) == 0) return 1;

    new Float:damage = 8.0;
    switch (weaponid)
    {
        case 22: damage = 12.0; // Colt 45
        case 23: damage = 18.0; // Silenced
        case 24: damage = 35.0; // Desert Eagle
        case 25: damage = 30.0; // Shotgun
        case 26: damage = 28.0; // Sawnoff
        case 27: damage = 35.0; // Combat shotgun
        case 28, 32: damage = 9.0; // UZI / Tec-9
        case 29: damage = 11.0; // MP5
        case 30, 31: damage = 15.0; // AK / M4
        case 33: damage = 28.0; // Rifle
        case 34: damage = 50.0; // Sniper
        case 38: damage = 12.0; // Minigun, na kazdy pocisk
    }

    new Float:health;
    GetVehicleHealth(hitid, health);
    health -= damage;
    if (health < 0.0) health = 0.0;
    SetVehicleHealth(hitid, health);
    return 1;
}

stock CreateVehicleHUD(playerid)
{
    g_SpeedText[playerid] = CreatePlayerTextDraw(playerid, 621.0, 346.0, "000 KM/H");
    PlayerTextDrawAlignment(playerid, g_SpeedText[playerid], 3);
    PlayerTextDrawFont(playerid, g_SpeedText[playerid], 2);
    PlayerTextDrawLetterSize(playerid, g_SpeedText[playerid], 0.54, 2.05);
    PlayerTextDrawColor(playerid, g_SpeedText[playerid], 0x40CFFFFF);
    PlayerTextDrawSetOutline(playerid, g_SpeedText[playerid], 1);
    PlayerTextDrawTextSize(playerid, g_SpeedText[playerid], 465.0, 0.0);

    g_FuelText[playerid] = CreatePlayerTextDraw(playerid, 474.0, 377.0, "PALIWO [||||||||||] 100%");
    PlayerTextDrawAlignment(playerid, g_FuelText[playerid], 1);
    PlayerTextDrawFont(playerid, g_FuelText[playerid], 1);
    PlayerTextDrawLetterSize(playerid, g_FuelText[playerid], 0.24, 1.0);
    PlayerTextDrawColor(playerid, g_FuelText[playerid], 0xFFD740FF);
    PlayerTextDrawSetOutline(playerid, g_FuelText[playerid], 1);

    g_VehicleText[playerid] = CreatePlayerTextDraw(playerid, 474.0, 393.0, "MODEL 000 | HP 1000 | 0.000 KM");
    PlayerTextDrawAlignment(playerid, g_VehicleText[playerid], 1);
    PlayerTextDrawFont(playerid, g_VehicleText[playerid], 1);
    PlayerTextDrawLetterSize(playerid, g_VehicleText[playerid], 0.205, 0.88);
    PlayerTextDrawColor(playerid, g_VehicleText[playerid], COLOR_WHITE);
    PlayerTextDrawSetOutline(playerid, g_VehicleText[playerid], 1);

    return 1;
}

stock DestroyVehicleHUD(playerid)
{
    PlayerTextDrawDestroy(playerid, g_SpeedText[playerid]);
    PlayerTextDrawDestroy(playerid, g_FuelText[playerid]);
    PlayerTextDrawDestroy(playerid, g_VehicleText[playerid]);
    return 1;
}

stock ShowVehicleHUD(playerid)
{
    PlayerTextDrawShow(playerid, g_SpeedText[playerid]);
    PlayerTextDrawShow(playerid, g_FuelText[playerid]);
    PlayerTextDrawShow(playerid, g_VehicleText[playerid]);
    g_SpeedVisible[playerid] = true;
    return 1;
}

stock HideVehicleHUD(playerid)
{
    PlayerTextDrawHide(playerid, g_SpeedText[playerid]);
    PlayerTextDrawHide(playerid, g_FuelText[playerid]);
    PlayerTextDrawHide(playerid, g_VehicleText[playerid]);
    g_SpeedVisible[playerid] = false;
    return 1;
}

stock ExitSumo(playerid, bool:notify)
{
    if (!g_InSumo[playerid]) return 0;
    g_InSumo[playerid] = false;

    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
    if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(g_PersonalVehicle[playerid]);
        g_PersonalVehicle[playerid] = INVALID_VEHICLE_ID;
    }
    if (g_SumoPlatform[playerid] != INVALID_OBJECT_ID)
    {
        DestroyPlayerObject(playerid, g_SumoPlatform[playerid]);
        g_SumoPlatform[playerid] = INVALID_OBJECT_ID;
    }

    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerInterior(playerid, 0);
    SetPlayerSkin(playerid, g_Skin[playerid]);
    ResetPlayerWeapons(playerid);
    GivePlayerWeapon(playerid, 24, 70);
    GivePlayerWeapon(playerid, 29, 250);
    SetPlayerPos(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z);
    SetPlayerFacingAngle(playerid, NEMQO_START_A);
    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, 0.0);
    SetCameraBehindPlayer(playerid);
    if (notify) SendClientMessage(playerid, COLOR_BLUE, "SUMO zakonczone. Wrociles automatycznie do Freeroam.");
    return 1;
}

public UpdateSpeedometers()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (IsPlayerConnected(playerid) && !IsPlayerNPC(playerid) && g_InSumo[playerid])
        {
            new Float:sumoX, Float:sumoY, Float:sumoZ;
            GetPlayerPos(playerid, sumoX, sumoY, sumoZ);
            #pragma unused sumoX
            #pragma unused sumoY
            if (sumoZ < SUMO_FALL_Z)
            {
                ExitSumo(playerid, true);
                continue;
            }
        }
        if (IsPlayerConnected(playerid) && !IsPlayerNPC(playerid) && g_Logged[playerid] && IsPlayerInAnyVehicle(playerid))
        {
            new vehicleid = GetPlayerVehicleID(playerid);
            new Float:vx, Float:vy, Float:vz;
            GetVehicleVelocity(vehicleid, vx, vy, vz);
            new speed = floatround(floatsqroot(vx * vx + vy * vy + vz * vz) * 180.0);
            new Float:vehicleHealth;
            GetVehicleHealth(vehicleid, vehicleHealth);
            new fuel = V4_GetFuelPercent(playerid), segments = fuel / 10;
            if (segments < 0) segments = 0;
            if (segments > 10) segments = 10;

            new speedText[32], fuelText[80] = "PALIWO [", vehicleText[96];
            format(speedText, sizeof(speedText), "%03d KM/H", speed);
            for (new segment = 0; segment < 10; segment++)
            {
                if (segment < segments) strcat(fuelText, "|");
                else strcat(fuelText, ".");
            }
            new suffix[20];
            format(suffix, sizeof(suffix), "] %d%%", fuel);
            strcat(fuelText, suffix);
            new odometer = 0, owner = g_V4VehicleOwner[vehicleid] - 1;
            if (owner >= 0 && owner < MAX_PLAYERS && g_V4OwnedVehicle[owner] == vehicleid)
                odometer = g_V4Odometer[owner];
            format(vehicleText, sizeof(vehicleText), "MODEL %d | HP %d | %d.%03d KM",
                GetVehicleModel(vehicleid), floatround(vehicleHealth), odometer / 1000, odometer % 1000);
            PlayerTextDrawSetString(playerid, g_SpeedText[playerid], speedText);
            PlayerTextDrawSetString(playerid, g_FuelText[playerid], fuelText);
            PlayerTextDrawSetString(playerid, g_VehicleText[playerid], vehicleText);
            if (!g_SpeedVisible[playerid]) ShowVehicleHUD(playerid);
            PSZ_UpdateDrift(playerid, vehicleid, speed);
        }
        else if (IsPlayerConnected(playerid) && !IsPlayerNPC(playerid) && g_SpeedVisible[playerid])
        {
            HideVehicleHUD(playerid);
        }
    }
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!g_Logged[playerid])
    {
        SendClientMessage(playerid, COLOR_RED, "Najpierw sie zaloguj.");
        return 1;
    }

    if (Island_OnCommandText(playerid, cmdtext)) return 1;
    if (PSZ_OnCommandText(playerid, cmdtext)) return 1;

    if (!strcmp(cmdtext, "/help", true))
    {
        ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX, "NEMQO PSZ Tribute - Pomoc", "/zabawy - panel szybkich trybow\n/wyspa - Wyspa NEMQO: domki, laguna i marina\n/drift, /topdrift, /tandem - drift i ranking\n/chowany - zapisy do Chowanego\n/plac - pustynny Plac Zabaw\n/rampa - rampa przed autem (takze klakson/H)\n/anim, /dance - animacje\n/event - aktualny automatyczny event\n/menu - systemy RPG v4\n/arena, /dm, /sumo - pozostale areny\n/rememberskin, /skinselect - pamietanie i wybor postaci\n/stats - pelne statystyki\nPelna lista: README_SERVER.md", "OK", "");
        return 1;
    }

    if (!strcmp(cmdtext, "/stats", true))
    {
        new text[700];
        format(text, sizeof(text),
            "Gracz: %s\nPieniadze: $%d | Bank: $%d\nLevel: %d | XP: %d/%d\nRespekt: %d | Ranga: %s\nDrift rekord: %d | Drift lacznie: %d\nChowany wygrane: %d | Eventy wygrane: %d\nZabojstwa: %d | Smierci: %d\nPraca: %s | Job XP: %d\nWanted: %d/6\nRekord wyscigu: %d s\nPojazd: %d | Paliwo: %d%% | Przebieg: %d m\nDom: %d | Firma: %d",
            g_Name[playerid], g_Money[playerid], g_V4Bank[playerid], g_Level[playerid], g_XP[playerid],
            XPForNextLevel(playerid), g_PSZRespect[playerid], g_PSZRankNames[PSZ_GetRankIndex(playerid)],
            g_PSZDriftBest[playerid], g_PSZDriftTotal[playerid], g_PSZHideWins[playerid], g_PSZEventWins[playerid],
            g_Kills[playerid], g_Deaths[playerid],
            g_V4JobNames[g_V4Job[playerid]], g_V4JobXP[playerid], g_V4Wanted[playerid],
            g_V4RaceBest[playerid], g_V4OwnedModel[playerid], g_V4Fuel[playerid],
            g_V4Odometer[playerid], g_V4House[playerid], g_V4Business[playerid]);
        ShowPlayerDialog(playerid, DIALOG_STATS, DIALOG_STYLE_MSGBOX, "Statystyki", text, "OK", "");
        return 1;
    }

    if (!strcmp(cmdtext, "/dm", true))
    {
        if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, false);
        PSZ_DisableDrift(playerid);
        if (V4_IsInArena(playerid)) V4_LeaveArena(playerid);
        if (g_InSumo[playerid]) ExitSumo(playerid, false);
        g_InDM[playerid] = true;
        if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
        if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID)
        {
            DestroyVehicle(g_PersonalVehicle[playerid]);
            g_PersonalVehicle[playerid] = INVALID_VEHICLE_ID;
        }
        if (g_SumoPlatform[playerid] != INVALID_OBJECT_ID)
        {
            DestroyPlayerObject(playerid, g_SumoPlatform[playerid]);
            g_SumoPlatform[playerid] = INVALID_OBJECT_ID;
        }
        SpawnInDMArena(playerid);
        SendClientMessage(playerid, COLOR_RED, "Arena DM: zabojstwo daje $500 i 25 XP. Powrot: /freeroam");
        return 1;
    }

    if (!strcmp(cmdtext, "/sumo", true))
    {
        if (g_InSumo[playerid])
        {
            SendClientMessage(playerid, COLOR_GREY, "Juz jestes na Sumo. Uzyj /leave, aby wrocic.");
            return 1;
        }
        if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, false);
        PSZ_DisableDrift(playerid);
        if (V4_IsInArena(playerid)) V4_LeaveArena(playerid);
        g_InDM[playerid] = false;
        if (!SpawnInSumoArena(playerid)) return 1;
        SendClientMessage(playerid, COLOR_YELLOW, "SUMO: zepchnij innych z podniebnej platformy. Powrot: /freeroam");
        return 1;
    }

    if (!strcmp(cmdtext, "/freeroam", true))
    {
        if (g_PSZHideJoined[playerid]) PSZ_LeaveHide(playerid, false);
        PSZ_DisableDrift(playerid);
        if (V4_IsInArena(playerid)) V4_LeaveArena(playerid);
        if (g_InSumo[playerid]) return ExitSumo(playerid, true), 1;
        g_InDM[playerid] = false;
        if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
        if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID)
        {
            DestroyVehicle(g_PersonalVehicle[playerid]);
            g_PersonalVehicle[playerid] = INVALID_VEHICLE_ID;
        }
        if (g_SumoPlatform[playerid] != INVALID_OBJECT_ID)
        {
            DestroyPlayerObject(playerid, g_SumoPlatform[playerid]);
            g_SumoPlatform[playerid] = INVALID_OBJECT_ID;
        }
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerInterior(playerid, 0);
        SetPlayerSkin(playerid, g_Skin[playerid]);
        ResetPlayerWeapons(playerid);
        GivePlayerWeapon(playerid, 24, 70);
        GivePlayerWeapon(playerid, 29, 250);
        SetPlayerPos(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z);
        SetPlayerFacingAngle(playerid, NEMQO_START_A);
        SetPlayerHealth(playerid, 100.0);
        SetPlayerArmour(playerid, 0.0);
        SendClientMessage(playerid, COLOR_BLUE, "Wrociles do Freeroam.");
        return 1;
    }

    if (!strcmp(cmdtext, "/leave", true))
    {
        CallLocalFunction("OnPlayerCommandText", "is", playerid, "/freeroam");
        return 1;
    }

    if (!strcmp(cmdtext, "/weapons", true))
    {
        if (g_InDM[playerid] || g_InSumo[playerid]) return SendClientMessage(playerid, COLOR_RED, "Na arenach dodatkowa bron jest wylaczona."), 1;
        ResetPlayerWeapons(playerid);
        GivePlayerWeapon(playerid, 24, 100);
        GivePlayerWeapon(playerid, 25, 100);
        GivePlayerWeapon(playerid, 29, 500);
        GivePlayerWeapon(playerid, 31, 500);
        SendClientMessage(playerid, COLOR_GREEN, "Otrzymales zestaw broni Freeroam.");
        return 1;
    }

    if (!strcmp(cmdtext, "/heal", true))
    {
        SetPlayerHealth(playerid, 100.0);
        if (!g_InDM[playerid]) SetPlayerArmour(playerid, 100.0);
        SendClientMessage(playerid, COLOR_GREEN, "Zdrowie odnowione.");
        return 1;
    }

    if (!strcmp(cmdtext, "/repair", true))
    {
        RepairPlayerVehicle(playerid);
        return 1;
    }

    if (!strcmp(cmdtext, "/tuning", true))
    {
        RandomTunePlayerVehicle(playerid);
        return 1;
    }

    if (!strcmp(cmdtext, "/paintjobs", true))
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Paintjoby 0-2: Camper, Remington, Slamvan, Blade, Uranus, Jester, Sultan, Stratum, Elegy, Flash, Savanna, Broadway, Tornado.");
        SendClientMessage(playerid, COLOR_WHITE, "Uzycie: /paintjob [0-2] albo /paintjob off");
        return 1;
    }

    if (!strfind(cmdtext, "/paintjob ", true))
    {
        if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
            return SendClientMessage(playerid, COLOR_RED, "Musisz byc kierowca."), 1;
        new vehicleid = GetPlayerVehicleID(playerid);
        if (!SupportsRandomPaintjob(GetVehicleModel(vehicleid)))
            return SendClientMessage(playerid, COLOR_RED, "Ten model nie ma fabrycznych paintjobow GTA SA."), 1;

        new paintjob = -1;
        if (!strcmp(cmdtext, "/paintjob off", true))
        {
            ChangeVehiclePaintjob(vehicleid, 3);
        }
        else
        {
            paintjob = strval(cmdtext[10]);
            if (paintjob < 0 || paintjob > 2)
                return SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /paintjob [0-2] albo /paintjob off"), 1;
            ChangeVehiclePaintjob(vehicleid, paintjob);
        }
        if (vehicleid == g_V4OwnedVehicle[playerid])
        {
            g_V4OwnedPaintjob[playerid] = paintjob;
            V4_Save(playerid);
        }
        SendClientMessage(playerid, COLOR_GREEN, "Paintjob zastosowany i zapisany dla wlasnego pojazdu.");
        return 1;
    }

    if (!strcmp(cmdtext, "/bonus", true))
    {
        new now = gettime();
        new remaining = BONUS_COOLDOWN - (now - g_LastBonus[playerid]);
        if (remaining > 0)
        {
            new message[96];
            format(message, sizeof(message), "Kolejny bonus za %d sekund.", remaining);
            SendClientMessage(playerid, COLOR_YELLOW, message);
            return 1;
        }
        g_LastBonus[playerid] = now;
        AddMoney(playerid, BONUS_MONEY);
        AddXP(playerid, BONUS_XP);
        SaveAccount(playerid);
        SendClientMessage(playerid, COLOR_GREEN, "Bonus: +$1000 i +5 XP.");
        return 1;
    }

    if (!strcmp(cmdtext, "/kill", true))
    {
        SetPlayerHealth(playerid, 0.0);
        return 1;
    }

    if (!strcmp(cmdtext, "/molo", true) || !strcmp(cmdtext, "/pier", true))
        return TeleportPlayer(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z, NEMQO_START_A), 1;
    if (!strcmp(cmdtext, "/ls", true)) return TeleportPlayer(playerid, 2491.75, -1668.20, 13.34, 180.0), 1;
    if (!strcmp(cmdtext, "/sf", true)) return TeleportPlayer(playerid, -1985.75, 137.92, 27.69, 270.0), 1;
    if (!strcmp(cmdtext, "/lv", true)) return TeleportPlayer(playerid, 2027.30, 1008.60, 10.82, 90.0), 1;
    if (!strcmp(cmdtext, "/airport", true)) return TeleportPlayer(playerid, 1687.10, -2334.50, 13.55, 0.0), 1;
    if (!strcmp(cmdtext, "/chilliad", true)) return TeleportPlayer(playerid, -2329.70, -1624.40, 483.70, 0.0), 1;

    if (!strfind(cmdtext, "/car ", true))
    {
        if (g_InDM[playerid] || g_InSumo[playerid]) return SendClientMessage(playerid, COLOR_RED, "Pojazdy /car sa wylaczone na arenach."), 1;
        new model = strval(cmdtext[5]);
        if (model < 400 || model > 611)
        {
            SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /car [model 400-611]");
            return 1;
        }
        if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID) DestroyVehicle(g_PersonalVehicle[playerid]);
        new Float:x, Float:y, Float:z, Float:a;
        GetPlayerPos(playerid, x, y, z);
        GetPlayerFacingAngle(playerid, a);
        g_PersonalVehicle[playerid] = CreateVehicle(model, x + 3.0, y, z, a, random(126), random(126), 300);
        if (g_PersonalVehicle[playerid] == INVALID_VEHICLE_ID)
            return SendClientMessage(playerid, COLOR_RED, "Limit pojazdow osiagniety. Sprobuj ponownie pozniej."), 1;
        PutPlayerInVehicle(playerid, g_PersonalVehicle[playerid], 0);
        SendClientMessage(playerid, COLOR_GREEN, "Pojazd utworzony.");
        return 1;
    }

    if (!strfind(cmdtext, "/skin ", true))
    {
        new skin = strval(cmdtext[6]);
        if (skin < 0 || skin > 311 || skin == 74)
        {
            SendClientMessage(playerid, COLOR_YELLOW, "Uzycie: /skin [0-311]");
            return 1;
        }
        g_Skin[playerid] = skin;
        SetPlayerSkin(playerid, g_Skin[playerid]);
        SaveAccount(playerid);
        SendClientMessage(playerid, COLOR_GREEN, "Skin zmieniony i zapisany na koncie.");
        return 1;
    }

    if (!strcmp(cmdtext, "/skinselect", true))
    {
        if (g_InDM[playerid] || g_InSumo[playerid] || V4_IsInArena(playerid) || g_PSZHideJoined[playerid])
            return SendClientMessage(playerid, COLOR_RED, "Najpierw opusc arene."), 1;
        SaveAccount(playerid);
        g_Logged[playerid] = false;
        TogglePlayerSpectating(playerid, true);
        ShowSkinSelection(playerid);
        return 1;
    }

    if (V4_OnPlayerCommandText(playerid, cmdtext)) return 1;

    SendClientMessage(playerid, COLOR_GREY, "Nieznana komenda. Uzyj /help.");
    return 1;
}

stock ResetPlayerData(playerid)
{
    g_Logged[playerid] = false;
    g_AccountExists[playerid] = false;
    g_InDM[playerid] = false;
    g_InSumo[playerid] = false;
    g_SpeedVisible[playerid] = false;
    g_LoginAttempts[playerid] = 0;
    g_Money[playerid] = 0;
    g_Level[playerid] = 1;
    g_XP[playerid] = 0;
    g_Kills[playerid] = 0;
    g_Deaths[playerid] = 0;
    g_LastBonus[playerid] = 0;
    g_Skin[playerid] = 0;
    g_PersonalVehicle[playerid] = INVALID_VEHICLE_ID;
    g_SumoPlatform[playerid] = INVALID_OBJECT_ID;
    g_Name[playerid][0] = EOS;
    g_PasswordHash[playerid][0] = EOS;
    g_Salt[playerid][0] = EOS;
    g_SelectingSkin[playerid] = false;
    g_SelectedSkin[playerid] = 0;
    V4_Reset(playerid);
    PSZ_Reset(playerid);
}

stock bool:IsValidAccountName(const name[])
{
    new len = strlen(name);
    if (len < 3 || len > MAX_PLAYER_NAME) return false;
    for (new i = 0; i < len; i++)
    {
        if (!((name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= '0' && name[i] <= '9') ||
               name[i] == '_' || name[i] == '[' || name[i] == ']')) return false;
    }
    return true;
}

stock LoadAccount(playerid)
{
    new query[128];
    format(query, sizeof(query), "SELECT * FROM accounts WHERE name='%s' LIMIT 1", g_Name[playerid]);
    new DBResult:result = db_query(g_DB, query);
    if (db_num_rows(result) > 0)
    {
        g_Money[playerid] = db_get_field_assoc_int(result, "money");
        g_Level[playerid] = db_get_field_assoc_int(result, "level");
        g_XP[playerid] = db_get_field_assoc_int(result, "xp");
        g_Kills[playerid] = db_get_field_assoc_int(result, "kills");
        g_Deaths[playerid] = db_get_field_assoc_int(result, "deaths");
        g_LastBonus[playerid] = db_get_field_assoc_int(result, "last_bonus");
        g_Skin[playerid] = db_get_field_assoc_int(result, "skin");
    }
    db_free_result(result);
    V4_Load(playerid);
    PSZ_Load(playerid);
}

stock SaveAccount(playerid)
{
    if (!g_Logged[playerid] || !g_AccountExists[playerid]) return 0;
    new query[384];
    format(query, sizeof(query),
        "UPDATE accounts SET money=%d,level=%d,xp=%d,kills=%d,deaths=%d,last_bonus=%d,skin=%d WHERE name='%s'",
        g_Money[playerid], g_Level[playerid], g_XP[playerid], g_Kills[playerid],
        g_Deaths[playerid], g_LastBonus[playerid], g_Skin[playerid], g_Name[playerid]);
    new DBResult:result = db_query(g_DB, query);
    db_free_result(result);
    V4_Save(playerid);
    PSZ_Save(playerid);
    return 1;
}

stock FinishLogin(playerid)
{
    g_Logged[playerid] = true;
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, g_Money[playerid]);
    SetPlayerScore(playerid, g_Level[playerid]);
    TogglePlayerSpectating(playerid, false);
    SetSpawnInfo(playerid, 0, g_Skin[playerid], NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z, NEMQO_START_A, 24, 70, 29, 250, 0, 0);
    SpawnPlayer(playerid);
    PSZ_OnLogin(playerid);
    SendClientMessage(playerid, COLOR_GREEN, "Zalogowano. Wpisz /help, aby zobaczyc komendy.");
}

stock ShowSkinSelection(playerid)
{
    g_SelectingSkin[playerid] = true;
    g_SelectedSkin[playerid] = g_Skin[playerid];
    SetSpawnInfo(playerid, 0, g_Skin[playerid], 2491.75, -1668.20, 13.34, 180.0, 0, 0, 0, 0, 0, 0);
    ForceClassSelection(playerid);
    TogglePlayerSpectating(playerid, false);
    SendClientMessage(playerid, COLOR_YELLOW, "Wybierz postac strzalkami i zatwierdz przyciskiem SPAWN.");
    return 1;
}

stock RepairPlayerVehicle(playerid)
{
    if (!g_Logged[playerid]) return 0;
    if (!IsPlayerInAnyVehicle(playerid))
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Musisz znajdowac sie w pojezdzie.");
        return 0;
    }
    new vehicleid = GetPlayerVehicleID(playerid);
    RepairVehicle(vehicleid);
    SetVehicleHealth(vehicleid, 1000.0);
    SendClientMessage(playerid, COLOR_GREEN, "Pojazd naprawiony.");
    return 1;
}

stock bool:IsSafeTuningModel(model)
{
    switch (model)
    {
        case 400, 401, 404, 405, 410, 411, 412, 415, 418, 419, 420, 421,
             422, 426, 429, 436, 439, 445, 451, 458, 466, 467, 470, 474,
             475, 477, 478, 479, 480, 483, 489, 491, 492, 496, 500, 506, 507,
             516, 517, 518, 526, 527, 529, 533, 534, 535, 536, 540, 541,
             542, 545, 546, 547, 549, 550, 551, 555, 558, 559, 560, 561,
             562, 565, 566, 567, 575, 576, 579, 580, 585, 587, 589, 596,
             597, 598, 600, 602, 603: return true;
    }
    return false;
}

stock bool:SupportsRandomPaintjob(model)
{
    switch (model)
    {
        case 483, 534, 535, 536, 558, 559, 560, 561, 562, 565, 567, 575, 576: return true;
    }
    return false;
}

stock RandomTunePlayerVehicle(playerid)
{
    if (!g_Logged[playerid]) return 0;
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Musisz siedziec za kierownica samochodu.");
        return 0;
    }

    new vehicleid = GetPlayerVehicleID(playerid);
    new model = GetVehicleModel(vehicleid);
    if (!IsSafeTuningModel(model))
    {
        SendClientMessage(playerid, COLOR_RED, "Ten typ pojazdu nie obsluguje bezpiecznego tuningu SA-MP.");
        return 0;
    }

    new component = GetVehicleComponentInSlot(vehicleid, CARMODTYPE_WHEELS);
    if (component != 0) RemoveVehicleComponent(vehicleid, component);
    new wheel = g_TuningWheels[random(sizeof(g_TuningWheels))];
    AddVehicleComponent(vehicleid, wheel);

    new color1 = random(126), color2 = random(126), paintjob = -1;
    ChangeVehicleColor(vehicleid, color1, color2);
    if (SupportsRandomPaintjob(model))
    {
        paintjob = random(3);
        ChangeVehiclePaintjob(vehicleid, paintjob);
    }

    component = GetVehicleComponentInSlot(vehicleid, CARMODTYPE_HYDRAULICS);
    if (component != 0) RemoveVehicleComponent(vehicleid, component);
    AddVehicleComponent(vehicleid, 1087);

    component = GetVehicleComponentInSlot(vehicleid, CARMODTYPE_STEREO);
    if (component != 0) RemoveVehicleComponent(vehicleid, component);
    if (random(2)) AddVehicleComponent(vehicleid, 1086);

    // Nitro 10x jest obowiazkowe i zawsze zakladane na koncu tuningu.
    RemoveVehicleComponent(vehicleid, 1008);
    RemoveVehicleComponent(vehicleid, 1009);
    RemoveVehicleComponent(vehicleid, 1010);
    AddVehicleComponent(vehicleid, 1010);

    if (vehicleid == g_V4OwnedVehicle[playerid])
    {
        g_V4OwnedColor1[playerid] = color1;
        g_V4OwnedColor2[playerid] = color2;
        g_V4OwnedPaintjob[playerid] = paintjob;
        g_V4OwnedWheels[playerid] = wheel;
        V4_Save(playerid);
    }

    SendClientMessage(playerid, COLOR_GREEN, "Losowy tuning gotowy. Nitro 10x i hydraulika sa zawsze zalozone.");
    return 1;
}

stock AddMoney(playerid, amount)
{
    // A grant can reach CELL_MAX; later achievement rewards must not wrap the balance.
    if (amount > 0 && g_Money[playerid] > V4_CELL_MAX - amount)
        amount = V4_CELL_MAX - g_Money[playerid];
    g_Money[playerid] += amount;
    GivePlayerMoney(playerid, amount);
}

stock XPForNextLevel(playerid)
{
    return g_Level[playerid] * 100;
}

stock AddXP(playerid, amount)
{
    new level, xp;
    if (!V4_PlanXP(g_Level[playerid], g_XP[playerid], amount, level, xp)) return 0;
    new bool:promoted = level > g_Level[playerid];
    g_XP[playerid] = xp;
    g_Level[playerid] = level;
    if (promoted)
    {
        SetPlayerScore(playerid, g_Level[playerid]);
        new message[96];
        format(message, sizeof(message), "Awans! Osiagnales level %d.", g_Level[playerid]);
        SendClientMessage(playerid, COLOR_YELLOW, message);
    }
    return 1;
}

stock SpawnInDMArena(playerid)
{
    static const Float:spawns[][4] =
    {
        {1305.20, 2098.20, 11.02, 90.0},
        {1350.50, 2120.80, 11.02, 180.0},
        {1405.10, 2101.60, 11.02, 270.0},
        {1378.30, 2055.40, 11.02, 0.0},
        {1330.60, 2059.10, 11.02, 45.0}
    };
    new index = random(sizeof(spawns));
    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
    SetPlayerVirtualWorld(playerid, 10);
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, spawns[index][0], spawns[index][1], spawns[index][2]);
    SetPlayerFacingAngle(playerid, spawns[index][3]);
    ResetPlayerWeapons(playerid);
    GivePlayerWeapon(playerid, 24, 100);
    GivePlayerWeapon(playerid, 25, 100);
    GivePlayerWeapon(playerid, 31, 500);
    SetPlayerHealth(playerid, 100.0);
    SetPlayerArmour(playerid, 100.0);
    SetCameraBehindPlayer(playerid);
}

stock SpawnInSumoArena(playerid)
{
    static const Float:spawns[][4] =
    {
        {1965.0, -1435.0, 203.0, 45.0},
        {2035.0, -1435.0, 203.0, 135.0},
        {1965.0, -1365.0, 203.0, 315.0},
        {2035.0, -1365.0, 203.0, 225.0},
        {2000.0, -1442.0, 203.0, 180.0},
        {2000.0, -1358.0, 203.0, 0.0},
        {1958.0, -1400.0, 203.0, 90.0},
        {2042.0, -1400.0, 203.0, 270.0}
    };
    static const monsterModels[] = {444, 556, 557};

    if (g_SumoPlatform[playerid] == INVALID_OBJECT_ID)
    {
        g_SumoPlatform[playerid] = CreatePlayerObject(playerid, 18753,
            SUMO_X, SUMO_Y, SUMO_Z, 0.0, 0.0, 0.0);
    }

    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);
    if (g_PersonalVehicle[playerid] != INVALID_VEHICLE_ID)
    {
        DestroyVehicle(g_PersonalVehicle[playerid]);
        g_PersonalVehicle[playerid] = INVALID_VEHICLE_ID;
    }
    g_InSumo[playerid] = true;

    new spawnIndex = random(sizeof(spawns));
    SetPlayerVirtualWorld(playerid, SUMO_WORLD);
    SetPlayerInterior(playerid, 0);
    ResetPlayerWeapons(playerid);
    SetPlayerPos(playerid, spawns[spawnIndex][0], spawns[spawnIndex][1], spawns[spawnIndex][2] + 1.0);

    g_PersonalVehicle[playerid] = CreateVehicle(monsterModels[random(sizeof(monsterModels))],
        spawns[spawnIndex][0], spawns[spawnIndex][1], spawns[spawnIndex][2], spawns[spawnIndex][3],
        random(126), random(126), -1);
    if (g_PersonalVehicle[playerid] == INVALID_VEHICLE_ID)
    {
        g_InSumo[playerid] = false;
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerInterior(playerid, 0);
        SetPlayerPos(playerid, NEMQO_START_X, NEMQO_START_Y, NEMQO_START_Z);
        if (g_SumoPlatform[playerid] != INVALID_OBJECT_ID)
        {
            DestroyPlayerObject(playerid, g_SumoPlatform[playerid]);
            g_SumoPlatform[playerid] = INVALID_OBJECT_ID;
        }
        SendClientMessage(playerid, COLOR_RED, "Nie mozna utworzyc Monster Trucka: limit pojazdow.");
        return 0;
    }
    SetVehicleVirtualWorld(g_PersonalVehicle[playerid], SUMO_WORLD);
    SetVehicleHealth(g_PersonalVehicle[playerid], 2000.0);
    PutPlayerInVehicle(playerid, g_PersonalVehicle[playerid], 0);
    SetCameraBehindPlayer(playerid);
    return 1;
}

stock TeleportPlayer(playerid, Float:x, Float:y, Float:z, Float:angle)
{
    if (g_InDM[playerid] || g_InSumo[playerid] || V4_IsInArena(playerid))
    {
        SendClientMessage(playerid, COLOR_RED, "Najpierw opusc arene: /freeroam");
        return 0;
    }
    if (IsPlayerInAnyVehicle(playerid))
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        SetVehiclePos(vehicleid, x, y, z);
        SetVehicleZAngle(vehicleid, angle);
        LinkVehicleToInterior(vehicleid, 0);
        SetVehicleVirtualWorld(vehicleid, 0);
    }
    else
    {
        SetPlayerPos(playerid, x, y, z);
        SetPlayerFacingAngle(playerid, angle);
    }
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetCameraBehindPlayer(playerid);
    return 1;
}

public KickDelayed(playerid)
{
    if (IsPlayerConnected(playerid)) Kick(playerid);
    return 1;
}

#include <nemqo_v4>
#include <nemqo_psz>
#include <nemqo_island>
