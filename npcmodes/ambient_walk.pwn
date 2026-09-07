// NEMQO native ambient walker NPC. No plugins or recording files required.
#include <a_npc>

#define WALK_INTERVAL 100
#define WALK_STEP     (0.14)
#define ROUTE_POINTS  4

new g_Route;
new g_TargetPoint = 1;
new g_WalkTimer;

new const Float:g_Routes[6][ROUTE_POINTS][3] =
{
    {{2490.0,-1670.0,13.34},{2450.0,-1670.0,13.34},{2450.0,-1710.0,13.34},{2490.0,-1710.0,13.34}},
    {{1480.0,-1750.0,13.20},{1520.0,-1750.0,13.20},{1520.0,-1710.0,13.20},{1480.0,-1710.0,13.20}},
    {{-1985.0,140.0,27.69},{-1940.0,140.0,27.69},{-1940.0,190.0,27.69},{-1985.0,190.0,27.69}},
    {{-1700.0,1300.0,7.00},{-1650.0,1300.0,7.00},{-1650.0,1345.0,7.00},{-1700.0,1345.0,7.00}},
    {{2030.0,1340.0,10.82},{2090.0,1340.0,10.82},{2090.0,1390.0,10.82},{2030.0,1390.0,10.82}},
    {{1680.0,1760.0,10.55},{1730.0,1760.0,10.55},{1730.0,1810.0,10.55},{1680.0,1810.0,10.55}}
};

forward WalkTick();

main()
{
    print("NEMQO ambient walker loaded");
}

public OnNPCConnect(myplayerid)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(myplayerid, name, sizeof(name));
    if (!strcmp(name,"Ped_LS_1",true)) g_Route=0;
    else if (!strcmp(name,"Ped_LS_2",true)) g_Route=1;
    else if (!strcmp(name,"Ped_SF_1",true)) g_Route=2;
    else if (!strcmp(name,"Ped_SF_2",true)) g_Route=3;
    else if (!strcmp(name,"Ped_LV_1",true)) g_Route=4;
    else g_Route=5;
    return 1;
}

public OnNPCSpawn()
{
    SetMyPos(g_Routes[g_Route][0][0],g_Routes[g_Route][0][1],g_Routes[g_Route][0][2]);
    g_TargetPoint=1;
    if (g_WalkTimer) KillTimer(g_WalkTimer);
    g_WalkTimer=SetTimer("WalkTick",WALK_INTERVAL,true);
    return 1;
}

public WalkTick()
{
    new Float:x,Float:y,Float:z;
    GetMyPos(x,y,z);
    new Float:dx=g_Routes[g_Route][g_TargetPoint][0]-x;
    new Float:dy=g_Routes[g_Route][g_TargetPoint][1]-y;
    new Float:dz=g_Routes[g_Route][g_TargetPoint][2]-z;
    new Float:distance=floatsqroot((dx*dx)+(dy*dy)+(dz*dz));
    if (distance<0.30)
    {
        g_TargetPoint++;
        if (g_TargetPoint>=ROUTE_POINTS) g_TargetPoint=0;
        return 1;
    }
    SetMyFacingAngle(-atan2(dx,dy));
    SetMyPos(x+((dx/distance)*WALK_STEP),y+((dy/distance)*WALK_STEP),z+((dz/distance)*WALK_STEP));
    return 1;
}

public OnNPCDisconnect(reason[])
{
    #pragma unused reason
    if (g_WalkTimer) KillTimer(g_WalkTimer);
    return 1;
}
