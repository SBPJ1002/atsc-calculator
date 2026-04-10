const input = document.getElementById('number_of_subframes');

if (!window.__NF_LISTENER__) {
    if (input) input.addEventListener('input', handleSubframeCountChange);
    window.__NF_LISTENER__ = true;
}

class ATSCApp {
    constructor() {
        this.currentWizardStep = 1;
        this.WIZARD_STEPS = {
            1: { label: 'Bootstrap', sections: ['botstrap'] },
            2: { label: 'Preamble', sections: ['preamble'] },
            3: { label: 'Subframes', sections: [] },
            4: { label: 'PLPs', sections: [] },
        };
        this.TOTAL_WIZARD_STEPS = 4;
    }

    filterPreambleStructureByMode() {
        const modeSelect = document.getElementById('preamble_mode');
        const structureSelect = document.getElementById('preamble_structure');

        if (!modeSelect || !structureSelect) return;

        const selectedMode = modeSelect.value;
        const currentStructureValue = structureSelect.value;
        let firstVisibleOption = null;
        let currentOptionStillVisible = false;

        Array.from(structureSelect.options).forEach(option => {
            const optionMode = option.getAttribute('data-mode');

            if (optionMode === selectedMode || optionMode === '0') {
                option.hidden = false;
                option.style.display = '';
                if (!firstVisibleOption) {
                    firstVisibleOption = option;
                }
                if (option.value === currentStructureValue) {
                    currentOptionStillVisible = true;
                }
            } else {
                option.hidden = true;
                option.style.display = 'none';
            }
        });

        if (!currentOptionStillVisible && firstVisibleOption) {
            structureSelect.value = firstVisibleOption.value;
            structureSelect.dispatchEvent(new Event('change', { bubbles: true }));
        }
    }

    syncModeWithStructure() {
        const modeSelect = document.getElementById('preamble_mode');
        const structureSelect = document.getElementById('preamble_structure');

        if (!modeSelect || !structureSelect) return;

        const selectedOption = structureSelect.options[structureSelect.selectedIndex];
        if (selectedOption) {
            const optionMode = selectedOption.getAttribute('data-mode');
            if (optionMode && optionMode !== '0') {
                modeSelect.value = optionMode;
            }
        }
    }

    initPreambleModeFilter() {
        const modeSelect = document.getElementById('preamble_mode');
        const structureSelect = document.getElementById('preamble_structure');

        if (modeSelect) {
            modeSelect.addEventListener('change', () => this.filterPreambleStructureByMode());
        }

        if (structureSelect && modeSelect) {
            this.syncModeWithStructure();
            this.filterPreambleStructureByMode();
        }
    }

    wizardGoToStep(step) {
        if (step < 1 || step > this.TOTAL_WIZARD_STEPS) return;
        this.currentWizardStep = step;
        this.updateWizardProgress();

        if (step === 1) {
            showSection('botstrap');
            this.hideSubTabs();
        } else if (step === 2) {
            showSection('preamble');
            this.hideSubTabs();
        } else if (step === 3) {
            this.buildSubframeTabs();
            const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
            if (numSubframes > 0) {
                showSection('subframe0');
                this.activateSubTab('subframe0');
            }
        } else if (step === 4) {
            this.buildPlpTabs();

            const firstPlp = document.getElementById('plp-0-0');
            if (firstPlp) {
                showSection('plp-0-0');
                this.activateSubTab('plp-0-0');
            }
        }
    }

    wizardNext() {
        if (this.currentWizardStep < this.TOTAL_WIZARD_STEPS) {
            this.wizardGoToStep(this.currentWizardStep + 1);
        }
    }

    wizardPrev() {
        if (this.currentWizardStep > 1) {
            this.wizardGoToStep(this.currentWizardStep - 1);
        }
    }

    updateWizardProgress() {
        const self = this;
        document.querySelectorAll('.wizard-step').forEach(el => {
            const stepNum = parseInt(el.dataset.step);
            el.classList.remove('active', 'completed');
            if (stepNum === self.currentWizardStep) {
                el.classList.add('active');
            } else if (stepNum < self.currentWizardStep) {
                el.classList.add('completed');
            }
        });

        const lines = document.querySelectorAll('.wizard-step-line');
        lines.forEach((line, idx) => {
            line.classList.toggle('completed', idx + 1 < self.currentWizardStep);
        });

        const prevBtn = document.getElementById('wizard-prev-btn');
        const nextBtn = document.getElementById('wizard-next-btn');
        const submitBtn = document.getElementById('submit-btn');
        const stepInfo = document.getElementById('wizard-step-info');

        if (prevBtn) prevBtn.disabled = (self.currentWizardStep === 1);
        if (nextBtn) nextBtn.style.display = (self.currentWizardStep < self.TOTAL_WIZARD_STEPS) ? '' : 'none';
        if (submitBtn) submitBtn.style.display = '';

        const stepLabels = ['', 'Bootstrap', 'Preamble', 'Subframes', 'PLPs'];
        if (stepInfo) stepInfo.textContent = `Step ${self.currentWizardStep} of ${self.TOTAL_WIZARD_STEPS} - ${stepLabels[self.currentWizardStep]}`;
    }

    syncWizardWithSection(sectionId) {
        if (sectionId === 'botstrap') {
            this.currentWizardStep = 1;
        } else if (sectionId === 'preamble') {
            this.currentWizardStep = 2;
        } else if (sectionId.startsWith('subframe')) {
            this.currentWizardStep = 3;
        } else if (sectionId.startsWith('plp-')) {
            this.currentWizardStep = 4;
        }
        this.updateWizardProgress();
    }

    buildSubframeTabs() {
        const container = document.getElementById('sub-tabs-container');
        const tabsWrapper = document.getElementById('wizard-sub-tabs');
        if (!container || !tabsWrapper) return;

        const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
        container.innerHTML = '';

        for (let i = 0; i < numSubframes; i++) {
            const tab = document.createElement('button');
            tab.className = 'sub-tab';
            tab.dataset.target = `subframe${i}`;
            tab.textContent = `Subframe ${i}`;
            tab.onclick = function() {
                showSection(`subframe${i}`);
                window.atscApp.activateSubTab(`subframe${i}`);
            };
            container.appendChild(tab);
        }

        tabsWrapper.style.display = numSubframes > 0 ? '' : 'none';
    }

    buildPlpTabs() {
        const container = document.getElementById('sub-tabs-container');
        const tabsWrapper = document.getElementById('wizard-sub-tabs');
        if (!container || !tabsWrapper) return;

        const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
        container.innerHTML = '';

        for (let sf = 0; sf < numSubframes; sf++) {
            const plpElements = document.querySelectorAll(`[id^="plp-${sf}-"]`);
            plpElements.forEach(plpEl => {
                const match = plpEl.id.match(/plp-(\d+)-(\d+)/);
                if (match) {
                    const plpId = plpEl.id;
                    const tab = document.createElement('button');
                    tab.className = 'sub-tab';
                    tab.dataset.target = plpId;
                    tab.textContent = `SF${match[1]} PLP${match[2]}`;
                    tab.onclick = function() {
                        showSection(plpId);
                        window.atscApp.activateSubTab(plpId);
                    };
                    container.appendChild(tab);
                }
            });
        }

        tabsWrapper.style.display = container.children.length > 0 ? '' : 'none';
    }

    activateSubTab(targetId) {
        document.querySelectorAll('.sub-tab').forEach(tab => {
            tab.classList.toggle('active', tab.dataset.target === targetId);
        });
    }

    hideSubTabs() {
        const tabsWrapper = document.getElementById('wizard-sub-tabs');
        if (tabsWrapper) tabsWrapper.style.display = 'none';
    }

    togglePlpAllocMode(subframeIndex, plpIndex) {
        const mode = document.getElementById(`plp_alloc_mode_${plpIndex}`)?.value;
        const startInput = document.getElementById(`start_${plpIndex}`);
        const sizeInput = document.getElementById(`size_${plpIndex}`);

        if (mode === 'manual') {
            if (startInput) startInput.readOnly = false;
            if (sizeInput) sizeInput.readOnly = false;
        } else {
            if (startInput) startInput.readOnly = true;
            if (sizeInput) sizeInput.readOnly = true;
            this.recalcAutoPlps(subframeIndex);
        }
    }

    async recalcAutoPlps(subframeIndex) {
        const result = await apiCall('fillPlpSize', {
            preambleConfig: gatherPreambleParams(),
            subframeConfig: gatherSubframeConfig(subframeIndex),
        });

        if (result.error) {
            return;
        }

        const totalAvailable = result.totalSize;
        const plpElements = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);
        const plps = [];

        plpElements.forEach(el => {
            const match = el.id.match(/plp-(\d+)-(\d+)/);
            if (match) plps.push(parseInt(match[2], 10));
        });
        plps.sort((a, b) => a - b);

        let manualUsed = 0;
        const autoPlps = [];

        for (const idx of plps) {
            const mode = document.getElementById(`plp_alloc_mode_${idx}`)?.value || 'auto';
            if (mode === 'manual') {
                manualUsed += parseInt(document.getElementById(`size_${idx}`)?.value || '0', 10);
            } else {
                autoPlps.push(idx);
            }
        }

        const availableForAuto = Math.max(0, totalAvailable - manualUsed);
        let currentStart = 0;

        for (const idx of plps) {
            const mode = document.getElementById(`plp_alloc_mode_${idx}`)?.value || 'auto';
            const startInput = document.getElementById(`start_${idx}`);
            const sizeInput = document.getElementById(`size_${idx}`);

            if (mode === 'auto') {
                const isFirstAuto = autoPlps[0] === idx;
                const size = isFirstAuto ? availableForAuto : 0;

                if (startInput) startInput.value = currentStart;
                if (sizeInput) sizeInput.value = size;
                currentStart += size;
            } else {
                const size = parseInt(sizeInput?.value || '0', 10);
                if (startInput) startInput.value = currentStart;
                currentStart += size;
            }
        }

