# NEMQO Drift & Zabawa | PSZ Tribute

Polski gamemode SA-MP 0.3.7-R2 łączący szybki freeroam, drift, minigry i lekkie RPG. Projekt jest napisany w Pawn i nie wymaga pluginów C++ ani filterscriptów.

## Najważniejsze funkcje

- drift z combo, tandemami i trwałym rankingiem,
- panel zabaw, Chowany, eventy, Derby, Sumo, DM i pozostałe areny,
- Plac Zabaw oraz Grove Island v3,
- 312 statycznych pojazdów i 112 modeli,
- prywatne pojazdy z tuningiem, neonami, paliwem i trwałym przebiegiem,
- publiczne pojazdy z runtime przebiegiem i paliwem,
- etykiety 3D pokazujące spawn-ID modelu GTA; dane właściciela występują tylko na autach prywatnych,
- 17 oznaczonych stacji paliw,
- SQLite dla kont i progresji,
- wbudowane testy bezpieczeństwa, storage i kompletności świata.

Pełny opis systemów i komend znajduje się w [README_SERVER.md](README_SERVER.md).

## Wymagania

- SA-MP Server `0.3.7-R2`,
- kompatybilny kompilator Pawn `3.2.3664.samp`,
- oficjalne include SA-MP (`a_samp.inc` i zależności),
- Linux lub inne środowisko zdolne uruchomić serwer SA-MP.

Standardowe include SA-MP nie są częścią kodu projektu. Należy pobrać je z własnego, legalnego pakietu serwera/SDK.

## Struktura repozytorium

```text
gamemodes/
  nemqo_freeroam_dm.pwn
include/
  nemqo_v4*.inc
  nemqo_psz*.inc
  nemqo_v41*.inc
  nemqo_island.inc
  samp_empty_prefix.inc
README.md
README_SERVER.md
.gitignore
```

## Budowanie

Umieść oficjalne include SA-MP w katalogu używanym przez kompilator. Dla legacy Pawn wymagany jest pusty prefix:

```bash
cp include/samp_empty_prefix.inc samp_empty_prefix.inc
LD_LIBRARY_PATH=/sciezka/do/compiler-compat \
  /sciezka/do/compiler-compat/pawncc gamemodes/nemqo_freeroam_dm.pwn \
  -iinclude -psamp_empty_prefix.inc \
  -ogamemodes/nemqo_freeroam_dm.amx
```

Kompilację należy odrzucić przy dowolnym błędzie lub ostrzeżeniu. Plik AMX jest artefaktem buildu: nie powinien trafiać do historii Git, ale może zostać dołączony do GitHub Release.

## Uruchomienie

1. Skompiluj `gamemodes/nemqo_freeroam_dm.pwn`.
2. Skopiuj AMX oraz wymagane moduły do własnego katalogu serwera SA-MP.
3. Utwórz lokalny `server.cfg` z własnym hasłem RCON.
4. Nie dodawaj `server.cfg`, bazy kont ani logów do Git.
5. Uruchom najpierw osobny staging i sprawdź logi oraz UDP query.

## Zweryfikowany stan

```text
Fuel markers: 17/17
Vehicle labels: 312/312
Unclassified vehicles: 0
Safety self-test: 126/126
Storage self-test: 12/12
Ambient NPC movement: 6/6
SQLite integrity_check: ok
```

## Bezpieczeństwo publikacji

Repozytorium zawiera wyłącznie kod źródłowy. Nie publikuj:

- `server.cfg` i hasła RCON,
- `scriptfiles/accounts.db`, plików WAL/SHM ani innych baz,
- `.env`, tokenów, kluczy SSH i danych dostępowych,
- logów, crash dumpów, PID-ów,
- backupów, release'ów produkcyjnych i prywatnych skryptów wdrożeniowych.

Reguły ochronne są zapisane w [.gitignore](.gitignore).
