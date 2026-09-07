PRAGMA foreign_keys = ON;
PRAGMA journal_mode = DELETE;
PRAGMA user_version = 1;

CREATE TABLE accounts (
    name TEXT PRIMARY KEY COLLATE NOCASE,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    money INTEGER NOT NULL DEFAULT 5000,
    level INTEGER NOT NULL DEFAULT 1,
    xp INTEGER NOT NULL DEFAULT 0,
    kills INTEGER NOT NULL DEFAULT 0,
    deaths INTEGER NOT NULL DEFAULT 0,
    last_bonus INTEGER NOT NULL DEFAULT 0,
    skin INTEGER NOT NULL DEFAULT 0,
    admin INTEGER NOT NULL DEFAULT 0,
    bank INTEGER NOT NULL DEFAULT 0,
    job INTEGER NOT NULL DEFAULT 0,
    job_xp INTEGER NOT NULL DEFAULT 0,
    wanted INTEGER NOT NULL DEFAULT 0,
    race_best INTEGER NOT NULL DEFAULT 0,
    owned_model INTEGER NOT NULL DEFAULT 0,
    owned_color1 INTEGER NOT NULL DEFAULT 1,
    owned_color2 INTEGER NOT NULL DEFAULT 1,
    owned_paintjob INTEGER NOT NULL DEFAULT -1,
    owned_wheels INTEGER NOT NULL DEFAULT 1080,
    owned_neon INTEGER NOT NULL DEFAULT 0,
    owned_fuel INTEGER NOT NULL DEFAULT 100,
    odometer INTEGER NOT NULL DEFAULT 0,
    odometer_fraction REAL NOT NULL DEFAULT 0,
    house_id INTEGER NOT NULL DEFAULT 0,
    business_id INTEGER NOT NULL DEFAULT 0,
    achievements INTEGER NOT NULL DEFAULT 0,
    daily_day INTEGER NOT NULL DEFAULT 0,
    daily_progress INTEGER NOT NULL DEFAULT 0,
    remember_skin INTEGER NOT NULL DEFAULT 0,
    respect INTEGER NOT NULL DEFAULT 0,
    drift_total INTEGER NOT NULL DEFAULT 0,
    drift_best INTEGER NOT NULL DEFAULT 0,
    hide_wins INTEGER NOT NULL DEFAULT 0,
    event_wins INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE houses (
    id INTEGER PRIMARY KEY,
    owner TEXT NOT NULL DEFAULT '',
    price INTEGER NOT NULL
);

CREATE TABLE businesses (
    id INTEGER PRIMARY KEY,
    owner TEXT NOT NULL DEFAULT '',
    price INTEGER NOT NULL,
    last_payout INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE reports (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    reporter TEXT NOT NULL,
    target TEXT NOT NULL,
    reason TEXT NOT NULL,
    created_at INTEGER NOT NULL,
    status INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE bans (
    name TEXT PRIMARY KEY COLLATE NOCASE,
    reason TEXT NOT NULL,
    admin TEXT NOT NULL,
    created_at INTEGER NOT NULL
);

CREATE TABLE admin_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    admin TEXT NOT NULL,
    target TEXT NOT NULL,
    action TEXT NOT NULL,
    created_at INTEGER NOT NULL
);

INSERT INTO houses (id, owner, price) VALUES
    (1, '', 50000),
    (2, '', 90000),
    (3, '', 120000),
    (4, '', 100000),
    (5, '', 80000);

INSERT INTO businesses (id, owner, price, last_payout) VALUES
    (1, '', 75000, 0),
    (2, '', 85000, 0),
    (3, '', 100000, 0);
