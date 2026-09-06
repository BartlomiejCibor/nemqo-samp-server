# NEMQO Drift & Zabawa | PSZ Tribute v4.1

## 1. Czym jest ten serwer

**NEMQO Drift & Zabawa | PSZ Tribute** to polski serwer GTA San Andreas Multiplayer oparty na SA-MP 0.3.7-R2. Odtwarza tempo i społeczną atmosferę dawnych serwerów typu PSZ-DM: natychmiastowy drift, panel zabaw, Chowany, Derby, wyścigi, animacje, wspólne ustawianie aut i eventy. Lekki RPG oraz ekonomia pozostają dodatkiem, a nie barierą wejścia.

Najważniejsza zasada rozgrywki: gracz ma wejść do zabawy w kilka sekund. Może driftować, wyskoczyć z rampy, wejść do Chowanego lub eventu, a dopiero później korzystać z pracy, domu, firmy i ekonomii.

Domyślnym miejscem startowym jest duże molo **Santa Maria Beach Pier** w Los Santos. Przy spawnie stoją trzy Elegy z fabrycznymi paintjobami GTA SA `0`, `1` i `2`.

- **Adres:** ustawiany przez operatora, standardowy port UDP `7777`
- **Nazwa:** `[PL] NEMQO Drift & Zabawa | PSZ Tribute`
- **Pojemność:** 32 ludzi + 6 natywnych NPC (`maxplayers 38`, `maxnpc 6`)
- **Język:** polski
- **Silnik:** SA-MP 0.3.7-R2
- **Dane graczy:** SQLite
- **Sieć:** VLAN 30

## 2. Rdzeń PSZ Tribute

### Panel „Zabawy”

Po zalogowaniu po lewej stronie wyświetla się panel z rzeczywistą liczbą zalogowanych ludzi w trybach: Drift, Derby, Race, Strzelnica, Chowany, Sumo, DM i Plac Zabaw. NPC nie zwiększają liczników. `/panel` ukrywa lub pokazuje panel, a `/zabawy` otwiera menu natychmiastowego wejścia.

### Drift, combo i tandem

- `/drift` otwiera pięć stref: Doki LS, lotnisko SF, parking LV, Mount Chilliad i Plac Zabaw.
- Punkty zależą od prędkości i różnicy między kierunkiem pojazdu a kierunkiem jego ruchu.
- Naliczanie wymaga minimum 35 km/h, kąta 14–88 stopni i kontaktu auta z nawierzchnią.
- Mnożnik combo rośnie do `x5`.
- `/tandem [id]` daje obu kierowcom +50% punktów podczas jazdy w odległości do 18 m.
- `/topdrift` pokazuje trwały ranking SQLite.
- `/driftstats` pokazuje rekord, sumę punktów, respekt i rangę.

### Przenośna rampa

Kierowca naciska akcję `KEY_CROUCH`, czyli w pojeździe domyślnie **klakson H/Caps Lock**. Serwer tworzy przed jego autem prywatny `PlayerObject` rampy model 1632. Rampa:

- pojawia się 8,5 m przed autem,
- istnieje 12 sekund,
- ma 5 sekund cooldownu,
- jest widoczna i kolizyjna tylko dla właściciela,
- jest niszczona przy utworzeniu następnej oraz po rozłączeniu.

Fallback: `/rampa`. Nie jest to surowy fizyczny klawisz H — SA-MP przekazuje akcję klaksonu, więc gracz może zmienić jej bind w ustawieniach GTA SA.

### Chowany

- `/chowany` uruchamia lub dołącza do 30-sekundowych zapisów.
- Minimum: 2 osoby.
- Serwer losuje jednego szukającego i jedną z trzech map: LS, SF albo LV.
- Światy są izolowane: `2000–2002`.
- Chowający mają 20 sekund na ukrycie; runda trwa 180 sekund.
- Dotknięcie z odległości do 2,5 m oznacza znalezienie.
- Opuszczenie obszaru przez 3 kolejne sekundy powoduje eliminację lub walkower.
- Szukający dostaje 150 respektu za znalezienie wszystkich.
- Ocaleni chowający dostają po 100 respektu i trwałe zwycięstwo.
- Przed rundą zapisywany jest świat, interior, pozycja, kąt, skin, kolor, HP, pancerz i 13 slotów broni; po rundzie stan jest odtwarzany.
- `/leavehide` opuszcza zapisy lub aktywną rundę z pełnym cleanupem.

