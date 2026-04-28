<?php

require_once __DIR__ . '/Database.php';

class UserRepository
{
    public static function findByEmail(string $email): ?array
    {
        $stmt = Database::getConnection()->prepare(
            'SELECT id, name, email, country, password_hash FROM users WHERE email = :email LIMIT 1'
        );
        $stmt->execute(['email' => strtolower(trim($email))]);
        $row = $stmt->fetch();
        return $row ?: null;
    }

    public static function emailExists(string $email): bool
    {
        return self::findByEmail($email) !== null;
    }

    /**
     * @return array{id:int,name:string,email:string,country:string}
     */
    public static function create(string $name, string $email, string $country, string $password): array
    {
        $hash = password_hash($password, PASSWORD_BCRYPT);

        $stmt = Database::getConnection()->prepare(
            'INSERT INTO users (name, email, country, password_hash)
             VALUES (:name, :email, :country, :hash)
             RETURNING id, name, email, country'
        );
        $stmt->execute([
            'name'    => trim($name),
            'email'   => strtolower(trim($email)),
            'country' => trim($country),
            'hash'    => $hash,
        ]);

        return $stmt->fetch();
    }

    public static function verifyPassword(array $user, string $password): bool
    {
        return password_verify($password, $user['password_hash']);
    }
}
