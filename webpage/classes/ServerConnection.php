<?php

class ServerConnection
{
    private string $host;
    private int $port;
    private int $timeout;

    public function __construct(?string $host = null, ?int $port = null, ?int $timeout = null)
    {
        $this->host = $host ?? (getenv('ATSC_SERVER_HOST') ?: '127.0.0.1');
        $this->port = $port ?? (int)(getenv('ATSC_SERVER_PORT') ?: 6000);
        // setConfig dispara calculo multi-frame + escrita de varios logs no
        // C++; 2s nao da. 15s e folga generosa pro pior caso.
        $this->timeout = $timeout ?? (int)(getenv('ATSC_SERVER_TIMEOUT') ?: 15);
    }

    public function isAvailable(): bool
    {
        $socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($socket === false) {
            return false;
        }

        $timeoutArr = ['sec' => $this->timeout, 'usec' => 0];
        socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, $timeoutArr);
        socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, $timeoutArr);

        $result = @socket_connect($socket, $this->host, $this->port);
        @socket_close($socket);

        return $result !== false;
    }

    public function sendJson(array $data): array
    {
        $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($socket === false) {
            throw new \RuntimeException('Erro ao criar o socket: ' . socket_strerror(socket_last_error()));
        }

        $timeoutArr = ['sec' => $this->timeout, 'usec' => 0];
        socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, $timeoutArr);
        socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, $timeoutArr);

        if (!socket_connect($socket, $this->host, $this->port)) {
            $errMsg = socket_strerror(socket_last_error($socket));
            socket_close($socket);
            throw new \RuntimeException('Erro ao conectar ao servidor: ' . $errMsg);
        }

        $payload = json_encode($data) . "\n";
        if (socket_write($socket, $payload, strlen($payload)) === false) {
            $errMsg = socket_strerror(socket_last_error($socket));
            socket_close($socket);
            throw new \RuntimeException('Erro ao enviar dados: ' . $errMsg);
        }

        $response = '';
        while (true) {
            $buf = '';
            $bytes = @socket_recv($socket, $buf, 4096, 0);
            if ($bytes === false || $bytes === 0) {
                break;
            }
            $response .= $buf;
            if (strpos($response, "\n") !== false) {
                break;
            }
        }

        socket_close($socket);

        $response = rtrim($response, "\n");
        $parsed = json_decode($response, true);

        if ($parsed === null) {
            throw new \RuntimeException('Resposta invalida do servidor');
        }

        return $parsed;
    }

    public function getConfig(): array
    {
        return $this->sendJson(['flag' => 'getConfig']);
    }

    public function setConfig(array $configData): array
    {
        $configData['flag'] = 'setConfig';
        return $this->sendJson($configData);
    }
}