### Plac Zabaw

`/plac` lub `/playground` teleportuje na pustynny kompleks około `400, 2500`:

- 32 obiekty,
- 20 driftowozów,
- parking i proste do ustawiania aut,
- małe i duże rampy,
- tunele/pętle,
- wallride,
- bariery i strefy skoków.

### Wyspa NEMQO — Grove Island v3

`/wyspa` / `/island`: osiedle wokół `370, -2600`, z ulicą i zaokrąglonym placem do zawracania inspirowanym Grove Street.

- 248 obiektów, 9 domów i 9 dokładnie spasowanych wnętrz, 5 modeli zewnętrznych: 19499, 19505, 19507, 19509, 19511.
- Każdy dom ma sofę, stolik i szafkę RTV (27 mebli). Wnętrza stoją na identycznym XYZ i obrocie jak ich skorupy; podłogi i lokalizacje mebli sprawdzono na rzeczywistych COL.
- 26 palm, 11 krzewów, 35 skał, 8 latarni i 12 natywnych pochodni tiki 3461; ławki, stoliki z parasolami i leżaki.
- Zatwierdzona geometria brzegu/podłoża Z=1.4, pomost i wieża 3279 zostały zachowane. Nie ma mostu na kontynent ani skał 17031.
- Spawn na wolnej ulicy: `362, -2600, 2.7`. Dojście do pomostu omija domy, a zaparkowane pojazdy nie blokują schodów.
- 4 pojazdy terenowe i 2 łodzie, VW 0, cleanup aktywności i teleport kierowcy wraz z pojazdem. Powrót: `/molo`.

Walidacja: legacy Pawn 3.2.3664.samp, runtime liczby/pozycji/rotacji i par skorupa–wnętrze; audyt AABB i 108 testów narożników mebli nad kolizyjną podłogą. **Wygląd, nocne efekty i przejście przez drzwi wymagają końcowego testu klientem.**

### Przebieg i granty administratora (v3)

Przebieg prywatnego kupionego pojazdu jest liczony z kolejnych pozycji podczas synchronizacji kierowcy (minimum 100 ms), nie z zaokrąglonej prędkości co sekundę. Zachowywane są ułamki metra w `odometer_fraction`, z zapisem co 30 sekund i przy standardowym zapisie/wylogowaniu. Teleport skryptowy, respawn, zmiana auta/kierowcy/świata/interioru i przerwa synchronizacji ponad 1500 ms resetują próbkę, nie przebieg. HUD pokazuje przebieg danego auta również pasażerowi, nie przebieg konta w dowolnym samochodzie.

- `/givemoney [id] [1-1000000]` — dodatni grant gotówki.
- `/givexp [id] [1-100000]` — dodatni grant XP, z normalnymi awansami.
- Wymagane zalogowane konto administratora poziomu 1+ lub zalogowane konto z RCON. Cel musi być zalogowanym człowiekiem. Nikt nie otrzymuje nowej rangi admina przez wdrożenie.
- Ścisłe liczby dziesiętne, kontrola overflow i limitów, cooldown 2 sekundy. Zapis konta i `admin_logs` w jednej transakcji przed zmianą stanu w grze. `/adminhelp` pokazuje składnię.
- 112 deterministycznych testów parsera/uprawnień/XP/dystansu. Dodatkowy test staging (`grove_storage_test.enable`) używa wyłącznie osobnego fixture DB: 12 testów transakcji, rollbacku oraz zapisu–zamknięcia–otwarcia–odczytu przebiegu. Marker nie jest instalowany na produkcji.

### Respekt i rangi

Respekt jest osobnym trwałym licznikiem, zdobywanym przez drift i eventy. Rangi:

| Próg | Ranga |
|---:|---|
| 0 | Nowy |
| 250 | Kierowca |
| 1000 | Drifter |
| 3000 | Street Racer |
| 7500 | Drift King |
| 15000 | Legenda NEMQO |

Komenda: `/respekt`.

### Automatyczne eventy i animacje

