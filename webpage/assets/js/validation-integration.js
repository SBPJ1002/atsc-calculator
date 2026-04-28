class CodeRateVisualValidator {
    constructor(integration) {
        this.integration = integration;
        this.initializeEventListeners();
    }

    initializeEventListeners() {
        document.addEventListener('change', (e) => {
            if (e.target.id && (e.target.id.startsWith('fec_type_') || e.target.id.startsWith('mod_order_'))) {
                const plpIndex = this.extractPlpIndex(e.target.id);
                if (plpIndex !== null) {
                    this.updateCodeRateOptions(plpIndex);
                }
            }
        });
    }

    extractPlpIndex(fieldId) {
        const match = fieldId.match(/_(\d+)$/);
        return match ? parseInt(match[1]) : null;
    }

    updateCodeRateOptions(plpIndex) {
        const fecTypeSelect = document.getElementById(`fec_type_${plpIndex}`);
        const modOrderSelect = document.getElementById(`mod_order_${plpIndex}`);
        const codeRateSelect = document.getElementById(`code_rate_${plpIndex}`);

        if (!fecTypeSelect || !modOrderSelect || !codeRateSelect) {
            return;
        }

        const fecType = fecTypeSelect.value;
        const modOrder = modOrderSelect.value;

        const validCodeRates = this.getValidCodeRatesForCombination(fecType, modOrder);

        this.applyCodeRateStyles(codeRateSelect, validCodeRates);

        if (!validCodeRates.includes(codeRateSelect.value)) {
            this.highlightInvalidSelection(codeRateSelect);
        } else {
            this.clearInvalidHighlight(codeRateSelect);
        }
    }

    getValidCodeRatesForCombination(fecType, modOrder) {
        if (!this.integration.FEC_TYPE_MAP || !this.integration.ATSC_MODULATION_CODING_TABLES) return [];
        const ldpcSize = this.integration.FEC_TYPE_MAP[fecType];
        if (!ldpcSize) return [];

        return this.integration.ATSC_MODULATION_CODING_TABLES[ldpcSize]?.[modOrder] || [];
    }

    applyCodeRateStyles(selectElement, validCodeRates) {
        Array.from(selectElement.options).forEach(option => {
            option.style.fontWeight = '';
            option.style.color = '';
            option.style.fontStyle = '';
        });

        Array.from(selectElement.options).forEach(option => {
            if (validCodeRates.includes(option.value)) {
                option.style.fontWeight = 'bold';
                option.style.color = '#28a745';
            } else {
                option.style.color = '#856404';
                option.style.fontStyle = 'italic';
            }
        });
    }

    highlightInvalidSelection(selectElement) {
        selectElement.style.borderColor = '#ff0000ff';
    }

    clearInvalidHighlight(selectElement) {
        selectElement.style.borderColor = '';
    }

    validateAllCodeRates() {
        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');

        for (let i = 0; i < subframeCount; i++) {
            const plpCountInput = document.getElementById(`plp-count-${i}`);
            const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 1 : 1;

            for (let j = 0; j < plpCount; j++) {
                this.updateCodeRateOptions(j);
            }
        }
    }

    showCodeRateTooltip(plpIndex) {
        const fecTypeSelect = document.getElementById(`fec_type_${plpIndex}`);
        const modOrderSelect = document.getElementById(`mod_order_${plpIndex}`);

        if (!fecTypeSelect || !modOrderSelect) return;

        const fecType = fecTypeSelect.value;
        const modOrder = modOrderSelect.value;
        const validCodeRates = this.getValidCodeRateNames(fecType, modOrder);

        const MODULATION_NAMES = {
            "0": "QPSK", "1": "16QAM", "2": "64QAM",
            "3": "256QAM", "4": "1024QAM", "5": "4096QAM"
        };

        const FEC_NAMES = {
            "0": "BCH+16K LDPC", "1": "BCH+64K LDPC", "2": "CRC+16K LDPC",
            "3": "CRC+64K LDPC", "4": "16K LDPC only", "5": "64K LDPC only"
        };

        const modName = MODULATION_NAMES[modOrder] || "Desconhecido";
        const fecName = FEC_NAMES[fecType] || "Desconhecido";
    }

    getValidCodeRateNames(fecType, modOrder) {
        const validCodeRates = this.getValidCodeRatesForCombination(fecType, modOrder);
        const CODE_RATE_NAMES = {
            "0": "2/15", "1": "3/15", "2": "4/15", "3": "5/15",
            "4": "6/15", "5": "7/15", "6": "8/15", "7": "9/15",
            "8": "10/15", "9": "11/15", "10": "12/15", "11": "13/15"
        };

        return validCodeRates.map(rate => CODE_RATE_NAMES[rate] || rate);
    }
}

