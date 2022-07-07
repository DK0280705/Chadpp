CREATE SCHEMA IF NOT EXISTS chadpp;
    
CREATE TABLE IF NOT EXISTS chadpp.afk_users (
    id          bigint          NOT NULL PRIMARY KEY,
    reason      varchar(1024)   NOT NULL,
    timestamp   bigint          DEFAULT extract(epoch FROM now())
);

CREATE TABLE IF NOT EXISTS chadpp.active_users (
    id          bigint          NOT NULL,
    guild_id    bigint          NOT NULL,
    msg_count   int             NOT NULL,
    msg_chars   bigint          NOT NULL,
    PRIMARY KEY(id, guild_id)
);

CREATE TABLE IF NOT EXISTS chadpp.guild_spam (
    id          bigint          NOT NULL PRIMARY KEY
);

CREATE TABLE IF NOT EXISTS chadpp.guild_lang (
    id          bigint          NOT NULL PRIMARY KEY,
    lang_id     int             NOT NULL
);

PREPARE find_afk_user (bigint) AS
    SELECT * FROM chadpp.afk_users WHERE id = $1;

PREPARE upsert_active_user (bigint, bigint, bigint) AS
    INSERT INTO chadpp.active_users
    VALUES ($1, $2, 1, $3)
    ON CONFLICT (id, guild_id) DO 
    UPDATE SET (msg_count, msg_chars) = (chadpp.active_users.msg_count + 1, chadpp.active_users.msg_chars + $3);

PREPARE upsert_guild_lang (bigint, int) AS
    INSERT INTO chadpp.guild_lang
    VALUES ($1, $2)
    ON CONFLICT (id) DO
    UPDATE SET lang_id = $2;

PREPARE get_guild_active_users (bigint) AS
    SELECT id, msg_count, msg_chars FROM chadpp.active_users WHERE guild_id = $1 LIMIT 10;