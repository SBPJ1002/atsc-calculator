CREATE TABLE IF NOT EXISTS users (
    id            BIGSERIAL PRIMARY KEY,
    name          VARCHAR(120) NOT NULL,
    email         VARCHAR(255) NOT NULL UNIQUE,
    country       CHAR(2)      NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at    TIMESTAMPTZ  NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_users_email ON users (email);

-- Seed do usuário admin original (mesma senha do login hardcoded anterior)
INSERT INTO users (name, email, country, password_hash)
VALUES (
    'Admin',
    'admin@atsc.local',
    'BR',
    '$2y$12$k9QpSbIl.n05eIpnjM5IYuvRQbMYpaU9GyENxfisnqr.r6oLKW6NG'
)
ON CONFLICT (email) DO NOTHING;
