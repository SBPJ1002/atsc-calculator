<?php
session_start();
header('X-SourceMap: none');
header('SourceMap: none');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

require_once __DIR__ . '/classes/ServerConnection.php';
require_once __DIR__ . '/classes/ConfigManager.php';

$server = new ServerConnection();
$server_available = $server->isAvailable();

if ($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['check_server'])) {
    header('Content-Type: application/json');
    echo json_encode(['server_available' => $server_available]);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['logout'])) {
    session_unset();
    session_destroy();
    header("Location: index.php");
    exit;
}

$config = ConfigManager::getDefaultConfig();
$subframes_data = [];

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['submit_config'])) {
    header('Content-Type: application/json');
    if (!$server_available) {
        echo json_encode(['status' => 'erro', 'mensagem' => 'Servidor offline']);
        exit;
    }

    $payload = ConfigManager::buildJsonPayload($_POST);

    try {
        $response = $server->sendJson($payload);
        echo json_encode($response);
    } catch (\RuntimeException $e) {
        echo json_encode(['status' => 'erro', 'mensagem' => $e->getMessage()]);
    }
    exit;
} else {
    if ($server_available) {
        try {
            $response = $server->getConfig();
            $parsed = ConfigManager::parseServerResponse($response);
            $config = $parsed['config'];
            $subframes_data = $parsed['subframes'];
        } catch (\RuntimeException $e) {
            $subframes_data = [];
        }
    }
}

function selected($value, $option) {
    return $value == $option ? 'selected' : '';
}
?>

<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ATSC 3.0 - Signaling</title>
    <link rel="stylesheet" href="assets/dist/app.min.css">
    <link rel="icon" href="assets/images/favicon.ico" type="image/x-icon">
</head>

