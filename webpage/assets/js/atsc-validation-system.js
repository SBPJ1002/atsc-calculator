let PREAMBLE_CONFIGURATIONS = {};
let GI_VALUE_MAP = {};
let FFT_VALUE_MAP = {};
let ALL_PILOT_PATTERNS = [];
let PILOT_PATTERN_NAME_TO_VALUE = {};
let PILOT_PATTERN_VALUE_TO_NAME = {};
let ALLOWED_PATTERNS_SISO = {};
let ALLOWED_PATTERNS_WALSH_HADAMARD = {};
let ALLOWED_PATTERNS_NULL_PILOT = {};

let _validationTablesLoaded = false;

async function loadValidationTables() {
    if (_validationTablesLoaded) return true;
    try {
        const resp = await fetch('api.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ flag: 'getValidationTables' })
        });
        const data = await resp.json();

        PREAMBLE_CONFIGURATIONS = {};
        data.preambleConfigurations.forEach((cfg, idx) => {
            PREAMBLE_CONFIGURATIONS[idx] = cfg;
        });

        GI_VALUE_MAP = {
            'GI1_192': '1', 'GI2_384': '2', 'GI3_512': '3', 'GI4_768': '4',
            'GI5_1024': '5', 'GI6_1536': '6', 'GI7_2048': '7', 'GI8_2432': '8',
            'GI9_3072': '9', 'GI10_3648': '10', 'GI11_4096': '11', 'GI12_4864': '12'
        };
        FFT_VALUE_MAP = { '8K': '0', '16K': '1', '32K': '2' };

        ALL_PILOT_PATTERNS = data.allPilotPatterns;

        PILOT_PATTERN_VALUE_TO_NAME = data.pilotPatternValueToName || {
            '0':'SP3_2','1':'SP3_4','2':'SP4_2','3':'SP4_4',
            '4':'SP6_2','5':'SP6_4','6':'SP8_2','7':'SP8_4',
            '8':'SP12_2','9':'SP12_4','10':'SP16_2','11':'SP16_4',
            '12':'SP24_2','13':'SP24_4','14':'SP32_2','15':'SP32_4'
        };
        PILOT_PATTERN_NAME_TO_VALUE = data.pilotPatternNameToValue || {};
        if (Object.keys(PILOT_PATTERN_NAME_TO_VALUE).length === 0) {
            Object.entries(PILOT_PATTERN_VALUE_TO_NAME).forEach(([v, n]) => {
                PILOT_PATTERN_NAME_TO_VALUE[n] = v;
            });
        }

        ALLOWED_PATTERNS_SISO = data.allowedPatternsSiso;

        const gis = ['GI1_192','GI2_384','GI3_512','GI4_768','GI5_1024','GI6_1536',
                      'GI7_2048','GI8_2432','GI9_3072','GI10_3648','GI11_4096','GI12_4864'];
        ALLOWED_PATTERNS_WALSH_HADAMARD = {};
        ALLOWED_PATTERNS_NULL_PILOT = {};
        ['8K','16K','32K'].forEach(fft => {
            ALLOWED_PATTERNS_WALSH_HADAMARD[fft] = {};
            ALLOWED_PATTERNS_NULL_PILOT[fft] = {};
            gis.forEach(gi => {
                ALLOWED_PATTERNS_WALSH_HADAMARD[fft][gi] = ALL_PILOT_PATTERNS;
                ALLOWED_PATTERNS_NULL_PILOT[fft][gi] = ALL_PILOT_PATTERNS;
            });
        });

        _validationTablesLoaded = true;
        return true;
    } catch (err) {
        return false;
    }
}

class ATSC3ValidationSystem {
    constructor() {
        this.currentPreambleConfig = null;
        this.validationErrors = [];
        this.warningMode = true;
        this._ready = loadValidationTables().then(() => {
            this.initializeEventListeners();
        });
    }

