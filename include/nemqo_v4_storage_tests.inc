// Staging-only integration test. Marker absent on production. No production DB writes.
stock V4_RunStorageSelfTest()
{
    if (!fexist("grove_storage_test.enable")) return 1;
    for(new p=0;p<MAX_PLAYERS;p++) if(IsPlayerConnected(p)) return 0;
    new DB:original = g_DB;
    fremove("grove_storage_fixture.db");
    g_DB=db_open("grove_storage_fixture.db");
    if(g_DB==DB:0) { g_DB=original; return 0; }
    new passed=0, failed=0;
    V4_SafetyAssert(V4_SQLExec("CREATE TABLE IF NOT EXISTS accounts (name TEXT PRIMARY KEY COLLATE NOCASE,password_hash TEXT NOT NULL,salt TEXT NOT NULL,money INTEGER NOT NULL DEFAULT 5000,level INTEGER NOT NULL DEFAULT 1,xp INTEGER NOT NULL DEFAULT 0,kills INTEGER NOT NULL DEFAULT 0,deaths INTEGER NOT NULL DEFAULT 0,last_bonus INTEGER NOT NULL DEFAULT 0,skin INTEGER NOT NULL DEFAULT 0,admin INTEGER NOT NULL DEFAULT 0,bank INTEGER NOT NULL DEFAULT 0,job INTEGER NOT NULL DEFAULT 0,job_xp INTEGER NOT NULL DEFAULT 0,wanted INTEGER NOT NULL DEFAULT 0,race_best INTEGER NOT NULL DEFAULT 0,owned_model INTEGER NOT NULL DEFAULT 0,owned_color1 INTEGER NOT NULL DEFAULT 1,owned_color2 INTEGER NOT NULL DEFAULT 1,owned_paintjob INTEGER NOT NULL DEFAULT -1,owned_wheels INTEGER NOT NULL DEFAULT 1080,owned_fuel INTEGER NOT NULL DEFAULT 100,odometer INTEGER NOT NULL DEFAULT 0,house_id INTEGER NOT NULL DEFAULT 0,business_id INTEGER NOT NULL DEFAULT 0,achievements INTEGER NOT NULL DEFAULT 0,daily_day INTEGER NOT NULL DEFAULT 0,daily_progress INTEGER NOT NULL DEFAULT 0,remember_skin INTEGER NOT NULL DEFAULT 0,respect INTEGER NOT NULL DEFAULT 0,drift_total INTEGER NOT NULL DEFAULT 0,drift_best INTEGER NOT NULL DEFAULT 0,hide_wins INTEGER NOT NULL DEFAULT 0,event_wins INTEGER NOT NULL DEFAULT 0)"),"fixture schema",passed,failed);
    V4_SafetyAssert(V4_SQLExec("CREATE TABLE admin_logs (id INTEGER PRIMARY KEY, admin TEXT, target TEXT, action TEXT, created_at INTEGER)"),"fixture audit",passed,failed);
    V4_SafetyAssert(V4_OdoMigrate() && V4_OdoMigrate() && V4_NeonMigrate() && V4_NeonMigrate(),"idempotent migrations",passed,failed);
    V4_SafetyAssert(V4_SQLExec("INSERT INTO accounts (name,password_hash,salt) VALUES ('GrantActorTest','test-only','test-only')") && V4_SQLExec("INSERT INTO accounts (name,password_hash,salt) VALUES ('GrantTargetTest','test-only','test-only')"),"fixture insert",passed,failed);
    format(g_Name[0],MAX_PLAYER_NAME+1,"GrantActorTest");
    format(g_Name[1],MAX_PLAYER_NAME+1,"GrantTargetTest");
    g_Money[1]=5000;g_Level[1]=1;g_XP[1]=0;
    V4_SafetyAssert(V4_CommitGrant(0,1,12345,false,17345,1,0),"actual money transaction",passed,failed);
    V4_SafetyAssert(V4_CommitGrant(0,1,100000,true,17345,45,1000),"actual XP transaction",passed,failed);
    new DBResult:r=db_query(g_DB,"SELECT money,level,xp FROM accounts WHERE name='GrantTargetTest'");
    V4_SafetyAssert(db_get_field_assoc_int(r,"money")==17345 && db_get_field_assoc_int(r,"level")==45 && db_get_field_assoc_int(r,"xp")==1000,"committed grant readback",passed,failed);
    db_free_result(r);
    r=db_query(g_DB,"SELECT count(*) AS n FROM admin_logs");
    V4_SafetyAssert(db_get_field_assoc_int(r,"n")==2,"audit rows",passed,failed);db_free_result(r);
    g_AccountExists[1]=true;g_V4Odometer[1]=123456;g_V4OdoFraction[1]=0.625;g_V4OwnedNeon[1]=4;
    V4_Save(1);
    db_close(g_DB);g_DB=db_open("grove_storage_fixture.db");
    g_V4Odometer[1]=0;g_V4OdoFraction[1]=0.0;g_V4OwnedNeon[1]=0;V4_Load(1);
    V4_SafetyAssert(g_V4Odometer[1]==123456 && floatabs(g_V4OdoFraction[1]-0.625)<0.00001 && g_V4OwnedNeon[1]==4,"vehicle data save close reopen load",passed,failed);
    V4_SafetyAssert(V4_SQLExec("CREATE TRIGGER reject_test_audit BEFORE INSERT ON admin_logs BEGIN SELECT RAISE(ABORT,'expected test rejection'); END"),"audit failure injection",passed,failed);
    V4_SafetyAssert(!V4_CommitGrant(0,1,1,false,99999,45,1000),"grant fails when audit fails",passed,failed);
    r=db_query(g_DB,"SELECT money FROM accounts WHERE name='GrantTargetTest'");
    V4_SafetyAssert(db_get_field_assoc_int(r,"money")==17345,"grant rollback unchanged",passed,failed);db_free_result(r);
    db_close(g_DB);g_DB=original;
    // Slots were unused, restore their standard disconnected state.
    ResetPlayerData(0);ResetPlayerData(1);
    fremove("grove_storage_fixture.db");
    printf("[V4 STORAGE SELFTEST] passed=%d failed=%d fixture_only=1",passed,failed);
    return failed==0;
}
