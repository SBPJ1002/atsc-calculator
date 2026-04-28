if (typeof window.__BOOT_ONCE__ === 'undefined') window.__BOOT_ONCE__ = false;
if (typeof window.__plpNextId === 'undefined') window.__plpNextId = 0;

class PlpCalculator {
    constructor() {
        this.API_URL = 'api.php';
        this._apiDebounceTimers = {};
    }

    startPlpIdSequence() {
        window.__plpNextId = 0;
    }

    nextPlpId() {
        return window.__plpNextId++;
    }

    getNextAvailablePlpId() {
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

    renumberAllPlps() {
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

    debounce(key, fn, delay = 150) {
        if (this._apiDebounceTimers[key]) clearTimeout(this._apiDebounceTimers[key]);
        this._apiDebounceTimers[key] = setTimeout(fn, delay);
    }

    async apiCall(flag, params) {
        const body = Object.assign({ flag }, params);
        const resp = await fetch(this.API_URL, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body),
        });
        if (!resp.ok) {
            throw new Error(`API error ${resp.status}: ${resp.statusText}`);
        }
        return resp.json();
    }

    async fetchL1dSizeBytesFromLog() {
        try {
            const result = await this.apiCall('getL1dSizeBytes', {});
            const field = document.getElementById('detail_size_bytes');
            if (field && result.bytes) {
                field.value = result.bytes;
            }
            return result.bytes || 25;
        } catch (e) {
            return 25;
        }
    }

    async fetchFrame2LogData() {
        try {
            const result = await this.apiCall('getFrame2LogData', {});
            if (!result.error) {
                window.__frame2LogData__ = result;
            }
        } catch (e) {
        }
    }

    async fetchFrameDuration() {
        try {
            const result = await this.apiCall('getFrameDuration', {});

            if (result.error) return null;
            if (result.totalDurationMs == null) return null;

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

            this.applyFrameDurationToDOM();

            return result;
        } catch (e) {
            return null;
        }
    }