    initializeEventListeners() {
        document.addEventListener('change', (e) => {
            if (e.target.id === 'preamble_structure') {
                this.handlePreambleStructureChange(e.target.value);
            }

            if (e.target.id === 'l1b_mimo_scatterred_pilot_encoding') {
                this.updateAllSubframePilotPatterns();
            }

            if (e.target.id && e.target.id.startsWith('plp_mimo_')) {
                const subframeIndex = e.target.id.split('_')[2];
                this.filterSubframePilotPattern(parseInt(subframeIndex));
            }

            if (e.target.id && e.target.id.startsWith('fft_size_')) {
                const subframeIndex = e.target.id.split('_')[2];
                this.validateSubframeFFT(subframeIndex);
            }

            if (e.target.id && e.target.id.startsWith('guard_interval_')) {
                const subframeIndex = e.target.id.split('_')[2];
                this.validateSubframeGuardInterval(subframeIndex);
            }

            if (e.target.id && e.target.id.startsWith('spilot_pattern_')) {
                const subframeIndex = e.target.id.split('_')[2];
                this.validateSubframePilotPattern(subframeIndex);
            }
        });
    }

    handlePreambleStructureChange(preambleValue) {
        const preambleIndex = parseInt(preambleValue);
        this.currentPreambleConfig = PREAMBLE_CONFIGURATIONS[preambleIndex];

        if (!this.currentPreambleConfig) {
            return;
        }

        this.applyPreambleConfigToAllSubframes();
        this.filterAllSubframePilotPatterns();
        this.showPreambleConfigurationAlert();

        setTimeout(() => {
            if (typeof window.autoFillAllPlps === 'function') {
                window.autoFillAllPlps();
            }
        }, 150);
    }

    applyPreambleConfigToAllSubframes() {
        if (!this.currentPreambleConfig) return;

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        const requiredFFT = FFT_VALUE_MAP[this.currentPreambleConfig.fft];
        const requiredGI = GI_VALUE_MAP[this.currentPreambleConfig.gi];

        for (let i = 0; i < subframeCount; i++) {
            const fftSelect = document.getElementById(`fft_size_${i}`);
            if (fftSelect) {
                fftSelect.value = requiredFFT;
                Array.from(fftSelect.options).forEach(option => {
                    if (option.value === requiredFFT) {
                        option.hidden = false;
                        option.style.display = '';
                    } else {
                        option.hidden = true;
                        option.style.display = 'none';
                    }
                });
            }

            const giSelect = document.getElementById(`guard_interval_${i}`);
            if (giSelect) {
                giSelect.value = requiredGI;
                Array.from(giSelect.options).forEach(option => {
                    if (option.value === requiredGI) {
                        option.hidden = false;
                        option.style.display = '';
                    } else {
                        option.hidden = true;
                        option.style.display = 'none';
                    }
                });
            }
        }

    }

    applyPreambleConfigToSubframe(subframeIndex) {
        if (!this.currentPreambleConfig) return;

        const requiredFFT = FFT_VALUE_MAP[this.currentPreambleConfig.fft];
        const requiredGI = GI_VALUE_MAP[this.currentPreambleConfig.gi];

        const fftSelect = document.getElementById(`fft_size_${subframeIndex}`);
        if (fftSelect) {
            fftSelect.value = requiredFFT;
            Array.from(fftSelect.options).forEach(option => {
                if (option.value === requiredFFT) {
                    option.hidden = false;
                    option.style.display = '';
                } else {
                    option.hidden = true;
                    option.style.display = 'none';
                }
            });
        }

        const giSelect = document.getElementById(`guard_interval_${subframeIndex}`);
        if (giSelect) {
            giSelect.value = requiredGI;
            Array.from(giSelect.options).forEach(option => {
                if (option.value === requiredGI) {
                    option.hidden = false;
                    option.style.display = '';
                } else {
                    option.hidden = true;
                    option.style.display = 'none';
                }
            });
        }

        this.filterSubframePilotPattern(subframeIndex);
    }

    filterAllSubframePilotPatterns() {
        if (!this.currentPreambleConfig) return;

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');

        for (let i = 0; i < subframeCount; i++) {
            this.filterSubframePilotPattern(i);
        }
    }

