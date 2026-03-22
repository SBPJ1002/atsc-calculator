if (typeof window.__BOOT_ONCE__ === 'undefined') window.__BOOT_ONCE__ = false;
if (typeof window.__plpNextId === 'undefined') window.__plpNextId = 0;

function startPlpIdSequence() {
    window.__plpNextId = 0;
}

function nextPlpId() {
    return window.__plpNextId++;
}

function getNextAvailablePlpId() {
    const existingIds = new Set();

    document.querySelectorAll('input[id^="plp_id_"]').forEach(input => {
        const value = parseInt(input.value, 10);
        if (Number.isFinite(value)) {
            existingIds.add(value);
        }
    });

    let nextId = 0;
    while (existingIds.has(nextId)) {
        nextId++;
    }

    return nextId;
}

function renumberAllPlps() {
    const allPlps = [];

    document.querySelectorAll('[id^="plp-"]').forEach(plp => {
        const match = plp.id.match(/plp-(\d+)-(\d+)/);
        if (match) {
            const subframeIndex = parseInt(match[1], 10);
            const plpIndex = parseInt(match[2], 10);
            const plpIdInput = plp.querySelector('input[id^="plp_id_"]');

            allPlps.push({
                subframeIndex,
                plpIndex,
                plpElement: plp,
                plpIdInput
            });
        }
    });

    allPlps.sort((a, b) => {
        if (a.subframeIndex !== b.subframeIndex) {
            return a.subframeIndex - b.subframeIndex;
        }
        return a.plpIndex - b.plpIndex;
    });

    allPlps.forEach((plp, index) => {
        if (plp.plpIdInput) {
            plp.plpIdInput.value = index;

            const form = plp.plpElement.querySelector('form');
            if (form) {
                const h3 = form.querySelector('h3');
                if (h3) {
                    h3.textContent = `Subframe ${plp.subframeIndex} - PLP ${index}`;
                }
            }
        }
    });
}

const API_URL = 'api.php';

let _apiDebounceTimers = {};

function debounce(key, fn, delay = 150) {
    if (_apiDebounceTimers[key]) clearTimeout(_apiDebounceTimers[key]);
    _apiDebounceTimers[key] = setTimeout(fn, delay);
}

async function apiCall(action, params) {
    const body = Object.assign({ action }, params);
    const resp = await fetch(API_URL, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
    });
    if (!resp.ok) {
        throw new Error(`API error ${resp.status}: ${resp.statusText}`);
    }
    return resp.json();
}

async function fetchL1dSizeBytesFromLog() {
    try {
        const result = await apiCall('getL1dSizeBytes', {});
        const field = document.getElementById('detail_size_bytes');
        if (field && result.bytes) {
            field.value = result.bytes;
        }
        return result.bytes || 25;
    } catch (e) {
        return 25;
    }
}

async function fetchFrame2LogData() {
    try {
        const result = await apiCall('getFrame2LogData', {});
        if (!result.error) {
            window.__frame2LogData__ = result;
        }
    } catch (e) {
    }
}

async function fetchFrameDuration() {
    try {
        const result = await apiCall('getFrameDuration', {});

        if (result.error) {
            return null;
        }

        if (result.totalDurationMs == null) {
            return null;
        }

        if (!window.__sfExtraData__) window.__sfExtraData__ = {};

        window.__frameDurationMs__ = result.totalDurationMs;

        const frameDurEl = document.getElementById('resultados-frame-duration');
        if (frameDurEl) {
            frameDurEl.textContent = result.totalDurationMs.toFixed(4) + ' ms';
        }

        const numSf = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
        const durations = result.subframeDurations || {};
        for (let i = 0; i < numSf; i++) {
            if (!window.__sfExtraData__[i]) window.__sfExtraData__[i] = {};
            const sfDur = durations[i] ?? durations[String(i)];
            if (sfDur != null) {
                window.__sfExtraData__[i].duracao = Number(sfDur).toFixed(4) + ' ms';
            }
        }

        applyFrameDurationToDOM();

        return result;
    } catch (e) {
        return null;
    }
}