    applyFrameDurationToDOM() {
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

    gatherPreambleParams() {
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

    fftLabel(v) {
        return v === "0" ? "8K" : v === "1" ? "16K" : "32K";
    }

    gatherSubframeConfig(subIdx) {
        const fftSel = document.getElementById(`fft_size_${subIdx}`);
        const rcSel = document.getElementById(`reduced_carrier_${subIdx}`);
        const spSel = document.getElementById(`spilot_pattern_${subIdx}`);
        const spBoostSel = document.getElementById(`spilot_boost_${subIdx}`);
        const nSymInp = document.getElementById(`num_ofdm_${subIdx}`);
        const sbsFSel = document.getElementById(`sbs_first_${subIdx}`);
        const sbsLSel = document.getElementById(`sbs_last_${subIdx}`);

        return {
            fft: this.fftLabel(fftSel?.value ?? "0"),
            cred: parseInt(rcSel?.value ?? "0", 10),
            spLabelText: spSel?.value ?? "8",
            spBoost: parseInt(spBoostSel?.value ?? "0", 10),
            numSymbols: parseInt(nSymInp?.value ?? "0", 10),
            sbsFirst: (sbsFSel?.value === "1"),
            sbsLast: (sbsLSel?.value === "1"),
        };
    }

    async calculatePreambleFields(subframeIndex) {
        const params = this.gatherPreambleParams();
        return this.apiCall('calculatePreambleFields', params);
    }

    async computePlpIndividualCapacity(subframeIndex, plpIndex) {
        const subframeCfg = this.gatherSubframeConfig(subframeIndex);
        return this.apiCall('computePlpCapacity', subframeCfg);
    }

    async fillPlpSize(subframeIndex, plpIndex) {
        const sizeInput = document.getElementById(`size_${plpIndex}`);
        if (!sizeInput) return;

        const result = await this.apiCall('fillPlpSize', {
            preambleConfig: this.gatherPreambleParams(),
            subframeConfig: this.gatherSubframeConfig(subframeIndex),
        });

        if (result.error) {
            alert('Erro: ' + result.reason);
            return;
        }

        sizeInput.value = result.totalSize;

        await updatePlpProgressBar(subframeIndex, plpIndex);
    }

    async autoFillPlpSize(subframeIndex, plpIndex) {
        const sizeInput = document.getElementById(`size_${plpIndex}`);
        if (!sizeInput) return;

        const result = await this.apiCall('fillPlpSize', {
            preambleConfig: this.gatherPreambleParams(),
            subframeConfig: this.gatherSubframeConfig(subframeIndex),
        });

        if (result.error) return;

        sizeInput.value = result.totalSize;

        if (typeof updatePlpProgressBar === 'function') {
            await updatePlpProgressBar(subframeIndex, plpIndex);
        }
    }

    async autoFillAllPlpsInSubframe(subframeIndex) {
        const plpElements = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);
        const promises = [];

        plpElements.forEach(plpEl => {
            const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
            if (match) {
                const plpIndex = parseInt(match[2], 10);
                promises.push(this.autoFillPlpSize(subframeIndex, plpIndex));
            }
        });

        await Promise.all(promises);
    }

    async autoFillAllPlps() {
        if (window.__plpDataLoaded__) {
            window.__plpDataLoaded__ = false;
            return;
        }
        const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);

        const preambleConfig = this.gatherPreambleParams();
        const subframesConfigs = [];

        for (let sf = 0; sf < numSubframes; sf++) {
            const plpElements = document.querySelectorAll(`[id^="plp-${sf}-"]`);
            const plps = [];

            plpElements.forEach(plpEl => {
                const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
                if (match) {
                    plps.push(this.gatherSubframeConfig(sf));
                }
            });

            if (plps.length === 0) {
                plps.push(this.gatherSubframeConfig(sf));
            }

            subframesConfigs.push({ plps });
        }

        try {
            const result = await this.apiCall('batchFillAllPlps', { preambleConfig, subframesConfigs });

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
                await this.autoFillAllPlpsInSubframe(sf);
            }
        }
    }

    // Gather PLP data for results computation on server
    gatherPlpData(subframeIndex, plpIndex) {
        return {
            fec_type: parseInt(document.getElementById(`fec_type_${plpIndex}`)?.value || '0', 10),
            mod_order: parseInt(document.getElementById(`mod_order_${plpIndex}`)?.value || '0', 10),
            code_rate: parseInt(document.getElementById(`code_rate_${plpIndex}`)?.value || '0', 10),
            size: parseInt(document.getElementById(`size_${plpIndex}`)?.value || '0', 10),
            start: parseInt(document.getElementById(`start_${plpIndex}`)?.value || '0', 10),
            ti_mode: parseInt(document.getElementById(`ti_mode_${plpIndex}`)?.value || '0', 10),
            ti_extended: parseInt(document.getElementById(`ti_extended_${plpIndex}`)?.value || '0', 10),
            cti_depth: parseInt(document.getElementById(`cti_depth_${plpIndex}`)?.value || '0', 10),
            num_fec_blocks_max: parseInt(document.getElementById(`num_fec_blocks_max_${plpIndex}`)?.value || '0', 10),
            plp_id: parseInt(document.getElementById(`plp_id_${plpIndex}`)?.value || '0', 10),
        };
    }

    // Request all results data from server
    async fetchResultsData(subframeIndex) {
        const preambleConfig = this.gatherPreambleParams();
        const sfConfig = this.gatherSubframeConfig(subframeIndex);

        // Gather PLP data
        const plpElements = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);
        const plps = [];
        plpElements.forEach(plpEl => {
            const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
            if (match) {
                const plpIndex = parseInt(match[2], 10);
                plps.push(this.gatherPlpData(subframeIndex, plpIndex));
            }
        });

        sfConfig.subframeIndex = subframeIndex;
        sfConfig.plps = plps;

        return this.apiCall('computeResultsData', {
            preambleConfig,
            subframesData: [sfConfig],
        });
    }

    setupPlpAutoSizeListeners() {
        const triggerFieldPrefixes = [
            'fft_size_', 'reduced_carrier_', 'spilot_pattern_', 'spilot_boost_',
            'num_ofdm_', 'sbs_first_', 'sbs_last_', 'guard_interval_'
        ];

        const globalTriggerFields = [
            'preamble_structure', 'preamble_reduced_carriers',
            'number_of_subframes', 'detail_fec_type', 'detail_size_bytes'
        ];

        document.addEventListener('change', (e) => {
            const fieldId = e.target.id || '';

            if (globalTriggerFields.includes(fieldId)) {
                this.debounce('global-recalc', () => {
                    const numSf = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
                    for (let sf = 0; sf < numSf; sf++) recalcAutoPlps(sf);
                }, 200);
                return;
            }

            const matchingPrefix = triggerFieldPrefixes.find(prefix => fieldId.startsWith(prefix));

            if (matchingPrefix) {
                const subframeIndex = parseInt(fieldId.replace(matchingPrefix, ''), 10);

                if (!isNaN(subframeIndex)) {
                    this.debounce(`sf-recalc-${subframeIndex}`, () => recalcAutoPlps(subframeIndex), 150);
                }
            }
        });
    }
}

