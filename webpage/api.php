<?php
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

require_once __DIR__ . '/classes/ServerConnection.php';

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

try {
    $server = new ServerConnection();
    $response = $server->sendJson($decoded);
    echo json_encode($response);
} catch (\RuntimeException $e) {
    http_response_code(502);
    echo json_encode(['status' => 'erro', 'mensagem' => $e->getMessage()]);
}
