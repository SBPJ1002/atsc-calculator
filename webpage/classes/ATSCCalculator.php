<?php

require_once __DIR__ . '/ATSCTables.php';

class ATSCCalculator
{
    public static function getPreambleDxFromStructure(int $preambleStructure): ?int
    {
        if (($preambleStructure >= 130 && $preambleStructure <= 134) ||
            ($preambleStructure >= 140 && $preambleStructure <= 144)) {
            return 8;
        }
        if (($preambleStructure >= 135 && $preambleStructure <= 139) ||
            ($preambleStructure >= 145 && $preambleStructure <= 149)) {
            return 3;
        }
        return null;
    }

    public static function getPreambleCells(string $fftName, int $giSamples, int $cred, ?int $preambleDx): ?int
    {
        $table = ATSCTables::$TABLE_7_2_PREAMBLE;

        if (!isset($table[$fftName])) return null;
        $fftData = $table[$fftName];

        if (!isset($fftData[$giSamples])) return null;
        $giData = $fftData[$giSamples];

        if (!isset($giData['cells'])) {
            $dxKey = $preambleDx ?: array_key_first($giData);
            if (!isset($giData[$dxKey])) return null;
            $giData = $giData[$dxKey];
        }

        $credIndex = max(0, min(4, $cred));
        return $giData['cells'][$credIndex];
    }

    public static function estimateL1DCells(int $l1dBytes, int $l1dFecMode): array
    {
        $x = max(25, $l1dBytes);

        switch ($l1dFecMode) {
            case 0: $cells = (int)round(48.4968 * $x + 1574.5629); break;
            case 1: $cells = (int)round(12.0000 * $x + 474.0000); break;
            case 2: $cells = (int)round(107.6082 * sqrt($x) + 43.9439); break;
            case 3: $cells = (int)round(3.8087 * $x + 242.6673); break;
            case 4: $cells = (int)round(2.3328 * $x + 145.3357); break;
            case 5: $cells = (int)round(1.6913 * $x + 81.2802); break;
            case 6: $cells = (int)round(1.1896 * $x + 54.9410); break;
            default: $cells = (int)round(1.1896 * $x + 54.9410); break;
        }

        return ['cells' => $cells, 'bytes' => $x];
    }

    public static function fftLabel(int $v): string
    {
        return ATSCTables::$FFT_VALUE_TO_NAME[$v] ?? '8K';
    }

    public static function parseSpLabel(?string $spValue): array
    {
        if ($spValue === null || $spValue === '') {
            return ['dx' => 12, 'group' => 'SP12to32', 'key' => 'SP12_2'];
        }

        $spPattern = ATSCTables::$SP_PATTERN_MAP[(string)$spValue] ?? 'SP12_2';

        if (!preg_match('/^SP(\d+)_(\d+)$/', $spPattern, $m)) {
            return ['dx' => 12, 'group' => 'SP12to32', 'key' => 'SP12_2'];
        }

        $dx = (int)$m[1];
        $group = ($dx >= 3 && $dx <= 8) ? 'SP3to8' : 'SP12to32';

        return ['dx' => $dx, 'group' => $group, 'key' => $spPattern];
    }

    public static function getSbsDataCells(string $fft, int $cred, string $spKey, int $spBoost): int
    {
        $table = ATSCTables::$SBS_ACTIVE_DATA_CELLS;

        if (!isset($table[$cred])) return 0;
        if (!isset($table[$cred][$spKey])) return 0;
        if (!isset($table[$cred][$spKey][$fft])) return 0;

        $boostIndex = max(0, min(4, $spBoost));
        $cells = $table[$cred][$spKey][$fft][$boostIndex];

        return $cells ?? 0;
    }