    filterSubframePilotPattern(subframeIndex) {
        const pilotSelect = document.getElementById(`spilot_pattern_${subframeIndex}`);
        if (!pilotSelect) return;

        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        const isWalshHadamard = mimoEncodingSelect ? mimoEncodingSelect.value === '0' : true;
        const allowedPatterns = this.getAllowedPilotPatterns(isWalshHadamard, subframeIndex);

        const currentValue = pilotSelect.value;
        const isCurrentValid = allowedPatterns.length === 0 || allowedPatterns.includes(currentValue);

        Array.from(pilotSelect.options).forEach(option => {
            const isAllowed = allowedPatterns.length === 0 || allowedPatterns.includes(option.value);

            option.hidden = false;
            option.disabled = false;
            option.style.display = '';

            if (isAllowed) {
                option.style.color = '#28a745';
                option.style.fontWeight = 'bold';
            } else {
                option.style.color = '#856404';
                option.style.fontWeight = 'normal';
            }
        });

        if (!isCurrentValid && allowedPatterns.length > 0) {
            pilotSelect.style.borderColor = '#ffc107';
            this.showWarningTooltip(pilotSelect, subframeIndex, allowedPatterns);
        } else {
            pilotSelect.style.borderColor = '#28a745';
            this.removeWarningTooltip(subframeIndex);
        }

    }

    showWarningTooltip(selectElement, subframeIndex, allowedPatterns) {
        this.removeWarningTooltip(subframeIndex);

        const currentValue = selectElement.value;
        const currentName = PILOT_PATTERN_VALUE_TO_NAME[currentValue] || `Pattern ${currentValue}`;
        const allowedNames = allowedPatterns.map(v => PILOT_PATTERN_VALUE_TO_NAME[v]).join(', ');

        const tooltip = document.createElement('div');
        tooltip.id = `pilot-warning-${subframeIndex}`;
        tooltip.className = 'pilot-pattern-warning';
        tooltip.innerHTML = `
            <div class="warning-icon">!</div>
            <div class="warning-content">
                <strong>Warning:</strong> ${currentName} is not recommended for this FFT/GI combination.<br>
                <small>Allowed patterns per Table 8.3: <strong>${allowedNames || 'N/A'}</strong></small>
            </div>
        `;

        selectElement.parentNode.insertBefore(tooltip, selectElement.nextSibling);
    }

    removeWarningTooltip(subframeIndex) {
        const existingTooltip = document.getElementById(`pilot-warning-${subframeIndex}`);
        if (existingTooltip) {
            existingTooltip.remove();
        }
    }

    addValidationIndicator() {
        if (!this.currentPreambleConfig) return;

        const existingIndicator = document.getElementById('validation-indicator');
        if (existingIndicator) {
            existingIndicator.remove();
        }

        const indicator = document.createElement('div');
        indicator.id = 'validation-indicator';
        indicator.className = 'validation-indicator-active';
        indicator.innerHTML = `
            <div class="indicator-content" title="ATSC 3.0 Validation Active - Click for details">
                <span class="indicator-text">Validation Active</span>
            </div>
        `;

        indicator.onclick = () => this.showDetailedRestrictions();

        document.body.appendChild(indicator);

        setTimeout(() => {
            if (indicator) {
                indicator.classList.add('minimized');
            }
        }, 3000);
    }

    showPreambleConfigurationAlert() {
        if (!this.currentPreambleConfig) return;

        const { fft, gi, dx, mode } = this.currentPreambleConfig;

        this.removeExistingAlerts();

        this.addValidationIndicator();

        setTimeout(() => {
            this.validateAllSubframes();
        }, 100);
    }

    removeExistingAlerts() {
        const existingAlert = document.getElementById('preamble-config-alert');
        if (existingAlert) {
            existingAlert.remove();
        }

        document.querySelectorAll('.validation-error, .subframe-validation-error').forEach(el => {
            el.remove();
        });
    }

    validateAllSubframes() {
        if (!this.currentPreambleConfig) return;

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');

        for (let i = 0; i < subframeCount; i++) {
            this.validateSubframe(i);
        }
    }

    validateSubframe(subframeIndex) {
        this.validateSubframeFFT(subframeIndex);
        this.validateSubframeGuardInterval(subframeIndex);
        this.validateSubframePilotPattern(subframeIndex);
    }

    validateSubframeFFT(subframeIndex) {
        if (!this.currentPreambleConfig) return;

        const fftSelect = document.getElementById(`fft_size_${subframeIndex}`);
        if (!fftSelect) return;

        const currentFFT = this.getFFTFromValue(fftSelect.value);
        const requiredFFT = this.currentPreambleConfig.fft;

        if (currentFFT !== requiredFFT) {
            fftSelect.style.borderColor = '#ff0000ff';
            this.restrictFFTOptions(fftSelect, requiredFFT);
        } else {
            fftSelect.style.borderColor = '';
            this.clearSelectStyles(fftSelect);
        }
    }