- Co 15 minut serwer wybiera Chowany, Drift Challenge, Derby, Race albo Monster Sumo.
- Zapisy przez `/event` trwają 120 sekund.
- Administrator uruchamia event ręcznie: `/startevent [chowany/drift/derby/race/sumo]`.
- `/anim` otwiera listę Dance 1–4, siedzenie, palenie, opieranie i ręce do góry.
- `/dance` losuje taniec, `/stopanim` go przerywa.

### Flota i zaludnienie mapy

- 306 bazowych pojazdów statycznych oraz 6 pojazdów Wyspy NEMQO (łącznie 312),
- 112 różnych modeli po dodaniu pojazdów Wyspy NEMQO,
- dokładnie 26 samolotów ustawionych przy hangarach i apronach, nie na pasach,
- 171 nowych aut parkingowych,
- w ramach tych 171 aut: trzy wystawowe Elegy przy starcie, z paintjobami `0/1/2`,
- 20 driftowozów Placu Zabaw,
- każdy pojazd pokazuje w etykiecie spawn-ID modelu GTA zamiast chwilowego `vehicleid`; publiczne auta uliczne pokazują model, przebieg i paliwo bez danych właściciela, prywatne dodatkowo nick oraz ID właściciela, a pojazdy tymczasowe/eventowe tylko model,
- 6 pieszych `samp-npc` na niezależnych trasach LS/SF/LV,
- 24 invulnerable Actors z animacjami,
- kontrola odległości parkingów: 0 kolizji poniżej 3,5 m z istniejącymi autami, NPC i Actors.

### Nowy HUD

HUD korzysta z wirtualnej siatki PlayerTextDraw `640×448`:

- prawy dolny prędkościomierz nie ma prostokątnego tła i pokazuje tylko dużą prędkość, segmentowy pasek paliwa oraz model, HP i przebieg,
- respekt i ranga nie są wyświetlane w prędkościomierzu; pozostają dostępne przez `/respekt`,
- panel `ZABAWY` znajduje się po lewej stronie, używa większej czcionki oraz bardziej przezroczystego tła (`alpha 0x88`),
- osobny centralny drift-combo.

### Pamiętanie skina

Po zatwierdzeniu postaci serwer pyta, czy automatycznie używać jej w kolejnych logowaniach. `/rememberskin` przełącza preferencję, a `/skinselect` zawsze pozwala wrócić do pełnego selektora.

## 3. Najciekawsze elementy

### Freeroam

Cała mapa San Andreas jest dostępna. Można teleportować się między miastami, tworzyć pojazdy, korzystać z GPS, tuningu, broni i radia. Na ulicach, parkingach, przy szpitalach, komisariatach, kasynach, dworcach i lotniskach rozmieszczono pojazdy pasujące do otoczenia.

Serwer ma:

- 312 statycznych pojazdów (306 bazowych + 6 na Wyspie NEMQO),
- 112 różnych modeli po dodaniu pojazdów Wyspy NEMQO,
- 26 samolotów,
- pojazdy służb, komunikacji miejskiej, taksówki, ciężarówki i pojazdy techniczne,
- samoloty przy hangarach i apronach lotnisk Los Santos, San Fierro i Las Venturas, poza pasami startowymi.

### Lekki RPG i ekonomia

Postęp gracza, pieniądze, bank, level, XP, praca, Wanted, rekord wyścigu, własny pojazd, paliwo, przebieg, tuning, dom, firma, osiągnięcia i zadanie dzienne są zapisywane w bazie.

Dostępne prace:

1. taksówkarz,
2. kurier,
3. kierowca tira,
4. mechanik,
5. policjant,
6. ratownik,
7. pilot.

Zlecenie uruchamia się przez `/work`. Mechanik może naprawiać pojazdy graczy przez `/fix`, ratownik leczyć przez `/medic`, a policjant wystawiać mandaty i aresztować poszukiwanych.

### Własne pojazdy

Gracz może kupić jeden trwały pojazd. Zapisywane są:

- model,
- dwa kolory,
- paintjob,
- felgi,
- neony podwozia (6 kolorów lub wyłączone),
- nitro 10×,
- hydraulika,
- poziom paliwa,
- przebieg.