class ValidationIntegration {
    constructor() {
        // Save original function refs from window
        this._originalSubmitForm = window.submitForm;
        this._originalGenerateMenus = window.generateMenus;
        this._originalGeneratePLPMenusAndFields = window.generatePLPMenusAndFields;
        this._originalLoadConfigBackup = window.loadConfigBackup;
        this._originalImportConfigFile = window.importConfigFile;
        this._originalToggleL1dBsid = window.toggleL1dBsid;

        // Init tables
        this.ATSC_MODULATION_CODING_TABLES = {};
        this.FEC_TYPE_MAP = {};
        this.MODULATION_MAP = {};
        this.CODE_RATE_MAP = {};
        this._modCodTablesLoaded = false;

        // Load modulation/coding tables
        this.loadModCodTables();

        // Create code rate validator
        this.codeRateValidator = new CodeRateVisualValidator(this);

        // Wrap global functions with validation
        this._wrapGlobalFunctions();

        // Inject CSS
        this._injectCSS();

        // Init validation on load
        this._initializeValidationOnLoad();
    }

    _wrapGlobalFunctions() {
        const self = this;

        window.generateMenus = function(count) {
            self._originalGenerateMenus(count);

            setTimeout(() => {
                if (window.atscValidation) {
                    if (window.atscValidation.currentPreambleConfig) {
                        window.atscValidation.applyPreambleConfigToAllSubframes();
                        window.atscValidation.filterAllSubframePilotPatterns();
                    }
                    window.atscValidation.validateAllSubframes();
                }
            }, 500);
        };

        window.generatePLPMenusAndFields = function(subframeIndex) {
            self._originalGeneratePLPMenusAndFields(subframeIndex);

            setTimeout(() => {
                if (window.atscValidation) {
                    if (window.atscValidation.currentPreambleConfig) {
                        window.atscValidation.applyPreambleConfigToSubframe(subframeIndex);
                    }
                    window.atscValidation.validateSubframe(subframeIndex);
                }
                self.codeRateValidator.validateAllCodeRates();
            }, 500);
        };

        if (self._originalLoadConfigBackup) {
            window.loadConfigBackup = function() {
                const result = self._originalLoadConfigBackup();

                setTimeout(() => {
                    if (window.atscValidation) {
                        window.atscValidation.validateAllSubframes();
                    }
                    self.codeRateValidator.validateAllCodeRates();
                }, 2000);

                return result;
            };
        }

        if (self._originalImportConfigFile) {
            window.importConfigFile = function(event) {
                self._originalImportConfigFile(event);

                setTimeout(() => {
                    if (window.atscValidation) {
                        window.atscValidation.validateAllSubframes();
                    }
                }, 2000);
            };
        }

        if (self._originalToggleL1dBsid) {
            window.toggleL1dBsid = function() {
                self._originalToggleL1dBsid();

                setTimeout(() => {
                    if (window.atscValidation) {
                        window.atscValidation.validateAllSubframes();
                    }
                }, 100);
            };
        }

        // Consolidated submitForm wrapper
        window.submitForm = function() {
            // Step 1: validateAllFields (global, from form-handler.js)
            const emptyFields = validateAllFields();

            if (emptyFields.length > 0) {
                let alertMessage = 'Os seguintes campos estão vazios:\n\n';
                emptyFields.forEach(field => {
                    alertMessage += '-> ' + field + '\n';
                });
                alertMessage += '\nPor favor, preencha todos os campos antes de enviar.';

                alert(alertMessage);
                return;
            }

            // Step 2: validateATSCConfiguration
            const atscValidationErrors = self.validateATSCConfiguration();

            if (atscValidationErrors.length > 0) {
                let alertMessage = 'ERRO: Configuração ATSC 3.0 inválida detectada!\n\n';
                alertMessage += 'Os seguintes campos estão com configuração incorreta:\n\n';
                atscValidationErrors.forEach(error => {
                    alertMessage += '-> ' + error + '\n';
                });
                alertMessage += '\nA configuração não pode ser enviada com erros de validação ATSC.\n';
                alertMessage += 'Por favor, corrija os valores antes de continuar.';

                alert(alertMessage);
                return;
            }

            // Step 3: validateModulationCodingCombinations
            const modCodErrors = self.validateModulationCodingCombinations();

            if (modCodErrors.length > 0) {
                let alertMessage = 'ERRO: Combinações de modulação e codificação inválidas!\n\n';
                alertMessage += 'As seguintes combinações não são permitidas pelo padrão ATSC 3.0:\n\n';
                modCodErrors.forEach(error => {
                    alertMessage += '-> ' + error + '\n';
                });
                alertMessage += '\nVerifique as Tables 6.12 e 6.13 do padrão ATSC 3.0.\n';
                alertMessage += 'Por favor, corrija as combinações antes de continuar.';

                alert(alertMessage);
                return;
            }

            // Step 4: validateFrameGeneration (global, from main.js)
            const frameValidationErrors = validateFrameGeneration();

            if (frameValidationErrors.length > 0) {
                let alertMessage = 'ERRO: Configuração inválida detectada!\n\n';

                const hasL1BL1DError = frameValidationErrors.some(error =>
                    error.message && error.message.includes('L1B Version = 1 e L1D Version = 0')
                );

                if (hasL1BL1DError) {
                    alertMessage += 'ERRO CRÍTICO DE VERSÃO:\n';
                    alertMessage += 'A combinação L1B Version = 1 e L1D Version = 0 não é permitida!\n\n';
                }

                frameValidationErrors.forEach(error => {
                    alertMessage += '->' + (error.message || error) + '\n';
                });
                alertMessage += '\nA configuração não pode ser enviada com erros.\nPor favor, corrija os valores antes de continuar.';

                alert(alertMessage);
                return;
            }

            // Call original submitForm
            if (self._originalSubmitForm) {
                self._originalSubmitForm();
            }
        };
    }

