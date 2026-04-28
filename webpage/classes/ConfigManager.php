<?php

class ConfigManager
{
    public static function getDefaultConfig(): array
    {
        return [
            'major_version' => 0,
            'minor_version' => 0,
            'bootstrap_symbol' => 0,
            'ea_wakeup' => 0,
            'system_bandwidth' => 0,
            'bsr_coefficient' => 0,
            'min_time_to_next' => 0,
            'preamble_structure' => 0,
            'number_of_frames' => 0,
            'l1b_version' => 0,
            'l1b_mimo_scatterred_pilot_encoding' => 0,
            'l1d_version' => 0,
            'l1d_bsid' => 0,
            'detail_size_bytes' => 25,
            'detail_fec_type' => 0,
            'time_info_flag' => 0,
            'frame_lenght_mode' => 0,
            'frame_lenght' => 0,
            'number_of_subframes' => 1,
        ];
    }

    public static function buildJsonPayload(array $post): array
    {
        $payload = ['flag' => 'setConfig'];

        $intFields = [
            'major_version', 'minor_version', 'bootstrap_symbol', 'ea_wakeup',
            'system_bandwidth', 'bsr_coefficient', 'min_time_to_next', 'preamble_structure',
            'number_of_frames', 'l1b_version', 'l1b_mimo_scatterred_pilot_encoding',
            'l1d_version', 'l1d_bsid', 'detail_fec_type', 'time_info_flag',
            'frame_lenght_mode', 'frame_lenght', 'number_of_subframes',
        ];

        foreach ($intFields as $field) {
            if (isset($post[$field])) {
                $payload[$field] = intval($post[$field]);
            }
        }

        $numSubframes = isset($post['number_of_subframes']) ? intval($post['number_of_subframes']) : 0;
        $subframes = [];

        for ($i = 0; $i < $numSubframes; $i++) {
            $sf = [];

            $subframeFields = [
                "plp_mimo_{$i}" => 'plp_mimo',
                "plp_mimo_mixed_{$i}" => 'plp_mimo_mixed',
                "plp_miso_{$i}" => 'plp_miso',
                "fft_size_{$i}" => 'fft_size',
                "reduced_carrier_{$i}" => 'reduced_carrier',
                "guard_interval_{$i}" => 'guard_interval',
                "num_ofdm_{$i}" => 'num_ofdm',
                "spilot_pattern_{$i}" => 'spilot_pattern',
                "spilot_boost_{$i}" => 'spilot_boost',
                "sbs_first_{$i}" => 'sbs_first',
                "sbs_last_{$i}" => 'sbs_last',
                "freq_interleaver_{$i}" => 'freq_interleaver',
            ];

            foreach ($subframeFields as $postKey => $jsonKey) {
                if (isset($post[$postKey])) {
                    $sf[$jsonKey] = intval($post[$postKey]);
                }
            }

            // PLPs
            $numPlps = 0;
            if (isset($post["plp-count-{$i}"])) {
                $numPlps = intval($post["plp-count-{$i}"]);
            } elseif (isset($post["plp_count_{$i}"])) {
                $numPlps = intval($post["plp_count_{$i}"]);
            }

            if ($numPlps > 0) {
                $plps = [];
                $plpParams = [
                    'plp_id' => 'plp_id', 'lls_flag' => 'lls_flag', 'layer' => 'layer',
                    'start' => 'start', 'size' => 'size', 'fec_type' => 'fec_type',
                    'mod_order' => 'mod_order', 'code_rate' => 'code_rate',
                    'ti_mode' => 'ti_mode', 'ti_extended' => 'ti_extended',
                    'cti_depth' => 'cti_depth', 'mimo_plp' => 'mimo_plp',
                    'plp_mimo_stream_combining' => 'stream_combining',
                    'plp_mimo_IQ_intervaling' => 'iq_intervaling',
                    'plp_mimo_PH' => 'phase_hopping', 'plp_type' => 'plp_type',
                    'num_subslice' => 'num_subslice', 'subslice_interval' => 'subslice_interval',
                    'cell_intervaler' => 'cell_intervaler', 'inter_subframe' => 'inter_subframe',
                    'num_ti_blocks' => 'num_ti_blocks', 'num_fec_blocks_max' => 'num_fec_blocks_max',
                    'num_fec_blocks' => 'num_fec_blocks', 'inter_ldm_injection_level' => 'ldm_injection_level',
                ];

                for ($j = 0; $j < $numPlps; $j++) {
                    $plp = [];
                    foreach ($plpParams as $paramName => $jsonKey) {
                        $fieldKey = "{$paramName}_{$i}_{$j}";
                        if (isset($post[$fieldKey])) {
                            $plp[$jsonKey] = intval($post[$fieldKey]);
                        }
                    }
                    $plps[] = $plp;
                }
                $sf['plps'] = $plps;
            }

            $subframes[] = $sf;
        }

        if (!empty($subframes)) {
            $payload['subframes'] = $subframes;
        }

        return $payload;
    }

    public static function parseServerResponse(array $response): array
    {
        if (!isset($response['config'])) {
            return ['config' => self::getDefaultConfig(), 'subframes' => []];
        }

        return [
            'config' => $response['config'],
            'subframes' => $response['subframes'] ?? [],
        ];
    }
}