Pojazd prywatny może być podstawiony, zamknięty, uruchomiony, zatankowany i sprzedany. Publiczne auta uliczne również spalają paliwo podczas jazdy, można je tankować na stacjach, a ich runtime paliwo wraca do 100% po respawnie. Siedemnaście stacji obejmujących miasta, wsie i pustynię ma ikonę informacji oraz napis 3D `STACJA PALIW / Zatankuj pojazd`; wjazd publicznym autem albo własnym prywatnym autem w strefę otwiera potwierdzenie. Każde rozpoczęte brakujące 2% kosztuje `$13`. Etykiety pojazdów zostały skrócone i mają zasięg 15 m. HUD bez tła pokazuje prędkość, segmenty paliwa, health, model i przebieg; respekt oraz ranga są dostępne przez `/respekt`.

### Wyścigi

Tryb `LS Sprint` prowadzi przez serię checkpointów w Los Santos. Mierzony jest czas, zapisywany rekord konta, a ukończenie daje pieniądze, XP i osiągnięcie.

### Areny i minigry

- klasyczny Deathmatch,
- podniebne Sumo na platformie 125 × 125 m,
- Derby,
- Paintball,
- Sniper,
- Minigun,
- Zombie Survival,
- Stunt,
- Parkour,
- Grove Street kontra Ballas,
- pojedynki 1 na 1.

Areny korzystają z osobnych Virtual Worldów, dlatego nie przeszkadzają graczom we Freeroam. Wyjście z Zombie, Gang lub innego trybu zawsze przywraca normalny skin zapisany na koncie. Sumo automatycznie kończy się po spadnięciu 6 m poniżej platformy; kontrola działa co 250 ms.

### Policja i Wanted

Zabójstwo poza areną podnosi Wanted. Policjant może używać radaru, wystawić mandat, skuć gracza i aresztować osoby z Wanted 3+. Areszt przenosi gracza do więzienia na określony czas.

### Osiągnięcia i zadania dzienne

Osiągnięcia obejmują między innymi:

- pierwsze zlecenie,
- ukończenie wyścigu,
- zakup pojazdu,
- zakup domu,
- osiągnięcie 200 km/h,
- wejście na dodatkową arenę,
- zgromadzenie $100000.

Dzienne zadanie wymaga wykonania trzech zleceń pracy i daje dodatkowe pieniądze oraz XP.

## 4. Wybór postaci

Po zalogowaniu otwiera się natywny ekran wyboru klas SA-MP:

- postać jest widoczna na środku ekranu,
- strzałki lewo/prawo zmieniają model,
- przycisk `SPAWN` zatwierdza wybór,
- dostępnych jest 311 bezpiecznych skinów GTA SA,
- skin ID 74 jest pomijany, ponieważ jest wadliwy w SA-MP 0.3.7,
- wybrany skin jest zapisywany na koncie,
- gracz może trwale włączyć automatyczne używanie ostatniego skina.

Selektor można ponownie uruchomić komendą `/skinselect`; preferencję zmienia `/rememberskin`.

## 5. Paintjoby i tuning

Czysty SA-MP 0.3.7 nie przesyła graczom własnych tekstur pojazdów. Zewnętrzne paczki paintjobów wymagałyby SA-MP 0.3.DL, open.mp albo instalowania moda klienta. Dlatego serwer używa wyłącznie bezpiecznych, fabrycznych paintjobów GTA SA.

Obsługiwane modele:

- Camper (483),
- Remington (534),
- Slamvan (535),
- Blade (536),
- Uranus (558),
- Jester (559),
- Sultan (560),
- Stratum (561),
- Elegy (562),
- Flash (565),
- Savanna (567),
- Broadway (575),
- Tornado (576).

Każdy z obsługiwanych modeli może użyć paintjobu 0, 1 lub 2. `/tuning` zawsze dodaje nitro 10× i hydraulikę, a pozostałe elementy dobiera losowo.

Elegy przy molo mają kolejno paintjob `0` (lewy), `1` (środkowy) i `2` (prawy). `OnVehicleSpawn` ponownie nakłada właściwe malowanie po automatycznym respawnie auta.

## 6. Pełna lista komend

### Główne i informacje

| Komenda | Opis |
|---|---|
| `/menu` | Otwiera główne menu wszystkich systemów. |
| `/help` | Wyświetla skróconą pomoc. |
| `/stats` | Pokazuje pełne statystyki konta, pracy, auta i nieruchomości. |
| `/bonus` | Odbiera okresowy bonus pieniędzy i XP. |
| `/daily` | Pokazuje postęp zadania dziennego. |
| `/achievements` | Pokazuje zdobyte osiągnięcia. |
| `/kill` | Zabija własną postać. |

