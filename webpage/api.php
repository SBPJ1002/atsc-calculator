<?php
header('Content-Type: application/json');
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
    echo json_encode(['error' => 'Method not allowed']);
    exit;
}

$input = json_decode(file_get_contents('php://input'), true);
if (!$input || !isset($input['action'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Missing action parameter']);
    exit;
}

$TABLE_7_2_PREAMBLE = [
    "8K" => [
        192  => ["Dx" => 16, "cells" => [6432, 6342, 6253, 6164, 6075]],
        384  => ["Dx" => 8,  "cells" => [6000, 5916, 5833, 5750, 5667]],
        512  => ["Dx" => 6,  "cells" => [5712, 5632, 5553, 5474, 5395]],
        768  => ["Dx" => 4,  "cells" => [5136, 5064, 4993, 4922, 4851]],
        1024 => ["Dx" => 3,  "cells" => [4560, 4496, 4433, 4370, 4307]],
        1536 => ["Dx" => 4,  "cells" => [5136, 5064, 4993, 4922, 4851]],
        2048 => ["Dx" => 3,  "cells" => [4560, 4496, 4433, 4370, 4307]],
    ],
    "16K" => [
        192  => ["Dx" => 32, "cells" => [13296, 13110, 12927, 12742, 12558]],
        384  => ["Dx" => 16, "cells" => [12864, 12684, 12507, 12328, 12150]],
        512  => ["Dx" => 12, "cells" => [12576, 12400, 12227, 12052, 11878]],
        768  => ["Dx" => 8,  "cells" => [12000, 11832, 11667, 11500, 11334]],
        1024 => ["Dx" => 6,  "cells" => [11424, 11264, 11107, 10948, 10790]],
        1536 => ["Dx" => 4,  "cells" => [10272, 10128, 9987, 9844, 9702]],
        2048 => ["Dx" => 3,  "cells" => [9120, 8992, 8867, 8740, 8614]],
        2432 => ["Dx" => 3,  "cells" => [9120, 8992, 8867, 8740, 8614]],
        3072 => ["Dx" => 4,  "cells" => [10272, 10128, 9987, 9844, 9702]],
        3648 => ["Dx" => 4,  "cells" => [10272, 10128, 9987, 9844, 9702]],
        4096 => ["Dx" => 3,  "cells" => [9120, 8992, 8867, 8740, 8614]],
    ],
    "32K" => [
        192  => ["Dx" => 32, "cells" => [26592, 26220, 25854, 25484, 25116]],
        384  => ["Dx" => 32, "cells" => [26592, 26220, 25854, 25484, 25116]],
        512  => ["Dx" => 24, "cells" => [26304, 25936, 25574, 25208, 24844]],
        768  => ["Dx" => 16, "cells" => [25728, 25368, 25014, 24656, 24300]],
        1024 => ["Dx" => 12, "cells" => [25152, 24800, 24454, 24104, 23756]],
        1536 => ["Dx" => 8,  "cells" => [24000, 23664, 23334, 23000, 22668]],
        2048 => ["Dx" => 6,  "cells" => [22848, 22528, 22214, 21896, 21580]],
        2432 => ["Dx" => 6,  "cells" => [22848, 22528, 22214, 21896, 21580]],
        3072 => [
            8 => ["Dx" => 8, "cells" => [24000, 23664, 23334, 23000, 22668]],
            3 => ["Dx" => 3, "cells" => [18240, 17984, 17734, 17480, 17228]],
        ],
        3648 => [
            8 => ["Dx" => 8, "cells" => [24000, 23664, 23334, 23000, 22668]],
            3 => ["Dx" => 3, "cells" => [18240, 17984, 17734, 17480, 17228]],
        ],
        4096 => ["Dx" => 3,  "cells" => [18240, 17984, 17734, 17480, 17228]],
        4864 => ["Dx" => 3,  "cells" => [18240, 17984, 17734, 17480, 17228]],
    ],
];

$GI_VALUE_TO_SAMPLES = [
    1 => 192, 2 => 384, 3 => 512, 4 => 768, 5 => 1024,
    6 => 1536, 7 => 2048, 8 => 2432, 9 => 3072, 10 => 3648,
    11 => 4096, 12 => 4864,
];

$FFT_VALUE_TO_NAME = [
    0 => "8K", 1 => "16K", 2 => "32K",
];

$L1B_CELLS_FIXED = 3820;

$TABLE_7_3 = [
    "8K" => [
        0 => ["SP3_2"=>5711,"SP3_4"=>6285,"SP4_2"=>5999,"SP4_4"=>6429,"SP6_2"=>6287,"SP6_4"=>6573,"SP8_2"=>6431,"SP8_4"=>6645],
        1 => ["SP3_2"=>5631,"SP3_4"=>6197,"SP4_2"=>5915,"SP4_4"=>6339,"SP6_2"=>6199,"SP6_4"=>6481,"SP8_2"=>6341,"SP8_4"=>6552],
        2 => ["SP3_2"=>5552,"SP3_4"=>6110,"SP4_2"=>5832,"SP4_4"=>6250,"SP6_2"=>6112,"SP6_4"=>6390,"SP8_2"=>6252,"SP8_4"=>6460],
        3 => ["SP3_2"=>5473,"SP3_4"=>6023,"SP4_2"=>5749,"SP4_4"=>6161,"SP6_2"=>6025,"SP6_4"=>6299,"SP8_2"=>6163,"SP8_4"=>6368],
        4 => ["SP3_2"=>5394,"SP3_4"=>5936,"SP4_2"=>5666,"SP4_4"=>6072,"SP6_2"=>5938,"SP6_4"=>6208,"SP8_2"=>6074,"SP8_4"=>6276],
    ],
    "16K" => [
        0 => ["SP3_2"=>11423,"SP3_4"=>12573,"SP4_2"=>11999,"SP4_4"=>12861,"SP6_2"=>12575,"SP6_4"=>13149,"SP8_2"=>12863,"SP8_4"=>13293],
        1 => ["SP3_2"=>11263,"SP3_4"=>12397,"SP4_2"=>11831,"SP4_4"=>12681,"SP6_2"=>12399,"SP6_4"=>12965,"SP8_2"=>12683,"SP8_4"=>13107],
        2 => ["SP3_2"=>11106,"SP3_4"=>12224,"SP4_2"=>11666,"SP4_4"=>12504,"SP6_2"=>12226,"SP6_4"=>12784,"SP8_2"=>12506,"SP8_4"=>12924],
        3 => ["SP3_2"=>10947,"SP3_4"=>12049,"SP4_2"=>11499,"SP4_4"=>12325,"SP6_2"=>12051,"SP6_4"=>12601,"SP8_2"=>12327,"SP8_4"=>12739],
        4 => ["SP3_2"=>10789,"SP3_4"=>11875,"SP4_2"=>11333,"SP4_4"=>12147,"SP6_2"=>11877,"SP6_4"=>12419,"SP8_2"=>12149,"SP8_4"=>12555],
    ],
    "32K" => [
        0 => ["SP3_2"=>22847,"SP3_4"=>25149,"SP6_2"=>25151,"SP6_4"=>26301,"SP8_2"=>25727,"SP8_4"=>26589],
        1 => ["SP3_2"=>22527,"SP3_4"=>24797,"SP6_2"=>24799,"SP6_4"=>25933,"SP8_2"=>25367,"SP8_4"=>26217],
        2 => ["SP3_2"=>22213,"SP3_4"=>24451,"SP6_2"=>24453,"SP6_4"=>25571,"SP8_2"=>25013,"SP8_4"=>25851],
        3 => ["SP3_2"=>21895,"SP3_4"=>24101,"SP6_2"=>24103,"SP6_4"=>25205,"SP8_2"=>24655,"SP8_4"=>25481],
        4 => ["SP3_2"=>21579,"SP3_4"=>23753,"SP6_2"=>23755,"SP6_4"=>24841,"SP8_2"=>24299,"SP8_4"=>25113],
    ],
];

$TABLE_7_4 = [
    "8K" => [
        0 => ["SP12_2"=>6575,"SP12_4"=>6717,"SP16_2"=>6647,"SP16_4"=>6753,"SP24_2"=>6719,"SP24_4"=>6789,"SP32_2"=>6755,"SP32_4"=>6807],
        1 => ["SP12_2"=>6483,"SP12_4"=>6623,"SP16_2"=>6554,"SP16_4"=>6660,"SP24_2"=>6625,"SP24_4"=>6694,"SP32_2"=>6661,"SP32_4"=>6714],
        2 => ["SP12_2"=>6392,"SP12_4"=>6530,"SP16_2"=>6462,"SP16_4"=>6565,"SP24_2"=>6532,"SP24_4"=>6600,"SP32_2"=>6567,"SP32_4"=>6619],
        3 => ["SP12_2"=>6301,"SP12_4"=>6437,"SP16_2"=>6370,"SP16_4"=>6473,"SP24_2"=>6439,"SP24_4"=>6506,"SP32_2"=>6474,"SP32_4"=>6524],
        4 => ["SP12_2"=>6210,"SP12_4"=>6344,"SP16_2"=>6278,"SP16_4"=>6378,"SP24_2"=>6346,"SP24_4"=>6412,"SP32_2"=>6380,"SP32_4"=>6429],
    ],
    "16K" => [
        0 => ["SP12_2"=>13151,"SP12_4"=>13437,"SP16_2"=>13295,"SP16_4"=>13509,"SP24_2"=>13439,"SP24_4"=>13581,"SP32_2"=>13511,"SP32_4"=>13617],
        1 => ["SP12_2"=>12967,"SP12_4"=>13249,"SP16_2"=>13109,"SP16_4"=>13320,"SP24_2"=>13251,"SP24_4"=>13391,"SP32_2"=>13322,"SP32_4"=>13428],
        2 => ["SP12_2"=>12786,"SP12_4"=>13064,"SP16_2"=>12926,"SP16_4"=>13134,"SP24_2"=>13066,"SP24_4"=>13204,"SP32_2"=>13136,"SP32_4"=>13239],
        3 => ["SP12_2"=>12603,"SP12_4"=>12877,"SP16_2"=>12741,"SP16_4"=>12946,"SP24_2"=>12879,"SP24_4"=>13015,"SP32_2"=>12948,"SP32_4"=>13051],
        4 => ["SP12_2"=>12421,"SP12_4"=>12691,"SP16_2"=>12557,"SP16_4"=>12759,"SP24_2"=>12693,"SP24_4"=>12827,"SP32_2"=>12761,"SP32_4"=>12861],
    ],
    "32K" => [
        0 => ["SP12_2"=>26303,"SP12_4"=>26877,"SP16_2"=>26591,"SP16_4"=>27021,"SP24_2"=>26879,"SP24_4"=>27165,"SP32_2"=>27023,"SP32_4"=>27237],
        1 => ["SP12_2"=>25935,"SP12_4"=>26501,"SP16_2"=>26219,"SP16_4"=>26643,"SP24_2"=>26503,"SP24_4"=>26785,"SP32_2"=>26645,"SP32_4"=>26856],
        2 => ["SP12_2"=>25573,"SP12_4"=>26131,"SP16_2"=>25853,"SP16_4"=>26271,"SP24_2"=>26133,"SP24_4"=>26411,"SP32_2"=>26273,"SP32_4"=>26481],
        3 => ["SP12_2"=>25207,"SP12_4"=>25757,"SP16_2"=>25483,"SP16_4"=>25895,"SP24_2"=>25759,"SP24_4"=>26033,"SP32_2"=>25897,"SP32_4"=>26102],
        4 => ["SP12_2"=>24843,"SP12_4"=>25385,"SP16_2"=>25115,"SP16_4"=>25521,"SP24_2"=>25387,"SP24_4"=>25657,"SP32_2"=>25523,"SP32_4"=>25725],
    ],
];

$SBS_ACTIVE_DATA_CELLS = [
    0 => [
        "SP3_2" => ["8K" => [4560,4560,4123,3801,3467], "16K" => [9120,9120,8244,7601,6933], "32K" => [18240,18240,16488,15202,13865]],
        "SP3_4" => ["8K" => [4560,3904,2922,2148,1534], "16K" => [9120,7807,5841,4290,3063], "32K" => [null,null,null,null,null]],
        "SP4_2" => ["8K" => [5136,5009,4600,4278,4022], "16K" => [10272,10017,9199,8554,8043], "32K" => [null,null,null,null,null]],
        "SP4_4" => ["8K" => [5136,4332,3467,2868,2245], "16K" => [10272,8663,6930,5731,4484], "32K" => [null,null,null,null,null]],
        "SP6_2" => ["8K" => [5712,5456,5114,4843,4629], "16K" => [11424,10912,10225,9684,9256], "32K" => [22848,21823,20449,19367,18510]],
        "SP6_4" => ["8K" => [5712,4856,4147,3588,3146], "16K" => [11424,9708,8288,7168,6282], "32K" => [null,null,null,null,null]],
        "SP8_2" => ["8K" => [6000,5716,5398,5188,4971], "16K" => [12000,11431,10793,10375,9939], "32K" => [24000,22861,21585,20747,19876]],
        "SP8_4" => ["8K" => [6000,5168,4558,4078,3697], "16K" => [12000,10331,9109,8146,7383], "32K" => [null,null,null,null,null]],
        "SP12_2" => ["8K" => [6288,5976,5729,5533,5379], "16K" => [12576,11950,11455,11064,10755], "32K" => [25152,23899,22907,22124,21505]],
        "SP12_4" => ["8K" => [6288,5508,5010,4616,4305], "16K" => [12576,11011,10010,9221,8596], "32K" => [null,null,null,null,null]],
        "SP16_2" => ["8K" => [6432,6132,5919,5751,5618], "16K" => [12864,12262,11835,11499,11233], "32K" => [25728,24521,23667,22994,22461]],
        "SP16_4" => ["8K" => [6432,5691,5252,4906,4633], "16K" => [12864,11374,10493,9798,9248], "32K" => [null,null,null,null,null]],
        "SP24_2" => ["8K" => [null,null,null,null,null], "16K" => [13152,12593,12243,11968,11750], "32K" => [26304,25183,24483,23931,23494]],
        "SP24_4" => ["8K" => [null,null,null,null,null], "16K" => [13152,11834,11113,10544,10094], "32K" => [null,null,null,null,null]],
        "SP32_2" => ["8K" => [6648,6384,6231,6125,6015], "16K" => [13296,12766,12458,12245,12024], "32K" => [26592,25529,24913,24486,24042]],
        "SP32_4" => ["8K" => [6648,6064,5757,5515,5324], "16K" => [13296,12116,11497,11008,10622], "32K" => [null,null,null,null,null]],
    ],
    1 => [
        "SP3_2" => ["8K" => [4496,4496,4065,3748,3418], "16K" => [8992,8992,8129,7495,6835], "32K" => [17984,17984,16256,14988,13669]],
        "SP3_4" => ["8K" => [4496,3849,2881,2117,1513], "16K" => [8992,7697,5758,4229,3019], "32K" => [null,null,null,null,null]],
        "SP4_2" => ["8K" => [5064,4938,4535,4218,3966], "16K" => [10128,9876,9070,8434,7930], "32K" => [null,null,null,null,null]],
        "SP4_4" => ["8K" => [5064,4272,3419,2828,2214], "16K" => [10128,8541,6833,5650,4420], "32K" => [null,null,null,null,null]],
        "SP6_2" => ["8K" => [5632,5380,5042,4775,4564], "16K" => [11264,10759,10082,9549,9126], "32K" => [22528,21517,20163,19095,18250]],
        "SP6_4" => ["8K" => [5632,4788,4089,3538,3102], "16K" => [11264,9572,8171,7068,6194], "32K" => [null,null,null,null,null]],
        "SP8_2" => ["8K" => [5916,5636,5322,5116,4901], "16K" => [11832,11271,10642,10229,9800], "32K" => [23664,22541,21282,20456,19597]],
        "SP8_4" => ["8K" => [5916,5096,4494,4021,3645], "16K" => [11832,10187,8982,8032,7280], "32K" => [null,null,null,null,null]],
        "SP12_2" => ["8K" => [6200,5892,5648,5456,5304], "16K" => [12400,11783,11294,10909,10604], "32K" => [24800,23564,22586,21815,21204]],
        "SP12_4" => ["8K" => [6200,5431,4940,4552,4245], "16K" => [12400,10857,9870,9091,8475], "32K" => [null,null,null,null,null]],
        "SP16_2" => ["8K" => [6342,6046,5836,5671,5540], "16K" => [12684,12090,11669,11338,11075], "32K" => [25368,24178,23336,22672,22146]],
        "SP16_4" => ["8K" => [6342,5608,5173,4831,4559], "16K" => [12684,11215,10346,9661,9118], "32K" => [null,null,null,null,null]],
        "SP24_2" => ["8K" => [null,null,null,null,null], "16K" => [12968,12416,12072,11800,11585], "32K" => [25936,24830,24140,23596,23165]],
        "SP24_4" => ["8K" => [null,null,null,null,null], "16K" => [12968,11668,10957,10397,9953], "32K" => [null,null,null,null,null]],
        "SP32_2" => ["8K" => [6555,6294,6142,6037,5928], "16K" => [13110,12587,12284,12074,11856], "32K" => [26220,25172,24564,24143,23705]],
        "SP32_4" => ["8K" => [6555,5971,5664,5422,5231], "16K" => [13110,11941,11327,10844,10461], "32K" => [null,null,null,null,null]],
    ],
    2 => [
        "SP3_2" => ["8K" => [4433,4433,4008,3695,3371], "16K" => [8867,8867,8016,7391,6741], "32K" => [17734,17734,16031,14780,13480]],
        "SP3_4" => ["8K" => [4433,3796,2841,2088,1492], "16K" => [8867,7591,5679,4172,2979], "32K" => [null,null,null,null,null]],
        "SP4_2" => ["8K" => [4993,4869,4472,4158,3910], "16K" => [9987,9739,8943,8316,7820], "32K" => [null,null,null,null,null]],
        "SP4_4" => ["8K" => [4993,4212,3371,2788,2183], "16K" => [9987,8422,6738,5572,4360], "32K" => [null,null,null,null,null]],
        "SP6_2" => ["8K" => [5553,5304,4971,4708,4500], "16K" => [11107,10609,9942,9416,8999], "32K" => [22214,21217,19882,18829,17996]],
        "SP6_4" => ["8K" => [5553,4720,4032,3488,3058], "16K" => [11107,9438,8058,6970,6108], "32K" => [null,null,null,null,null]],
        "SP8_2" => ["8K" => [5833,5557,5247,5044,4833], "16K" => [11667,11114,10494,10087,9664], "32K" => [23334,22227,20986,20171,19324]],
        "SP8_4" => ["8K" => [5833,5024,4432,3964,3595], "16K" => [11667,10045,8857,7920,7179], "32K" => [null,null,null,null,null]],
        "SP12_2" => ["8K" => [6113,5810,5569,5380,5229], "16K" => [12227,11619,11137,10757,10456], "32K" => [24454,23236,22271,21511,20909]],
        "SP12_4" => ["8K" => [6113,5355,4870,4488,4186], "16K" => [12227,10706,9732,8965,8358], "32K" => [null,null,null,null,null]],
        "SP16_2" => ["8K" => [6253,5961,5754,5591,5462], "16K" => [12507,11921,11507,11180,10921], "32K" => [25014,23841,23011,22356,21838]],
        "SP16_4" => ["8K" => [6253,5532,5106,4770,4504], "16K" => [12507,11058,10202,9526,8992], "32K" => [null,null,null,null,null]],
        "SP24_2" => ["8K" => [null,null,null,null,null], "16K" => [12787,12243,11903,11636,11424], "32K" => [25574,24484,23803,23267,22842]],
        "SP24_4" => ["8K" => [null,null,null,null,null], "16K" => [12787,11506,10805,10252,9815], "32K" => [null,null,null,null,null]],
        "SP32_2" => ["8K" => [6463,6207,6058,5955,5848], "16K" => [12927,12412,12113,11906,11691], "32K" => [25854,24821,24221,23806,23375]],
        "SP32_4" => ["8K" => [6463,5890,5589,5351,5164], "16K" => [12927,11780,11178,10703,10328], "32K" => [null,null,null,null,null]],
    ],
    3 => [
        "SP3_2" => ["8K" => [4370,4370,3951,3643,3323], "16K" => [8740,8740,7901,7285,6644], "32K" => [17480,17480,15801,14568,13287]],
        "SP3_4" => ["8K" => [4370,3742,2800,2058,1471], "16K" => [8740,7482,5597,4112,2936], "32K" => [null,null,null,null,null]],
        "SP4_2" => ["8K" => [4922,4800,4408,4099,3855], "16K" => [9844,9599,8815,8197,7708], "32K" => [null,null,null,null,null]],
        "SP4_4" => ["8K" => [4922,4152,3323,2749,2152], "16K" => [9844,8302,6642,5492,4297], "32K" => [null,null,null,null,null]],
        "SP6_2" => ["8K" => [5474,5229,4901,4641,4436], "16K" => [10948,10457,9799,9281,8870], "32K" => [21896,20913,19597,18560,17738]],
        "SP6_4" => ["8K" => [5474,4653,3974,3439,3015], "16K" => [10948,9303,7943,6870,6021], "32K" => [null,null,null,null,null]],
        "SP8_2" => ["8K" => [5750,5478,5173,4972,4764], "16K" => [11500,10955,10344,9942,9525], "32K" => [23000,21909,20685,19882,19048]],
        "SP8_4" => ["8K" => [5750,4953,4369,3908,3544], "16K" => [11500,9901,8730,7807,7076], "32K" => [null,null,null,null,null]],
        "SP12_2" => ["8K" => [6026,5727,5490,5303,5155], "16K" => [12052,11452,10977,10603,10307], "32K" => [24104,22903,21952,21203,20609]],
        "SP12_4" => ["8K" => [6026,5279,4801,4425,4126], "16K" => [12052,10552,9593,8837,8238], "32K" => [null,null,null,null,null]],
        "SP16_2" => ["8K" => [6164,5876,5672,5512,5385], "16K" => [12328,11751,11342,11020,10765], "32K" => [24656,23500,22681,22036,21525]],
        "SP16_4" => ["8K" => [6164,5450,5028,4695,4432], "16K" => [12328,10900,10056,9390,8863], "32K" => [null,null,null,null,null]],
        "SP24_2" => ["8K" => [null,null,null,null,null], "16K" => [12604,12068,11733,11469,11260], "32K" => [25208,24133,23463,22934,22515]],
        "SP24_4" => ["8K" => [null,null,null,null,null], "16K" => [12604,11341,10650,10106,9675], "32K" => [null,null,null,null,null]],
        "SP32_2" => ["8K" => [6371,6117,5970,5868,5762], "16K" => [12742,12234,11940,11735,11523], "32K" => [25484,24465,23875,23466,23040]],
        "SP32_4" => ["8K" => [6371,5809,5514,5281,5097], "16K" => [12742,11606,11010,10540,10168], "32K" => [null,null,null,null,null]],
    ],
    4 => [
        "SP3_2" => ["8K" => [4307,4307,3894,3591,3275], "16K" => [8614,8614,7787,7180,6549], "32K" => [17228,17228,15573,14359,13096]],
        "SP3_4" => ["8K" => [4307,3688,2760,2029,1450], "16K" => [8614,7374,5517,4053,2894], "32K" => [null,null,null,null,null]],
        "SP4_2" => ["8K" => [4851,4731,4345,4040,3799], "16K" => [9702,9461,8688,8079,7597], "32K" => [null,null,null,null,null]],
        "SP4_4" => ["8K" => [4851,4092,3275,2710,2121], "16K" => [9702,8182,6546,5413,4236], "32K" => [null,null,null,null,null]],
        "SP6_2" => ["8K" => [5395,5154,4830,4575,4372], "16K" => [10790,10306,9658,9147,8743], "32K" => [21580,20612,19315,18292,17483]],
        "SP6_4" => ["8K" => [5395,4586,3917,3390,2972], "16K" => [10790,9169,7828,6771,5934], "32K" => [null,null,null,null,null]],
        "SP8_2" => ["8K" => [5667,5399,5098,4901,4695], "16K" => [11334,10797,10194,9799,9388], "32K" => [22668,21593,20387,19596,18773]],
        "SP8_4" => ["8K" => [5667,4881,4306,3852,3493], "16K" => [11334,9758,8604,7695,6974], "32K" => [null,null,null,null,null]],
        "SP12_2" => ["8K" => [5939,5644,5411,5227,5081], "16K" => [11878,11287,10819,10450,10158], "32K" => [23756,22572,21636,20897,20312]],
        "SP12_4" => ["8K" => [5939,5203,4732,4361,4067], "16K" => [11878,10400,9455,8710,8120], "32K" => [null,null,null,null,null]],
        "SP16_2" => ["8K" => [6075,5792,5591,5432,5307], "16K" => [12150,11581,11178,10861,10609], "32K" => [24300,23160,22354,21718,21215]],
        "SP16_4" => ["8K" => [6075,5375,4961,4635,4377], "16K" => [12150,10743,9911,9255,8736], "32K" => [null,null,null,null,null]],
        "SP24_2" => ["8K" => [null,null,null,null,null], "16K" => [12422,11894,11564,11304,11098], "32K" => [24844,23785,23124,22603,22190]],
        "SP24_4" => ["8K" => [null,null,null,null,null], "16K" => [12422,11178,10497,9960,9536], "32K" => [null,null,null,null,null]],
        "SP32_2" => ["8K" => [6279,6030,5886,5786,5682], "16K" => [12558,12058,11767,11566,11357], "32K" => [25116,24112,23530,23127,22708]],
        "SP32_4" => ["8K" => [6279,5728,5438,5210,5030], "16K" => [12558,11444,10860,10399,10034], "32K" => [null,null,null,null,null]],
    ],
];

$SP_PATTERN_MAP = [
    '0' => 'SP3_2', '1' => 'SP3_4', '2' => 'SP4_2', '3' => 'SP4_4',
    '4' => 'SP6_2', '5' => 'SP6_4', '6' => 'SP8_2', '7' => 'SP8_4',
    '8' => 'SP12_2', '9' => 'SP12_4', '10' => 'SP16_2', '11' => 'SP16_4',
    '12' => 'SP24_2', '13' => 'SP24_4', '14' => 'SP32_2', '15' => 'SP32_4',
];

function getPreambleDxFromStructure($preambleStructure) {
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

function getPreambleCells($fftName, $giSamples, $cred, $preambleDx) {
    global $TABLE_7_2_PREAMBLE;

    if (!isset($TABLE_7_2_PREAMBLE[$fftName])) return null;
    $fftData = $TABLE_7_2_PREAMBLE[$fftName];

    if (!isset($fftData[$giSamples])) return null;
    $giData = $fftData[$giSamples];

    if (!isset($giData['cells'])) {
        $dxKey = $preambleDx ?: array_key_first($giData);
        if (!isset($giData[$dxKey])) return null;
        $giData = $giData[$dxKey];
    }

    $credIndex = max(0, min(4, (int)$cred));
    return $giData['cells'][$credIndex];
}

function estimateL1DCells($l1dBytes, $l1dFecMode) {
    $x = max(25, (int)$l1dBytes);

    switch ((int)$l1dFecMode) {
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

function fftLabel($v) {
    global $FFT_VALUE_TO_NAME;
    return isset($FFT_VALUE_TO_NAME[(int)$v]) ? $FFT_VALUE_TO_NAME[(int)$v] : "8K";
}

function parseSpLabel($spValue) {
    global $SP_PATTERN_MAP;

    if ($spValue === null || $spValue === '') {
        return ['dx' => 12, 'group' => 'SP12to32', 'key' => 'SP12_2'];
    }

    $spPattern = isset($SP_PATTERN_MAP[(string)$spValue]) ? $SP_PATTERN_MAP[(string)$spValue] : 'SP12_2';

    if (!preg_match('/^SP(\d+)_(\d+)$/', $spPattern, $m)) {
        return ['dx' => 12, 'group' => 'SP12to32', 'key' => 'SP12_2'];
    }

    $dx = (int)$m[1];
    $group = ($dx >= 3 && $dx <= 8) ? 'SP3to8' : 'SP12to32';

    return ['dx' => $dx, 'group' => $group, 'key' => $spPattern];
}

function getSbsDataCells($fft, $cred, $spKey, $spBoost) {
    global $SBS_ACTIVE_DATA_CELLS;

    if (!isset($SBS_ACTIVE_DATA_CELLS[$cred])) return 0;
    $credData = $SBS_ACTIVE_DATA_CELLS[$cred];

    if (!isset($credData[$spKey])) return 0;
    $spData = $credData[$spKey];

    if (!isset($spData[$fft])) return 0;
    $fftData = $spData[$fft];

    $boostIndex = max(0, min(4, (int)$spBoost));
    $cells = $fftData[$boostIndex];

    if ($cells === null) return 0;
    return $cells;
}

function doCalculatePreambleFields($params) {
    global $L1B_CELLS_FIXED, $GI_VALUE_TO_SAMPLES, $FFT_VALUE_TO_NAME;

    $fftValue = (int)($params['fftValue'] ?? 0);
    $giValue = (int)($params['giValue'] ?? 5);
    $preambleReducedCarriers = (int)($params['preambleReducedCarriers'] ?? 0);
    $preambleStructure = (int)($params['preambleStructure'] ?? 0);
    $l1dFecMode = (int)($params['l1dFecMode'] ?? 0);
    $l1dBytes = (int)($params['l1dBytes'] ?? 25);

    $fftName = isset($FFT_VALUE_TO_NAME[$fftValue]) ? $FFT_VALUE_TO_NAME[$fftValue] : "8K";
    $giSamples = isset($GI_VALUE_TO_SAMPLES[$giValue]) ? $GI_VALUE_TO_SAMPLES[$giValue] : 1024;
    $preambleDx = getPreambleDxFromStructure($preambleStructure);

    $cellsInFirst = getPreambleCells($fftName, $giSamples, 4, $preambleDx) ?: 0;
    $cellsInNext = getPreambleCells($fftName, $giSamples, $preambleReducedCarriers, $preambleDx) ?: 0;

    $l1dResult = estimateL1DCells($l1dBytes, $l1dFecMode);
    $l1dCells = $l1dResult['cells'];
    $l1dBytes = $l1dResult['bytes'];

    $l1Total = $L1B_CELLS_FIXED + $l1dCells;

    $numPreambleSymbols = ($l1Total > $cellsInFirst) ? 2 : 1;

    $totalPreambleCells = $cellsInFirst + ($numPreambleSymbols - 1) * $cellsInNext;
    $plpCells = max(0, $totalPreambleCells - $L1B_CELLS_FIXED - $l1dCells);

    return [
        'l1bCells' => $L1B_CELLS_FIXED,
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

function doComputePlpCapacity($params) {
    global $TABLE_7_3, $TABLE_7_4;

    $fft = $params['fft'] ?? '8K';
    $cred = (int)($params['cred'] ?? 0);
    $spLabelText = $params['spLabelText'] ?? '8';
    $spBoost = (int)($params['spBoost'] ?? 0);
    $numSymbols = (int)($params['numSymbols'] ?? 0);
    $sbsFirst = (bool)($params['sbsFirst'] ?? false);
    $sbsLast = (bool)($params['sbsLast'] ?? false);

    $sp = parseSpLabel($spLabelText);
    $table = ($sp['group'] === 'SP3to8') ? $TABLE_7_3 : $TABLE_7_4;

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
    $sbsDataCells = getSbsDataCells($fft, $cred, $sp['key'], $spBoost);

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

function doFillPlpSize($params) {
    $preambleParams = $params['preambleConfig'] ?? [];
    $subframeConfig = $params['subframeConfig'] ?? [];

    $preambleFields = doCalculatePreambleFields($preambleParams);
    $plpCells = $preambleFields['plpCells'];

    $plpCapacity = doComputePlpCapacity($subframeConfig);

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

function doBatchFillAllPlps($params) {
    $preambleConfig = $params['preambleConfig'] ?? [];
    $subframesConfigs = $params['subframesConfigs'] ?? [];

    $preambleFields = doCalculatePreambleFields($preambleConfig);
    $plpCells = $preambleFields['plpCells'];

    $results = [];
    foreach ($subframesConfigs as $sfIdx => $sfConfig) {
        $plps = $sfConfig['plps'] ?? [$sfConfig];
        $sfResults = [];

        foreach ($plps as $plpIdx => $plpConfig) {
            $plpCapacity = doComputePlpCapacity($plpConfig);

            if ($plpCapacity['capacity'] <= 0) {
                $sfResults[] = [
                    'error' => true,
                    'reason' => $plpCapacity['reason'],
                    'totalSize' => 0,
                    'plpCapacity' => $plpCapacity,
                ];
            } else {
                $totalSize = $plpCapacity['capacity'] + $plpCells;
                $sfResults[] = [
                    'error' => false,
                    'totalSize' => $totalSize,
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

function doGetL1dSizeBytes() {
    $logDir = __DIR__ . '/../server_generator/config/log/';

    if (!is_dir($logDir)) {
        return ['error' => 'Log directory not found', 'bytes' => 25];
    }

    $logFiles = glob($logDir . 'Frame_*.log');
    if (empty($logFiles)) {
        return ['error' => 'No log files found', 'bytes' => 25];
    }

    usort($logFiles, function($a, $b) {
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

function doGetFrame2LogData() {
    $logDir = __DIR__ . '/../server_generator/config/log/';
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
    $plpIds = [];

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
        'source' => 'Frame_2.log'
    ];
}

function doGetFrameDuration() {
    $durationFile = __DIR__ . '/../server_generator/config/log/frame_duration.txt';

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

function doGetValidationTables() {
    global $SP_PATTERN_MAP;

    $ALL_PILOT_PATTERNS = ['0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15'];

    $pilotPatternValueToName = $SP_PATTERN_MAP;
    $pilotPatternNameToValue = array_flip($SP_PATTERN_MAP);

    return [
        'pilotPatternValueToName' => $pilotPatternValueToName,
        'pilotPatternNameToValue' => $pilotPatternNameToValue,
        'preambleConfigurations' => [
            ['fft'=>'8K','gi'=>'GI1_192','dx'=>16,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI1_192','dx'=>16,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI1_192','dx'=>16,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI1_192','dx'=>16,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI1_192','dx'=>16,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI2_384','dx'=>8,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI2_384','dx'=>8,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI2_384','dx'=>8,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI2_384','dx'=>8,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI2_384','dx'=>8,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI3_512','dx'=>6,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI3_512','dx'=>6,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI3_512','dx'=>6,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI3_512','dx'=>6,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI3_512','dx'=>6,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI4_768','dx'=>4,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI4_768','dx'=>4,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI4_768','dx'=>4,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI4_768','dx'=>4,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI4_768','dx'=>4,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI5_1024','dx'=>3,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI5_1024','dx'=>3,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI5_1024','dx'=>3,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI5_1024','dx'=>3,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI5_1024','dx'=>3,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI6_1536','dx'=>4,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI6_1536','dx'=>4,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI6_1536','dx'=>4,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI6_1536','dx'=>4,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI6_1536','dx'=>4,'mode'=>5],
            ['fft'=>'8K','gi'=>'GI7_2048','dx'=>3,'mode'=>1],
            ['fft'=>'8K','gi'=>'GI7_2048','dx'=>3,'mode'=>2],
            ['fft'=>'8K','gi'=>'GI7_2048','dx'=>3,'mode'=>3],
            ['fft'=>'8K','gi'=>'GI7_2048','dx'=>3,'mode'=>4],
            ['fft'=>'8K','gi'=>'GI7_2048','dx'=>3,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI1_192','dx'=>32,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI1_192','dx'=>32,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI1_192','dx'=>32,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI1_192','dx'=>32,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI1_192','dx'=>32,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI2_384','dx'=>16,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI2_384','dx'=>16,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI2_384','dx'=>16,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI2_384','dx'=>16,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI2_384','dx'=>16,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI3_512','dx'=>12,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI3_512','dx'=>12,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI3_512','dx'=>12,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI3_512','dx'=>12,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI3_512','dx'=>12,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI4_768','dx'=>8,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI4_768','dx'=>8,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI4_768','dx'=>8,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI4_768','dx'=>8,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI4_768','dx'=>8,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI5_1024','dx'=>6,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI5_1024','dx'=>6,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI5_1024','dx'=>6,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI5_1024','dx'=>6,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI5_1024','dx'=>6,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI6_1536','dx'=>4,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI6_1536','dx'=>4,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI6_1536','dx'=>4,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI6_1536','dx'=>4,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI6_1536','dx'=>4,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI7_2048','dx'=>3,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI7_2048','dx'=>3,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI7_2048','dx'=>3,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI7_2048','dx'=>3,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI7_2048','dx'=>3,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI8_2432','dx'=>3,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI8_2432','dx'=>3,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI8_2432','dx'=>3,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI8_2432','dx'=>3,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI8_2432','dx'=>3,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI9_3072','dx'=>4,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI9_3072','dx'=>4,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI9_3072','dx'=>4,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI9_3072','dx'=>4,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI9_3072','dx'=>4,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI10_3648','dx'=>4,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI10_3648','dx'=>4,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI10_3648','dx'=>4,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI10_3648','dx'=>4,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI10_3648','dx'=>4,'mode'=>5],
            ['fft'=>'16K','gi'=>'GI11_4096','dx'=>3,'mode'=>1],
            ['fft'=>'16K','gi'=>'GI11_4096','dx'=>3,'mode'=>2],
            ['fft'=>'16K','gi'=>'GI11_4096','dx'=>3,'mode'=>3],
            ['fft'=>'16K','gi'=>'GI11_4096','dx'=>3,'mode'=>4],
            ['fft'=>'16K','gi'=>'GI11_4096','dx'=>3,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI1_192','dx'=>32,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI1_192','dx'=>32,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI1_192','dx'=>32,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI1_192','dx'=>32,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI1_192','dx'=>32,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI2_384','dx'=>32,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI2_384','dx'=>32,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI2_384','dx'=>32,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI2_384','dx'=>32,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI2_384','dx'=>32,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI3_512','dx'=>24,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI3_512','dx'=>24,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI3_512','dx'=>24,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI3_512','dx'=>24,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI3_512','dx'=>24,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI4_768','dx'=>16,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI4_768','dx'=>16,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI4_768','dx'=>16,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI4_768','dx'=>16,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI4_768','dx'=>16,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI5_1024','dx'=>12,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI5_1024','dx'=>12,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI5_1024','dx'=>12,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI5_1024','dx'=>12,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI5_1024','dx'=>12,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI6_1536','dx'=>8,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI6_1536','dx'=>8,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI6_1536','dx'=>8,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI6_1536','dx'=>8,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI6_1536','dx'=>8,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI7_2048','dx'=>6,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI7_2048','dx'=>6,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI7_2048','dx'=>6,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI7_2048','dx'=>6,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI7_2048','dx'=>6,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI8_2432','dx'=>6,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI8_2432','dx'=>6,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI8_2432','dx'=>6,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI8_2432','dx'=>6,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI8_2432','dx'=>6,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>8,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>8,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>8,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>8,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>8,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>3,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>3,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>3,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>3,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI9_3072','dx'=>3,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>8,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>8,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>8,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>8,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>8,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>3,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>3,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>3,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>3,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI10_3648','dx'=>3,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI11_4096','dx'=>3,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI11_4096','dx'=>3,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI11_4096','dx'=>3,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI11_4096','dx'=>3,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI11_4096','dx'=>3,'mode'=>5],
            ['fft'=>'32K','gi'=>'GI12_4864','dx'=>3,'mode'=>1],
            ['fft'=>'32K','gi'=>'GI12_4864','dx'=>3,'mode'=>2],
            ['fft'=>'32K','gi'=>'GI12_4864','dx'=>3,'mode'=>3],
            ['fft'=>'32K','gi'=>'GI12_4864','dx'=>3,'mode'=>4],
            ['fft'=>'32K','gi'=>'GI12_4864','dx'=>3,'mode'=>5],
        ],

        'allowedPatternsSiso' => [
            '8K' => [
                'GI1_192'=>['10','11','14','15'],
                'GI2_384'=>['6','7','10','11'],
                'GI3_512'=>['4','5','8','9'],
                'GI4_768'=>['2','3','6','7'],
                'GI5_1024'=>['0','1','4','5'],
                'GI6_1536'=>['2','3'],
                'GI7_2048'=>['0','1'],
                'GI8_2432'=>[],'GI9_3072'=>[],'GI10_3648'=>[],'GI11_4096'=>[],'GI12_4864'=>[],
            ],
            '16K' => [
                'GI1_192'=>['14','15'],
                'GI2_384'=>['10','11','14','15'],
                'GI3_512'=>['8','9','12','13'],
                'GI4_768'=>['6','7','10','11'],
                'GI5_1024'=>['4','5','8','9'],
                'GI6_1536'=>['2','3','6','7'],
                'GI7_2048'=>['0','1','4','5'],
                'GI8_2432'=>['0','1','4','5'],
                'GI9_3072'=>['2','3'],
                'GI10_3648'=>['2','3'],
                'GI11_4096'=>['0','1'],
                'GI12_4864'=>[],
            ],
            '32K' => [
                'GI1_192'=>['14'],
                'GI2_384'=>['14'],
                'GI3_512'=>['12'],
                'GI4_768'=>['10','14'],
                'GI5_1024'=>['8','12'],
                'GI6_1536'=>['6','10'],
                'GI7_2048'=>['4','8'],
                'GI8_2432'=>['4','8'],
                'GI9_3072'=>['0','6'],
                'GI10_3648'=>['0','6'],
                'GI11_4096'=>['0','4'],
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

$action = $input['action'];

switch ($action) {
    case 'calculatePreambleFields':
        echo json_encode(doCalculatePreambleFields($input));
        break;

    case 'computePlpCapacity':
        echo json_encode(doComputePlpCapacity($input));
        break;

    case 'fillPlpSize':
        echo json_encode(doFillPlpSize($input));
        break;

    case 'batchFillAllPlps':
        echo json_encode(doBatchFillAllPlps($input));
        break;

    case 'getL1dSizeBytes':
        echo json_encode(doGetL1dSizeBytes());
        break;

    case 'getFrame2LogData':
        echo json_encode(doGetFrame2LogData());
        break;

    case 'getFrameDuration':
        echo json_encode(doGetFrameDuration());
        break;

    case 'getValidationTables':
        echo json_encode(doGetValidationTables());
        break;

    default:
        http_response_code(400);
        echo json_encode(['error' => "Unknown action: $action"]);
        break;
}
