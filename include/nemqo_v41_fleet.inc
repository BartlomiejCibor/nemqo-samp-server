// NEMQO v4.1 - rozbudowana flota parkingowa i lotnicza.
// 171 nowych pojazdow parkingowych + 20 driftowozow na Placu Zabaw
// + 13 samolotow = 204 nowe instancje.

new g_V41GroundVehiclesCreated;
new g_V41AircraftCreated;
new g_V41PierElegy[3] = {INVALID_VEHICLE_ID, INVALID_VEHICLE_ID, INVALID_VEHICLE_ID};

// Trzy wystawowe Elegy na duzym molo Santa Maria Beach: x, y, z, kat.
new const Float:g_V41PierElegySpawn[3][4] =
{
    {358.0, -2018.0, 7.58, 180.0},
    {370.0, -2018.0, 7.58, 180.0},
    {382.0, -2018.0, 7.58, 180.0}
};

new const g_V41ParkingModels[] =
{
    400, 401, 402, 404, 405, 410, 411, 412, 415, 418,
    419, 421, 422, 426, 429, 436, 439, 445, 451, 458,
    461, 462, 463, 466, 467, 474, 475, 477, 478, 479,
    480, 482, 489, 491, 492, 496, 500, 506, 507, 516,
    517, 518, 526, 527, 529, 533, 534, 535, 536, 540,
    541, 542, 545, 546, 547, 549, 550, 551, 555, 558,
    559, 560, 561, 562, 565, 566, 567, 575, 576, 579,
    580, 585, 587, 589, 596, 597, 598, 600, 602, 603
};

// x, y, z, kat, wektor kolumny x/y, wektor rzedu x/y.
new const Float:g_V41ParkingHubs[][8] =
{
    // Los Santos: City Hall, szpital, stadion, parking terminala LS Airport.
    {1435.0, -1770.0, 13.20,  90.0, 0.0, 5.2, 4.5, 0.0},
    {1135.0, -1360.0, 13.35, 270.0, 0.0, 5.2, 4.5, 0.0},
    {2680.0, -1692.0,  9.05,   0.0, 4.5, 0.0, 0.0, 6.0},
    {1652.0, -2296.0, 13.15, 180.0, 4.5, 0.0, 0.0, 6.0},

    // San Fierro: Doherty, Pier 69, terminal lotniska, szpital.
    {-1995.0,  250.0, 35.00,  90.0, 0.0, 5.2, 4.5, 0.0},
    {-1750.0, 1300.0,  7.00,   0.0, 4.5, 0.0, 0.0, 6.0},
    {-1360.0, -130.0, 14.00, 315.0, 3.7, 3.7, -3.7, 3.7},
    {-2675.0,  620.0, 14.20, 180.0, 4.5, 0.0, 0.0, 6.0},

    // Las Venturas: kasyna, parking terminala, komisariat, stadion.
    {2020.0, 1285.0, 10.40,   0.0, 4.5, 0.0, 0.0, 6.0},
    {1740.0, 1785.0, 10.55, 270.0, 0.0, 5.2, 4.5, 0.0},
    {2250.0, 2440.0, 10.50, 180.0, 4.5, 0.0, 0.0, 6.0},
    {1360.0, 1160.0, 10.85,   0.0, 4.5, 0.0, 0.0, 6.0}
};

stock V41_CreateExpandedFleet()
{
    g_V41GroundVehiclesCreated = 0;
    g_V41AircraftCreated = 0;
    new configuredGround = 0;

    for (new hub = 0; hub < sizeof(g_V41ParkingHubs); hub++)
    {
        for (new row = 0; row < 4; row++)
        {
            for (new column = 0; column < 4; column++)
            {
                if (configuredGround >= 168) break;

                new Float:x = g_V41ParkingHubs[hub][0]
                    + (float(column) * g_V41ParkingHubs[hub][4])
                    + (float(row) * g_V41ParkingHubs[hub][6]);
                new Float:y = g_V41ParkingHubs[hub][1]
                    + (float(column) * g_V41ParkingHubs[hub][5])
                    + (float(row) * g_V41ParkingHubs[hub][7]);
                new model = g_V41ParkingModels[configuredGround % sizeof(g_V41ParkingModels)];
                new vehicleid = AddStaticVehicleEx(model, x, y, g_V41ParkingHubs[hub][2],
                    g_V41ParkingHubs[hub][3], (configuredGround * 17) % 126,
                    (configuredGround * 29 + 7) % 126, 300);
                if (vehicleid != INVALID_VEHICLE_ID) g_V41GroundVehiclesCreated++;
                configuredGround++;
            }
            if (configuredGround >= 168) break;
        }
        if (configuredGround >= 168) break;
    }

    // Stockowe paintjoby Elegy 0/1/2. Zastepuja trzy auta parkingowe,
    // dlatego laczna flota naziemna pozostaje bez zmian: 171 nowych sztuk.
    for (new i = 0; i < 3; i++)
    {
        g_V41PierElegy[i] = AddStaticVehicleEx(562,
            g_V41PierElegySpawn[i][0], g_V41PierElegySpawn[i][1],
            g_V41PierElegySpawn[i][2], g_V41PierElegySpawn[i][3],
            1, 1, 300);
        if (g_V41PierElegy[i] != INVALID_VEHICLE_ID)
        {
            ChangeVehiclePaintjob(g_V41PierElegy[i], i);
            g_V41GroundVehiclesCreated++;
        }
        configuredGround++;
    }

    // Nowe samoloty stojace przy hangarach i na apronach, nie na pasach startowych.
    if (AddStaticVehicleEx(593, 1990.0, -2350.0, 14.0,  90.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(512, 1945.0, -2350.0, 14.0,  90.0, 39, 39, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(511, 1895.0, -2350.0, 14.5,  90.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(476, 1840.0, -2350.0, 14.6,  90.0, 7, 6, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;

    if (AddStaticVehicleEx(593, -1380.0, -185.0, 14.1, 315.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(512, -1420.0, -220.0, 14.2, 315.0, 39, 39, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(476, -1460.0, -255.0, 14.8, 315.0, 7, 6, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(511, -1500.0, -290.0, 14.8, 315.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;

    if (AddStaticVehicleEx(593, 1660.0, 1160.0, 11.2, 270.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(512, 1705.0, 1160.0, 11.4, 270.0, 39, 39, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(476, 1755.0, 1160.0, 11.7, 270.0, 7, 6, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(511, 1810.0, 1160.0, 12.0, 270.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;
    if (AddStaticVehicleEx(519, 1870.0, 1160.0, 12.4, 270.0, 1, 1, 600) != INVALID_VEHICLE_ID) g_V41AircraftCreated++;

    printf("NEMQO v4.1 fleet: added %d/171 parking, 20 playground and %d/13 aircraft; configured static total 306, aircraft total 26",
        g_V41GroundVehiclesCreated, g_V41AircraftCreated);
    return g_V41GroundVehiclesCreated + g_V41AircraftCreated;
}

stock V41_OnVehicleSpawn(vehicleid)
{
    for (new i = 0; i < 3; i++)
    {
        if (vehicleid != g_V41PierElegy[i]) continue;
        ChangeVehiclePaintjob(vehicleid, i);
        return 1;
    }
    return 0;
}
