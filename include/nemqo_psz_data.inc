// NEMQO PSZ Tribute - dane i stale. Legacy SA-MP 0.3.7-R2.
#define DIALOG_REMEMBER_SKIN       1005
#define PSZ_DIALOG_PLAY            3000
#define PSZ_DIALOG_DRIFT_ZONES     3001
#define PSZ_DIALOG_ANIMS           3002
#define PSZ_DIALOG_EVENT           3003

#define PSZ_WORLD_HIDE_BASE        2000
#define PSZ_WORLD_PLAYGROUND       2100
#define PSZ_HIDE_IDLE              0
#define PSZ_HIDE_SIGNUP            1
#define PSZ_HIDE_HIDING            2
#define PSZ_HIDE_SEEKING           3
#define PSZ_HIDE_MAPS              3
#define PSZ_HIDE_POINTS            10
#define PSZ_RAMP_LIFETIME_MS       12000
#define PSZ_RAMP_COOLDOWN_MS       5000
#define PSZ_PLAYGROUND_OBJECTS     32
#define PSZ_PLAYGROUND_VEHICLES    20
#define PSZ_EVENT_NONE             0
#define PSZ_EVENT_HIDE             1
#define PSZ_EVENT_DRIFT            2
#define PSZ_EVENT_DERBY            3
#define PSZ_EVENT_RACE             4
#define PSZ_EVENT_SUMO             5

new bool:g_RememberSkin[MAX_PLAYERS];
new g_PSZRespect[MAX_PLAYERS];
new g_PSZDriftTotal[MAX_PLAYERS];
new g_PSZDriftBest[MAX_PLAYERS];
new g_PSZHideWins[MAX_PLAYERS];
new g_PSZEventWins[MAX_PLAYERS];

new bool:g_PSZDriftMode[MAX_PLAYERS];
new g_PSZDriftCombo[MAX_PLAYERS];
new g_PSZDriftTicks[MAX_PLAYERS];
new g_PSZTandemPartner[MAX_PLAYERS];
new PlayerText:g_PSZDriftText[MAX_PLAYERS];
new bool:g_PSZDriftTextVisible[MAX_PLAYERS];

new g_PSZRampObject[MAX_PLAYERS];
new g_PSZRampLastTick[MAX_PLAYERS];

new bool:g_PSZPanelEnabled[MAX_PLAYERS];
new PlayerText:g_PSZPanelBackground[MAX_PLAYERS];
new PlayerText:g_PSZPanelText[MAX_PLAYERS];
new bool:g_PSZPanelVisible[MAX_PLAYERS];

new bool:g_PSZHideJoined[MAX_PLAYERS];
new bool:g_PSZHideSeeker[MAX_PLAYERS];
new bool:g_PSZHideFound[MAX_PLAYERS];
new g_PSZHideOutsideTicks[MAX_PLAYERS];
new bool:g_PSZHideSnapshotValid[MAX_PLAYERS];
new g_PSZHideReturnWorld[MAX_PLAYERS];
new g_PSZHideReturnInterior[MAX_PLAYERS];
new g_PSZHideReturnSkin[MAX_PLAYERS];
new g_PSZHideReturnColor[MAX_PLAYERS];
new Float:g_PSZHideReturnX[MAX_PLAYERS];
new Float:g_PSZHideReturnY[MAX_PLAYERS];
new Float:g_PSZHideReturnZ[MAX_PLAYERS];
new Float:g_PSZHideReturnA[MAX_PLAYERS];
new Float:g_PSZHideReturnHealth[MAX_PLAYERS];
new Float:g_PSZHideReturnArmour[MAX_PLAYERS];
new g_PSZHideReturnWeapon[MAX_PLAYERS][13];
new g_PSZHideReturnAmmo[MAX_PLAYERS][13];
new g_PSZHidePhase;
new g_PSZHideSeconds;
new g_PSZHideMap;

new g_PSZCurrentEvent;
new g_PSZEventSeconds;

new g_PSZPlaygroundObjects[PSZ_PLAYGROUND_OBJECTS];
new g_PSZPlaygroundVehicles[PSZ_PLAYGROUND_VEHICLES];

new const g_PSZRankNames[][20] =
{
    "Nowy", "Kierowca", "Drifter", "Street Racer", "Drift King", "Legenda NEMQO"
};
new const g_PSZRankLimits[] = {0, 250, 1000, 3000, 7500, 15000};

new const Float:g_PSZDriftZones[][4] =
{
    {2445.0, -2110.0, 13.55, 180.0},
    {-1650.0, -160.0, 14.15, 315.0},
    {2050.0, 1280.0, 10.82, 90.0},
    {-2329.7, -1624.4, 483.7, 0.0},
    {400.0, 2500.0, 16.5, 90.0}
};
new const g_PSZDriftZoneNames[][28] =
{
    "Doki Los Santos", "Lotnisko San Fierro", "Parking Las Venturas",
    "Mount Chilliad", "Plac Zabaw"
};

new const Float:g_PSZHideSeekerSpawn[PSZ_HIDE_MAPS][4] =
{
    {2490.0, -1668.0, 13.34, 180.0},
    {-1700.0, 1300.0, 7.00, 90.0},
    {2030.0, 1340.0, 10.82, 90.0}
};
new const Float:g_PSZHideSpawns[PSZ_HIDE_MAPS][PSZ_HIDE_POINTS][4] =
{
    {
        {2455.0,-1655.0,13.3,90.0},{2510.0,-1690.0,13.3,180.0},
        {2470.0,-1735.0,13.3,0.0},{2425.0,-1715.0,13.3,45.0},
        {2525.0,-1650.0,13.3,225.0},{2440.0,-1615.0,13.3,90.0},
        {2550.0,-1710.0,13.3,270.0},{2390.0,-1685.0,13.3,0.0},
        {2500.0,-1760.0,13.3,180.0},{2475.0,-1585.0,13.3,90.0}
    },
    {
        {-1650.0,1285.0,7.0,90.0},{-1735.0,1320.0,7.0,270.0},
        {-1680.0,1360.0,7.0,180.0},{-1760.0,1260.0,7.0,0.0},
        {-1605.0,1325.0,7.0,90.0},{-1710.0,1390.0,7.0,180.0},
        {-1790.0,1345.0,7.0,270.0},{-1630.0,1240.0,7.0,0.0},
        {-1745.0,1410.0,7.0,180.0},{-1580.0,1270.0,7.0,90.0}
    },
    {
        {2070.0,1320.0,10.8,90.0},{1980.0,1360.0,10.8,270.0},
        {2020.0,1430.0,10.8,180.0},{2110.0,1385.0,10.8,90.0},
        {1950.0,1290.0,10.8,0.0},{2160.0,1330.0,10.8,180.0},
        {2050.0,1480.0,10.8,270.0},{1910.0,1400.0,10.8,0.0},
        {2130.0,1460.0,10.8,180.0},{1990.0,1240.0,10.8,90.0}
    }
};
