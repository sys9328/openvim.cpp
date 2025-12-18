-- openvim initial schema rollback

DROP INDEX IF EXISTS idx_messages_session_created;
DROP TABLE IF EXISTS messages;
DROP TABLE IF EXISTS sessions;
-- Note: keep schema_migrations unless you want to wipe migration history.