### Drift, zabawy i PSZ Tribute

| Komenda | Opis |
|---|---|
| `/zabawy` | Otwiera panel wejścia do wszystkich zabaw. |
| `/panel` | Pokazuje lub ukrywa licznik zajętości zabaw. |
| `/drift` | Otwiera wybór pięciu stref driftu. |
| `/drift off` | Wyłącza punktację i kończy bieżące combo. |
| `/driftstats` | Pokazuje sumę, rekord, respekt i rangę. |
| `/topdrift` | Pokazuje trwały ranking TOP 10. |
| `/tandem [id]` | Ustawia partnera tandemowego; `/tandem off` wyłącza. |
| `/rampa` | Tworzy rampę przed autem; odpowiednik klaksonu. |
| `/plac`, `/playground` | Teleportuje na pustynny Plac Zabaw. |
| `/chowany` | Otwiera zapisy lub dołącza do Chowanego. |
| `/leavehide` | Opuszcza zapisy albo aktywną rundę Chowanego. |
| `/event` | Dołącza do bieżącego automatycznego eventu. |
| `/anim`, `/taniec` | Otwiera panel animacji. |
| `/dance` | Uruchamia losowy taniec. |
| `/stopanim` | Przerywa animację. |
| `/respekt` | Pokazuje respekt, rangę i zwycięstwa. |

### Freeroam, teleporty i GPS

| Komenda | Opis |
|---|---|
| `/freeroam`, `/leave` | Opuszcza aktywną arenę, przywraca kontowy skin i wraca do swobodnej gry. |
| `/gps` | Otwiera menu najważniejszych lokalizacji. |
| `/molo`, `/pier` | Teleportuje na start przy dużym molo Santa Maria Beach. |
| `/wyspa`, `/island` | Teleportuje na Wyspę NEMQO z laguną, domkami, mariną i plażą. |
| `/ls` | Teleportuje naprzeciwko domu CJ na Grove Street. |
| `/sf` | Teleportuje do San Fierro. |
| `/lv` | Teleportuje do Las Venturas. |
| `/airport` | Teleportuje na lotnisko Los Santos. |
| `/chilliad` | Teleportuje na Mount Chiliad. |

### Wyścigi

| Komenda | Opis |
|---|---|
| `/race` | Rozpoczyna trasę LS Sprint. Wymaga pojazdu. |
| `/race quit` | Anuluje aktywny wyścig. |

### Prace

| Komenda | Opis |
|---|---|
| `/jobs` | Otwiera wybór pracy. |
| `/work` | Rozpoczyna zlecenie aktualnej pracy. |
| `/stopwork` | Anuluje aktywne zlecenie. |
| `/fix [id]` | Mechanik naprawia pojazd wskazanego gracza. |
| `/medic [id]` | Ratownik odnawia zdrowie wskazanego gracza. |

### Bank i ekonomia

| Komenda | Opis |
|---|---|
| `/balance` | Pokazuje gotówkę i saldo bankowe. |
| `/deposit [kwota]` | Wpłaca gotówkę do banku. |
| `/withdraw [kwota]` | Wypłaca pieniądze z banku. |

### Pojazdy

| Komenda | Opis |
|---|---|
| `/car [400-611]` | Tworzy tymczasowy pojazd Freeroam. |
| `/buycar [400-611]` | Kupuje trwały pojazd za $25000. |
| `/v` | Otwiera menu własnego pojazdu, w tym wybór neonów. |
| `/neon` | Otwiera wybór koloru neonów pod prywatnym autem. |
| `/engine` | Włącza lub wyłącza silnik. |
| `/lights` | Włącza lub wyłącza światła. |
| `/seatbelt` | Zapina lub odpina pasy. |
| `/refuel` | Otwiera potwierdzenie tankowania auta publicznego albo własnego prywatnego auta przy jednej z 17 stacji. |
| `/repair` | Naprawia aktualny pojazd. |
| `/tuning` | Losuje tuning; zawsze dodaje nitro 10× i hydraulikę. |
| `/paintjobs` | Pokazuje listę modeli z paintjobami. |
| `/paintjob [0-2]` | Zakłada wybrany fabryczny paintjob. |
| `/paintjob off` | Usuwa paintjob. |