    validateSubframeGuardInterval(subframeIndex) {
        if (!this.currentPreambleConfig) return;

        const giSelect = document.getElementById(`guard_interval_${subframeIndex}`);
        if (!giSelect) return;

        const requiredGI = GI_VALUE_MAP[this.currentPreambleConfig.gi];

        if (giSelect.value !== requiredGI) {
            giSelect.style.borderColor = '#ff0000ff';
            this.restrictGuardIntervalOptions(giSelect, requiredGI);
        } else {
            giSelect.style.borderColor = '';
            this.clearSelectStyles(giSelect);
        }
    }

    validateSubframePilotPattern(subframeIndex) {
        if (!this.currentPreambleConfig) return;

        const pilotSelect = document.getElementById(`spilot_pattern_${subframeIndex}`);
        if (!pilotSelect) return;

        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        if (!mimoEncodingSelect) return;

        const isWalshHadamard = mimoEncodingSelect.value === '0';
        this.filterSubframePilotPattern(subframeIndex);
    }

    clearSelectStyles(selectElement) {
        Array.from(selectElement.options).forEach(option => {
            option.style.fontWeight = '';
            option.style.color = '';
            option.style.fontStyle = '';
        });
    }

    getAllowedPilotPatterns(isWalshHadamard, subframeIndex = null) {
        if (!this.currentPreambleConfig) return [];

        const { fft, gi } = this.currentPreambleConfig;

        let isSISO = true;

        if (subframeIndex !== null) {
            const mimoSelect = document.getElementById(`plp_mimo_${subframeIndex}`);
            if (mimoSelect) {
                isSISO = mimoSelect.value === '0';
            }
        }

        let patternsTable;
        if (isSISO) {
            patternsTable = ALLOWED_PATTERNS_SISO;
        } else {
            patternsTable = isWalshHadamard ? ALLOWED_PATTERNS_WALSH_HADAMARD : ALLOWED_PATTERNS_NULL_PILOT;
        }

        return patternsTable[fft] && patternsTable[fft][gi] ? patternsTable[fft][gi] : [];
    }

    getAllowedPilotPatternNames(isWalshHadamard, subframeIndex = null) {
        const allowedValues = this.getAllowedPilotPatterns(isWalshHadamard, subframeIndex);
        return allowedValues.map(v => PILOT_PATTERN_VALUE_TO_NAME[v] || v);
    }

    restrictFFTOptions(selectElement, requiredFFT) {
        const requiredValue = FFT_VALUE_MAP[requiredFFT];

        Array.from(selectElement.options).forEach(option => {
            if (option.value === requiredValue) {
                option.style.fontWeight = 'bold';
                option.style.color = '#28a745';
            } else {
                option.style.color = '#856404';
                option.style.fontStyle = 'italic';
            }
        });

        if (selectElement.value !== requiredValue) {
            selectElement.style.borderColor = '#ff0000ff';
        }
    }

    restrictGuardIntervalOptions(selectElement, requiredValue) {
        Array.from(selectElement.options).forEach(option => {
            if (option.value === requiredValue) {
                option.style.fontWeight = 'bold';
                option.style.color = '#28a745';
            } else {
                option.style.color = '#856404';
                option.style.fontStyle = 'italic';
            }
        });

        if (selectElement.value !== requiredValue) {
            selectElement.style.borderColor = '#ff0000ff';
        }
    }

    restrictPilotPatternOptions(selectElement, allowedValues) {
        Array.from(selectElement.options).forEach(option => {
            if (allowedValues.includes(option.value)) {
                option.style.fontWeight = 'bold';
                option.style.color = '#28a745';
            } else {
                option.style.color = '#856404';
                option.style.fontStyle = 'italic';
            }
        });

        if (!allowedValues.includes(selectElement.value)) {
            selectElement.style.borderColor = '#ffc107';
        }
    }

    resetSelectOptions() {
        document.querySelectorAll('select').forEach(select => {
            select.style.borderColor = '';

            Array.from(select.options).forEach(option => {
                option.disabled = false;
                option.style.color = '';
                option.style.fontWeight = '';
                option.style.fontStyle = '';
            });
        });
    }