    public static function calculatePreambleFields(array $params): array
    {
        $fftValue = (int)($params['fftValue'] ?? 0);
        $giValue = (int)($params['giValue'] ?? 5);
        $preambleReducedCarriers = (int)($params['preambleReducedCarriers'] ?? 0);
        $preambleStructure = (int)($params['preambleStructure'] ?? 0);
        $l1dFecMode = (int)($params['l1dFecMode'] ?? 0);
        $l1dBytes = (int)($params['l1dBytes'] ?? 25);

        $fftName = ATSCTables::$FFT_VALUE_TO_NAME[$fftValue] ?? '8K';
        $giSamples = ATSCTables::$GI_VALUE_TO_SAMPLES[$giValue] ?? 1024;
        $preambleDx = self::getPreambleDxFromStructure($preambleStructure);

        $cellsInFirst = self::getPreambleCells($fftName, $giSamples, 4, $preambleDx) ?: 0;
        $cellsInNext = self::getPreambleCells($fftName, $giSamples, $preambleReducedCarriers, $preambleDx) ?: 0;

        $l1dResult = self::estimateL1DCells($l1dBytes, $l1dFecMode);
        $l1dCells = $l1dResult['cells'];
        $l1dBytes = $l1dResult['bytes'];

        $l1Total = ATSCTables::L1B_CELLS_FIXED + $l1dCells;

        $numPreambleSymbols = ($l1Total > $cellsInFirst) ? 2 : 1;

        $totalPreambleCells = $cellsInFirst + ($numPreambleSymbols - 1) * $cellsInNext;
        $plpCells = max(0, $totalPreambleCells - ATSCTables::L1B_CELLS_FIXED - $l1dCells);

        return [
            'l1bCells' => ATSCTables::L1B_CELLS_FIXED,
            'l1dCells' => $l1dCells,
            'l1dBytes' => $l1dBytes,
            'cellsInFirst' => $cellsInFirst,
            'cellsInNext' => $cellsInNext,
            'plpCells' => $plpCells,
            'numSymbols' => $numPreambleSymbols,
            'totalPreambleCells' => $totalPreambleCells,
            'fftName' => $fftName,
            'giSamples' => $giSamples,
            'preambleReducedCarriers' => $preambleReducedCarriers,
            'preambleDx' => $preambleDx,
        ];
    }

    public static function computePlpCapacity(array $params): array
    {
        $fft = $params['fft'] ?? '8K';
        $cred = (int)($params['cred'] ?? 0);
        $spLabelText = $params['spLabelText'] ?? '8';
        $spBoost = (int)($params['spBoost'] ?? 0);
        $numSymbols = (int)($params['numSymbols'] ?? 0);
        $sbsFirst = (bool)($params['sbsFirst'] ?? false);
        $sbsLast = (bool)($params['sbsLast'] ?? false);

        $sp = self::parseSpLabel($spLabelText);
        $table = ($sp['group'] === 'SP3to8') ? ATSCTables::$TABLE_7_3 : ATSCTables::$TABLE_7_4;

        if (!isset($table[$fft]) || !isset($table[$fft][$cred])) {
            return ['capacity' => 0, 'reason' => "Combination not found: {$sp['key']}/{$fft}/{$cred}"];
        }

        $row = $table[$fft][$cred];
        if (!isset($row[$sp['key']])) {
            return ['capacity' => 0, 'reason' => "SP pattern not found: {$sp['key']}"];
        }

        $cps = $row[$sp['key']];
        $nDataSymbols = $numSymbols - ($sbsFirst ? 1 : 0) - ($sbsLast ? 1 : 0);
        $nSbsSymbols = ($sbsFirst ? 1 : 0) + ($sbsLast ? 1 : 0);
        $sbsDataCells = self::getSbsDataCells($fft, $cred, $sp['key'], $spBoost);

        $capacity = ($nDataSymbols * $cps) + ($nSbsSymbols * $sbsDataCells);
        $capacitySBS = ($nSbsSymbols * $sbsDataCells);

        return [
            'capacity' => $capacity,
            'capacitySBS' => $capacitySBS,
            'cps' => $cps,
            'nDataSymbols' => $nDataSymbols,
            'nSbsSymbols' => $nSbsSymbols,
            'sbsDataCells' => $sbsDataCells,
            'fft' => $fft,
            'spBoost' => $spBoost,
            'cred' => $cred,
            'reason' => null,
        ];
    }