function applyFrameDurationToDOM() {

    const frameDurEl = document.getElementById('resultados-frame-duration');
    if (frameDurEl && window.__frameDurationMs__ != null) {
        frameDurEl.textContent = Number(window.__frameDurationMs__).toFixed(4) + ' ms';
    }

    if (!window.__sfExtraData__) return;
    const numSf = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
    for (let i = 0; i < numSf; i++) {
        const duracao = window.__sfExtraData__[i]?.duracao;
        if (duracao) {
            const el = document.getElementById(`sf-duracao-${i}`);
            if (el) el.textContent = duracao;
        }
    }
}

function gatherPreambleParams() {
    const fftSel = document.getElementById('fft_size_0');
    const giSel = document.getElementById('guard_interval_0');
    const preambleReducedCarriersField = document.getElementById('preamble_reduced_carriers');
    const preambleStructureField = document.getElementById('preamble_structure');
    const l1dFecMode = parseInt(document.getElementById('detail_fec_type')?.value || '0', 10);
    const l1dBytes = parseInt(document.getElementById('detail_size_bytes')?.value || '25', 10);

    return {
        fftValue: parseInt(fftSel?.value || '0', 10),
        giValue: parseInt(giSel?.value || '5', 10),
        preambleReducedCarriers: preambleReducedCarriersField ? parseInt(preambleReducedCarriersField.value || '0', 10) : 0,
        preambleStructure: preambleStructureField ? parseInt(preambleStructureField.value || '0', 10) : 0,
        l1dFecMode,
        l1dBytes,
    };
}

function fftLabel(v) {
    return v === "0" ? "8K" : v === "1" ? "16K" : "32K";
}

function gatherSubframeConfig(subIdx) {
    const fftSel = document.getElementById(`fft_size_${subIdx}`);
    const rcSel = document.getElementById(`reduced_carrier_${subIdx}`);
    const spSel = document.getElementById(`spilot_pattern_${subIdx}`);
    const spBoostSel = document.getElementById(`spilot_boost_${subIdx}`);
    const nSymInp = document.getElementById(`num_ofdm_${subIdx}`);
    const sbsFSel = document.getElementById(`sbs_first_${subIdx}`);
    const sbsLSel = document.getElementById(`sbs_last_${subIdx}`);

    return {
        fft: fftLabel(fftSel?.value ?? "0"),
        cred: parseInt(rcSel?.value ?? "0", 10),
        spLabelText: spSel?.value ?? "8",
        spBoost: parseInt(spBoostSel?.value ?? "0", 10),
        numSymbols: parseInt(nSymInp?.value ?? "0", 10),
        sbsFirst: (sbsFSel?.value === "1"),
        sbsLast: (sbsLSel?.value === "1"),
    };
}

async function calculatePreambleFields(subframeIndex) {
    const params = gatherPreambleParams();
    return apiCall('calculatePreambleFields', params);
}

async function computePlpIndividualCapacity(subframeIndex, plpIndex) {
    const subframeCfg = gatherSubframeConfig(subframeIndex);
    return apiCall('computePlpCapacity', subframeCfg);
}

async function fillPlpSize(subframeIndex, plpIndex) {

    const sizeInput = document.getElementById(`size_${plpIndex}`);
    if (!sizeInput) {
        return;
    }

    const result = await apiCall('fillPlpSize', {
        preambleConfig: gatherPreambleParams(),
        subframeConfig: gatherSubframeConfig(subframeIndex),
    });

    if (result.error) {
        alert('Erro: ' + result.reason);
        return;
    }

    sizeInput.value = result.totalSize;

    await updatePlpProgressBar(subframeIndex, plpIndex);
}

async function autoFillPlpSize(subframeIndex, plpIndex) {
    const sizeInput = document.getElementById(`size_${plpIndex}`);
    if (!sizeInput) return;

    const result = await apiCall('fillPlpSize', {
        preambleConfig: gatherPreambleParams(),
        subframeConfig: gatherSubframeConfig(subframeIndex),
    });

    if (result.error) {
        return;
    }

    sizeInput.value = result.totalSize;

    if (typeof updatePlpProgressBar === 'function') {
        await updatePlpProgressBar(subframeIndex, plpIndex);
    }
}