    validateSubframeQuietly(subframeIndex) {
        if (!this.currentPreambleConfig) return;

        const fftSelect = document.getElementById(`fft_size_${subframeIndex}`);
        if (fftSelect) {
            const currentFFT = this.getFFTFromValue(fftSelect.value);
            const requiredFFT = this.currentPreambleConfig.fft;

            if (currentFFT !== requiredFFT) {
                this.restrictFFTOptions(fftSelect, requiredFFT);
            } else {
                this.clearSelectStyles(fftSelect);
            }
        }

        const giSelect = document.getElementById(`guard_interval_${subframeIndex}`);
        if (giSelect) {
            const requiredGI = GI_VALUE_MAP[this.currentPreambleConfig.gi];

            if (giSelect.value !== requiredGI) {
                this.restrictGuardIntervalOptions(giSelect, requiredGI);
            } else {
                this.clearSelectStyles(giSelect);
            }
        }

        const pilotSelect = document.getElementById(`spilot_pattern_${subframeIndex}`);
        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');

        if (pilotSelect && mimoEncodingSelect) {
            const isWalshHadamard = mimoEncodingSelect.value === '0';
            const allowedPatterns = this.getAllowedPilotPatterns(isWalshHadamard, subframeIndex);

            if (!allowedPatterns.includes(pilotSelect.value)) {
                this.restrictPilotPatternOptions(pilotSelect, allowedPatterns);
            } else {
                this.clearSelectStyles(pilotSelect);
            }
        }
    }

    updateAllSubframePilotPatterns() {
        this.filterAllSubframePilotPatterns();
    }

    showDetailedRestrictions() {
        if (!this.currentPreambleConfig) return;

        const { fft, gi, dx, mode } = this.currentPreambleConfig;
        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        const isWalshHadamard = mimoEncodingSelect ? mimoEncodingSelect.value === '0' : true;
        const allowedPatterns = this.getAllowedPilotPatterns(isWalshHadamard);
        const allowedNames = allowedPatterns.map(v => PILOT_PATTERN_VALUE_TO_NAME[v]).join(', ');

        const modal = document.createElement('div');
        modal.className = 'modal-overlay';
        modal.id = 'restrictions-modal';
        modal.innerHTML = `
            <div class="modal-content">
                <div class="modal-header">
                    <h3>ATSC 3.0 Configuration Details (Table 8.3)</h3>
                    <button class="modal-close" onclick="document.getElementById('restrictions-modal').remove()">×</button>
                </div>
                <div class="modal-body">
                    <h4>Current Preamble Configuration</h4>
                    <ul>
                        <li><strong>FFT Size:</strong> ${fft}</li>
                        <li><strong>Guard Interval:</strong> ${gi}</li>
                        <li><strong>Dx Value:</strong> ${dx}</li>
                        <li><strong>L1-Basic Mode:</strong> ${mode}</li>
                    </ul>

                    <h4>Allowed Scattered Pilot Patterns (Table 8.3)</h4>
                    <p style="font-size: 14px; background: #d4edda; padding: 10px; border-radius: 5px;">
                        <strong>${allowedNames || 'N/A - This combination is not defined in Table 8.3'}</strong>
                    </p>

                    <div class="restrictions-note">
                        <p><strong>Note:</strong> All patterns are selectable, but patterns outside Table 8.3
                        will be highlighted in yellow. This follows ATSC A/322:2025-07 specification.</p>
                    </div>

                    <h4>Legend</h4>
                    <ul>
                        <li><span style="color: #155724; background: #d4edda; padding: 2px 6px; border-radius: 3px;">Green</span> - Pattern allowed by Table 8.3</li>
                        <li><span style="color: #856404; background: #fff3cd; padding: 2px 6px; border-radius: 3px;">Yellow</span> - Pattern not recommended (warning only)</li>
                    </ul>
                </div>
                <div class="modal-footer">
                    <button class="btn btn-secondary" onclick="document.getElementById('restrictions-modal').remove()">Close</button>
                </div>
            </div>
        `;

        document.body.appendChild(modal);
    }

    getFFTFromValue(value) {
        const reverseMap = { '0': '8K', '1': '16K', '2': '32K' };
        return reverseMap[value] || null;
    }

    getGIFromValue(value) {
        const reverseMap = {};
        Object.entries(GI_VALUE_MAP).forEach(([name, val]) => {
            reverseMap[val] = name;
        });
        return reverseMap[value] || null;
    }