    public static function fillPlpSize(array $params): array
    {
        $preambleParams = $params['preambleConfig'] ?? [];
        $subframeConfig = $params['subframeConfig'] ?? [];

        $preambleFields = self::calculatePreambleFields($preambleParams);
        $plpCells = $preambleFields['plpCells'];

        $plpCapacity = self::computePlpCapacity($subframeConfig);

        if ($plpCapacity['capacity'] <= 0) {
            return [
                'error' => true,
                'reason' => $plpCapacity['reason'],
                'totalSize' => 0,
                'plpCapacity' => $plpCapacity,
                'plpCells' => $plpCells,
                'preambleFields' => $preambleFields,
            ];
        }

        $totalSize = $plpCapacity['capacity'] + $plpCells;

        return [
            'error' => false,
            'totalSize' => $totalSize,
            'plpCapacity' => $plpCapacity,
            'plpCells' => $plpCells,
            'preambleFields' => $preambleFields,
        ];
    }

    public static function batchFillAllPlps(array $params): array
    {
        $preambleConfig = $params['preambleConfig'] ?? [];
        $subframesConfigs = $params['subframesConfigs'] ?? [];

        $preambleFields = self::calculatePreambleFields($preambleConfig);
        $plpCells = $preambleFields['plpCells'];

        $results = [];
        foreach ($subframesConfigs as $sfConfig) {
            $plps = $sfConfig['plps'] ?? [$sfConfig];
            $sfResults = [];

            foreach ($plps as $plpConfig) {
                $plpCapacity = self::computePlpCapacity($plpConfig);

                if ($plpCapacity['capacity'] <= 0) {
                    $sfResults[] = [
                        'error' => true,
                        'reason' => $plpCapacity['reason'],
                        'totalSize' => 0,
                        'plpCapacity' => $plpCapacity,
                    ];
                } else {
                    $sfResults[] = [
                        'error' => false,
                        'totalSize' => $plpCapacity['capacity'] + $plpCells,
                        'plpCapacity' => $plpCapacity,
                    ];
                }
            }

            $results[] = $sfResults;
        }

        return [
            'results' => $results,
            'preambleFields' => $preambleFields,
            'plpCells' => $plpCells,
        ];
    }

    public static function getL1dSizeBytes(): array
    {
        $logDir = __DIR__ . '/../../server_generator/config/log/';

        if (!is_dir($logDir)) {
            return ['error' => 'Log directory not found', 'bytes' => 25];
        }

        $logFiles = glob($logDir . 'Frame_*.log');
        if (empty($logFiles)) {
            return ['error' => 'No log files found', 'bytes' => 25];
        }

        usort($logFiles, function ($a, $b) {
            return filemtime($b) - filemtime($a);
        });

        $content = file_get_contents($logFiles[0]);
        if ($content === false) {
            return ['error' => 'Cannot read log file', 'bytes' => 25];
        }

        if (preg_match('/L1B_L1_Detail_size_bytes\s*→\s*(\d+)\s*bytes/', $content, $matches)) {
            return ['bytes' => max(25, (int)$matches[1]), 'source' => basename($logFiles[0])];
        }

        return ['error' => 'L1B_L1_Detail_size_bytes not found in log', 'bytes' => 25];
    }

    public static function getFrame2LogData(): array
    {
        $logDir = __DIR__ . '/../../server_generator/config/log/';
        $logFile = $logDir . 'Frame_2.log';

        if (!file_exists($logFile)) {
            return ['error' => 'Frame_2.log not found'];
        }

        $content = file_get_contents($logFile);
        if ($content === false) {
            return ['error' => 'Cannot read Frame_2.log'];
        }

        $timeOffset = null;
        if (preg_match('/L1B_time_offset\s*→\s*(\d+)/', $content, $m)) {
            $timeOffset = (int)$m[1];
        }

        $fecBlockStarts = [];

        if (preg_match_all('/L1D_plp_id\s*→\s*(\d+)(.*?)(?=L1D_plp_id|L1D_reserved|$)/s', $content, $plpMatches, PREG_SET_ORDER)) {
            foreach ($plpMatches as $plpMatch) {
                $plpId = (int)$plpMatch[1];
                $plpSection = $plpMatch[2];
                $fecStart = null;
                if (preg_match('/L1D_plp_fec_block_start\s*→\s*(\d+)/', $plpSection, $fecMatch)) {
                    $fecStart = (int)$fecMatch[1];
                }
                $fecBlockStarts[] = ['plpId' => $plpId, 'fecBlockStart' => $fecStart];
            }
        }

        return [
            'timeOffset' => $timeOffset,
            'fecBlockStarts' => $fecBlockStarts,
            'source' => 'Frame_2.log',
        ];
    }