async function autoFillAllPlpsInSubframe(subframeIndex) {
    const plpElements = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);
    const promises = [];

    plpElements.forEach(plpEl => {
        const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
        if (match) {
            const plpIndex = parseInt(match[2], 10);
            promises.push(autoFillPlpSize(subframeIndex, plpIndex));
        }
    });

    await Promise.all(promises);
}

async function autoFillAllPlps() {

    if (window.__plpDataLoaded__) {
        window.__plpDataLoaded__ = false;
        return;
    }
    const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);

    const preambleConfig = gatherPreambleParams();
    const subframesConfigs = [];

    for (let sf = 0; sf < numSubframes; sf++) {
        const plpElements = document.querySelectorAll(`[id^="plp-${sf}-"]`);
        const plps = [];

        plpElements.forEach(plpEl => {
            const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
            if (match) {
                plps.push(gatherSubframeConfig(sf));
            }
        });

        if (plps.length === 0) {
            plps.push(gatherSubframeConfig(sf));
        }

        subframesConfigs.push({ plps });
    }

    try {
        const result = await apiCall('batchFillAllPlps', { preambleConfig, subframesConfigs });

        for (let sf = 0; sf < numSubframes; sf++) {
            const sfResults = result.results[sf];
            if (!sfResults) continue;

            const plpElements = document.querySelectorAll(`[id^="plp-${sf}-"]`);
            let plpIdx = 0;

            plpElements.forEach(plpEl => {
                const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
                if (match && sfResults[plpIdx]) {
                    const plpIndex = parseInt(match[2], 10);
                    const r = sfResults[plpIdx];

                    if (!r.error) {
                        const sizeInput = document.getElementById(`size_${plpIndex}`);
                        if (sizeInput) {
                            sizeInput.value = r.totalSize;
                        }
                    }
                    plpIdx++;
                }
            });
        }

        for (let sf = 0; sf < numSubframes; sf++) {
            const plpElements = document.querySelectorAll(`[id^="plp-${sf}-"]`);
            plpElements.forEach(plpEl => {
                const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
                if (match) {
                    const plpIndex = parseInt(match[2], 10);
                    if (typeof updatePlpProgressBar === 'function') {
                        updatePlpProgressBar(sf, plpIndex);
                    }
                }
            });
        }
    } catch (err) {

        for (let sf = 0; sf < numSubframes; sf++) {
            await autoFillAllPlpsInSubframe(sf);
        }
    }

}

function setupPlpAutoSizeListeners() {
    const triggerFieldPrefixes = [
        'fft_size_',
        'reduced_carrier_',
        'spilot_pattern_',
        'spilot_boost_',
        'num_ofdm_',
        'sbs_first_',
        'sbs_last_',
        'guard_interval_'
    ];

    const globalTriggerFields = [
        'preamble_structure',
        'preamble_reduced_carriers',
        'number_of_subframes',
        'detail_fec_type',
        'detail_size_bytes'
    ];

    document.addEventListener('change', function(e) {
        const fieldId = e.target.id || '';

        if (globalTriggerFields.includes(fieldId)) {
            debounce('global-recalc', () => {
                const numSf = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
                for (let sf = 0; sf < numSf; sf++) recalcAutoPlps(sf);
            }, 200);
            return;
        }

        const matchingPrefix = triggerFieldPrefixes.find(prefix => fieldId.startsWith(prefix));

        if (matchingPrefix) {
            const subframeIndex = parseInt(fieldId.replace(matchingPrefix, ''), 10);

            if (!isNaN(subframeIndex)) {
                debounce(`sf-recalc-${subframeIndex}`, () => recalcAutoPlps(subframeIndex), 150);
            }
        }
    });

}

window.autoFillAllPlps = autoFillAllPlps;
window.autoFillAllPlpsInSubframe = autoFillAllPlpsInSubframe;

document.addEventListener('DOMContentLoaded', function() {
    setTimeout(setupPlpAutoSizeListeners, 500);
});

if (document.readyState === 'complete' || document.readyState === 'interactive') {
    setTimeout(setupPlpAutoSizeListeners, 500);
}