    getValidationWarnings() {
        const warnings = [];

        if (!this.currentPreambleConfig) return warnings;

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        const { fft, gi } = this.currentPreambleConfig;

        for (let i = 0; i < subframeCount; i++) {
            const pilotSelect = document.getElementById(`spilot_pattern_${i}`);
            const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');

            if (pilotSelect && mimoEncodingSelect) {
                const isWalshHadamard = mimoEncodingSelect.value === '0';
                const allowedPatterns = this.getAllowedPilotPatterns(isWalshHadamard, i);

                if (allowedPatterns.length > 0 && !allowedPatterns.includes(pilotSelect.value)) {
                    const currentName = PILOT_PATTERN_VALUE_TO_NAME[pilotSelect.value];
                    const allowedNames = allowedPatterns.map(v => PILOT_PATTERN_VALUE_TO_NAME[v]).join(', ');

                    warnings.push({
                        subframe: i,
                        type: 'pilot_pattern',
                        message: `Subframe ${i}: ${currentName} is not recommended for ${fft} + ${gi}. Allowed: ${allowedNames}`
                    });
                }
            }
        }

        return warnings;
    }

    isCurrentConfigurationValid() {
        if (!this.currentPreambleConfig) return true;
        const warnings = this.getValidationWarnings();
        return warnings.length === 0;
    }

    applyAutoCorrection() {
        if (!this.currentPreambleConfig) return;
        this.applyPreambleConfigToAllSubframes();
        this.filterAllSubframePilotPatterns();

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        const isWalshHadamard = mimoEncodingSelect ? mimoEncodingSelect.value === '0' : true;

        for (let i = 0; i < subframeCount; i++) {
            const pilotSelect = document.getElementById(`spilot_pattern_${i}`);
            if (pilotSelect) {
                const allowedPatterns = this.getAllowedPilotPatterns(isWalshHadamard, i);
                if (allowedPatterns.length > 0 && !allowedPatterns.includes(pilotSelect.value)) {
                    pilotSelect.value = allowedPatterns[0];
                }
            }
        }

        this.validateAllSubframes();
    }
}