    public static function getFrameDuration(): array
    {
        $durationFile = __DIR__ . '/../../server_generator/config/log/frame_duration.txt';

        if (!file_exists($durationFile)) {
            return ['error' => 'frame_duration.txt not found', 'totalDurationMs' => null, 'subframeDurations' => []];
        }

        $content = file_get_contents($durationFile);
        if ($content === false) {
            return ['error' => 'Cannot read frame_duration.txt', 'totalDurationMs' => null, 'subframeDurations' => []];
        }

        $totalDurationMs = null;
        $subframeDurations = [];

        foreach (explode("\n", $content) as $line) {
            $line = trim($line);
            if ($line === '') continue;

            if (preg_match('/^frame_duration_ms=(.+)$/', $line, $m)) {
                $totalDurationMs = (float)$m[1];
            } elseif (preg_match('/^subframe_(\d+)_duration_ms=(.+)$/', $line, $m)) {
                $subframeDurations[(int)$m[1]] = (float)$m[2];
            }
        }

        return [
            'totalDurationMs' => $totalDurationMs,
            'subframeDurations' => $subframeDurations,
        ];
    }

    public static function getValidationTables(): array
    {
        $ALL_PILOT_PATTERNS = ['0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15'];

        return [
            'pilotPatternValueToName' => ATSCTables::$SP_PATTERN_MAP,
            'pilotPatternNameToValue' => array_flip(ATSCTables::$SP_PATTERN_MAP),
            'preambleConfigurations' => self::buildPreambleConfigurations(),
            'allowedPatternsSiso' => [
                '8K' => [
                    'GI1_192'=>['10','11','14','15'], 'GI2_384'=>['6','7','10','11'],
                    'GI3_512'=>['4','5','8','9'], 'GI4_768'=>['2','3','6','7'],
                    'GI5_1024'=>['0','1','4','5'], 'GI6_1536'=>['2','3'],
                    'GI7_2048'=>['0','1'],
                    'GI8_2432'=>[], 'GI9_3072'=>[], 'GI10_3648'=>[], 'GI11_4096'=>[], 'GI12_4864'=>[],
                ],
                '16K' => [
                    'GI1_192'=>['14','15'], 'GI2_384'=>['10','11','14','15'],
                    'GI3_512'=>['8','9','12','13'], 'GI4_768'=>['6','7','10','11'],
                    'GI5_1024'=>['4','5','8','9'], 'GI6_1536'=>['2','3','6','7'],
                    'GI7_2048'=>['0','1','4','5'], 'GI8_2432'=>['0','1','4','5'],
                    'GI9_3072'=>['2','3'], 'GI10_3648'=>['2','3'],
                    'GI11_4096'=>['0','1'], 'GI12_4864'=>[],
                ],
                '32K' => [
                    'GI1_192'=>['14'], 'GI2_384'=>['14'], 'GI3_512'=>['12'],
                    'GI4_768'=>['10','14'], 'GI5_1024'=>['8','12'],
                    'GI6_1536'=>['6','10'], 'GI7_2048'=>['4','8'],
                    'GI8_2432'=>['4','8'], 'GI9_3072'=>['0','6'],
                    'GI10_3648'=>['0','6'], 'GI11_4096'=>['0','4'],
                    'GI12_4864'=>['0','4'],
                ],
            ],
            'allPilotPatterns' => $ALL_PILOT_PATTERNS,
            'modulationCodingTables' => [
                '64K' => [
                    '0'=>['0','1','2','3','4','5','6','7','9'],
                    '1'=>['2','3','5','6','7','9'],
                    '2'=>['1','2','3','4','5','6','7','8','9'],
                    '3'=>['2','3','5','6','7','8','9','10','11'],
                    '4'=>['3','5','6','7','8','9','10','11'],
                    '5'=>['5','7','9','10','11'],
                ],
                '16K' => [
                    '0'=>['0','1','2','3','4','5','6','7'],
                    '1'=>['3','4','5','6','9'],
                    '2'=>['3','4','5','6','7','8','9'],
                    '3'=>['3','5','6','7','8','9','10','11'],
                ],
            ],
            'fecTypeMap' => [
                '0'=>'16K','1'=>'64K','2'=>'16K','3'=>'64K','4'=>'16K','5'=>'64K',
            ],
            'modulationMap' => [
                '0'=>'QPSK','1'=>'16QAM','2'=>'64QAM','3'=>'256QAM','4'=>'1024QAM','5'=>'4096QAM',
            ],
            'codeRateMap' => [
                '0'=>'2/15','1'=>'3/15','2'=>'4/15','3'=>'5/15',
                '4'=>'6/15','5'=>'7/15','6'=>'8/15','7'=>'9/15',
                '8'=>'10/15','9'=>'11/15','10'=>'12/15','11'=>'13/15',
            ],
        ];
    }

