<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Method not allowed']);
    exit;
}

$input = file_get_contents('php://input');
$decoded = json_decode($input, true);

if (!$decoded || !isset($decoded['flag'])) {
    http_response_code(400);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Missing flag parameter']);
    exit;
}

// TCP connection to C++ server
$host = '127.0.0.1';
$port = 6000;
$timeout = 2;

$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false) {
    http_response_code(503);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Erro ao criar socket: ' . socket_strerror(socket_last_error())]);
    exit;
}

// Set timeouts
$timeoutArr = ['sec' => $timeout, 'usec' => 0];
socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, $timeoutArr);
socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, $timeoutArr);

if (!@socket_connect($socket, $host, $port)) {
    $errMsg = socket_strerror(socket_last_error($socket));
    @socket_close($socket);
    http_response_code(503);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Servidor C++ indisponivel: ' . $errMsg]);
    exit;
}

// Send JSON + \n
$payload = $input . "\n";
if (socket_write($socket, $payload, strlen($payload)) === false) {
    $errMsg = socket_strerror(socket_last_error($socket));
    @socket_close($socket);
    http_response_code(502);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Erro ao enviar dados: ' . $errMsg]);
    exit;
}

// Read response until \n
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

@socket_close($socket);

// Trim trailing newline and output
$response = rtrim($response, "\n");

if (empty($response)) {
    http_response_code(502);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Resposta vazia do servidor']);
    exit;
}

// Validate JSON
$parsed = json_decode($response, true);
if ($parsed === null) {
    http_response_code(502);
    echo json_encode(['status' => 'erro', 'mensagem' => 'Resposta invalida do servidor']);
    exit;
}

echo $response;
