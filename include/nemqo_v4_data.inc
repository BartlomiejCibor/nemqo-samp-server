// NEMQO v4 - deklaracje danych i stale
#define V4_DIALOG_MENU          2000
#define V4_DIALOG_JOBS          2001
#define V4_DIALOG_VEHICLE       2002
#define V4_DIALOG_GPS           2003
#define V4_DIALOG_ARENAS        2004
#define V4_DIALOG_RADIO         2005
#define V4_DIALOG_ACHIEVEMENTS  2006
#define V4_DIALOG_ADMIN         2007
#define V4_DIALOG_NEON          2008
#define V4_DIALOG_REFUEL        2009
#define V4_FUEL_STATION_COUNT   17

#define V4_VEHICLE_KIND_NONE     0
#define V4_VEHICLE_KIND_PUBLIC   1
#define V4_VEHICLE_KIND_TEMP     2
#define V4_VEHICLE_KIND_PRIVATE  3

#define V4_JOB_NONE       0
#define V4_JOB_TAXI       1
#define V4_JOB_COURIER    2
#define V4_JOB_TRUCKER    3
#define V4_JOB_MECHANIC   4
#define V4_JOB_POLICE     5
#define V4_JOB_MEDIC      6
#define V4_JOB_PILOT      7

#define V4_ARENA_NONE       0
#define V4_ARENA_DERBY      1
#define V4_ARENA_PAINTBALL  2
#define V4_ARENA_SNIPER     3
#define V4_ARENA_MINIGUN    4
#define V4_ARENA_ZOMBIE     5
#define V4_ARENA_STUNT      6
#define V4_ARENA_PARKOUR    7
#define V4_ARENA_GANG       8
#define V4_ARENA_DUEL       9

#define V4_ACH_FIRST_JOB   1
#define V4_ACH_RACE        2
#define V4_ACH_CAR         4
#define V4_ACH_HOUSE       8
#define V4_ACH_SPEED       16
#define V4_ACH_ARENA       32
#define V4_ACH_RICH        64

new g_V4Admin[MAX_PLAYERS];
new g_V4Bank[MAX_PLAYERS];
new g_V4Job[MAX_PLAYERS];
new g_V4JobXP[MAX_PLAYERS];
new g_V4Wanted[MAX_PLAYERS];
new g_V4RaceBest[MAX_PLAYERS];
new g_V4OwnedModel[MAX_PLAYERS];
new g_V4OwnedColor1[MAX_PLAYERS];
new g_V4OwnedColor2[MAX_PLAYERS];
new g_V4OwnedPaintjob[MAX_PLAYERS];
new g_V4OwnedWheels[MAX_PLAYERS];
new g_V4OwnedNeon[MAX_PLAYERS];
new g_V4Fuel[MAX_PLAYERS];
new g_V4Odometer[MAX_PLAYERS];
new g_V4House[MAX_PLAYERS];
new g_V4Business[MAX_PLAYERS];
new g_V4Achievements[MAX_PLAYERS];
new g_V4DailyDay[MAX_PLAYERS];
new g_V4DailyProgress[MAX_PLAYERS];
new g_V4JailSeconds[MAX_PLAYERS];
new g_V4Muted[MAX_PLAYERS];
new g_V4WorkActive[MAX_PLAYERS];
new g_V4RaceActive[MAX_PLAYERS];
new g_V4RaceCP[MAX_PLAYERS];
new g_V4RaceStart[MAX_PLAYERS];
new g_V4OwnedVehicle[MAX_PLAYERS];
new bool:g_V4VehicleLocked[MAX_PLAYERS];
new bool:g_V4Engine[MAX_PLAYERS];
new bool:g_V4Lights[MAX_PLAYERS];
new bool:g_V4Seatbelt[MAX_PLAYERS];
new g_V4Arena[MAX_PLAYERS];
new g_V4ArenaVehicle[MAX_PLAYERS];
new g_V4FuelTick[MAX_PLAYERS];
new g_V4FuelArea[MAX_PLAYERS];
new g_V4VehicleOwner[MAX_VEHICLES];
new Text3D:g_V4VehicleLabel[MAX_VEHICLES] = {Text3D:INVALID_3DTEXT_ID, ...};
new g_V4VehicleLabelWorld[MAX_VEHICLES];
new g_V4VehicleLabelOwner[MAX_VEHICLES];
new g_V4VehicleLabelDeciKm[MAX_VEHICLES];
new g_V4VehicleLabelFuel[MAX_VEHICLES];
new g_V4VehicleLabelKind[MAX_VEHICLES];
new g_V4VehicleKind[MAX_VEHICLES];
new g_V4PublicFuel[MAX_VEHICLES];
new g_V4ParkourObjects[MAX_PLAYERS][6];
new g_V4NeonObjects[MAX_PLAYERS][2];

new const g_V4NeonNames[][12] =
{
    "Wylaczone", "Czerwony", "Niebieski", "Zielony",
    "Zolty", "Rozowy", "Bialy"
};

new const Float:g_V4FuelStationPos[V4_FUEL_STATION_COUNT][3] =
{
    {1941.7, -1778.4, 13.4},
    {1004.0, -939.3, 42.2},
    {655.7, -565.3, 16.3},
    {1381.7, 459.2, 20.3},
    {-90.5, -1169.1, 2.4},
    {-1609.8, -2718.0, 48.5},
    {-1676.0, 414.0, 7.2},
    {-1327.4, 2677.4, 50.1},
    {609.0, 1702.4, 7.0},
    {1596.0, 2199.0, 10.8},
    {2147.8, 2748.0, 10.8},
    {-2029.5, 156.4, 28.9},
    {-2408.8, 976.1, 45.4},
    {-2244.0, -2560.6, 31.9},
    {2202.2, 2474.3, 10.5},
    {70.4, 1218.7, 18.5},
    {2113.7, 920.1, 10.5}
};
new g_V4FuelPickups[V4_FUEL_STATION_COUNT];
new Text3D:g_V4FuelLabels[V4_FUEL_STATION_COUNT];

new const g_V4JobNames[][18] =
{
    "Bezrobotny", "Taksowkarz", "Kurier", "Kierowca tira",
    "Mechanik", "Policjant", "Ratownik", "Pilot"
};

new const Float:g_V4RacePoints[][3] =
{
    {2491.7, -1668.2, 13.3},
    {2225.0, -1725.0, 13.2},
    {1815.0, -1860.0, 13.2},
    {1465.0, -1735.0, 13.2},
    {1180.0, -1405.0, 13.4},
    {1340.0, -970.0, 34.0},
    {1885.0, -1130.0, 24.0},
    {2491.7, -1668.2, 13.3}
};

new const Float:g_V4HousePos[][3] =
{
    {0.0, 0.0, 0.0},
    {2495.3, -1690.7, 14.7},
    {1298.5, -798.4, 84.1},
    {1527.2, -772.8, 80.6},
    {-2706.2, 865.1, 70.7},
    {1111.5, 1608.6, 12.5}
};
new const g_V4HousePrice[] = {0, 50000, 90000, 120000, 100000, 80000};

new const Float:g_V4BusinessPos[][3] =
{
    {0.0, 0.0, 0.0},
    {1833.5, -1842.5, 13.6},
    {-1912.4, 829.9, 35.2},
    {2194.8, 1677.1, 12.4}
};
new const g_V4BusinessPrice[] = {0, 75000, 85000, 100000};