    private static function buildPreambleConfigurations(): array
    {
        $configs = [];
        $entries = [
            ['8K', 'GI1_192', 16], ['8K', 'GI2_384', 8], ['8K', 'GI3_512', 6],
            ['8K', 'GI4_768', 4], ['8K', 'GI5_1024', 3], ['8K', 'GI6_1536', 4],
            ['8K', 'GI7_2048', 3],
            ['16K', 'GI1_192', 32], ['16K', 'GI2_384', 16], ['16K', 'GI3_512', 12],
            ['16K', 'GI4_768', 8], ['16K', 'GI5_1024', 6], ['16K', 'GI6_1536', 4],
            ['16K', 'GI7_2048', 3], ['16K', 'GI8_2432', 3], ['16K', 'GI9_3072', 4],
            ['16K', 'GI10_3648', 4], ['16K', 'GI11_4096', 3],
            ['32K', 'GI1_192', 32], ['32K', 'GI2_384', 32], ['32K', 'GI3_512', 24],
            ['32K', 'GI4_768', 16], ['32K', 'GI5_1024', 12], ['32K', 'GI6_1536', 8],
            ['32K', 'GI7_2048', 6], ['32K', 'GI8_2432', 6],
            ['32K', 'GI9_3072', 8], ['32K', 'GI9_3072', 3],
            ['32K', 'GI10_3648', 8], ['32K', 'GI10_3648', 3],
            ['32K', 'GI11_4096', 3], ['32K', 'GI12_4864', 3],
        ];

        foreach ($entries as [$fft, $gi, $dx]) {
            for ($mode = 1; $mode <= 5; $mode++) {
                $configs[] = ['fft' => $fft, 'gi' => $gi, 'dx' => $dx, 'mode' => $mode];
            }
        }

        return $configs;
    }

    public static function handleRequest(array $input): array
    {
        $action = $input['action'] ?? '';

        switch ($action) {
            case 'calculatePreambleFields':
                return self::calculatePreambleFields($input);
            case 'computePlpCapacity':
                return self::computePlpCapacity($input);
            case 'fillPlpSize':
                return self::fillPlpSize($input);
            case 'batchFillAllPlps':
                return self::batchFillAllPlps($input);
            case 'getL1dSizeBytes':
                return self::getL1dSizeBytes();
            case 'getFrame2LogData':
                return self::getFrame2LogData();
            case 'getFrameDuration':
                return self::getFrameDuration();
            case 'getValidationTables':
                return self::getValidationTables();
            default:
                return ['error' => "Unknown action: {$action}"];
        }
    }
}