const validationStyles = `
<style id="atsc-validation-styles">
.pilot-pattern-warning {
    display: flex;
    align-items: flex-start;
    gap: 8px;
    background: linear-gradient(135deg, #fff3cd 0%, #ffeeba 100%);
    border: 1px solid #ffc107;
    border-left: 4px solid #ffc107;
    border-radius: 4px;
    padding: 10px 12px;
    margin-top: 8px;
    font-size: 12px;
    color: #856404;
    animation: warningSlideIn 0.3s ease;
}

.pilot-pattern-warning .warning-icon {
    font-size: 14px;
    font-weight: bold;
    flex-shrink: 0;
    width: 20px;
    height: 20px;
    background: #ffc107;
    color: #856404;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
}

.pilot-pattern-warning .warning-content {
    flex: 1;
    line-height: 1.4;
}

.pilot-pattern-warning .warning-content strong {
    color: #664d03;
}

.pilot-pattern-warning .warning-content small {
    display: block;
    margin-top: 4px;
    color: #856404;
}

@keyframes warningSlideIn {
    from {
        opacity: 0;
        transform: translateY(-10px);
    }
    to {
        opacity: 1;
        transform: translateY(0);
    }
}

select.has-warning {
    border-color: #ffc107 !important;
}

select.is-valid {
    border-color: #28a745 !important;
}

@keyframes shake {
    0%, 100% { transform: translateX(0); }
    10%, 30%, 50%, 70%, 90% { transform: translateX(-2px); }
    20%, 40%, 60%, 80% { transform: translateX(2px); }
}

.shake {
    animation: shake 0.5s ease;
}

.error-content {
    padding: 8px 12px;
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 13px;
}

.error-icon {
    font-size: 14px;
}

.error-message {
    flex: 1;
    color: #721c24;
    font-weight: 500;
}

.error-close {
    background: none;
    border: none;
    font-size: 16px;
    font-weight: bold;
    cursor: pointer;
    color: #721c24;
    opacity: 0.7;
    transition: opacity 0.2s;
}

.error-close:hover {
    opacity: 1;
}

.validation-error {
    border: 2px solid #dc3545 !important;
    animation: pulse 0.5s ease;
}

@keyframes pulse {
    0% { transform: scale(1); }
    50% { transform: scale(1.02); }
    100% { transform: scale(1); }
}

.modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0,0,0,0.5);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 1000;
    animation: fadeIn 0.3s ease;
}

@keyframes fadeIn {
    from { opacity: 0; }
    to { opacity: 1; }
}

.modal-content {
    background: white;
    border-radius: 8px;
    max-width: 600px;
    width: 90%;
    max-height: 80vh;
    overflow-y: auto;
    box-shadow: 0 10px 25px rgba(0,0,0,0.2);
    animation: slideUp 0.3s ease;
}

@keyframes slideUp {
    from { opacity: 0; transform: translateY(30px); }
    to { opacity: 1; transform: translateY(0); }
}

.modal-header {
    background: #f8f9fa;
    padding: 15px 20px;
    border-bottom: 1px solid #dee2e6;
    display: flex;
    justify-content: space-between;
    align-items: center;
}

.modal-header h3 {
    margin: 0;
    font-size: 18px;
    color: #495057;
}

.modal-close {
    background: none;
    border: none;
    font-size: 24px;
    font-weight: bold;
    cursor: pointer;
    color: #6c757d;
    opacity: 0.7;
    transition: opacity 0.2s;
}

.modal-close:hover {
    opacity: 1;
}

.modal-body {
    padding: 20px;
}

.modal-body h4 {
    color: #495057;
    margin: 15px 0 10px 0;
    font-size: 16px;
    border-bottom: 1px solid #dee2e6;
    padding-bottom: 5px;
}

.modal-body ul {
    margin: 10px 0;
    padding-left: 20px;
}

.modal-body li {
    margin-bottom: 5px;
}

.restrictions-note {
    background: #e9ecef;
    padding: 15px;
    border-radius: 5px;
    margin-top: 15px;
    border-left: 4px solid #007bff;
}

.restrictions-note p {
    margin: 0;
    font-style: italic;
    color: #495057;
}

.modal-footer {
    background: #f8f9fa;
    padding: 15px 20px;
    border-top: 1px solid #dee2e6;
    display: flex;
    justify-content: flex-end;
}

.modal-footer .btn {
    padding: 8px 20px;
    border-radius: 4px;
    border: none;
    font-size: 14px;
    font-weight: 500;
    cursor: pointer;
    transition: all 0.2s;
}

.modal-footer .btn.btn-secondary {
    background: #6c757d;
    color: white;
}

.validation-indicator-active {
    position: fixed;
    top: 20px;
    right: 20px;
    z-index: 999;
    background: linear-gradient(135deg, #28a745, #20c997);
    color: white;
    border-radius: 25px;
    padding: 8px 15px;
    box-shadow: 0 4px 12px rgba(0,0,0,0.15);
    cursor: pointer;
    transition: all 0.3s ease;
    animation: slideInRight 0.5s ease;
}

.validation-indicator-active:hover {
    transform: translateY(-2px);
    box-shadow: 0 6px 16px rgba(0,0,0,0.2);
}

.validation-indicator-active.minimized {
    padding: 8px;
    border-radius: 50%;
    width: 40px;
    height: 40px;
    display: flex;
    align-items: center;
    justify-content: center;
}

.validation-indicator-active.minimized .indicator-text {
    display: none;
}

.indicator-content {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 12px;
    font-weight: 500;
}

.indicator-text {
    white-space: nowrap;
}

@keyframes slideInRight {
    from {
        opacity: 0;
        transform: translateX(100px);
    }
    to {
        opacity: 1;
        transform: translateX(0);
    }
}

select[style*="border-color: rgb(255, 193, 7)"] {
    transition: all 0.3s ease;
}
</style>
`;

document.head.insertAdjacentHTML('beforeend', validationStyles);

const atscValidation = new ATSC3ValidationSystem();

window.atscValidation = atscValidation;

window.PILOT_PATTERN_VALUE_TO_NAME = PILOT_PATTERN_VALUE_TO_NAME;
window.PILOT_PATTERN_NAME_TO_VALUE = PILOT_PATTERN_NAME_TO_VALUE;
window.ALLOWED_PATTERNS_SISO = ALLOWED_PATTERNS_SISO;
