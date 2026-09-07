# NEMQO Drift & Fun | PSZ Tribute

A complete Polish-language SA-MP 0.3.7-R2 gamemode combining instant freeroam, drift, minigames, vehicle culture and lightweight RPG progression. The project is written in legacy-compatible Pawn and runs without third-party C++ plugins or filterscripts.

> Original test deployment: `10.0.30.123:7777` (private LAN address, VLAN 30).

## What the server offers

### Fast freeroam

- Santa Maria Beach Pier as the default spawn.
- Teleports to Los Santos, San Fierro, Las Venturas, airports, Mount Chiliad, the playground and Grove Island.
- A large static fleet with public-service, civilian, tuned, off-road, sea and aircraft models.
- Player-owned vehicles with fuel, mileage, paintjobs, wheels, neon, nitro and hydraulics.
- Attached 3D labels with model information and persistent odometers.

### Drift and PSZ-style activities

- Drift scoring based on speed and the angle between vehicle heading and velocity.
- Combo multiplier up to x5.
- Persistent SQLite drift totals, personal records and TOP 10.
- Tandem bonus for nearby paired drivers.
- Player-private ramp spawned by the horn action or `/rampa`.
- Activity occupancy panel and `/zabawy` quick menu.
- Scheduled events and manual event administration.

### Minigames and arenas

- Classic Deathmatch.
- Monster Truck Sumo with automatic cleanup after falling or leaving the vehicle.
- Derby, Paintball, Sniper, Minigun and Zombie Survival.
- Motorcycle stunt arena and sky parkour.
- Grove Street vs Ballas.
- One-on-one duels.
- Hide and Seek with three maps, isolated virtual worlds, seeker selection, rewards and full state restoration.
- Mode isolation prevents players from remaining in two activities at once.

### Custom game areas

- Desert playground with drift vehicles, jumps, tunnels, wallrides and stunt elements.
- Grove Island v3 with native GTA/SA-MP objects, houses, interiors, vegetation, marina, off-road vehicles and boats.
- Three Elegy vehicles at the pier with stock paintjobs 0, 1 and 2.

### Lightweight RPG

- Accounts secured with salted hashes.
- Money, bank, level, XP, kills, deaths and bonuses.
- Seven jobs: taxi driver, courier, trucker, mechanic, police officer, medic and pilot.
- Houses, businesses, hourly income and daily objectives.
- Wanted, tickets, arrests and jail.
- Achievements and persistent respect ranks.
- Reports, bans and administrator audit logs.

### User interface and population

- Minimal speedometer with speed, fuel, vehicle health, model and mileage.
- Left-side activity panel with live player counts.
- Drift combo display.
- 24 animated invulnerable actors.
- Six native `samp-npc` ambient walkers in LS, SF and LV.

## Important commands

| Area | Commands |
|---|---|
| Main | `/menu`, `/help`, `/stats`, `/bonus`, `/daily`, `/achievements` |
| Activities | `/zabawy`, `/panel`, `/event`, `/arena`, `/leave`, `/freeroam` |
| Drift | `/drift`, `/drift off`, `/driftstats`, `/topdrift`, `/tandem [id]` |
| Ramp | vehicle horn action or `/rampa` |
| Hide and Seek | `/chowany`, `/leavehide` |
| Areas | `/molo`, `/wyspa`, `/plac`, `/ls`, `/sf`, `/lv`, `/airport`, `/chilliad` |
| Arenas | `/dm`, `/sumo`, `/derby`, `/paintball`, `/sniper`, `/minigun`, `/zombie`, `/stunt`, `/parkour`, `/gang` |
| Race | `/race`, `/race quit` |
| Vehicles | `/car [400-611]`, `/buycar [400-611]`, `/v`, `/engine`, `/lights`, `/seatbelt`, `/repair`, `/refuel`, `/tuning` |
| Paintjobs | `/paintjobs`, `/paintjob [0-2]`, `/paintjob off` |
| Character | `/skinselect`, `/skin [0-311]`, `/rememberskin` |
| Economy | `/jobs`, `/work`, `/stopwork`, `/balance`, `/deposit`, `/withdraw`, `/house`, `/business` |
| Communication | `/pm`, `/global`, `/radio`, `/report` |

The in-game text and commands intentionally remain Polish because this is a Polish community server.

## Technical profile

- **Runtime:** SA-MP Server 0.3.7-R2 for Linux.
- **Language:** Pawn, compiled with `3.2.3664.samp` compatibility toolchain.
- **Database:** native SA-MP SQLite (`scriptfiles/accounts.db`).
- **Plugins:** none.
- **Filterscripts:** none.
- **Default port:** UDP 7777.
- **NPC slots:** 6.
- **Recommended minimum:** 1 vCPU, 512 MB RAM, 2 GB free disk.
- **Verified OS:** Debian 12 Bookworm x86-64 with 32-bit runtime libraries.

## Repository contents

```text
gamemodes/                 Pawn gamemode source
include/                   project-owned Pawn modules
npcmodes/                  ambient NPC source
config/                    sanitized SA-MP and systemd templates
database/                  reproducible SQL schema and clean SQLite database
server-files/              ready overlay for an official SA-MP runtime
scripts/build.sh            reproducible Pawn build
scripts/create-clean-db.py  creates a new database without production data
scripts/install.sh          installs the ready overlay safely
scripts/verify.sh           service, database, log and UDP checks
packages-debian.txt         exact Debian package list
Makefile                    build/database/syntax helper targets
CHECKSUMS.sha256            integrity list for distributable artifacts
INSTALLATION.md             complete installation manual
```

## Start here

Follow **[INSTALLATION.md](INSTALLATION.md)**. It contains both an automated path and a fully manual path.

The repository does not include proprietary official SA-MP server executables or the official SDK includes. Obtain a legitimate SA-MP 0.3.7-R2 Linux server package separately, then apply `server-files/` or run `scripts/install.sh`.

## Verified release checks

The exported package is accepted only when:

- the gamemode and NPC source compile with no errors or warnings;
- a clean runtime starts without plugins;
- SQLite `PRAGMA integrity_check` returns `ok`;
- startup self-tests pass;
- all six NPCs connect and move;
- the server answers the native SA-MP UDP information query;
- the source export contains no RCON password, SSH private key, player account or production log.

## Security and clean-state guarantees

- `server.cfg.example` contains a placeholder, never the production RCON password.
- `accounts.clean.db` and the ready `accounts.db` are built from `schema.sql`, not copied from production.
- Account, report, ban and admin-log tables are empty.
- House and business catalogue rows contain no owners.
- No SSH keys, tokens, `.env` files, logs or backups are included.
- The private LAN address `10.0.30.123` is retained only as documentation of the original deployment; it is not an authentication secret.

## Third-party components

SA-MP server binaries, standard SA-MP includes and the legacy Pawn compiler are third-party components. Their names and required versions are documented for reproducibility, but redistributing them is outside this source package. Use copies obtained under their respective terms.