        updateResultadosSubframeProgress(subframeIndex);

        if (currentStart > totalAvailable) {
            alert(`Alerta: O total alocado (${currentStart.toLocaleString()}) excede a capacidade do subframe (${totalAvailable.toLocaleString()}).`);
        }
    }

    validatePlpManualSize(subframeIndex, plpIndex) {
        const mode = document.getElementById(`plp_alloc_mode_${plpIndex}`)?.value || 'auto';
        if (mode === 'manual') {
            this.recalcAutoPlps(subframeIndex);
        }
    }

    toggleL1dBsid() {
        var l1dVersion = document.getElementById('l1d_version');
        var l1dBsidContainer = document.getElementById('l1d_bsid_container');
        var l1dBsidInput = document.getElementById('l1d_bsid');

        if (l1dVersion && l1dBsidContainer) {
            if (l1dVersion.value > '0') {
                l1dBsidContainer.style.display = '';
                l1dBsidInput.setAttribute('required', 'required');
                if (!l1dBsidInput.value || l1dBsidInput.value === '') {
                    l1dBsidInput.value = '0';
                }
            } else {
                l1dBsidContainer.style.display = 'none';
                l1dBsidInput.removeAttribute('required');
                l1dBsidInput.value = '0';
            }
        }
    }

    generateMenus(count) {
        const savedData = saveAllSubframesData();
        const geradorGroup = document.getElementById('gerador-group');

        if (geradorGroup) {
            const subframesAndPlps = geradorGroup.querySelectorAll('[data-subframe], .menu-item');
            subframesAndPlps.forEach(el => el.remove());
        }
        document.querySelectorAll('.dynamic-section').forEach(el => el.remove());

        for (let i = 0; i < count; i++) {
            const subframeId = `subframe${i}`;

            if (geradorGroup) {
                const subframeItem = document.createElement('li');
                subframeItem.classList.add('menu-item');
                subframeItem.innerHTML = `<a href="#${subframeId}"><span class="menu-text">Subframe ${i}</span></a>`;
                geradorGroup.appendChild(subframeItem);
            }

            const section = document.createElement('div');
            section.className = 'dynamic-section config-section';
            section.id = subframeId;
            section.style.display = 'none';
            section.innerHTML = `
        <form class="custom-form">
            <h3>Subframe ${i}</h3>
            <div class="form-row">
                <div class="form-group">
                    <label for="plp_mimo_${i}">MIMO:</label>
                        <select name="plp_mimo_${i}" id="plp_mimo_${i}" required onchange="toggleMimoMixed(${i})">
                        <option value="0">Disable</option>
                        <option value="1">Enable</option>
                    </select>
                </div>

                <div class="form-group">
                    <label for="plp_mimo_mixed_${i}">MIMO Mixed:</label>
                    <select name="plp_mimo_mixed_${i}" id="plp_mimo_mixed_${i}" required onchange="toggleMimoFieldsBasedOnMixed(${i})">
                        <option value="0">OFF</option>
                        <option value="1">ON</option>
                    </select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="plp_miso_${i}">MISO:</label>
                    <select name="plp_miso_${i}" id="plp_miso_${i}" required>
                        <option value="0">No MISO</option>
                        <option value="1">MISO with 64 coefficients</option>
                        <option value="2">MISO with 256 coefficients</option>
                        <option value="3">Reserved</option>
                    </select>
                </div>

                <div class="form-group">
                    <label for="fft_size_${i}">FFT Size:</label>
                    <select name="fft_size_${i}" id="fft_size_${i}" required>
                        <option value="0">8K</option>
                        <option value="1">16K</option>
                        <option value="2">32K</option>
                    </select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="reduced_carrier_${i}">Reduced Carrier:</label>
                    <select name="reduced_carrier_${i}" id="reduced_carrier_${i}" required>
                        <option value="0">0  (97.2%)</option>
                        <option value="1">1  (95.9%)</option>
                        <option value="2">2  (94.5%)</option>
                        <option value="3">3  (93.2%)</option>
                        <option value="4">4  (91.8%)</option>
                    </select>
                </div>

                <div class="form-group">
                    <label for="guard_interval_${i}">Guard Interval:</label>
                    <select name="guard_interval_${i}" id="guard_interval_${i}" required>
                        <option value="1">192</option>
                        <option value="2">384</option>
                        <option value="3">512</option>
                        <option value="4">768</option>
                        <option value="5">1024</option>
                        <option value="6">1536</option>
                        <option value="7">2048</option>
                        <option value="8">2432</option>
                        <option value="9">3072</option>
                        <option value="10">3648</option>
                        <option value="11">4096</option>
                        <option value="12">4864</option>
                    </select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="num_ofdm_${i}">Num OFDM Symbol:</label>
                    <input name="num_ofdm_${i}" id="num_ofdm_${i}" type="number" placeholder="NÃºmero de SÃ­mbolos OFDM" min="1" required>
                </div>

                <div class="form-group">
                    <label for="spilot_pattern_${i}">Spilot Pattern:</label>
                    <select name="spilot_pattern_${i}" id="spilot_pattern_${i}" required>
                        <option value="0">3_2</option>
                        <option value="1">3_4</option>
                        <option value="2">4_2</option>
                        <option value="3">4_4</option>
                        <option value="4">6_2</option>
                        <option value="5">6_4</option>
                        <option value="6">8_2</option>
                        <option value="7">8_4</option>
                        <option value="8">12_2</option>
                        <option value="9">12_4</option>
                        <option value="10">16_2</option>
                        <option value="11">16_4</option>
                        <option value="12">24_2</option>
                        <option value="13">24_4</option>
                        <option value="14">32_2</option>
                        <option value="15">32_4</option>
                    </select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="spilot_boost_${i}">Spilot Boost:</label>
                    <select name="spilot_boost_${i}" id="spilot_boost_${i}" required>
                        <option value="0">1.000 (0.00 dB)</option>
                        <option value="1">1.288 (2.20 dB)</option>
                        <option value="2">1.549 (3.80 dB)</option>
                        <option value="3">1.698 (4.60 dB)</option>
                        <option value="4">1.841 (5.30 dB)</option>
                    </select>
                </div>

                <div class="form-group">
                    <label for="freq_interleaver_${i}">Freq. Interleaver:</label>
                    <select name="freq_interleaver_${i}" id="freq_interleaver_${i}" required>
                        <option value="0">OFF</option>
                        <option value="1">ON</option>
                    </select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="sbs_first_${i}">SBS First:</label>
                    <select name="sbs_first_${i}" id="sbs_first_${i}" required>
                        <option value="0">OFF</option>
                        <option value="1">ON</option>
                    </select>
                </div>

                <div class="form-group">
                    <label for="sbs_last_${i}">SBS Last:</label>
                    <select name="sbs_last_${i}" id="sbs_last_${i}" required>
                        <option value="0">OFF</option>
                        <option value="1">ON</option>
                    </select>
                </div>
            </div>

            <div class="form-group">
                <label for="plp-count-${i}">Number of PLPs:</label>
                <input type="number" id="plp-count-${i}" name="subframe[${i}][plp_count]" placeholder="Number of PLPs" min="1" max="64" value="1" onchange="generatePLPMenusAndFields(${i})" required>
                <div id="plp-fields-${i}" class="plp-fields" style="display: none;"></div>
            </div>
        </form>
        `;

            const sections = document.getElementById('sections');
            sections.appendChild(section);
        }

        setTimeout(() => {
        restoreAllData(savedData);
        renumberAllPlps();

        for (let i = 0; i < count; i++) {
            toggleMimoMixed(i);
        }
    }, 100);
    }

    generatePLPMenusAndFields(subframeIndex) {
        const plpCountInput = document.getElementById(`plp-count-${subframeIndex}`);
        const geradorGroup = document.getElementById('gerador-group');
        const plpFieldsContainer = document.getElementById(`plp-fields-${subframeIndex}`);

        if (!plpCountInput || !plpFieldsContainer) {
            return;
        }

        const plpCount = Math.max(parseInt(plpCountInput.value) || 0, 1);

        const l1dVersion = parseInt(document.getElementById('l1d_version')?.value || '0');
        let startValue;

        if (l1dVersion === 2) {
            startValue = plpCount === 1 ? 4388 : 3757;
        } else if (l1dVersion === 1) {
            startValue = plpCount === 1 ? 4485 : 3854;
        } else {
            startValue = plpCount === 1 ? 4388 : 3757;
        }

        const savedPLPData = {};
        document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`).forEach(plp => {
            const match = plp.id.match(/plp-\d+-(\d+)/);
            if (match) {
                const plpIndex = match[1];
                const form = plp.querySelector('form');
                if (form) {
                    savedPLPData[plpIndex] = saveFormData(form);
                }
            }
        });

        if (geradorGroup) {
            const existingPlps = geradorGroup.querySelectorAll(`[data-subframe="${subframeIndex}"]`);
            existingPlps.forEach(plp => plp.remove());
        }
        document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`).forEach(el => el.remove());

        for (let plpIndex = 0; plpIndex < plpCount; plpIndex++) {
            const plpId = `plp-${subframeIndex}-${plpIndex}`;
            const plpIdValue = plpIndex;

            const plpSection = document.createElement('div');
            plpSection.className = 'dynamic-section config-section';
            plpSection.id = plpId;
            plpSection.style.display = 'none';
            plpSection.innerHTML = `
        <form class="custom-form">
    <h3>Subframe ${subframeIndex} - PLP ${plpIdValue}</h3>

    <div class="form-row">
        <div class="form-group">
            <label for="plp_id_${plpIndex}">ID:</label>
            <input name="plp_id_${plpIndex}" id="plp_id_${plpIndex}" type="number" value="${plpIdValue}" readonly required>
        </div>
        <div class="form-group">
            <label for="plp_alloc_mode_${plpIndex}">Start/Size:</label>
            <select id="plp_alloc_mode_${plpIndex}" onchange="togglePlpAllocMode(${subframeIndex}, ${plpIndex})">
                <option value="auto">Auto</option>
                <option value="manual">Manual</option>
            </select>
        </div>
    </div>

    <div class="form-row">
        <div class="form-group">
        <label for="lls_flag_${plpIndex}">LLS Flag:</label>
        <select name="lls_flag_${plpIndex}" id="lls_flag_${plpIndex}" required>
            <option value="0">OFF</option>
            <option value="1">ON</option>
        </select>
        </div>

        <div class="form-group">
        <label for="layer_${plpIndex}">Layer:</label>
        <select name="layer_${plpIndex}" id="layer_${plpIndex}" required onchange="toggleEnhancedLayerFields(${plpIndex})">
            <option value="0">Core Layer</option>
            <option value="1">Enhanced Layer</option>
        </select>
        </div>
    </div>

<div class="form-row">
    <div class="form-group">
        <label for="start_${plpIndex}">Start:</label>
        <input name="start_${plpIndex}" id="start_${plpIndex}" type="number" placeholder="Start - PLP ${plpIndex}" value="${startValue}" readonly required>
    </div>

    <div class="form-group">
        <label for="size_${plpIndex}">Size:</label>
        <input name="size_${plpIndex}" id="size_${plpIndex}" class="plp-size-input" type="number" placeholder="Size - PLP ${plpIndex}" min="0" readonly required oninput="updatePlpProgressBar(${subframeIndex}, ${plpIndex}); validatePlpManualSize(${subframeIndex}, ${plpIndex})">
    </div>
</div>

    <label for="fec_type_${plpIndex}">FEC Type:</label>
    <select name="fec_type_${plpIndex}" id="fec_type_${plpIndex}" required>
        <option value="0">BCH+16K LDPC</option>
        <option value="1">BCH+64K LDPC</option>
        <option value="2">CRC+16K LDPC</option>
        <option value="3">CRC+64K LDPC</option>
        <option value="4">16K LDPC only</option>
        <option value="5">64K LDPC only</option>
    </select>

    <label for="mod_order_${plpIndex}">MOD Order:</label>
    <select name="mod_order_${plpIndex}" id="mod_order_${plpIndex}" required>
        <option value="0">QPSK</option>
        <option value="1">16QAM</option>
        <option value="2">64QAM</option>
        <option value="3">256QAM</option>
        <option value="4">1024QAM</option>
        <option value="5">4096QAM</option>
    </select>

    <label for="code_rate_${plpIndex}">Code Rate:</label>
    <select name="code_rate_${plpIndex}" id="code_rate_${plpIndex}" required>
        <option value="0">2/15</option>
        <option value="1">3/15</option>
        <option value="2">4/15</option>
        <option value="3">5/15</option>
        <option value="4">6/15</option>
        <option value="5">7/15</option>
        <option value="6">8/15</option>
        <option value="7">9/15</option>
        <option value="8">10/15</option>
        <option value="9">11/15</option>
        <option value="10">12/15</option>
        <option value="11">13/15</option>
    </select>

    <label for="plp_type_${plpIndex}">Type:</label>
    <select name="plp_type_${plpIndex}" id="plp_type_${plpIndex}" required onchange="toggleSubsliceFields(${plpIndex})">
        <option value="0">non-dispersed</option>
        <option value="1">dispersed</option>
    </select>

    <label for="num_subslice_${plpIndex}">Num Subslice:</label>
    <input name="num_subslice_${plpIndex}" id="num_subslice_${plpIndex}" type="number" placeholder="Num Subslice - PLP ${plpIndex}" min="0" required>

    <label for="subslice_interval_${plpIndex}">Subslice Interval:</label>
    <input name="subslice_interval_${plpIndex}" id="subslice_interval_${plpIndex}" type="number" placeholder="Subslice Interval - PLP ${plpIndex}" min="0" required>

    <label for="ti_mode_${plpIndex}">TI Mode:</label>
    <select name="ti_mode_${plpIndex}" id="ti_mode_${plpIndex}" required onchange="toggleTIFields(${plpIndex})">
        <option value="0">No TI</option>
        <option value="1">CTI</option>
        <option value="2">HTI</option>
    </select>

    <label for="ti_extended_${plpIndex}">TI Extended:</label>
    <select name="ti_extended_${plpIndex}" id="ti_extended_${plpIndex}" required>
        <option value="0">OFF</option>
        <option value="1">ON</option>
    </select>

    <label for="cti_depth_${plpIndex}">CTI Depth:</label>
    <select name="cti_depth_${plpIndex}" id="cti_depth_${plpIndex}" required>
        <option value="0">512</option>
        <option value="1">724</option>
        <option value="2">887</option>
        <option value="3">1024</option>
    </select>

    <label for="cell_intervaler_${plpIndex}">Cell Interleaver:</label>
    <select name="cell_intervaler_${plpIndex}" id="cell_intervaler_${plpIndex}" required>
        <option value="0">OFF</option>
        <option value="1">ON</option>
    </select>

    <label for="inter_subframe_${plpIndex}">Inter Subframe:</label>
    <select name="inter_subframe_${plpIndex}" id="inter_subframe_${plpIndex}" required>
        <option value="0">intra subframe HTI</option>
        <option value="1">inter subframe HTI</option>
    </select>

    <label for="num_ti_blocks_${plpIndex}">Num TI Blocks:</label>
    <input name="num_ti_blocks_${plpIndex}" id="num_ti_blocks_${plpIndex}" type="number" placeholder="Num TI Blocks - PLP ${plpIndex}" min="0" required>

    <label for="num_fec_blocks_max_${plpIndex}">Num FEC Blocks Max:</label>
    <input name="num_fec_blocks_max_${plpIndex}" id="num_fec_blocks_max_${plpIndex}" type="number" placeholder="Num FEC Blocks Max - PLP ${plpIndex}" min="0" required>

    <label for="num_fec_blocks_${plpIndex}">Num FEC Blocks:</label>
    <input name="num_fec_blocks_${plpIndex}" id="num_fec_blocks_${plpIndex}" type="number" placeholder="Num FEC Blocks - PLP ${plpIndex}" min="0" required>

    <label for="inter_ldm_injection_level_${plpIndex}">LDM Injection Level:</label>
    <select name="inter_ldm_injection_level_${plpIndex}" id="inter_ldm_injection_level_${plpIndex}" required>
        <option value="0">0.0dB</option>
        <option value="1">0.5dB</option>
        <option value="2">1.0dB</option>
        <option value="3">1.5dB</option>
        <option value="4">2.0dB</option>
        <option value="5">2.5dB</option>
        <option value="6">3.0dB</option>
        <option value="7">3.5dB</option>
        <option value="8">4.0dB</option>
        <option value="9">4.5dB</option>
        <option value="10">5.0dB</option>
        <option value="11">6.0dB</option>
        <option value="12">7.0dB</option>
        <option value="13">8.0dB</option>
        <option value="14">9.0dB</option>
        <option value="15">10.0dB</option>
        <option value="16">11.0dB</option>
        <option value="17">12.0dB</option>
        <option value="18">13.0dB</option>
        <option value="19">14.0dB</option>
        <option value="20">15.0dB</option>
        <option value="21">16.0dB</option>
        <option value="22">17.0dB</option>
        <option value="23">18.0dB</option>
        <option value="24">19.0dB</option>
        <option value="25">20.0dB</option>
        <option value="26">21.0dB</option>
        <option value="27">22.0dB</option>
        <option value="28">23.0dB</option>
        <option value="29">24.0dB</option>
        <option value="30">25.0dB</option>
    </select>

    <label for="mimo_plp_${plpIndex}">MIMO PLP:</label>
    <select name="mimo_plp_${plpIndex}" id="mimo_plp_${plpIndex}" required onchange="toggleMimoStreamFields(${plpIndex})">
        <option value="0">OFF</option>
        <option value="1">ON</option>
    </select>

    <label for="plp_mimo_stream_combining_${plpIndex}">Stream Combining:</label>
    <select name="plp_mimo_stream_combining_${plpIndex}" id="plp_mimo_stream_combining_${plpIndex}" required>
        <option value="0">Disable</option>
        <option value="1">Enable</option>
    </select>

    <label for="plp_mimo_IQ_intervaling_${plpIndex}">IQ Interleaving:</label>
    <select name="plp_mimo_IQ_intervaling_${plpIndex}" id="plp_mimo_IQ_intervaling_${plpIndex}" required>
        <option value="0">Disable</option>
        <option value="1">Enable</option>
    </select>

    <label for="plp_mimo_PH_${plpIndex}">Phase Hopping:</label>
    <select name="plp_mimo_PH_${plpIndex}" id="plp_mimo_PH_${plpIndex}" required>
        <option value="0">Disable</option>
        <option value="1">Enable</option>
    </select>
</form>
        `;

            const sections = document.getElementById('sections');
            sections.appendChild(plpSection);

            const hiddenFieldset = document.createElement('div');
            hiddenFieldset.innerHTML = `
                <input type="hidden" name="subframe[${subframeIndex}][plp][${plpIndex}][exists]" value="1">
            `;
            plpFieldsContainer.appendChild(hiddenFieldset);
        }

        setTimeout(() => {
            renumberAllPlps();

            Object.keys(savedPLPData).forEach(plpIndex => {
                const plp = document.getElementById(`plp-${subframeIndex}-${plpIndex}`);
                if (plp) {
                    const form = plp.querySelector('form');
                    if (form) {
                        restoreFormData(form, savedPLPData[plpIndex]);
                    }
                }
            });

        for (let plpIndex = 0; plpIndex < plpCount; plpIndex++) {
            updatePlpProgressBar(subframeIndex, plpIndex);
            this.toggleEnhancedLayerFields(plpIndex);
            this.toggleSubsliceFields(plpIndex);
            this.toggleTIFields(plpIndex);
            this.toggleMimoFieldsBasedOnMixed(subframeIndex);
        }
        this.toggleMimoFieldsBasedOnMixed(subframeIndex);
        this.toggleMimoMixed(subframeIndex);
        this.recalcAutoPlps(subframeIndex);
    }, 100);
    }

    submitForm() {
        var subframeCount = parseInt(document.getElementById('number_of_subframes').value) || 0;

        for (let i = 0; i < subframeCount; i++) {
            const mimoSelect = document.getElementById(`plp_mimo_${i}`);
            const mimoMixedSelect = document.getElementById(`plp_mimo_mixed_${i}`);

            if (mimoSelect && mimoMixedSelect) {
                if (mimoSelect.value !== "0") {
                    mimoMixedSelect.value = "0";
                }
            }

            const plpCountInput = document.getElementById(`plp-count-${i}`);
            const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 1 : 1;

            for (let j = 0; j < plpCount; j++) {
                const layerSelect = document.getElementById(`layer_${j}`);
                if (layerSelect && layerSelect.value === "1") {
                    const fieldsToReset = [
                        'fec_type', 'mod_order', 'code_rate', 'plp_type',
                        'num_subslice', 'subslice_interval', 'ti_extended',
                        'cti_depth', 'cell_intervaler', 'inter_subframe',
                        'num_ti_blocks', 'num_fec_blocks_max', 'num_fec_blocks'
                    ];

                    fieldsToReset.forEach(fieldName => {
                        const field = document.getElementById(`${fieldName}_${j}`);
                        if (field) {
                            field.value = "0";
                        }
                    });
                } else if (layerSelect && layerSelect.value === "0") {
                    const ldmField = document.getElementById(`inter_ldm_injection_level_${j}`);
                    if (ldmField) {
                        ldmField.value = "0";
                    }
                }
            }
        }

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

    const frameValidationErrors = this.validateFrameGeneration();

    if (frameValidationErrors.length > 0) {
        let alertMessage = 'ERRO: Configuração invÃ¡lida detectada!\n\n';

        const hasL1BL1DError = frameValidationErrors.some(error =>
            error.message.includes('L1B Version = 1 e L1D Version = 0')
        );

        if (hasL1BL1DError) {
            alertMessage += 'ERRO CRÍTICO DE VERSÃO:\n';
            alertMessage += 'A combinação L1B Version = 1 e L1D Version = 0 não é permitida!\n\n';
        }

        frameValidationErrors.forEach(error => {
            alertMessage += '-> ' + error.message + '\n';
        });
        alertMessage += '\nA Configuração não pode ser enviada com erros.\nPor favor, corrija os valores antes de continuar.';

        alert(alertMessage);
        return;
    }

        if (!serverStatus.available) {
            showBackupNotification('Servidor offline. Configuração salva apenas localmente.', 'warning');
            saveConfigBackup();
            return;
        }

        var allData = new FormData();

        var botstrapForm = document.getElementById('botstrap_form');
        for (var element of botstrapForm.elements) {
            if (element.name) {
                allData.append(element.name, element.value);
            }
        }

        var preambleForm = document.getElementById('preamble_form');
        for (var element of preambleForm.elements) {
            if (element.name) {
                allData.append(element.name, element.value);
            }
        }

        var subframeCount2 = parseInt(document.getElementById('number_of_subframes').value) || 0;

        for (var i = 0; i < subframeCount2; i++) {
            var subframeForm = document.querySelector(`#subframe${i} form`);
            if (subframeForm) {
                for (var element of subframeForm.elements) {
                    if (element.name) {
                        if (element.name.startsWith('plp_mimo') ||
                            element.name.startsWith('plp_miso') ||
                            !element.name.includes('plp_')) {
                            allData.append(element.name, element.value);
                        }
                    }
                }

                var plpCountInput = document.getElementById(`plp-count-${i}`);
                var plpCount = plpCountInput ? parseInt(plpCountInput.value) || 0 : 0;

                if (plpCount > 0) {
                    allData.append(`plp-count-${i}`, plpCount);

                    for (var j = 0; j < plpCount; j++) {
                        var plpForm = document.querySelector(`#plp-${i}-${j} form`);
                        if (plpForm) {
                            for (var plpElement of plpForm.elements) {
                                if (plpElement.name) {
                                    var newFieldName = plpElement.name.replace(/_(\d+)$/, `_${i}_${j}`);
                                    allData.append(newFieldName, plpElement.value);
                                }
                            }
                        } else {
                            var plpFields = [
                                'plp_id', 'lls_flag', 'layer', 'start', 'size', 'fec_type',
                                'mod_order', 'code_rate', 'ti_mode', 'ti_extended', 'cti_depth',
                                'mimo_plp', 'plp_mimo_stream_combining', 'plp_mimo_IQ_intervaling',
                                'plp_mimo_PH', 'plp_type', 'num_subslice', 'subslice_interval', 'cell_intervaler',
                                'inter_subframe', 'num_ti_blocks', 'num_fec_blocks_max',
                                'num_fec_blocks', 'inter_ldm_injection_level'
                            ];

                            plpFields.forEach(fieldName => {
                                var fieldId = `${fieldName}_${j}`;
                                var fieldElement = document.getElementById(fieldId);
                                if (fieldElement) {
                                    var newFieldName2 = `${fieldName}_${i}_${j}`;
                                    allData.append(newFieldName2, fieldElement.value);
                                }
                            });
                        }
                    }
                }
            }
        }

        allData.append('submit_config', '1');

        var xhr = new XMLHttpRequest();
        xhr.open('POST', '', true);
        xhr.onload = function () {
            if (xhr.status === 200) {
                showBackupNotification('Configuração enviada com sucesso!', 'success');

                setTimeout(async () => {
                    saveConfigBackup();
                    fetchL1dSizeBytesFromLog();
                    await Promise.all([fetchFrame2LogData(), fetchFrameDuration()]);
                    const numSf = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
                    for (let sf = 0; sf < numSf; sf++) {
                        updateResultadosSubframeProgress(sf);
                    }
                }, 500);

                alert('Configuração enviada com sucesso!');
            } else {
                showBackupNotification('Erro ao enviar ao servidor. Salvando backup local.', 'error');
                saveConfigBackup();
                alert('Erro ao enviar Configuração!');
            }
        };

        xhr.onerror = function() {
            showBackupNotification('Falha de conexÃ£o. Configuração salva localmente.', 'error');
            serverStatus.available = false;
            serverStatus.usingBackup = true;
            updateServerStatusIndicator();
            saveConfigBackup();
            alert('Falha de conexÃ£o. Configuração salva localmente.');
        };

        xhr.send(allData);
    }

    setFieldValue(fieldId, value) {
        var field = document.getElementById(fieldId);
        if (field) {
            field.value = value;
            if (fieldId.includes('plp_mimo_') && !fieldId.includes('mixed')) {
                const subframeIndex = fieldId.split('_')[2];
                setTimeout(() => this.toggleMimoMixed(subframeIndex), 10);
            }

            if (fieldId.includes('layer_')) {
                const plpIndex = fieldId.split('_')[1];
                setTimeout(() => this.toggleEnhancedLayerFields(plpIndex), 10);
            }

            if (fieldId.includes('plp_type_')) {
                const plpIndex = fieldId.split('_')[2];
                setTimeout(() => this.toggleSubsliceFields(plpIndex), 10);
            }

            if (fieldId.includes('ti_mode_')) {
                const plpIndex = fieldId.split('_')[2];
                setTimeout(() => this.toggleTIFields(plpIndex), 10);
            }

            if (fieldId.includes('mimo_plp_') && !fieldId.includes('stream') && !fieldId.includes('IQ') && !fieldId.includes('PH')) {
                const plpIndex = fieldId.split('_')[2];
                setTimeout(() => this.toggleMimoStreamFields(plpIndex), 10);
            }

            if (fieldId.startsWith('size_')) {
                const plpIndexMatch = fieldId.match(/size_(\d+)/);
                if (plpIndexMatch) {
                    const plpIndex = parseInt(plpIndexMatch[1]);
                    setTimeout(() => {
                        document.querySelectorAll(`[id^="plp-"][id$="-${plpIndex}"]`).forEach(plpSection => {
                            const subframeMatch = plpSection.id.match(/plp-(\d+)-\d+/);
                            if (subframeMatch) {
                                const subframeIndex = parseInt(subframeMatch[1]);
                                updatePlpProgressBar(subframeIndex, plpIndex);
                            }
                        });
                    }, 200);
                }
            }
        }
    }

    loadSubframesData(subframesData) {
        const self = this;
        for (var i in subframesData) {
            var subframe = subframesData[i];

            self.setFieldValue('plp_mimo_' + i, subframe.plp_mimo);
            self.setFieldValue('plp_mimo_mixed_' + i, subframe.plp_mimo_mixed);
            self.setFieldValue('plp_miso_' + i, subframe.plp_miso);
            self.setFieldValue('fft_size_' + i, subframe.fft_size);
            self.setFieldValue('reduced_carrier_' + i, subframe.reduced_carrier);
            self.setFieldValue('guard_interval_' + i, subframe.guard_interval);
            self.setFieldValue('num_ofdm_' + i, subframe.num_ofdm);
            self.setFieldValue('spilot_pattern_' + i, subframe.spilot_pattern);
            self.setFieldValue('spilot_boost_' + i, subframe.spilot_boost);
            self.setFieldValue('sbs_first_' + i, subframe.sbs_first);
            self.setFieldValue('sbs_last_' + i, subframe.sbs_last);
            self.setFieldValue('freq_interleaver_' + i, subframe.freq_interleaver);
            self.setFieldValue('plp-count-' + i, subframe.plp_count);

            if (subframe.plps && Object.keys(subframe.plps).length > 0) {
                self.generatePLPMenusAndFields(parseInt(i));

                setTimeout(function(subframeIndex, plps) {
                    for (var j in plps) {
                        var plp = plps[j];

                        self.setFieldValue('plp_id_' + j, plp.id);
                        self.setFieldValue('lls_flag_' + j, plp.lls_flag);
                        self.setFieldValue('layer_' + j, plp.layer);
                        self.setFieldValue('start_' + j, plp.start);
                        self.setFieldValue('size_' + j, plp.size);
                        self.setFieldValue('fec_type_' + j, plp.fec_type);
                        self.setFieldValue('mod_order_' + j, plp.mod_order);
                        self.setFieldValue('code_rate_' + j, plp.code_rate);
                        self.setFieldValue('ti_mode_' + j, plp.ti_mode);
                        self.setFieldValue('ti_extended_' + j, plp.ti_extended);
                        self.setFieldValue('cti_depth_' + j, plp.cti_depth);
                        self.setFieldValue('mimo_plp_' + j, plp.mimo_plp);
                        self.setFieldValue('plp_mimo_stream_combining_' + j, plp.stream_combining);
                        self.setFieldValue('plp_mimo_IQ_intervaling_' + j, plp.iq_intervaling);
                        self.setFieldValue('plp_mimo_PH_' + j, plp.phase_hopping);
                        self.setFieldValue('plp_type_' + j, plp.type);
                        self.setFieldValue('num_subslice_' + j, plp.num_subslice);
                        self.setFieldValue('subslice_interval_' + j, plp.subslice_interval);
                        self.setFieldValue('cell_intervaler_' + j, plp.cell_intervaler);
                        self.setFieldValue('inter_subframe_' + j, plp.inter_subframe);
                        self.setFieldValue('num_ti_blocks_' + j, plp.num_ti_blocks);
                        self.setFieldValue('num_fec_blocks_max_' + j, plp.num_fec_blocks_max);
                        self.setFieldValue('num_fec_blocks_' + j, plp.num_fec_blocks);
                        self.setFieldValue('inter_ldm_injection_level_' + j, plp.ldm_injection_level);
                    }
                }.bind(null, i, subframe.plps), 500);
            }
        }

        window.__plpDataLoaded__ = true;

        setTimeout(function() {
        self.initializeSubsliceControls();
        self.initializeTIControls();
        self.initializeMimoControls();

        for (var sfIdx in subframesData) {
            updateResultadosSubframeProgress(parseInt(sfIdx));
        }
    }, 1000);
    }

    parseConfFile(confContent) {
        const config = {
            bootstrap: {},
            preamble: {},
            subframes: {},
            timestamp: Date.now()
        };

        const lines = confContent.split('\n');

        for (const line of lines) {
            const trimmed = line.trim();

            if (!trimmed || trimmed.startsWith('#') || trimmed.startsWith('=')) {
                continue;
            }

            const equalIndex = trimmed.indexOf('=');
            if (equalIndex === -1) continue;

            const key = trimmed.substring(0, equalIndex).trim();
            const value = trimmed.substring(equalIndex + 1).trim();

            if (key === 'major_version') config.bootstrap.major_version = value;
            else if (key === 'minor_version') config.bootstrap.minor_version = value;
            else if (key === 'bootstrap_symbol') config.bootstrap.bootstrap_symbol = value;
            else if (key === 'ea_wakeup') config.bootstrap.ea_wakeup = value;
            else if (key === 'system_bandwidth') config.bootstrap.system_bandwidth = value;
            else if (key === 'bsr_coefficient') config.bootstrap.bsr_coefficient = value;
            else if (key === 'min_time_to_next') config.bootstrap.min_time_to_next = value;
            else if (key === 'preamble_structure') config.bootstrap.preamble_structure = value;
            else if (key === 'number_of_frames') config.bootstrap.number_of_frames = value;

            else if (key === 'L1B_version') config.preamble.l1b_version = value;
            else if (key === 'L1B_mimo_scattered_pilot_encoding') config.preamble.l1b_mimo_scatterred_pilot_encoding = value;
            else if (key === 'L1B_L1_Detail_size_bytes') config.preamble.detail_size_bytes = value;
            else if (key === 'L1B_L1_Detail_fec_type') config.preamble.detail_fec_type = value;
            else if (key === 'L1B_time_info_flag') config.preamble.time_info_flag = value;
            else if (key === 'L1B_frame_length_mode') config.preamble.frame_lenght_mode = value;
            else if (key === 'L1B_frame_length') config.preamble.frame_lenght = value;
            else if (key === 'L1B_num_subframes') config.preamble.number_of_subframes = value;
            else if (key === 'L1B_preamble_num_symbols') config.preamble.preamble_num_symbols = value;
            else if (key === 'L1D_version') config.preamble.l1d_version = value;
            else if (key === 'L1D_bsid') config.preamble.l1d_bsid = value;

            else if (key.startsWith('subframe.')) {
                const subframeMatch = key.match(/subframe\.\[(\d+)\]\.(.+)/);
                if (subframeMatch) {
                    const subframeIndex = subframeMatch[1];
                    const subframeField = subframeMatch[2];

                    if (!config.subframes[subframeIndex]) {
                        config.subframes[subframeIndex] = { config: {}, plps: {} };
                    }

                    if (subframeField.startsWith('plp.')) {
                        const plpMatch = subframeField.match(/plp\.\[(\d+)\]\.(.+)/);
                        if (plpMatch) {
                            const plpIndex = plpMatch[1];
                            const plpField = plpMatch[2];

                            if (!config.subframes[subframeIndex].plps[plpIndex]) {
                                config.subframes[subframeIndex].plps[plpIndex] = {};
                            }

                            const plpFieldMap = {
                                'L1D_plp_id': `plp_id_${plpIndex}`,
                                'L1D_plp_lls_flag': `lls_flag_${plpIndex}`,
                                'L1D_plp_layer': `layer_${plpIndex}`,
                                'L1D_plp_start': `start_${plpIndex}`,
                                'L1D_plp_size': `size_${plpIndex}`,
                                'L1D_plp_fec_type': `fec_type_${plpIndex}`,
                                'L1D_plp_mod': `mod_order_${plpIndex}`,
                                'L1D_plp_cod': `code_rate_${plpIndex}`,
                                'L1D_plp_TI_mode': `ti_mode_${plpIndex}`,
                                'L1D_plp_TI_extended_interleaving': `ti_extended_${plpIndex}`,
                                'L1D_plp_CTI_depth': `cti_depth_${plpIndex}`,
                                'L1D_plp_mimo': `mimo_plp_${plpIndex}`,
                                'L1D_plp_mimo_stream_combining': `plp_mimo_stream_combining_${plpIndex}`,
                                'L1D_plp_mimo_IQ_interleaving': `plp_mimo_IQ_intervaling_${plpIndex}`,
                                'L1D_plp_mimo_PH': `plp_mimo_PH_${plpIndex}`,
                                'L1D_plp_type': `plp_type_${plpIndex}`,
                                'L1D_plp_num_subslices': `num_subslice_${plpIndex}`,
                                'L1D_plp_subslice_interval': `subslice_interval_${plpIndex}`,
                                'L1D_plp_HTI_cell_interleaver': `cell_intervaler_${plpIndex}`,
                                'L1D_plp_HTI_inter_subframe': `inter_subframe_${plpIndex}`,
                                'L1D_plp_HTI_num_ti_blocks': `num_ti_blocks_${plpIndex}`,
                                'L1D_plp_HTI_num_fec_blocks_max': `num_fec_blocks_max_${plpIndex}`,
                                'L1D_plp_HTI_num_fec_blocks': `num_fec_blocks_${plpIndex}`,
                                'L1D_plp_ldm_injection_level': `inter_ldm_injection_level_${plpIndex}`
                            };

                            const mappedField = plpFieldMap[plpField];
                            if (mappedField) {
                                config.subframes[subframeIndex].plps[plpIndex][mappedField] = value;
                            }
                        }
                    }
                    else {
                        const subframeFieldMap = {
                            'L1B_first_sub_mimo': `plp_mimo_${subframeIndex}`,
                            'L1B_first_sub_mimo_mixed': `plp_mimo_mixed_${subframeIndex}`,
                            'L1B_first_sub_miso': `plp_miso_${subframeIndex}`,
                            'L1B_first_sub_fft_size': `fft_size_${subframeIndex}`,
                            'L1B_first_sub_reduced_carriers': `reduced_carrier_${subframeIndex}`,
                            'L1B_first_sub_guard_interval': `guard_interval_${subframeIndex}`,
                            'L1B_first_sub_num_ofdm_symbols': `num_ofdm_${subframeIndex}`,
                            'L1B_first_sub_scattered_pilot_pattern': `spilot_pattern_${subframeIndex}`,
                            'L1B_first_sub_scattered_pilot_boost': `spilot_boost_${subframeIndex}`,
                            'L1B_first_sub_sbs_first': `sbs_first_${subframeIndex}`,
                            'L1B_first_sub_sbs_last': `sbs_last_${subframeIndex}`,

                            'L1D_mimo': `plp_mimo_${subframeIndex}`,
                            'L1D_mimo_mixed': `plp_mimo_mixed_${subframeIndex}`,
                            'L1D_miso': `plp_miso_${subframeIndex}`,
                            'L1D_fft_size': `fft_size_${subframeIndex}`,
                            'L1D_reduced_carriers': `reduced_carrier_${subframeIndex}`,
                            'L1D_guard_interval': `guard_interval_${subframeIndex}`,
                            'L1D_num_ofdm_symbols': `num_ofdm_${subframeIndex}`,
                            'L1D_scattered_pilot_pattern': `spilot_pattern_${subframeIndex}`,
                            'L1D_scattered_pilot_boost': `spilot_boost_${subframeIndex}`,
                            'L1D_sbs_first': `sbs_first_${subframeIndex}`,
                            'L1D_sbs_last': `sbs_last_${subframeIndex}`,

                            'L1D_frequency_interleaver': `freq_interleaver_${subframeIndex}`,
                            'L1D_num_plp': `plp-count-${subframeIndex}`
                        };

                        const mappedField = subframeFieldMap[subframeField];
                        if (mappedField) {
                            config.subframes[subframeIndex].config[mappedField] = value;
                        }
                    }
                }
            }
        }

        return config;
    }

    generateConfFile() {
        const timestamp = new Date().toString();
        let confContent = `# =============================================
# ATSC 3.0 Modulator Configuration File
# Auto-generated - Do not edit manually
# =============================================

# Last updated: ${timestamp}

# =============================================
# BOOTSTRAP CONFIGURATION
# =============================================
`;

        const bootstrapForm = document.getElementById('botstrap_form');
        if (bootstrapForm) {
            const bootstrapData = getFormDataAsObject(bootstrapForm);

            confContent += `major_version=${bootstrapData.major_version || '0'}\n`;
            confContent += `minor_version=${bootstrapData.minor_version || '0'}\n`;
            confContent += `bootstrap_symbol=${bootstrapData.bootstrap_symbol || '0'}\n`;
            confContent += `ea_wakeup=${bootstrapData.ea_wakeup || '0'}\n`;
            confContent += `system_bandwidth=${bootstrapData.system_bandwidth || '0'}\n`;
            confContent += `bsr_coefficient=${bootstrapData.bsr_coefficient || '2'}\n`;
            confContent += `min_time_to_next=${bootstrapData.min_time_to_next || '300'}\n`;
            confContent += `preamble_structure=${bootstrapData.preamble_structure || '50'}\n`;
            confContent += `number_of_frames=${bootstrapData.number_of_frames || '1'}\n`;
        }

        confContent += `# =============================================
# PREAMBLE CONFIGURATION
# =============================================
`;

        const preambleForm = document.getElementById('preamble_form');
        if (preambleForm) {
            const preambleData = getFormDataAsObject(preambleForm);

            confContent += `L1B_version=${preambleData.l1b_version || '0'}\n`;
            confContent += `L1B_mimo_scattered_pilot_encoding=${preambleData.l1b_mimo_scatterred_pilot_encoding || '0'}\n`;
            confContent += `L1B_lls_flag=0\n`;
            confContent += `L1B_time_info_flag=${preambleData.time_info_flag || '3'}\n`;
            confContent += `L1B_return_channel_flag=0\n`;
            confContent += `L1B_papr_reduction=0\n`;
            confContent += `L1B_frame_length_mode=${preambleData.frame_lenght_mode || '1'}\n`;
            confContent += `L1B_frame_length=${preambleData.frame_lenght || '0'}\n`;
            confContent += `L1B_excess_samples_per_symbol=0\n`;
            confContent += `L1B_time_offset=2422\n`;
            confContent += `L1B_additional_samples=0\n`;
            confContent += `L1B_num_subframes=${preambleData.number_of_subframes || '1'}\n`;
            confContent += `L1B_preamble_num_symbols=${window.__preambleNumSymbols__ ?? '0'}\n`;
            confContent += `L1B_preamble_reduced_carriers=0\n`;
            confContent += `L1B_L1_Detail_content_tag=0\n`;
            confContent += `L1B_L1_Detail_size_bytes=${preambleData.detail_size_bytes || '25'}\n`;
            confContent += `L1B_L1_Detail_fec_type=${preambleData.detail_fec_type || '0'}\n`;
            confContent += `L1B_L1_Detail_additional_parity_mode=0\n`;
            confContent += `L1B_L1_Detail_total_cells=180\n`;
            confContent += `L1D_version=${preambleData.l1d_version || '1'}\n`;
            confContent += `L1D_bsid=${preambleData.l1d_bsid || '0'}\n`;
            confContent += `L1D_num_rf=0\n`;
            confContent += `L1D_rf_id=0\n`;
            confContent += `L1D_bonded_bsid=0\n`;
            confContent += `L1D_time_sec=${Math.floor(Date.now() / 1000)}\n`;
            confContent += `L1D_time_msec=${Date.now() % 1000}\n`;
            confContent += `L1D_time_usec=${Math.floor(Math.random() * 1000)}\n`;
            confContent += `L1D_time_nsec=${Math.floor(Math.random() * 1000)}\n`;
            confContent += `L1D_reserved=1\n`;
            confContent += `L1B_reserved=1\n`;
        }

        confContent += `# =============================================
# SUBFRAMES & PLPs CONFIGURATION
# =============================================

`;

        const subframeCountInput = document.getElementById('number_of_subframes');
        const subframeCount = subframeCountInput ? parseInt(subframeCountInput.value) || 1 : 1;

        for (let i = 0; i < subframeCount; i++) {
            confContent += `# --- Subframe Basic ${i} ---\n`;

            const subframeForm = document.querySelector(`#subframe${i} form`);
            if (subframeForm) {
                const subframeData = getFormDataAsObject(subframeForm);

                const prefix = i === 0 ? 'L1B_first_sub' : 'L1D';

                confContent += `subframe.[${i}].${prefix}_mimo=${subframeData[`plp_mimo_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_mimo_mixed=${subframeData[`plp_mimo_mixed_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_miso=${subframeData[`plp_miso_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_fft_size=${subframeData[`fft_size_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_reduced_carriers=${subframeData[`reduced_carrier_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_guard_interval=${subframeData[`guard_interval_${i}`] || '1'}\n`;
                confContent += `subframe.[${i}].${prefix}_num_ofdm_symbols=${subframeData[`num_ofdm_${i}`] || '1'}\n`;
                confContent += `subframe.[${i}].${prefix}_scattered_pilot_pattern=${subframeData[`spilot_pattern_${i}`] || '8'}\n`;
                confContent += `subframe.[${i}].${prefix}_scattered_pilot_boost=${subframeData[`spilot_boost_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_sbs_first=${subframeData[`sbs_first_${i}`] || '0'}\n`;
                confContent += `subframe.[${i}].${prefix}_sbs_last=${subframeData[`sbs_last_${i}`] || '0'}\n`;

                if (i > 0) {
                    confContent += `subframe.[${i}].L1D_subframe_multiplex=0\n`;
                }
                confContent += `subframe.[${i}].L1D_frequency_interleaver=${subframeData[`freq_interleaver_${i}`] || '0'}\n`;
                if (i > 0) {
                    confContent += `subframe.[${i}].L1D_sbs_null_cells=0\n`;
                }

                const plpCountInput = document.getElementById(`plp-count-${i}`);
                const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 1 : 1;

                confContent += `subframe.[${i}].L1D_num_plp=${plpCount}\n\n`;

                for (let j = 0; j < plpCount; j++) {
                    confContent += `# Subframe ${i} - PLP ${j}\n`;

                    const plpForm = document.querySelector(`#plp-${i}-${j} form`);
                    if (plpForm) {
                        const plpData = getFormDataAsObject(plpForm);

                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_id=${plpData[`plp_id_${j}`] || j}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_lls_flag=${plpData[`lls_flag_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_layer=${plpData[`layer_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_start=${plpData[`start_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_size=${plpData[`size_${j}`] || '1000'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_scrambler_type=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_fec_type=${plpData[`fec_type_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_mod=${plpData[`mod_order_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_cod=${plpData[`code_rate_${j}`] || '4'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_TI_mode=${plpData[`ti_mode_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_fec_block_start=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_CTI_fec_block_start=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_num_channel_bonded=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_channel_bonding_format=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_bonded_rf_id=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_TI_extended_interleaving=${plpData[`ti_extended_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_CTI_depth=${plpData[`cti_depth_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_CTI_start_row=0\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_mimo=${plpData[`mimo_plp_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_mimo_stream_combining=${plpData[`plp_mimo_stream_combining_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_mimo_IQ_interleaving=${plpData[`plp_mimo_IQ_intervaling_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_mimo_PH=${plpData[`plp_mimo_PH_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_type=${plpData[`plp_type_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_num_subslices=${plpData[`num_subslice_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_subslice_interval=${plpData[`subslice_interval_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_HTI_inter_subframe=${plpData[`inter_subframe_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_HTI_num_ti_blocks=${plpData[`num_ti_blocks_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_HTI_num_fec_blocks_max=${plpData[`num_fec_blocks_max_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_HTI_num_fec_blocks=${plpData[`num_fec_blocks_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_HTI_cell_interleaver=${plpData[`cell_intervaler_${j}`] || '0'}\n`;
                        confContent += `subframe.[${i}].plp.[${j}].L1D_plp_ldm_injection_level=${plpData[`inter_ldm_injection_level_${j}`] || '0'}\n`;
                        confContent += `\n`;
                    }
                }
            }
        }

        confContent += `# =============================================
# END OF CONFIGURATION FILE
# =============================================
`;

        return confContent;
    }

    exportConfig() {
        try {
            const confContent = this.generateConfFile();

            const blob = new Blob([confContent], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);

            const timestamp = new Date().toISOString().slice(0, 19).replace(/[:-]/g, '');
            const filename = `atsc3_config_${timestamp}.conf`;

            const a = document.createElement('a');
            a.href = url;
            a.download = filename;
            a.style.display = 'none';

            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);

            URL.revokeObjectURL(url);

            showBackupNotification(`Configuração exportada: ${filename}`, 'success');

        } catch (error) {
            showBackupNotification('Erro ao exportar Configuração!', 'error');
            alert('Erro ao exportar Configuração!');
        }
    }

    importConfigFile(event) {
        const self = this;
        const file = event.target.files[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = function(e) {
            try {
                let configData;
                const fileContent = e.target.result;

                if (file.name.toLowerCase().endsWith('.conf')) {
                    configData = self.parseConfFile(fileContent);
                } else {
                    configData = JSON.parse(fileContent);
                }

                localStorage.setItem('atsc_config_backup', JSON.stringify(configData));
                localStorage.setItem('atsc_backup_timestamp', Date.now().toString());

                loadConfigBackup();

                showBackupNotification('Configuração importada com sucesso!', 'success');

            } catch (error) {
                alert('Erro ao importar arquivo de Configuração! Verifique se o formato estÃ¡ correto.');
            }
        };
        reader.readAsText(file);

        event.target.value = '';
    }

    reloadPage() {
        location.reload();
    }

    validateFrameGeneration() {
        const errors = [];

        const l1bVersion = parseInt(document.getElementById('l1b_version')?.value || '0');
        const l1dVersion = parseInt(document.getElementById('l1d_version')?.value || '0');

        if (l1bVersion === 1 && l1dVersion <= 1) {
            let specificMessage;
            if (l1dVersion === 0) {
                specificMessage = 'ERRO CRITICO: L1B Version = 1 e L1D Version = 0 nao e uma combinacao valida. L1D Version deve ser maior que 1 quando L1B Version = 1';
            } else {
                specificMessage = 'Quando L1B Version e igual a 1, L1D Version deve ser maior que 1';
            }

            errors.push({
                message: specificMessage,
                location: 'Secao Preamble',
                field: 'L1D Version',
                action: () => showSection('preamble')
            });
        }

        return errors;
    }

    adjustStartValues() {
        const l1dVersion = parseInt(document.getElementById('l1d_version')?.value || '0');
        const subframeCount = parseInt(document.getElementById('number_of_subframes')?.value || '1');
        let adjustedCount = 0;

        for (let i = 0; i < subframeCount; i++) {
            const plpCountInput = document.getElementById(`plp-count-${i}`);
            const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 1 : 1;

            for (let j = 0; j < plpCount; j++) {
                const startInput = document.getElementById(`start_${j}`);
                if (startInput) {
                    let newStartValue;

                    if (l1dVersion === 2) {
                        newStartValue = plpCount === 1 ? 4388 : 3757;
                    } else if (l1dVersion === 1) {
                        newStartValue = plpCount === 1 ? 4485 : 3854;
                    } else {
                        newStartValue = plpCount === 1 ? 4388 : 3757;
                    }

                    if (parseInt(startInput.value) !== newStartValue) {
                        startInput.value = newStartValue;
                        adjustedCount++;

                        if (typeof updatePlpProgressBar === 'function') {
                            updatePlpProgressBar(i, j);
                        }
                    }
                }
            }
        }

        if (adjustedCount > 0) {
            alert(`${adjustedCount} valores START foram ajustados automaticamente`);
        } else {
            alert('Todos os valores START jÃ¡ estão corretos');
        }
    }

    showConfigurationWarning() {
        const l1bVersion = parseInt(document.getElementById('l1b_version')?.value || '0');
        const l1dVersion = parseInt(document.getElementById('l1d_version')?.value || '0');

        const existingWarning = document.getElementById('config-warning');
        if (existingWarning) {
            existingWarning.remove();
        }

        if (l1bVersion === 1 && l1dVersion <= 1) {
            const warning = document.createElement('div');
            warning.id = 'config-warning';
            warning.className = 'config-warning';
            warning.innerHTML = 'AtenÃ§Ã£o: Com L1B Version = 1, L1D Version deve ser maior que 1';

            const l1dVersionContainer = document.getElementById('l1d_version').closest('.form-group');
            if (l1dVersionContainer && l1dVersionContainer.parentNode) {
                l1dVersionContainer.parentNode.insertBefore(warning, l1dVersionContainer.nextSibling);
            }
        }
    }

    toggleMimoFieldsBasedOnMixed(subframeIndex) {
        const mimoMixedSelect = document.getElementById(`plp_mimo_mixed_${subframeIndex}`);
        if (!mimoMixedSelect) {
            return;
        }

        const isMimoMixedEnabled = mimoMixedSelect.value === "1";

        const plpSections = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);

        plpSections.forEach(section => {
            const match = section.id.match(/plp-\d+-(\d+)/);
            if (match) {
                const plpIndex = parseInt(match[1]);

                const mimoFields = [
                    `mimo_plp_${plpIndex}`,
                    `plp_mimo_stream_combining_${plpIndex}`,
                    `plp_mimo_IQ_intervaling_${plpIndex}`,
                    `plp_mimo_PH_${plpIndex}`
                ];

                mimoFields.forEach(fieldId => {
                    const field = document.getElementById(fieldId);
                    const label = document.querySelector(`label[for="${fieldId}"]`);

                    if (field && label) {
                        if (isMimoMixedEnabled) {
                            label.style.display = '';
                            field.style.display = '';
                        } else {
                            label.style.display = 'none';
                            field.style.display = 'none';
                            field.value = "0";
                        }
                    }
                });
            }
        });
    }

    initializeMimoControls() {
        const self = this;
        setTimeout(() => {
            const mimoMixedSelects = document.querySelectorAll('[id^="plp_mimo_mixed_"]');

            mimoMixedSelects.forEach(select => {
                const subframeIndex = select.id.replace('plp_mimo_mixed_', '');

                if (!select.hasAttribute('data-mimo-mixed-listener')) {
                    select.addEventListener('change', function() {
                        self.toggleMimoFieldsBasedOnMixed(subframeIndex);
                    });
                    select.setAttribute('data-mimo-mixed-listener', 'true');

                    self.toggleMimoFieldsBasedOnMixed(subframeIndex);
                }
            });
        }, 300);
    }

    toggleMimoMixed(subframeIndex) {
        const mimoSelect = document.getElementById(`plp_mimo_${subframeIndex}`);
        const mimoMixedContainer = document.querySelector(`#subframe${subframeIndex} .form-group:has(#plp_mimo_mixed_${subframeIndex})`);
        const mimoMixedSelect = document.getElementById(`plp_mimo_mixed_${subframeIndex}`);

        if (!mimoSelect || !mimoMixedContainer || !mimoMixedSelect) {
            return;
        }

        const mimoValue = mimoSelect.value;

        if (mimoValue === "0") {
            mimoMixedContainer.style.display = '';
            if (!mimoMixedSelect.value) {
                mimoMixedSelect.value = "0";
            }
        } else {
            mimoMixedContainer.style.display = 'none';
            mimoMixedSelect.value = "0";
        }

        this.toggleMimoFieldsBasedOnMixed(subframeIndex);
    }

    toggleMimoStreamFields(plpIndex) {
        const mimoPlpSelect = document.getElementById(`mimo_plp_${plpIndex}`);
        if (!mimoPlpSelect) return;

        const isMimoEnabled = mimoPlpSelect.value === "1";
        const mimoFields = [
            `plp_mimo_stream_combining_${plpIndex}`,
            `plp_mimo_IQ_intervaling_${plpIndex}`,
            `plp_mimo_PH_${plpIndex}`
        ];

        mimoFields.forEach(fieldId => {
            const field = document.getElementById(fieldId);
            const label = document.querySelector(`label[for="${fieldId}"]`);

            if (field && label) {
                if (isMimoEnabled) {
                    label.style.display = '';
                    field.style.display = '';
                } else {
                    label.style.display = 'none';
                    field.style.display = 'none';
                    field.value = "0";
                }
            }
        });
    }

    toggleEnhancedLayerFields(plpIndex) {
        const layerSelect = document.getElementById(`layer_${plpIndex}`);
        if (!layerSelect) {
            return;
        }

        const isEnhancedLayer = layerSelect.value === "1";

        const fieldsToHideOnEnhanced = [
            `fec_type_${plpIndex}`, `mod_order_${plpIndex}`, `code_rate_${plpIndex}`,
            `plp_type_${plpIndex}`, `num_subslice_${plpIndex}`, `subslice_interval_${plpIndex}`,
            `ti_extended_${plpIndex}`, `cti_depth_${plpIndex}`, `cell_intervaler_${plpIndex}`,
            `inter_subframe_${plpIndex}`, `num_ti_blocks_${plpIndex}`,
            `num_fec_blocks_max_${plpIndex}`, `num_fec_blocks_${plpIndex}`
        ];

        const enhancedOnlyFields = [`inter_ldm_injection_level_${plpIndex}`];

        function toggleFieldContainer(fieldId, show) {
            const field = document.getElementById(fieldId);
            if (!field) return;

            const label = document.querySelector(`label[for="${fieldId}"]`);

            if (label) {
                label.style.display = show ? '' : 'none';
                field.style.display = show ? '' : 'none';
                if (!show) field.value = "0";
            } else {
                let container = field.parentElement;
                if (container && (container.tagName === 'DIV' || container.classList.contains('form-group'))) {
                    container.style.display = show ? '' : 'none';
                    if (!show) field.value = "0";
                } else {
                    field.style.display = show ? '' : 'none';
                    if (!show) field.value = "0";
                }
            }
        }

        if (isEnhancedLayer) {
            fieldsToHideOnEnhanced.forEach(fieldId => toggleFieldContainer(fieldId, false));
            enhancedOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, true));
        } else {
            fieldsToHideOnEnhanced.forEach(fieldId => toggleFieldContainer(fieldId, true));
            enhancedOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
        }
    }

    toggleSubsliceFields(plpIndex) {
        const typeSelect = document.getElementById(`plp_type_${plpIndex}`);
        if (!typeSelect) return;

        const isNonDispersed = typeSelect.value === "0";

        const subsliceFields = [
            `num_subslice_${plpIndex}`,
            `subslice_interval_${plpIndex}`
        ];

        function toggleFieldContainer(fieldId, show) {
            const field = document.getElementById(fieldId);
            if (!field) return;

            const label = document.querySelector(`label[for="${fieldId}"]`);

            if (label) {
                label.style.display = show ? '' : 'none';
                field.style.display = show ? '' : 'none';
                if (!show) field.value = "0";
            } else {
                field.style.display = show ? '' : 'none';
                if (!show) field.value = "0";
            }
        }

        if (isNonDispersed) {
            subsliceFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
        } else {
            subsliceFields.forEach(fieldId => toggleFieldContainer(fieldId, true));
        }
    }

    initializeSubsliceControls() {
        const self = this;
        setTimeout(() => {
            const typeSelects = document.querySelectorAll('[id^="plp_type_"]');

            typeSelects.forEach(select => {
                const plpIndex = select.id.replace('plp_type_', '');

                if (!select.hasAttribute('data-subslice-listener')) {
                    select.addEventListener('change', function() {
                        self.toggleSubsliceFields(plpIndex);
                    });
                    select.setAttribute('data-subslice-listener', 'true');

                    self.toggleSubsliceFields(plpIndex);
                }
            });
        }, 200);
    }

    toggleTIFields(plpIndex) {
        const tiModeSelect = document.getElementById(`ti_mode_${plpIndex}`);
        const layerSelect = document.getElementById(`layer_${plpIndex}`);

        if (!tiModeSelect) return;

        const tiModeValue = tiModeSelect.value;
        const isNoTI = tiModeValue === "0";
        const isCTI = tiModeValue === "1";
        const isHTI = tiModeValue === "2";

        const isEnhancedLayer = layerSelect ? layerSelect.value === "1" : false;

        const ctiOnlyFields = [`cti_depth_${plpIndex}`];

        const htiOnlyFields = [
            `cell_intervaler_${plpIndex}`,
            `num_ti_blocks_${plpIndex}`,
            `num_fec_blocks_max_${plpIndex}`,
            `num_fec_blocks_${plpIndex}`
        ];

        const neverShowFields = [
            `ti_extended_${plpIndex}`,
            `inter_subframe_${plpIndex}`
        ];

        function toggleFieldContainer(fieldId, show) {
            const field = document.getElementById(fieldId);
            if (!field) return;

            const label = document.querySelector(`label[for="${fieldId}"]`);

            if (label) {
                label.style.display = show ? '' : 'none';
                field.style.display = show ? '' : 'none';
                if (!show) field.value = "0";
            } else {
                field.style.display = show ? '' : 'none';
                if (!show) field.value = "0";
            }
        }

        neverShowFields.forEach(fieldId => toggleFieldContainer(fieldId, false));

        if (isNoTI) {
            ctiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
            htiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
        } else if (isCTI) {
            ctiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, !isEnhancedLayer));
            htiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
        } else if (isHTI) {
            ctiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, false));
            htiOnlyFields.forEach(fieldId => toggleFieldContainer(fieldId, !isEnhancedLayer));
        }
    }

    initializeTIControls() {
        const self = this;
        setTimeout(() => {
            const tiModeSelects = document.querySelectorAll('[id^="ti_mode_"]');

            tiModeSelects.forEach(select => {
                const plpIndex = select.id.replace('ti_mode_', '');

                if (!select.hasAttribute('data-ti-listener')) {
                    select.addEventListener('change', function() {
                        self.toggleTIFields(plpIndex);
                    });
                    select.setAttribute('data-ti-listener', 'true');

                    self.toggleTIFields(plpIndex);
                }
            });
        }, 200);
    }

    toggleFrameLength() {
        var frameLengthMode = document.getElementById('frame_lenght_mode');
        var frameLengthInput = document.getElementById('frame_lenght');

        if (frameLengthMode && frameLengthInput) {
            var frameLengthContainer = frameLengthInput.closest('.form-group');

            if (frameLengthMode.value === '0') {
                if (frameLengthContainer) {
                    frameLengthContainer.style.display = '';
                }
                frameLengthInput.setAttribute('required', 'required');
                if (!frameLengthInput.value || frameLengthInput.value === '') {
                    frameLengthInput.value = '0';
                }
            } else {
                if (frameLengthContainer) {
                    frameLengthContainer.style.display = 'none';
                }
                frameLengthInput.removeAttribute('required');
                frameLengthInput.value = '0';
            }
        }
    }
}

// Create singleton
window.atscApp = new ATSCApp();

// Export all functions to window for backward compatibility
window.filterPreambleStructureByMode = (...args) => window.atscApp.filterPreambleStructureByMode(...args);
window.syncModeWithStructure = (...args) => window.atscApp.syncModeWithStructure(...args);
window.initPreambleModeFilter = (...args) => window.atscApp.initPreambleModeFilter(...args);
window.wizardGoToStep = (...args) => window.atscApp.wizardGoToStep(...args);
window.wizardNext = (...args) => window.atscApp.wizardNext(...args);
window.wizardPrev = (...args) => window.atscApp.wizardPrev(...args);
window.updateWizardProgress = (...args) => window.atscApp.updateWizardProgress(...args);
window.syncWizardWithSection = (...args) => window.atscApp.syncWizardWithSection(...args);
window.buildSubframeTabs = (...args) => window.atscApp.buildSubframeTabs(...args);
window.buildPlpTabs = (...args) => window.atscApp.buildPlpTabs(...args);
window.activateSubTab = (...args) => window.atscApp.activateSubTab(...args);
window.hideSubTabs = (...args) => window.atscApp.hideSubTabs(...args);
window.togglePlpAllocMode = (...args) => window.atscApp.togglePlpAllocMode(...args);
window.recalcAutoPlps = (...args) => window.atscApp.recalcAutoPlps(...args);
window.validatePlpManualSize = (...args) => window.atscApp.validatePlpManualSize(...args);
window.toggleL1dBsid = (...args) => window.atscApp.toggleL1dBsid(...args);
window.generateMenus = (...args) => window.atscApp.generateMenus(...args);
window.generatePLPMenusAndFields = (...args) => window.atscApp.generatePLPMenusAndFields(...args);
window.submitForm = (...args) => window.atscApp.submitForm(...args);
window.setFieldValue = (...args) => window.atscApp.setFieldValue(...args);
window.loadSubframesData = (...args) => window.atscApp.loadSubframesData(...args);
window.parseConfFile = (...args) => window.atscApp.parseConfFile(...args);
window.generateConfFile = (...args) => window.atscApp.generateConfFile(...args);
window.exportConfig = (...args) => window.atscApp.exportConfig(...args);
window.importConfigFile = (...args) => window.atscApp.importConfigFile(...args);
window.reloadPage = (...args) => window.atscApp.reloadPage(...args);
window.validateFrameGeneration = (...args) => window.atscApp.validateFrameGeneration(...args);
window.adjustStartValues = (...args) => window.atscApp.adjustStartValues(...args);
window.showConfigurationWarning = (...args) => window.atscApp.showConfigurationWarning(...args);
window.toggleMimoFieldsBasedOnMixed = (...args) => window.atscApp.toggleMimoFieldsBasedOnMixed(...args);
window.initializeMimoControls = (...args) => window.atscApp.initializeMimoControls(...args);
window.toggleMimoMixed = (...args) => window.atscApp.toggleMimoMixed(...args);
window.toggleMimoStreamFields = (...args) => window.atscApp.toggleMimoStreamFields(...args);
window.toggleEnhancedLayerFields = (...args) => window.atscApp.toggleEnhancedLayerFields(...args);
window.toggleSubsliceFields = (...args) => window.atscApp.toggleSubsliceFields(...args);
window.initializeSubsliceControls = (...args) => window.atscApp.initializeSubsliceControls(...args);
window.toggleTIFields = (...args) => window.atscApp.toggleTIFields(...args);
window.initializeTIControls = (...args) => window.atscApp.initializeTIControls(...args);
window.toggleFrameLength = (...args) => window.atscApp.toggleFrameLength(...args);

// Init preamble mode filter
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => window.atscApp.initPreambleModeFilter());
} else {
    window.atscApp.initPreambleModeFilter();
}

window.addEventListener('DOMContentLoaded', () => {
    if (!window.__BOOT_ONCE__) {
        window.__BOOT_ONCE__ = true;

        var l1dVersionInput = document.getElementById('l1d_version');
        if (l1dVersionInput) {
            window.atscApp.toggleL1dBsid();
            l1dVersionInput.addEventListener('change', function() {
                window.atscApp.toggleL1dBsid();
            });
        }

        var frameLengthMode = document.getElementById('frame_lenght_mode');
        if (frameLengthMode) {
            window.atscApp.toggleFrameLength();
            frameLengthMode.addEventListener('change', () => window.atscApp.toggleFrameLength());
        }

        var l1bVersionInput = document.getElementById('l1b_version');
        if (l1bVersionInput) {
            l1bVersionInput.addEventListener('change', () => window.atscApp.showConfigurationWarning());
        }

        startPlpIdSequence();

        if (input) {
            if (!input.value || parseInt(input.value) === 0) {
                input.value = 1;
            }

            const subframeCount = parseInt(input.value) || 1;
            generateMenus(subframeCount);

            for (let i = 0; i < subframeCount; i++) {
                const plpCountInput = document.getElementById(`plp-count-${i}`);
                if (plpCountInput && (!plpCountInput.value || parseInt(plpCountInput.value) === 0)) {
                    plpCountInput.value = 1;
                }
                generatePLPMenusAndFields(i);
            }

            showSection('botstrap');
        }

        setTimeout(() => {
            initializeBackupSystem();
            window.atscApp.initializeSubsliceControls();
            window.atscApp.initializeTIControls();
            window.atscApp.initializeMimoControls();

            window.atscApp.wizardGoToStep(1);

            setTimeout(() => {
                updateAllPlpProgressBars();
                window.atscApp.initializeSubsliceControls();
                window.atscApp.initializeTIControls();
                window.atscApp.initializeMimoControls();
            }, 1000);
        }, 1000);
    }
});

window.addEventListener('hashchange', function () {
    const sectionId = window.location.hash.substring(1);
    if (sectionId) {
        showSection(sectionId);
    }
});

window.addEventListener('DOMContentLoaded', async function() {
    fetchL1dSizeBytesFromLog();
    await Promise.all([fetchFrame2LogData(), fetchFrameDuration()]);

    setTimeout(function() {
        if (typeof window.subframesData !== 'undefined') {
            window.atscApp.loadSubframesData(window.subframesData);
        }
    }, 1500);
});
