# atsc-calculator

Calculadora de parâmetros do padrão **ATSC 3.0**, dividida em dois módulos:

- **`server_generator/`** — núcleo de cálculos em C++ que expõe um servidor TCP/JSON na porta `6000`.
- **`webpage/`** — interface web em PHP/JS que atua como proxy fino, delegando os cálculos ao servidor C++ e gerenciando autenticação/usuários no PostgreSQL.

A stack inteira sobe via Docker Compose (`db` + `server` + `web`).

---

## Deploy em outro Linux

### 1. Pré-requisitos

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y docker.io docker-compose-plugin git

# Permitir rodar docker sem sudo (faça logout/login depois)
sudo usermod -aG docker $USER
```

Para outras distros, ver https://docs.docker.com/engine/install/.

### 2. Clonar o repositório

```bash
git clone https://github.com/SBPJ1002/atsc-calculator.git
cd atsc-calculator
```

### 3. Configurar variáveis de ambiente

```bash
cp .env.example .env
nano .env
```

Ajuste no mínimo:

- `DB_PASSWORD` — senha forte para o PostgreSQL
- `WEB_PORT` — porta exposta no host (padrão `8080`)

### 4. Subir a stack

```bash
docker compose up -d --build
```

O que acontece:

- `db` (postgres:16-alpine) sobe e executa as migrações em `webpage/db/migrations/` automaticamente.
- `server` é compilado a partir de `docker/server.Dockerfile` e expõe TCP `6000` apenas na rede interna do compose.
- `web` (Apache + PHP) é compilado a partir de `docker/web.Dockerfile`, aguarda o `db` ficar saudável e publica em `http://<host>:<WEB_PORT>`.

### 5. Verificar

```bash
docker compose ps
docker compose logs -f web
docker compose logs -f server
```

Acesse `http://<ip-do-host>:8080` (ou a porta configurada).

### 6. Operação

```bash
docker compose down                              # parar (mantém dados)
docker compose down -v                           # parar e APAGAR volumes
docker compose up -d --build                     # rebuild após git pull
docker compose exec db psql -U atsc -d atsc     # console do Postgres
docker compose logs -f <serviço>                # logs em tempo real
```

### 7. Atualizar para nova versão

```bash
git pull
docker compose up -d --build
```

---

## Estrutura

```
atsc-calculator/
├── docker-compose.yml
├── docker/
│   ├── server.Dockerfile     # build do servidor C++
│   └── web.Dockerfile        # build do Apache + PHP
├── server_generator/         # código C++ (cálculos ATSC 3.0)
└── webpage/
    ├── api.php               # proxy para o servidor C++
    ├── webpage.php           # frontend
    └── db/migrations/        # SQL aplicado no boot do Postgres
```

## Volumes

- `db_data` — dados do PostgreSQL.
- `server_state` — estado/configuração do servidor C++ (compartilhado com `web` em `/var/www/server_generator/config`).

Ambos persistem entre restarts. São apagados apenas com `docker compose down -v`.

## Notas de produção

- Não exponha o Apache do container diretamente na internet — coloque um reverse proxy (nginx/caddy) com TLS na frente.
- Defina uma `DB_PASSWORD` forte no `.env`; o `.env` está no `.gitignore` e não deve ser commitado.
- Se a porta `8080` estiver ocupada no host, ajuste `WEB_PORT` no `.env`.