    _injectCSS() {
        const codeRateStyles = `
<style>
select[id^="code_rate_"] option {
    transition: all 0.3s ease;
}

select[id^="code_rate_"][style*="border-color: rgb(255, 0, 0)"] {
    animation: invalidPulse 1s ease-in-out;
}

@keyframes invalidPulse {
    0%, 100% {
        border-color: #ff0000ff;
        box-shadow: 0 0 0 0 rgba(255, 0, 0, 0.7);
    }
    50% {
        border-color: #ff0000ff;
        box-shadow: 0 0 0 4px rgba(255, 0, 0, 0.3);
    }
}

.code-rate-info {
    position: relative;
    display: inline-block;
    margin-left: 5px;
    cursor: help;
}

.code-rate-info:hover::after {
    content: attr(data-tooltip);
    position: absolute;
    bottom: 125%;
    left: 50%;
    transform: translateX(-50%);
    background: #333;
    color: white;
    padding: 8px 12px;
    border-radius: 4px;
    white-space: nowrap;
    z-index: 1000;
    font-size: 12px;
}
</style>
`;
        document.head.insertAdjacentHTML('beforeend', codeRateStyles);
    }

    _initializeValidationOnLoad() {
        const init = () => {
            setTimeout(() => {
                if (window.atscValidation) {
                    const preambleSelect = document.getElementById('preamble_structure');
                    if (preambleSelect && preambleSelect.value) {
                        window.atscValidation.handlePreambleStructureChange(preambleSelect.value);
                    }
                }
            }, 3000);

            setTimeout(() => {
                this._checkSystemCompatibility();
            }, 1000);
        };

        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', init);
        } else {
            init();
        }
    }

    _checkSystemCompatibility() {
        const requiredFunctions = [
            'generateMenus',
            'generatePLPMenusAndFields',
            'submitForm'
        ];

        const requiredElements = [
            'preamble_structure',
            'l1b_mimo_scatterred_pilot_encoding',
            'number_of_subframes'
        ];

        let missingFunctions = requiredFunctions.filter(fn => typeof window[fn] !== 'function');
        let missingElements = requiredElements.filter(id => !document.getElementById(id));

        if (missingFunctions.length > 0 || missingElements.length > 0) {
            return false;
        }

        return true;
    }

    async loadModCodTables() {
        if (this._modCodTablesLoaded) return true;
        try {
            const resp = await fetch('api.php', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ flag: 'getValidationTables' })
            });
            const data = await resp.json();

            this.ATSC_MODULATION_CODING_TABLES = data.modulationCodingTables;
            this.FEC_TYPE_MAP = data.fecTypeMap;
            this.MODULATION_MAP = data.modulationMap;
            this.CODE_RATE_MAP = data.codeRateMap;

            this._modCodTablesLoaded = true;
            return true;
        } catch (err) {
            return false;
        }
    }

    showValidationStatus() {
        if (!window.atscValidation) {
            return;
        }

        const isValid = window.atscValidation.isCurrentConfigurationValid();
        const preambleConfig = window.atscValidation.currentPreambleConfig;

        let status = 'Sistema de Validação ATSC 3.0:\n';
        status += `Status: ${isValid ? ' Configuração válida' : ' Configuração com problemas'}\n`;

        if (preambleConfig) {
            status += `Preâmbulo: ${preambleConfig.fft}, ${preambleConfig.gi}, Dx_${preambleConfig.dx}, Mode ${preambleConfig.mode}\n`;
        } else {
            status += 'Preâmbulo: não definido\n';
        }

        if (!isValid) {
            const notification = document.createElement('div');
            notification.className = 'validation-status-notification';
            notification.innerHTML = `
                <div style="background: #fff3cd; border: 1px solid #ffc107; padding: 10px; border-radius: 5px; margin: 10px 0;">
                    <strong> Atenção:</strong> A configuração atual não está correta.
                    <button onclick="window.atscValidation.applyAutoCorrection(); this.parentElement.remove();"
                            style="margin-left: 10px; padding: 5px 10px; background: #28a745; color: white; border: none; border-radius: 3px; cursor: pointer;">
                        Corrigir Automaticamente
                    </button>
                    <button onclick="this.parentElement.remove();"
                            style="margin-left: 5px; padding: 5px 10px; background: #6c757d; color: white; border: none; border-radius: 3px; cursor: pointer;">
                        Fechar
                    </button>
                </div>
            `;

            const body = document.body;
            if (body.firstChild) {
                body.insertBefore(notification, body.firstChild);
            } else {
                body.appendChild(notification);
            }

            setTimeout(() => {
                if (notification.parentNode) {
                    notification.remove();
                }
            }, 10000);
        }
    }

    debugValidation() {
        if (!window.atscValidation) {
            return;
        }

        const mimoSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        if (mimoSelect) {
            const isWalshHadamard = mimoSelect.value === '0';
        }

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        for (let i = 0; i < subframeCount; i++) {
            const fftSelect = document.getElementById(`fft_size_${i}`);
            const giSelect = document.getElementById(`guard_interval_${i}`);
            const pilotSelect = document.getElementById(`spilot_pattern_${i}`);
        }
    }

    validateATSCConfiguration() {
        const errors = [];

        if (!window.atscValidation || !window.atscValidation.currentPreambleConfig) {
            return errors;
        }

        const { fft: requiredFFT, gi: requiredGI } = window.atscValidation.currentPreambleConfig;
        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        const mimoEncodingSelect = document.getElementById('l1b_mimo_scatterred_pilot_encoding');
        const isWalshHadamard = mimoEncodingSelect ? mimoEncodingSelect.value === '0' : true;

        const fftMap = { '0': '8K', '1': '16K', '2': '32K' };
        const giMap = {
            '1': 'GI1_192', '2': 'GI2_384', '3': 'GI3_512', '4': 'GI4_768',
            '5': 'GI5_1024', '6': 'GI6_1536', '7': 'GI7_2048', '8': 'GI8_2432',
            '9': 'GI9_3072', '10': 'GI10_3648', '11': 'GI11_4096', '12': 'GI12_4864'
        };
        const pilotMap = {
            '0': 'SP3_2', '1': 'SP3_4', '2': 'SP4_2', '3': 'SP4_4',
            '4': 'SP6_2', '5': 'SP6_4', '6': 'SP8_2', '7': 'SP8_4',
            '8': 'SP12_2', '9': 'SP12_4', '10': 'SP16_2', '11': 'SP16_4',
            '12': 'SP24_2', '13': 'SP24_4', '14': 'SP32_2', '15': 'SP32_4'
        };

        const giValueMap = {
            'GI1_192': '1', 'GI2_384': '2', 'GI3_512': '3', 'GI4_768': '4',
            'GI5_1024': '5', 'GI6_1536': '6', 'GI7_2048': '7', 'GI8_2432': '8',
            'GI9_3072': '9', 'GI10_3648': '10', 'GI11_4096': '11', 'GI12_4864': '12'
        };

        for (let i = 0; i < subframeCount; i++) {
            const fftSelect = document.getElementById(`fft_size_${i}`);
            if (fftSelect) {
                const currentFFT = fftMap[fftSelect.value];
                if (currentFFT !== requiredFFT) {
                    errors.push(`Subframe ${i}: FFT Size deve ser ${requiredFFT} (atual: ${currentFFT})`);
                }
            }

            const giSelect = document.getElementById(`guard_interval_${i}`);
            if (giSelect) {
                const requiredGIValue = giValueMap[requiredGI] || '5';

                if (giSelect.value !== requiredGIValue) {
                    const currentGI = giMap[giSelect.value] || `Valor ${giSelect.value}`;
                    errors.push(`Subframe ${i}: Guard Interval deve ser ${requiredGI.replace('_', ' ')} (atual: ${currentGI.replace('_', ' ')})`);
                }
            }

            const pilotSelect = document.getElementById(`spilot_pattern_${i}`);
            if (pilotSelect) {
                const allowedPatterns = window.atscValidation.getAllowedPilotPatterns(isWalshHadamard);
                if (!allowedPatterns.includes(pilotSelect.value)) {
                    const currentPilot = pilotMap[pilotSelect.value] || `Valor ${pilotSelect.value}`;
                    const allowedNames = allowedPatterns.map(p => pilotMap[p] || p).join(', ');
                    errors.push(`Subframe ${i}: Pilot Pattern inválido - ${currentPilot} (permitidos: ${allowedNames})`);
                }
            }
        }

        return errors;
    }

    validateModulationCodingCombinations() {
        const errors = [];

        if (!this.ATSC_MODULATION_CODING_TABLES || !this.FEC_TYPE_MAP) {
            return errors;
        }

        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');

        for (let i = 0; i < subframeCount; i++) {
            const plpCountInput = document.getElementById(`plp-count-${i}`);
            const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 1 : 1;

            for (let j = 0; j < plpCount; j++) {
                const fecTypeSelect = document.getElementById(`fec_type_${j}`);
                const modOrderSelect = document.getElementById(`mod_order_${j}`);
                const codeRateSelect = document.getElementById(`code_rate_${j}`);

                if (!fecTypeSelect || !modOrderSelect || !codeRateSelect) {
                    continue;
                }

                const fecType = fecTypeSelect.value;
                const modOrder = modOrderSelect.value;
                const codeRate = codeRateSelect.value;

                const ldpcSize = this.FEC_TYPE_MAP[fecType];
                if (!ldpcSize) {
                    errors.push(`Subframe ${i} - PLP ${j}: FEC Type inválido (${fecType})`);
                    continue;
                }

                const supportedCodeRates = this.ATSC_MODULATION_CODING_TABLES[ldpcSize]?.[modOrder];
                if (!supportedCodeRates) {
                    const modName = this.MODULATION_MAP[modOrder] || `Mod ${modOrder}`;
                    errors.push(`Subframe ${i} - PLP ${j}: Modulação ${modName} não suportada para ${ldpcSize} LDPC`);
                    continue;
                }

                if (!supportedCodeRates.includes(codeRate)) {
                    const modName = this.MODULATION_MAP[modOrder] || `Mod ${modOrder}`;
                    const rateName = this.CODE_RATE_MAP[codeRate] || `Rate ${codeRate}`;
                    const validRates = supportedCodeRates.map(r => this.CODE_RATE_MAP[r] || r).join(', ');

                    errors.push(`Subframe ${i} - PLP ${j}: Combinação inválida - ${modName} + ${rateName} (válidos para ${modName} com ${ldpcSize} LDPC: ${validRates})`);
                }
            }
        }

        return errors;
    }

    isValidModulationCodingCombination(fecType, modOrder, codeRate) {
        const ldpcSize = this.FEC_TYPE_MAP[fecType];
        if (!ldpcSize) return false;

        const supportedCodeRates = this.ATSC_MODULATION_CODING_TABLES[ldpcSize]?.[modOrder];
        if (!supportedCodeRates) return false;

        return supportedCodeRates.includes(codeRate);
    }

    getValidCodeRatesForModulation(fecType, modOrder) {
        const ldpcSize = this.FEC_TYPE_MAP[fecType];
        if (!ldpcSize) return [];

        const supportedCodeRates = this.ATSC_MODULATION_CODING_TABLES[ldpcSize]?.[modOrder];
        return supportedCodeRates || [];
    }
}

// Singleton instance + window exports
const validationIntegration = new ValidationIntegration();

window.showValidationStatus = function() {
    validationIntegration.showValidationStatus();
};

window.validateATSCConfiguration = function() {
    return validationIntegration.validateATSCConfiguration();
};

window.validateModulationCodingCombinations = function() {
    return validationIntegration.validateModulationCodingCombinations();
};

window.isValidModulationCodingCombination = function(fecType, modOrder, codeRate) {
    return validationIntegration.isValidModulationCodingCombination(fecType, modOrder, codeRate);
};

window.getValidCodeRatesForModulation = function(fecType, modOrder) {
    return validationIntegration.getValidCodeRatesForModulation(fecType, modOrder);
};

window.initializeCodeRateValidation = function() {
    validationIntegration.codeRateValidator.validateAllCodeRates();
};

window.showCodeRateInfo = function(plpIndex) {
    validationIntegration.codeRateValidator.showCodeRateTooltip(plpIndex);
};

window.debugValidation = function() {
    validationIntegration.debugValidation();
};