Dodatkowo klawisz `Submission` — domyślnie `2` lub `Numpad +` — naprawia aktualny pojazd.

### Domy i firmy

| Komenda | Opis |
|---|---|
| `/house` | Pokazuje informacje o systemie domów. |
| `/buyhouse [1-5]` | Kupuje wybrany wolny dom. |
| `/home` | Teleportuje do własnego domu. |
| `/sellhouse` | Sprzedaje własny dom za połowę ceny. |
| `/business` | Pokazuje informacje o firmach. |
| `/buybiz [1-3]` | Kupuje wybraną wolną firmę. |
| `/bizbonus` | Odbiera godzinny zysk firmy. |

Ceny domów: $50000, $90000, $120000, $100000 i $80000. Ceny firm: $75000, $85000 i $100000.

### Policja i Wanted

| Komenda | Opis |
|---|---|
| `/wanted` | Pokazuje aktualny poziom Wanted. |
| `/ticket [id]` | Policjant wystawia poszukiwanemu mandat. |
| `/arrest [id]` | Policjant aresztuje pobliskiego gracza z Wanted 3+. |
| `/cuff [id]` | Policjant skuwa pobliskiego gracza. |
| `/uncuff [id]` | Policjant zdejmuje kajdanki. |
| `/radar` | Pokazuje najszybszy pojazd w promieniu radaru. |

### Areny i walka

| Komenda | Opis |
|---|---|
| `/arena` | Otwiera menu wszystkich dodatkowych aren. |
| `/leavearena` | Opuszcza dodatkową arenę. |
| `/dm` | Wchodzi na klasyczny Deathmatch. |
| `/sumo` | Wchodzi na podniebną arenę Monster Trucków; spadek lub wyjście z pojazdu automatycznie wraca do Freeroam. |
| `/derby` | Wchodzi na arenę Derby. |
| `/paintball` | Wchodzi na arenę Paintball. |
| `/sniper` | Wchodzi na arenę snajperską. |
| `/minigun` | Wchodzi na arenę Minigun. |
| `/zombie` | Wchodzi do trybu Zombie Survival. |
| `/stunt` | Wchodzi do trybu motocyklowych stuntów. |
| `/parkour` | Wchodzi na podniebną trasę Parkour. |
| `/gang` | Wchodzi do walki Grove Street kontra Ballas. |
| `/duel [id]` | Rozpoczyna pojedynek z wybranym graczem. |
| `/weapons` | Daje zestaw broni Freeroam. |
| `/heal` | Odnawia zdrowie i pancerz poza ograniczonymi arenami. |

### Postać

| Komenda | Opis |
|---|---|
| `/skinselect` | Otwiera pełny selektor skinów ze strzałkami i podglądem. |
| `/skin [0-311]` | Ustawia skin bezpośrednio; ID 74 jest zablokowane. |
| `/rememberskin` | Włącza lub wyłącza automatyczne użycie ostatniego skina. |

### Komunikacja i radio

| Komenda | Opis |
|---|---|
| `/pm [id] [tekst]` | Wysyła prywatną wiadomość. |
| `/global [tekst]` | Wysyła wiadomość na czacie globalnym. |
| `/radio` | Otwiera menu radia internetowego. |

Zwykły czat ma zasięg lokalny 100 metrów. Radio oferuje Radio Paradise, SomaFM Groove Salad i Radio Paradise Rock.

### Zgłoszenia i administracja

| Komenda | Opis |
|---|---|
| `/report [id] [powód]` | Zapisuje zgłoszenie gracza w SQLite. |
| `/adminhelp` | Pokazuje pomoc administratora. |
| `/reports` | Pokazuje pięć ostatnich otwartych zgłoszeń. |
| `/kick [id]` | Wyrzuca gracza. |
| `/ban [id]` | Zakłada trwały ban konta. |
| `/unban [nick]` | Usuwa ban konta. |
| `/mute [id]` | Włącza lub wyłącza blokadę czatu. |
| `/jail [id]` | Wysyła gracza do więzienia na 120 sekund. |
| `/goto [id]` | Teleportuje administratora do gracza. |
| `/gethere [id]` | Teleportuje gracza do administratora. |
| `/spec [id]` | Rozpoczyna obserwowanie gracza. |
| `/specoff` | Kończy obserwowanie. |