<body>
        <div class="header-layout">
        <div class="header-left">
            <div class="logo-container">
                <div class="logo-image">
                    ATSC<br>SIGNALING
                </div>
            </div>
            <div class="server-status <?php echo $server_available ? 'online' : ''; ?>" id="server-status">
                <?php echo $server_available ? 'Servidor Online' : 'Modo Offline (Backup)'; ?>
            </div>
        </div>

        <div class="header-center">
            <button class="header-btn" onclick="document.getElementById('import-file').click()">Importar Config</button>
            <button class="header-btn" onclick="exportConfig()">Exportar Config</button>
            <input type="file" id="import-file" accept=".conf,.json" style="display: none;" onchange="importConfigFile(event)">
        </div>

        <button class="logout-btn" onclick="document.getElementById('logout-form').submit();">Logout</button>
    </div>

    <form id="logout-form" method="POST" action="" style="display: none;">
        <input type="hidden" name="logout" value="1">
    </form>

    <div id="main-container">

        <div class="wizard-progress">
            <div class="wizard-step active" data-step="1" onclick="wizardGoToStep(1)">
                <div class="wizard-step-circle"></div>
                <div class="wizard-step-label">Bootstrap</div>
            </div>
            <div class="wizard-step-line"></div>
            <div class="wizard-step" data-step="2" onclick="wizardGoToStep(2)">
                <div class="wizard-step-circle"></div>
                <div class="wizard-step-label">Preamble</div>
            </div>
            <div class="wizard-step-line"></div>
            <div class="wizard-step" data-step="3" onclick="wizardGoToStep(3)">
                <div class="wizard-step-circle"></div>
                <div class="wizard-step-label">Subframes</div>
            </div>
            <div class="wizard-step-line"></div>
            <div class="wizard-step" data-step="4" onclick="wizardGoToStep(4)">
                <div class="wizard-step-circle"></div>
                <div class="wizard-step-label">PLPs</div>
            </div>
        </div>

        <ul id="menuList" style="display:none;">
            <li class="menu-item"><ul id="gerador-group"></ul></li>
        </ul>

        <div class="split-layout">
            <div class="left-panel">
                <div class="content-layout">

                    <div class="wizard-sub-tabs" id="wizard-sub-tabs" style="display: none;">
                        <div class="sub-tabs-container" id="sub-tabs-container"></div>
                    </div>

                    <div class="content-container">
                        <div class="content-body">
                    <div id="sections">
                    <div id="botstrap" class="config-section" style="display: block;">
                        <form id="botstrap_form" method="POST" action="">
                            <h3>Bootstrap</h3>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="major_version">Major Version:</label>
                                    <select name="major_version" id="major_version" required>
                                        <option value="0" <?php echo selected($config['major_version'], "0"); ?>>137</option>
                                        <option value="1" <?php echo selected($config['major_version'], "1"); ?>>400</option>
                                    </select>
                                </div>

                                <div class="form-group">
                                    <label for="minor_version">Minor Version:</label>
                                    <select name="minor_version" id="minor_version" required>
                                        <option value="0" <?php echo selected($config['minor_version'], "0"); ?>>0x019D</option>
                                        <option value="1" <?php echo selected($config['minor_version'], "1"); ?>>0x00ED</option>
                                        <option value="2" <?php echo selected($config['minor_version'], "2"); ?>>0x01E8</option>
                                        <option value="3" <?php echo selected($config['minor_version'], "3"); ?>>0x00E8</option>
                                        <option value="4" <?php echo selected($config['minor_version'], "4"); ?>>0x00FB</option>
                                        <option value="5" <?php echo selected($config['minor_version'], "5"); ?>>0x0021</option>
                                        <option value="6" <?php echo selected($config['minor_version'], "6"); ?>>0x0054</option>
                                        <option value="7" <?php echo selected($config['minor_version'], "7"); ?>>0x00EC</option>
                                        <option value="8" <?php echo selected($config['minor_version'], "8"); ?>>0x0AA0</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="bootstrap_symbol">Bootstrap Symbol:</label>
                                    <select name="bootstrap_symbol" id="bootstrap_symbol" required>
                                        <option value="0" <?php echo selected($config['bootstrap_symbol'], "0"); ?>>4</option>
                                        <option value="1" <?php echo selected($config['bootstrap_symbol'], "1"); ?>>5</option>
                                    </select>
                                </div>

                                <div class="form-group">
                                    <label for="ea_wakeup">EA Wakeup:</label>
                                    <select name="ea_wakeup" id="ea_wakeup" required>
                                        <option value="0" <?php echo selected($config['ea_wakeup'], "0"); ?>>0</option>
                                        <option value="1" <?php echo selected($config['ea_wakeup'], "1"); ?>>1</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="system_bandwidth">System Bandwidth:</label>
                                    <select name="system_bandwidth" id="system_bandwidth" required>
                                        <option value="0" <?php echo selected($config['system_bandwidth'], "0"); ?>>6 MHz</option>
                                        <option value="1" <?php echo selected($config['system_bandwidth'], "1"); ?>>7 MHz</option>
                                        <option value="2" <?php echo selected($config['system_bandwidth'], "2"); ?>>8 MHz</option>
                                    </select>
                                </div>

                                <div class="form-group">
                                    <label for="bsr_coefficient">BSR Coefficient(MHz):</label>
                                    <select name="bsr_coefficient" id="bsr_coefficient" required>
                                        <option value="2" <?php echo selected($config['bsr_coefficient'], "2"); ?>>6.912</option>
                                        <option value="10" <?php echo selected($config['bsr_coefficient'], "10"); ?>>9.984</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                            <div class="form-group">
                                <label for="min_time_to_next">Min Time to Next(ms):</label>
                                <select name="min_time_to_next" id="min_time_to_next" required>
                                    <option value="50" <?php echo selected($config['min_time_to_next'], "50"); ?>>50</option>
                                    <option value="100" <?php echo selected($config['min_time_to_next'], "100"); ?>>100</option>
                                    <option value="150" <?php echo selected($config['min_time_to_next'], "150"); ?>>150</option>
                                    <option value="200" <?php echo selected($config['min_time_to_next'], "200"); ?>>200</option>
                                    <option value="250" <?php echo selected($config['min_time_to_next'], "250"); ?>>250</option>
                                    <option value="300" <?php echo selected($config['min_time_to_next'], "300"); ?>>300</option>
                                    <option value="350" <?php echo selected($config['min_time_to_next'], "350"); ?>>350</option>
                                    <option value="400" <?php echo selected($config['min_time_to_next'], "400"); ?>>400</option>
                                    <option value="500" <?php echo selected($config['min_time_to_next'], "500"); ?>>500</option>
                                    <option value="600" <?php echo selected($config['min_time_to_next'], "600"); ?>>600</option>
                                    <option value="700" <?php echo selected($config['min_time_to_next'], "700"); ?>>700</option>
                                    <option value="800" <?php echo selected($config['min_time_to_next'], "800"); ?>>800</option>
                                    <option value="900" <?php echo selected($config['min_time_to_next'], "900"); ?>>900</option>
                                    <option value="1000" <?php echo selected($config['min_time_to_next'], "1000"); ?>>1000</option>
                                    <option value="1100" <?php echo selected($config['min_time_to_next'], "1100"); ?>>1100</option>
                                    <option value="1200" <?php echo selected($config['min_time_to_next'], "1200"); ?>>1200</option>
                                    <option value="1300" <?php echo selected($config['min_time_to_next'], "1300"); ?>>1300</option>
                                    <option value="1500" <?php echo selected($config['min_time_to_next'], "1500"); ?>>1500</option>
                                    <option value="1700" <?php echo selected($config['min_time_to_next'], "1700"); ?>>1700</option>
                                    <option value="1900" <?php echo selected($config['min_time_to_next'], "1900"); ?>>1900</option>
                                    <option value="2100" <?php echo selected($config['min_time_to_next'], "2100"); ?>>2100</option>
                                    <option value="2300" <?php echo selected($config['min_time_to_next'], "2300"); ?>>2300</option>
                                    <option value="2500" <?php echo selected($config['min_time_to_next'], "2500"); ?>>2500</option>
                                    <option value="2700" <?php echo selected($config['min_time_to_next'], "2700"); ?>>2700</option>
                                    <option value="2900" <?php echo selected($config['min_time_to_next'], "2900"); ?>>2900</option>
                                    <option value="3300" <?php echo selected($config['min_time_to_next'], "3300"); ?>>3300</option>
                                    <option value="3700" <?php echo selected($config['min_time_to_next'], "3700"); ?>>3700</option>
                                    <option value="4100" <?php echo selected($config['min_time_to_next'], "4100"); ?>>4100</option>
                                    <option value="4500" <?php echo selected($config['min_time_to_next'], "4500"); ?>>4500</option>
                                    <option value="4900" <?php echo selected($config['min_time_to_next'], "4900"); ?>>4900</option>
                                    <option value="5300" <?php echo selected($config['min_time_to_next'], "5300"); ?>>5300</option>
                                </select>
                            </div>

                            <div class="form-group">
                                <label for="preamble_mode">L1-Basic Mode:</label>
                                <select name="preamble_mode" id="preamble_mode" required>
                                    <option value="1">Mode 1</option>
                                    <option value="2">Mode 2</option>
                                    <option value="3">Mode 3</option>
                                    <option value="4">Mode 4</option>
                                    <option value="5">Mode 5</option>
                                </select>
                            </div>

                            <div class="form-group">
                                <label for="preamble_structure">Preamble Structure:</label>
                                <select name="preamble_structure" id="preamble_structure" required>
                                    <option value="0" data-mode="1" <?php echo selected($config['preamble_structure'], "0"); ?>>[0] 8K, GI1_192, Dx_16</option>
                                    <option value="1" data-mode="2" <?php echo selected($config['preamble_structure'], "1"); ?>>[1] 8K, GI1_192, Dx_16</option>
                                    <option value="2" data-mode="3" <?php echo selected($config['preamble_structure'], "2"); ?>>[2] 8K, GI1_192, Dx_16</option>
                                    <option value="3" data-mode="4" <?php echo selected($config['preamble_structure'], "3"); ?>>[3] 8K, GI1_192, Dx_16</option>
                                    <option value="4" data-mode="5" <?php echo selected($config['preamble_structure'], "4"); ?>>[4] 8K, GI1_192, Dx_16</option>
                                    <option value="5" data-mode="1" <?php echo selected($config['preamble_structure'], "5"); ?>>[5] 8K, GI2_384, Dx_8</option>
                                    <option value="6" data-mode="2" <?php echo selected($config['preamble_structure'], "6"); ?>>[6] 8K, GI2_384, Dx_8</option>
                                    <option value="7" data-mode="3" <?php echo selected($config['preamble_structure'], "7"); ?>>[7] 8K, GI2_384, Dx_8</option>
                                    <option value="8" data-mode="4" <?php echo selected($config['preamble_structure'], "8"); ?>>[8] 8K, GI2_384, Dx_8</option>
                                    <option value="9" data-mode="5" <?php echo selected($config['preamble_structure'], "9"); ?>>[9] 8K, GI2_384, Dx_8</option>
                                    <option value="10" data-mode="1" <?php echo selected($config['preamble_structure'], "10"); ?>>[10] 8K, GI3_512, Dx_6</option>
                                    <option value="11" data-mode="2" <?php echo selected($config['preamble_structure'], "11"); ?>>[11] 8K, GI3_512, Dx_6</option>
                                    <option value="12" data-mode="3" <?php echo selected($config['preamble_structure'], "12"); ?>>[12] 8K, GI3_512, Dx_6</option>
                                    <option value="13" data-mode="4" <?php echo selected($config['preamble_structure'], "13"); ?>>[13] 8K, GI3_512, Dx_6</option>
                                    <option value="14" data-mode="5" <?php echo selected($config['preamble_structure'], "14"); ?>>[14] 8K, GI3_512, Dx_6</option>
                                    <option value="15" data-mode="1" <?php echo selected($config['preamble_structure'], "15"); ?>>[15] 8K, GI4_768, Dx_4</option>
                                    <option value="16" data-mode="2" <?php echo selected($config['preamble_structure'], "16"); ?>>[16] 8K, GI4_768, Dx_4</option>
                                    <option value="17" data-mode="3" <?php echo selected($config['preamble_structure'], "17"); ?>>[17] 8K, GI4_768, Dx_4</option>
                                    <option value="18" data-mode="4" <?php echo selected($config['preamble_structure'], "18"); ?>>[18] 8K, GI4_768, Dx_4</option>
                                    <option value="19" data-mode="5" <?php echo selected($config['preamble_structure'], "19"); ?>>[19] 8K, GI4_768, Dx_4</option>
                                    <option value="20" data-mode="1" <?php echo selected($config['preamble_structure'], "20"); ?>>[20] 8K, GI5_1024, Dx_3</option>
                                    <option value="21" data-mode="2" <?php echo selected($config['preamble_structure'], "21"); ?>>[21] 8K, GI5_1024, Dx_3</option>
                                    <option value="22" data-mode="3" <?php echo selected($config['preamble_structure'], "22"); ?>>[22] 8K, GI5_1024, Dx_3</option>
                                    <option value="23" data-mode="4" <?php echo selected($config['preamble_structure'], "23"); ?>>[23] 8K, GI5_1024, Dx_3</option>
                                    <option value="24" data-mode="5" <?php echo selected($config['preamble_structure'], "24"); ?>>[24] 8K, GI5_1024, Dx_3</option>
                                    <option value="25" data-mode="1" <?php echo selected($config['preamble_structure'], "25"); ?>>[25] 8K, GI6_1536, Dx_4</option>
                                    <option value="26" data-mode="2" <?php echo selected($config['preamble_structure'], "26"); ?>>[26] 8K, GI6_1536, Dx_4</option>
                                    <option value="27" data-mode="3" <?php echo selected($config['preamble_structure'], "27"); ?>>[27] 8K, GI6_1536, Dx_4</option>
                                    <option value="28" data-mode="4" <?php echo selected($config['preamble_structure'], "28"); ?>>[28] 8K, GI6_1536, Dx_4</option>
                                    <option value="29" data-mode="5" <?php echo selected($config['preamble_structure'], "29"); ?>>[29] 8K, GI6_1536, Dx_4</option>
                                    <option value="30" data-mode="1" <?php echo selected($config['preamble_structure'], "30"); ?>>[30] 8K, GI7_2048, Dx_3</option>
                                    <option value="31" data-mode="2" <?php echo selected($config['preamble_structure'], "31"); ?>>[31] 8K, GI7_2048, Dx_3</option>
                                    <option value="32" data-mode="3" <?php echo selected($config['preamble_structure'], "32"); ?>>[32] 8K, GI7_2048, Dx_3</option>
                                    <option value="33" data-mode="4" <?php echo selected($config['preamble_structure'], "33"); ?>>[33] 8K, GI7_2048, Dx_3</option>
                                    <option value="34" data-mode="5" <?php echo selected($config['preamble_structure'], "34"); ?>>[34] 8K, GI7_2048, Dx_3</option>
                                    <option value="35" data-mode="1" <?php echo selected($config['preamble_structure'], "35"); ?>>[35] 16K, GI1_192, Dx_32</option>
                                    <option value="36" data-mode="2" <?php echo selected($config['preamble_structure'], "36"); ?>>[36] 16K, GI1_192, Dx_32</option>
                                    <option value="37" data-mode="3" <?php echo selected($config['preamble_structure'], "37"); ?>>[37] 16K, GI1_192, Dx_32</option>
                                    <option value="38" data-mode="4" <?php echo selected($config['preamble_structure'], "38"); ?>>[38] 16K, GI1_192, Dx_32</option>
                                    <option value="39" data-mode="5" <?php echo selected($config['preamble_structure'], "39"); ?>>[39] 16K, GI1_192, Dx_32</option>
                                    <option value="40" data-mode="1" <?php echo selected($config['preamble_structure'], "40"); ?>>[40] 16K, GI2_384, Dx_16</option>
                                    <option value="41" data-mode="2" <?php echo selected($config['preamble_structure'], "41"); ?>>[41] 16K, GI2_384, Dx_16</option>
                                    <option value="42" data-mode="3" <?php echo selected($config['preamble_structure'], "42"); ?>>[42] 16K, GI2_384, Dx_16</option>
                                    <option value="43" data-mode="4" <?php echo selected($config['preamble_structure'], "43"); ?>>[43] 16K, GI2_384, Dx_16</option>
                                    <option value="44" data-mode="5" <?php echo selected($config['preamble_structure'], "44"); ?>>[44] 16K, GI2_384, Dx_16</option>
                                    <option value="45" data-mode="1" <?php echo selected($config['preamble_structure'], "45"); ?>>[45] 16K, GI3_512, Dx_12</option>
                                    <option value="46" data-mode="2" <?php echo selected($config['preamble_structure'], "46"); ?>>[46] 16K, GI3_512, Dx_12</option>
                                    <option value="47" data-mode="3" <?php echo selected($config['preamble_structure'], "47"); ?>>[47] 16K, GI3_512, Dx_12</option>
                                    <option value="48" data-mode="4" <?php echo selected($config['preamble_structure'], "48"); ?>>[48] 16K, GI3_512, Dx_12</option>
                                    <option value="49" data-mode="5" <?php echo selected($config['preamble_structure'], "49"); ?>>[49] 16K, GI3_512, Dx_12</option>
                                    <option value="50" data-mode="1" <?php echo selected($config['preamble_structure'], "50"); ?>>[50] 16K, GI4_768, Dx_8</option>
                                    <option value="51" data-mode="2" <?php echo selected($config['preamble_structure'], "51"); ?>>[51] 16K, GI4_768, Dx_8</option>
                                    <option value="52" data-mode="3" <?php echo selected($config['preamble_structure'], "52"); ?>>[52] 16K, GI4_768, Dx_8</option>
                                    <option value="53" data-mode="4" <?php echo selected($config['preamble_structure'], "53"); ?>>[53] 16K, GI4_768, Dx_8</option>
                                    <option value="54" data-mode="5" <?php echo selected($config['preamble_structure'], "54"); ?>>[54] 16K, GI4_768, Dx_8</option>
                                    <option value="55" data-mode="1" <?php echo selected($config['preamble_structure'], "55"); ?>>[55] 16K, GI5_1024, Dx_6</option>
                                    <option value="56" data-mode="2" <?php echo selected($config['preamble_structure'], "56"); ?>>[56] 16K, GI5_1024, Dx_6</option>
                                    <option value="57" data-mode="3" <?php echo selected($config['preamble_structure'], "57"); ?>>[57] 16K, GI5_1024, Dx_6</option>
                                    <option value="58" data-mode="4" <?php echo selected($config['preamble_structure'], "58"); ?>>[58] 16K, GI5_1024, Dx_6</option>
                                    <option value="59" data-mode="5" <?php echo selected($config['preamble_structure'], "59"); ?>>[59] 16K, GI5_1024, Dx_6</option>
                                    <option value="60" data-mode="1" <?php echo selected($config['preamble_structure'], "60"); ?>>[60] 16K, GI6_1536, Dx_4</option>
                                    <option value="61" data-mode="2" <?php echo selected($config['preamble_structure'], "61"); ?>>[61] 16K, GI6_1536, Dx_4</option>
                                    <option value="62" data-mode="3" <?php echo selected($config['preamble_structure'], "62"); ?>>[62] 16K, GI6_1536, Dx_4</option>
                                    <option value="63" data-mode="4" <?php echo selected($config['preamble_structure'], "63"); ?>>[63] 16K, GI6_1536, Dx_4</option>
                                    <option value="64" data-mode="5" <?php echo selected($config['preamble_structure'], "64"); ?>>[64] 16K, GI6_1536, Dx_4</option>
                                    <option value="65" data-mode="1" <?php echo selected($config['preamble_structure'], "65"); ?>>[65] 16K, GI7_2048, Dx_3</option>
                                    <option value="66" data-mode="2" <?php echo selected($config['preamble_structure'], "66"); ?>>[66] 16K, GI7_2048, Dx_3</option>
                                    <option value="67" data-mode="3" <?php echo selected($config['preamble_structure'], "67"); ?>>[67] 16K, GI7_2048, Dx_3</option>
                                    <option value="68" data-mode="4" <?php echo selected($config['preamble_structure'], "68"); ?>>[68] 16K, GI7_2048, Dx_3</option>
                                    <option value="69" data-mode="5" <?php echo selected($config['preamble_structure'], "69"); ?>>[69] 16K, GI7_2048, Dx_3</option>
                                    <option value="70" data-mode="1" <?php echo selected($config['preamble_structure'], "70"); ?>>[70] 16K, GI8_2432, Dx_3</option>
                                    <option value="71" data-mode="2" <?php echo selected($config['preamble_structure'], "71"); ?>>[71] 16K, GI8_2432, Dx_3</option>
                                    <option value="72" data-mode="3" <?php echo selected($config['preamble_structure'], "72"); ?>>[72] 16K, GI8_2432, Dx_3</option>
                                    <option value="73" data-mode="4" <?php echo selected($config['preamble_structure'], "73"); ?>>[73] 16K, GI8_2432, Dx_3</option>
                                    <option value="74" data-mode="5" <?php echo selected($config['preamble_structure'], "74"); ?>>[74] 16K, GI8_2432, Dx_3</option>
                                    <option value="75" data-mode="1" <?php echo selected($config['preamble_structure'], "75"); ?>>[75] 16K, GI9_3072, Dx_4</option>
                                    <option value="76" data-mode="2" <?php echo selected($config['preamble_structure'], "76"); ?>>[76] 16K, GI9_3072, Dx_4</option>
                                    <option value="77" data-mode="3" <?php echo selected($config['preamble_structure'], "77"); ?>>[77] 16K, GI9_3072, Dx_4</option>
                                    <option value="78" data-mode="4" <?php echo selected($config['preamble_structure'], "78"); ?>>[78] 16K, GI9_3072, Dx_4</option>
                                    <option value="79" data-mode="5" <?php echo selected($config['preamble_structure'], "79"); ?>>[79] 16K, GI9_3072, Dx_4</option>
                                    <option value="80" data-mode="1" <?php echo selected($config['preamble_structure'], "80"); ?>>[80] 16K, GI10_3648, Dx_4</option>
                                    <option value="81" data-mode="2" <?php echo selected($config['preamble_structure'], "81"); ?>>[81] 16K, GI10_3648, Dx_4</option>
                                    <option value="82" data-mode="3" <?php echo selected($config['preamble_structure'], "82"); ?>>[82] 16K, GI10_3648, Dx_4</option>
                                    <option value="83" data-mode="4" <?php echo selected($config['preamble_structure'], "83"); ?>>[83] 16K, GI10_3648, Dx_4</option>
                                    <option value="84" data-mode="5" <?php echo selected($config['preamble_structure'], "84"); ?>>[84] 16K, GI10_3648, Dx_4</option>
                                    <option value="85" data-mode="1" <?php echo selected($config['preamble_structure'], "85"); ?>>[85] 16K, GI11_4096, Dx_3</option>
                                    <option value="86" data-mode="2" <?php echo selected($config['preamble_structure'], "86"); ?>>[86] 16K, GI11_4096, Dx_3</option>
                                    <option value="87" data-mode="3" <?php echo selected($config['preamble_structure'], "87"); ?>>[87] 16K, GI11_4096, Dx_3</option>
                                    <option value="88" data-mode="4" <?php echo selected($config['preamble_structure'], "88"); ?>>[88] 16K, GI11_4096, Dx_3</option>
                                    <option value="89" data-mode="5" <?php echo selected($config['preamble_structure'], "89"); ?>>[89] 16K, GI11_4096, Dx_3</option>
                                    <option value="90" data-mode="1" <?php echo selected($config['preamble_structure'], "90"); ?>>[90] 32K, GI1_192, Dx_32</option>
                                    <option value="91" data-mode="2" <?php echo selected($config['preamble_structure'], "91"); ?>>[91] 32K, GI1_192, Dx_32</option>
                                    <option value="92" data-mode="3" <?php echo selected($config['preamble_structure'], "92"); ?>>[92] 32K, GI1_192, Dx_32</option>
                                    <option value="93" data-mode="4" <?php echo selected($config['preamble_structure'], "93"); ?>>[93] 32K, GI1_192, Dx_32</option>
                                    <option value="94" data-mode="5" <?php echo selected($config['preamble_structure'], "94"); ?>>[94] 32K, GI1_192, Dx_32</option>
                                    <option value="95" data-mode="1" <?php echo selected($config['preamble_structure'], "95"); ?>>[95] 32K, GI2_384, Dx_32</option>
                                    <option value="96" data-mode="2" <?php echo selected($config['preamble_structure'], "96"); ?>>[96] 32K, GI2_384, Dx_32</option>
                                    <option value="97" data-mode="3" <?php echo selected($config['preamble_structure'], "97"); ?>>[97] 32K, GI2_384, Dx_32</option>
                                    <option value="98" data-mode="4" <?php echo selected($config['preamble_structure'], "98"); ?>>[98] 32K, GI2_384, Dx_32</option>
                                    <option value="99" data-mode="5" <?php echo selected($config['preamble_structure'], "99"); ?>>[99] 32K, GI2_384, Dx_32</option>
                                    <option value="100" data-mode="1" <?php echo selected($config['preamble_structure'], "100"); ?>>[100] 32K, GI3_512, Dx_24</option>
                                    <option value="101" data-mode="2" <?php echo selected($config['preamble_structure'], "101"); ?>>[101] 32K, GI3_512, Dx_24</option>
                                    <option value="102" data-mode="3" <?php echo selected($config['preamble_structure'], "102"); ?>>[102] 32K, GI3_512, Dx_24</option>
                                    <option value="103" data-mode="4" <?php echo selected($config['preamble_structure'], "103"); ?>>[103] 32K, GI3_512, Dx_24</option>
                                    <option value="104" data-mode="5" <?php echo selected($config['preamble_structure'], "104"); ?>>[104] 32K, GI3_512, Dx_24</option>
                                    <option value="105" data-mode="1" <?php echo selected($config['preamble_structure'], "105"); ?>>[105] 32K, GI4_768, Dx_16</option>
                                    <option value="106" data-mode="2" <?php echo selected($config['preamble_structure'], "106"); ?>>[106] 32K, GI4_768, Dx_16</option>
                                    <option value="107" data-mode="3" <?php echo selected($config['preamble_structure'], "107"); ?>>[107] 32K, GI4_768, Dx_16</option>
                                    <option value="108" data-mode="4" <?php echo selected($config['preamble_structure'], "108"); ?>>[108] 32K, GI4_768, Dx_16</option>
                                    <option value="109" data-mode="5" <?php echo selected($config['preamble_structure'], "109"); ?>>[109] 32K, GI4_768, Dx_16</option>
                                    <option value="110" data-mode="1" <?php echo selected($config['preamble_structure'], "110"); ?>>[110] 32K, GI5_1024, Dx_12</option>
                                    <option value="111" data-mode="2" <?php echo selected($config['preamble_structure'], "111"); ?>>[111] 32K, GI5_1024, Dx_12</option>
                                    <option value="112" data-mode="3" <?php echo selected($config['preamble_structure'], "112"); ?>>[112] 32K, GI5_1024, Dx_12</option>
                                    <option value="113" data-mode="4" <?php echo selected($config['preamble_structure'], "113"); ?>>[113] 32K, GI5_1024, Dx_12</option>
                                    <option value="114" data-mode="5" <?php echo selected($config['preamble_structure'], "114"); ?>>[114] 32K, GI5_1024, Dx_12</option>
                                    <option value="115" data-mode="1" <?php echo selected($config['preamble_structure'], "115"); ?>>[115] 32K, GI6_1536, Dx_8</option>
                                    <option value="116" data-mode="2" <?php echo selected($config['preamble_structure'], "116"); ?>>[116] 32K, GI6_1536, Dx_8</option>
                                    <option value="117" data-mode="3" <?php echo selected($config['preamble_structure'], "117"); ?>>[117] 32K, GI6_1536, Dx_8</option>
                                    <option value="118" data-mode="4" <?php echo selected($config['preamble_structure'], "118"); ?>>[118] 32K, GI6_1536, Dx_8</option>
                                    <option value="119" data-mode="5" <?php echo selected($config['preamble_structure'], "119"); ?>>[119] 32K, GI6_1536, Dx_8</option>
                                    <option value="120" data-mode="1" <?php echo selected($config['preamble_structure'], "120"); ?>>[120] 32K, GI7_2048, Dx_6</option>
                                    <option value="121" data-mode="2" <?php echo selected($config['preamble_structure'], "121"); ?>>[121] 32K, GI7_2048, Dx_6</option>
                                    <option value="122" data-mode="3" <?php echo selected($config['preamble_structure'], "122"); ?>>[122] 32K, GI7_2048, Dx_6</option>
                                    <option value="123" data-mode="4" <?php echo selected($config['preamble_structure'], "123"); ?>>[123] 32K, GI7_2048, Dx_6</option>
                                    <option value="124" data-mode="5" <?php echo selected($config['preamble_structure'], "124"); ?>>[124] 32K, GI7_2048, Dx_6</option>
                                    <option value="125" data-mode="1" <?php echo selected($config['preamble_structure'], "125"); ?>>[125] 32K, GI8_2432, Dx_6</option>
                                    <option value="126" data-mode="2" <?php echo selected($config['preamble_structure'], "126"); ?>>[126] 32K, GI8_2432, Dx_6</option>
                                    <option value="127" data-mode="3" <?php echo selected($config['preamble_structure'], "127"); ?>>[127] 32K, GI8_2432, Dx_6</option>
                                    <option value="128" data-mode="4" <?php echo selected($config['preamble_structure'], "128"); ?>>[128] 32K, GI8_2432, Dx_6</option>
                                    <option value="129" data-mode="5" <?php echo selected($config['preamble_structure'], "129"); ?>>[129] 32K, GI8_2432, Dx_6</option>
                                    <option value="130" data-mode="1" <?php echo selected($config['preamble_structure'], "130"); ?>>[130] 32K, GI9_3072, Dx_8</option>
                                    <option value="131" data-mode="2" <?php echo selected($config['preamble_structure'], "131"); ?>>[131] 32K, GI9_3072, Dx_8</option>
                                    <option value="132" data-mode="3" <?php echo selected($config['preamble_structure'], "132"); ?>>[132] 32K, GI9_3072, Dx_8</option>
                                    <option value="133" data-mode="4" <?php echo selected($config['preamble_structure'], "133"); ?>>[133] 32K, GI9_3072, Dx_8</option>
                                    <option value="134" data-mode="5" <?php echo selected($config['preamble_structure'], "134"); ?>>[134] 32K, GI9_3072, Dx_8</option>
                                    <option value="135" data-mode="1" <?php echo selected($config['preamble_structure'], "135"); ?>>[135] 32K, GI9_3072, Dx_3</option>
                                    <option value="136" data-mode="2" <?php echo selected($config['preamble_structure'], "136"); ?>>[136] 32K, GI9_3072, Dx_3</option>
                                    <option value="137" data-mode="3" <?php echo selected($config['preamble_structure'], "137"); ?>>[137] 32K, GI9_3072, Dx_3</option>
                                    <option value="138" data-mode="4" <?php echo selected($config['preamble_structure'], "138"); ?>>[138] 32K, GI9_3072, Dx_3</option>
                                    <option value="139" data-mode="5" <?php echo selected($config['preamble_structure'], "139"); ?>>[139] 32K, GI9_3072, Dx_3</option>
                                    <option value="140" data-mode="1" <?php echo selected($config['preamble_structure'], "140"); ?>>[140] 32K, GI10_3648, Dx_8</option>
                                    <option value="141" data-mode="2" <?php echo selected($config['preamble_structure'], "141"); ?>>[141] 32K, GI10_3648, Dx_8</option>
                                    <option value="142" data-mode="3" <?php echo selected($config['preamble_structure'], "142"); ?>>[142] 32K, GI10_3648, Dx_8</option>
                                    <option value="143" data-mode="4" <?php echo selected($config['preamble_structure'], "143"); ?>>[143] 32K, GI10_3648, Dx_8</option>
                                    <option value="144" data-mode="5" <?php echo selected($config['preamble_structure'], "144"); ?>>[144] 32K, GI10_3648, Dx_8</option>
                                    <option value="145" data-mode="1" <?php echo selected($config['preamble_structure'], "145"); ?>>[145] 32K, GI10_3648, Dx_3</option>
                                    <option value="146" data-mode="2" <?php echo selected($config['preamble_structure'], "146"); ?>>[146] 32K, GI10_3648, Dx_3</option>
                                    <option value="147" data-mode="3" <?php echo selected($config['preamble_structure'], "147"); ?>>[147] 32K, GI10_3648, Dx_3</option>
                                    <option value="148" data-mode="4" <?php echo selected($config['preamble_structure'], "148"); ?>>[148] 32K, GI10_3648, Dx_3</option>
                                    <option value="149" data-mode="5" <?php echo selected($config['preamble_structure'], "149"); ?>>[149] 32K, GI10_3648, Dx_3</option>
                                    <option value="150" data-mode="1" <?php echo selected($config['preamble_structure'], "150"); ?>>[150] 32K, GI11_4096, Dx_3</option>
                                    <option value="151" data-mode="2" <?php echo selected($config['preamble_structure'], "151"); ?>>[151] 32K, GI11_4096, Dx_3</option>
                                    <option value="152" data-mode="3" <?php echo selected($config['preamble_structure'], "152"); ?>>[152] 32K, GI11_4096, Dx_3</option>
                                    <option value="153" data-mode="4" <?php echo selected($config['preamble_structure'], "153"); ?>>[153] 32K, GI11_4096, Dx_3</option>
                                    <option value="154" data-mode="5" <?php echo selected($config['preamble_structure'], "154"); ?>>[154] 32K, GI11_4096, Dx_3</option>
                                    <option value="155" data-mode="1" <?php echo selected($config['preamble_structure'], "155"); ?>>[155] 32K, GI12_4864, Dx_3</option>
                                    <option value="156" data-mode="2" <?php echo selected($config['preamble_structure'], "156"); ?>>[156] 32K, GI12_4864, Dx_3</option>
                                    <option value="157" data-mode="3" <?php echo selected($config['preamble_structure'], "157"); ?>>[157] 32K, GI12_4864, Dx_3</option>
                                    <option value="158" data-mode="4" <?php echo selected($config['preamble_structure'], "158"); ?>>[158] 32K, GI12_4864, Dx_3</option>
                                    <option value="159" data-mode="5" <?php echo selected($config['preamble_structure'], "159"); ?>>[159] 32K, GI12_4864, Dx_3</option>
                                    <option value="160" data-mode="0" <?php echo selected($config['preamble_structure'], "160"); ?>>[160] reserved</option>
                                </select>
                            </div>

                            <div class="form-group">
                                <label for="number_of_frames">Number of Frames:</label>
                                <input name="number_of_frames" id="number_of_frames" type="number" placeholder="Number of Frames" min="0" max="100" required value="<?php echo htmlspecialchars($config['number_of_frames']); ?>">
                            </div>
                        </div>
                        </form>
                    </div>
                    <div id="preamble" class="config-section">
                        <form id="preamble_form" method="POST" action="">
                            <h3>Preamble</h3>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="l1b_version">L1B Version:</label>
                                    <input name="l1b_version" id="l1b_version" type="number" placeholder="L1B Version" min="0" max="1" required value="<?php echo htmlspecialchars($config['l1b_version']); ?>">
                                </div>

                                <div class="form-group">
                                    <label for="l1b_mimo_scatterred_pilot_encoding">L1B Mimo Scattered Pilot Encoding:</label>
                                    <select name="l1b_mimo_scatterred_pilot_encoding" id="l1b_mimo_scatterred_pilot_encoding" required>
                                        <option value="0" <?php echo selected($config['l1b_mimo_scatterred_pilot_encoding'], "0"); ?>>Walsh-Hadamard pilots or no MIMO subframes</option>
                                        <option value="1" <?php echo selected($config['l1b_mimo_scatterred_pilot_encoding'], "1"); ?>>Null pilots</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="detail_fec_type">Detail FEC Type:</label>
                                    <select name="detail_fec_type" id="detail_fec_type" required>
                                        <option value="0" <?php echo selected($config['detail_fec_type'], "0"); ?>>Mode 1</option>
                                        <option value="1" <?php echo selected($config['detail_fec_type'], "1"); ?>>Mode 2</option>
                                        <option value="2" <?php echo selected($config['detail_fec_type'], "2"); ?>>Mode 3</option>
                                        <option value="3" <?php echo selected($config['detail_fec_type'], "3"); ?>>Mode 4</option>
                                        <option value="4" <?php echo selected($config['detail_fec_type'], "4"); ?>>Mode 5</option>
                                        <option value="5" <?php echo selected($config['detail_fec_type'], "5"); ?>>Mode 6</option>
                                        <option value="6" <?php echo selected($config['detail_fec_type'], "6"); ?>>Mode 7</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="time_info_flag">Time Info Flag:</label>
                                    <select name="time_info_flag" id="time_info_flag" required>
                                        <option value="0" <?php echo selected($config['time_info_flag'], "0"); ?>>No time info</option>
                                        <option value="1" <?php echo selected($config['time_info_flag'], "1"); ?>>msec</option>
                                        <option value="2" <?php echo selected($config['time_info_flag'], "2"); ?>>usec</option>
                                        <option value="3" <?php echo selected($config['time_info_flag'], "3"); ?>>nsec</option>
                                    </select>
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="frame_lenght_mode">Frame Length Mode:</label>
                                    <select name="frame_lenght_mode" id="frame_lenght_mode" required>
                                        <option value="0" <?php echo selected($config['frame_lenght_mode'], "0"); ?>>Time-aligned</option>
                                        <option value="1" <?php echo selected($config['frame_lenght_mode'], "1"); ?>>Symbol-aligned</option>
                                    </select>
                                </div>

                                <div class="form-group" id="frame_lenght_container">
                                    <label for="frame_lenght">Frame Length:</label>
                                    <input name="frame_lenght" id="frame_lenght" type="number" placeholder="Frame Length" required value="<?php echo htmlspecialchars($config['frame_lenght']); ?>">
                                </div>
                            </div>

                            <div class="form-row">
                                <div class="form-group">
                                    <label for="l1d_version">L1D Version:</label>
                                    <input name="l1d_version" id="l1d_version" type="number" placeholder="L1D Version" min="0" max="2" required value="<?php echo htmlspecialchars($config['l1d_version']); ?>">
                                </div>

                                <div class="form-group" id="l1d_bsid_container" style="<?php echo ($config['l1d_version'] == 1) ? '' : 'display: none;'; ?>">
                                    <label for="l1d_bsid">L1D BSID:</label>
                                    <input name="l1d_bsid" id="l1d_bsid" type="number" placeholder="L1D BSID" min="0" max="65535" value="<?php echo htmlspecialchars($config['l1d_bsid']); ?>">
                                </div>
                            </div>

                            <div class="form-group">
                                <label for="number_of_subframes">Number of Subframes:</label>
                                <input name="number_of_subframes" id="number_of_subframes" type="number" placeholder="Number of Subframes" min="0" max="4" required value="<?php echo htmlspecialchars($config['number_of_subframes']); ?>">
                            </div>

                        </form>
                    </div>

                        </div>
                    </div>
                </div>
                </div>
            </div>

            <div class="right-panel">
                <div id="resultados">
                    <h3>Resultados</h3>

                    <div class="resultados-card">
                        <div class="resultados-card-title">Frame</div>
                        <div id="resultados-frame-duration-container">
                            <div class="preamble-fields-grid">
                                <div class="preamble-field">
                                    <span class="preamble-field-label">Duração do Frame:</span>
                                    <span class="preamble-field-value" id="resultados-frame-duration">—</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="resultados-card">
                        <div class="resultados-card-title">Preâmbulo</div>
                        <div id="resultados-preamble-cells">
                            <p class="resultados-placeholder">Os valores serão calculados automaticamente.</p>
                        </div>
                    </div>

                    <div class="resultados-card">
                        <div class="resultados-card-title">Subframes</div>
                        <div id="resultados-subframe-progress">
                            <p class="resultados-placeholder">Os valores serão calculados automaticamente.</p>
                        </div>
                    </div>

                    <div id="resultados-content"></div>
                </div>
            </div>
        </div>

    <div class="wizard-bottom-bar">
        <button class="wizard-btn wizard-btn-prev" id="wizard-prev-btn" onclick="wizardPrev()" disabled>Anterior</button>
        <div class="wizard-step-info" id="wizard-step-info">Step 1 of 4 - Bootstrap</div>
        <div class="wizard-btn-group">
            <button class="wizard-btn wizard-btn-next" id="wizard-next-btn" onclick="wizardNext()">Proximo</button>
            <button class="wizard-btn wizard-btn-generate" id="submit-btn" onclick="submitForm()">Gerar</button>
        </div>
    </div>
<script src="assets/dist/app.min.js"></script>

    <script>
        window.subframesData = <?php echo json_encode($subframes_data); ?>;
    </script>
</body>
</html>