window.plpCalculator = new PlpCalculator();
window.startPlpIdSequence = (...args) => window.plpCalculator.startPlpIdSequence(...args);
window.nextPlpId = (...args) => window.plpCalculator.nextPlpId(...args);
window.getNextAvailablePlpId = (...args) => window.plpCalculator.getNextAvailablePlpId(...args);
window.renumberAllPlps = (...args) => window.plpCalculator.renumberAllPlps(...args);
window.debounce = (...args) => window.plpCalculator.debounce(...args);
window.apiCall = (...args) => window.plpCalculator.apiCall(...args);
window.fetchL1dSizeBytesFromLog = (...args) => window.plpCalculator.fetchL1dSizeBytesFromLog(...args);
window.fetchFrame2LogData = (...args) => window.plpCalculator.fetchFrame2LogData(...args);
window.fetchFrameDuration = (...args) => window.plpCalculator.fetchFrameDuration(...args);
window.applyFrameDurationToDOM = (...args) => window.plpCalculator.applyFrameDurationToDOM(...args);
window.gatherPreambleParams = (...args) => window.plpCalculator.gatherPreambleParams(...args);
window.fftLabel = (...args) => window.plpCalculator.fftLabel(...args);
window.gatherSubframeConfig = (...args) => window.plpCalculator.gatherSubframeConfig(...args);
window.calculatePreambleFields = (...args) => window.plpCalculator.calculatePreambleFields(...args);
window.computePlpIndividualCapacity = (...args) => window.plpCalculator.computePlpIndividualCapacity(...args);
window.fillPlpSize = (...args) => window.plpCalculator.fillPlpSize(...args);
window.autoFillPlpSize = (...args) => window.plpCalculator.autoFillPlpSize(...args);
window.autoFillAllPlpsInSubframe = (...args) => window.plpCalculator.autoFillAllPlpsInSubframe(...args);
window.autoFillAllPlps = (...args) => window.plpCalculator.autoFillAllPlps(...args);
window.setupPlpAutoSizeListeners = (...args) => window.plpCalculator.setupPlpAutoSizeListeners(...args);
window.fetchResultsData = (...args) => window.plpCalculator.fetchResultsData(...args);

document.addEventListener('DOMContentLoaded', function() {
    setTimeout(() => window.plpCalculator.setupPlpAutoSizeListeners(), 500);
});

if (document.readyState === 'complete' || document.readyState === 'interactive') {
    setTimeout(() => window.plpCalculator.setupPlpAutoSizeListeners(), 500);
}