Kary administracyjne są zapisywane w tabeli `admin_logs`.

## 7. Szybki start

1. Połącz się z adresem skonfigurowanym przez operatora serwera na porcie UDP `7777`.
2. Zaloguj się lub utwórz konto.
3. Wybierz skin i zdecyduj, czy serwer ma go pamiętać; pojawisz się na dużym molo Santa Maria Beach.
4. Wpisz `/zabawy` i wybierz Drift albo Plac Zabaw.
5. W aucie naciśnij klakson, aby utworzyć rampę.
6. Użyj `/chowany`, `/event`, `/derby`, `/sumo` albo `/arena`.
7. Dopiero jeśli chcesz lekkiego RPG, otwórz `/menu`, `/jobs`, `/house` i `/business`.

## 8. Architektura

| Plik | Rola |
|---|---|
| `gamemodes/nemqo_freeroam_dm.pwn` | Główny gamemode i integracja callbacków. |
| `include/nemqo_v4_data.inc` | Stałe oraz stan systemów RPG/aren v4. |
| `include/nemqo_v4.inc` | Ekonomia, prace, pojazdy, wyścigi i areny v4. |
| `include/nemqo_psz_data.inc` | Stałe, rangi, strefy driftu i mapy Chowanego. |
| `include/nemqo_psz.inc` | Drift, tandem, panel, rampy, Chowany, eventy, animacje i Plac Zabaw. |
| `include/nemqo_v41_fleet.inc` | 171 parkingowych aut i 13 dodatkowych samolotów. |
| `include/nemqo_v41_ambient.inc` | 6 NPC, 24 Actors, preload animacji i runtime test ruchu. |
| `npcmodes/ambient_walk.pwn/.amx` | Opcjonalne źródło i artefakt ruchu pieszych NPC; plik AMX buduje się lokalnie. |
| `scriptfiles/accounts.db` | Runtime: konta i progresja SQLite. **Tego pliku nie publikuje się.** |
| `README_SERVER.md` | Dokumentacja użytkowa i operacyjna. |

Produkcja działa jako `samp.service` na UDP 7777. Zachowane środowisko stagingowe jest uruchamiane na żądanie jako `samp-staging-psz.service` na UDP 7778; po udanym wdrożeniu pozostaje wyłączone, aby nie zużywać dodatkowych procesów NPC.

## 9. SQLite v4.1

Tabela `accounts` ma 34 kolumny. Migracja v4.1 dodaje idempotentnie:

```sql
remember_skin INTEGER NOT NULL DEFAULT 0;
respect       INTEGER NOT NULL DEFAULT 0;
drift_total   INTEGER NOT NULL DEFAULT 0;
drift_best    INTEGER NOT NULL DEFAULT 0;
hide_wins     INTEGER NOT NULL DEFAULT 0;
event_wins    INTEGER NOT NULL DEFAULT 0;
```

Nie wolno sprawdzać schematu wyłącznie po liczbie kolumn. Deployment weryfikuje obecność wymaganych nazw oraz `PRAGMA integrity_check = ok`.

## 10. Budowanie legacy Pawn

Serwer nie używa pluginów C++ ani filterscriptów. Wymagany jest kompatybilny Pawn 3.2.3664 i pusty prefix include:

```bash
cp include/samp_empty_prefix.inc samp_empty_prefix.inc
LD_LIBRARY_PATH=/ścieżka/do/compiler-compat \
  /ścieżka/do/compiler-compat/pawncc gamemodes/nemqo_freeroam_dm.pwn \
  -iinclude -psamp_empty_prefix.inc \
  -ogamemodes/nemqo_freeroam_dm.amx
```

Warunek dopuszczenia: plik AMX istnieje, kompilator nie zgłasza błędów ani ostrzeżeń, a staging przechodzi testy opisane niżej.

## 11. Wdrożenie i automatyczny rollback

Skrypt wdrożeniowy i dane dostępowe operatora są utrzymywane poza publicznym repozytorium.

Przed każdą zmianą skrypt:

1. sprawdza wszystkie artefakty,
2. tworzy datowane pełne archiwum katalogu produkcyjnego,
3. testuje archiwum i zapisuje SHA-256,
4. zatrzymuje produkcję,
5. wykonuje idempotentną migrację SQLite,
6. instaluje gamemode, NPC, wszystkie include i README,
7. ustawia `hostname`, `gamemode0`, `maxplayers 38`, `maxnpc 6` i `port 7777`, nie dotykając hasła RCON,
8. uruchamia usługę i sprawdza `NRestarts=0`.

Dowolny błąd po zatrzymaniu usługi uruchamia trap, który odtwarza cały katalog produkcyjny z archiwum i ponownie uruchamia `samp.service`.

## 12. Kopie zapasowe

Backupy produkcji, bazy kont, logi i archiwa rollback są utrzymywane wyłącznie na infrastrukturze operatora. Nie należą do publicznego repozytorium GitHub.

## 13. Checklista po wdrożeniu

- `systemctl is-active samp.service` zwraca `active`.
- `NRestarts=0`.
- UDP query na skonfigurowany adres i port `7777` zwraca właściwy hostname i gamemode.
- Log zawiera `NEMQO PSZ self-test: PASS`.
- Log zawiera `ambient NPC movement: PASS 6/6`.
- Flota: `306`, samoloty: `26`, Actors: `24/24`, Plac Zabaw: `32` obiekty i `20` aut.
- SQLite: `PRAGMA integrity_check` zwraca `ok`.
- AMX produkcyjny ma SHA-256 identyczny z artefaktem buildu.
- Test klientem: login, zapamiętany skin, `/zabawy`, HUD, `/drift`, klakson/rampa, `/plac`, Chowany z drugą osobą, wejście i wyjście z aren, respawn i zapis statystyk.

## 14. Zweryfikowany staging

Aktualny build przeszedł na stagingowym porcie `7778`:

```text
NEMQO PSZ self-test: PASS | fleet=306 aircraft=26 actors=24 objects=32 playground_vehicles=20 pier_elegy=3 angle_diff=20
NEMQO v4.1 fuel markers: 17/17 created
NEMQO vehicle labels: 312/312 | public=312 temporary=0 private=0 unclassified=0
[V4 SAFETY SELFTEST] passed=126 failed=0
[V4 STORAGE SELFTEST] passed=12 failed=0 fixture_only=1
NEMQO Island self-test: PASS | grove-v3 objects=248 houses=9 interiors=9 pairs=verified tower=3279 vehicles=6 positions=verified
NEMQO v4.1 ambient NPC movement: PASS 6/6
```

Dodatkowo: `PRAGMA integrity_check=ok`, test zapisu nowych pól SQLite zakończony rollbackiem transakcji, 0 kolizji parkingów poniżej 3,5 m, usługa aktywna i `NRestarts=0`.

## 15. Ważne ograniczenie

Nie instaluj losowych paczek paintjobów przeznaczonych do modyfikacji lokalnego GTA SA i nie przedstawiaj ich jako funkcji serwera. Standardowy SA-MP 0.3.7 nie potrafi automatycznie wysyłać takich tekstur klientowi. Migracja do open.mp lub 0.3.DL może w przyszłości umożliwić własne skiny, obiekty i tekstury, ale wymaga osobnego testu zgodności klienta.

## 16. Publikacja na GitHubie

Do publicznego repozytorium należą wyłącznie źródła serwera:

- `gamemodes/*.pwn`,
- własne moduły `include/nemqo_*.inc`,
- `include/samp_empty_prefix.inc`,
- `README.md`, `README_SERVER.md` i `.gitignore`.

Nie publikuj: `server.cfg`, haseł RCON, `.env`, kluczy SSH, `scriptfiles/*.db`, plików `*.db-wal`/`*.db-shm`, logów, PID-ów, crash dumpów, backupów, release'ów produkcyjnych ani prywatnych skryptów wdrożeniowych. Oficjalna biblioteka standardowa SA-MP (`a_samp.inc`, `a_*.inc`, `core.inc` itd.) jest zależnością i nie musi być kopiowana do repozytorium projektu. Pliki `*.amx` są artefaktami buildu: nie trafiają do historii Git, ale mogą być dołączane osobno do wersjonowanego GitHub Release.
