class UiComponents {

    updateActiveMenu(activeId) {
        document.querySelectorAll('.wizard-tab, .sub-tab').forEach(item => {
            item.classList.remove('active');
        });
    }

    async promptPercentage(subframeIndex, plpIndex) {
        const plpCapacity = await computePlpIndividualCapacity(subframeIndex, plpIndex);
        if (plpCapacity.capacity <= 0) {
            alert('Erro: ' + plpCapacity.reason);
            return;
        }

        this.createPercentageModal(subframeIndex, plpIndex, plpCapacity.capacity);
    }

    createPercentageModal(subframeIndex, plpIndex, capacity) {
        const existing = document.getElementById('percentModal');
        if (existing) existing.remove();

        const modal = document.createElement('div');
        modal.id = 'percentModal';
        modal.className = 'modal-overlay';
        modal.innerHTML = `
            <div class="modal-content">
                <div class="modal-header">
                    <h3>Definir Porcentagem - PLP ${plpIndex}</h3>
                    <span class="modal-close" onclick="closePercentModal()">&times;</span>
                </div>
                <div class="modal-body">
                    <p>Capacidade do PLP: <strong>${capacity.toLocaleString()} cells</strong></p>

                    <div class="percentage-group">
                        <label>Porcentagem:</label>
                        <input type="range" id="percentSlider" min="0" max="100" value="50"
                               class="percentage-slider"
                               oninput="updatePercentPreview(${capacity})">
                        <input type="number" id="percentInput" min="0" max="100" value="50"
                               style="width: 80px;"
                               oninput="updatePercentFromInput(${capacity})">%
                    </div>

                    <div class="quick-buttons">
                        <button class="quick-btn" onclick="setPercent(25, ${capacity})">25%</button>
                        <button class="quick-btn" onclick="setPercent(50, ${capacity})">50%</button>
                        <button class="quick-btn" onclick="setPercent(75, ${capacity})">75%</button>
                        <button class="quick-btn" onclick="setPercent(100, ${capacity})">100%</button>
                    </div>

                    <div id="percentPreview" style="margin: 10px 0; font-weight: 600;"></div>
                </div>
                <div class="modal-footer">
                    <button class="btn" onclick="closePercentModal()">Cancelar</button>
                    <button class="btn btn-fill" onclick="applyPercent(${subframeIndex}, ${plpIndex}, ${capacity})">Aplicar</button>
                </div>
            </div>
        `;

        document.body.appendChild(modal);
        this.updatePercentPreview(capacity);
    }

    updatePercentPreview(capacity) {
        const slider = document.getElementById('percentSlider');
        const input = document.getElementById('percentInput');
        const preview = document.getElementById('percentPreview');

        if (!slider || !input || !preview) return;

        const percent = parseFloat(slider.value);
        input.value = percent;

        const cells = Math.floor(capacity * (percent / 100));
        preview.textContent = `${cells.toLocaleString()} cells`;
    }

    updatePercentFromInput(capacity) {
        const slider = document.getElementById('percentSlider');
        const input = document.getElementById('percentInput');

        if (!slider || !input) return;

        const percent = Math.max(0, Math.min(100, parseFloat(input.value) || 0));
        slider.value = percent;
        input.value = percent;

        this.updatePercentPreview(capacity);
    }

    setPercent(percent, capacity) {
        const slider = document.getElementById('percentSlider');
        const input = document.getElementById('percentInput');

        if (slider) slider.value = percent;
        if (input) input.value = percent;

        this.updatePercentPreview(capacity);
    }

    applyPercent(subframeIndex, plpIndex, capacity) {
        const input = document.getElementById('percentInput');
        if (!input) return;

        const percent = parseFloat(input.value) || 0;
        const cells = Math.floor(capacity * (percent / 100));

        const sizeInput = document.getElementById(`size_${plpIndex}`);
        if (sizeInput) {
            sizeInput.value = cells;
            this.updatePlpProgressBar(subframeIndex, plpIndex);
        }

        this.closePercentModal();
    }

    closePercentModal() {
        const modal = document.getElementById('percentModal');
        if (modal) modal.remove();
    }

    async updatePlpProgressBar(subframeIndex, plpIndex) {
        this.updateResultadosSubframeProgress(subframeIndex);
    }

    async updateResultadosSubframeProgress(subframeIndex) {
        const container = document.getElementById('resultados-subframe-progress');
        if (!container) return;

        // All data comes from server
        let resultsData;
        try {
            resultsData = await fetchResultsData(subframeIndex);
        } catch (e) {
            console.error('Error fetching results data:', e);
            return;
        }

        if (!resultsData || resultsData.status !== 'ok') return;

        const preambleFields = resultsData.preambleFields;
        const sfResultData = resultsData.subframeResults?.[0];
        const plpCapacity = sfResultData?.plpCapacity;

        if (!plpCapacity || plpCapacity.capacity <= 0) return;

        const plpCells = preambleFields ? preambleFields.plpCells : 0;
        const totalCapacity = plpCapacity.capacity + plpCells;
        const cellsSBSS = plpCapacity.capacitySBS;
        const cps = plpCapacity.cps;

        const plpElements = document.querySelectorAll(`[id^="plp-${subframeIndex}-"]`);
        const plpRows = [];
        let totalUsed = 0;

        plpElements.forEach(el => {
            const match = el.id.match(/plp-(\d+)-(\d+)/);
            if (match) {
                const idx = parseInt(match[2], 10);
                const size = parseInt(document.getElementById(`size_${idx}`)?.value || '0', 10);
                totalUsed += size;
                plpRows.push({ index: idx, size });
            }
        });

        const usedPercent = Math.min(100, (totalUsed / totalCapacity) * 100);
        const remaining = totalCapacity - totalUsed;
        const isOverCapacity = totalUsed > totalCapacity;

        const sfExtra = (window.__sfExtraData__ && window.__sfExtraData__[subframeIndex]) || {};

        // Build PLP breakdown using server-computed data
        let plpBreakdownHTML = '';
        plpRows.sort((a, b) => a.index - b.index);
        const fecStarts = resultsData.frame2LogData?.fecBlockStarts || [];
        const plpResults = sfResultData?.plpResults || [];

        const f = (v) => v != null ? v.toFixed(2) : '—';

        for (let plpIdx = 0; plpIdx < plpRows.length; plpIdx++) {
            const plp = plpRows[plpIdx];
            const plpIdVal = document.getElementById(`plp_id_${plp.index}`)?.value ?? plp.index;
            const startCell = parseInt(document.getElementById(`start_${plp.index}`)?.value || '0', 10);
            const fecEntry = fecStarts[plpIdx];
            const fecDisplay = fecEntry?.fecBlockStart != null ? fecEntry.fecBlockStart.toLocaleString() : '—';

            // Server-computed PLP results
            const pr = plpResults[plpIdx] || {};
            const fecBlockSize = pr.fecBlockSize || 0;
            const celulasTI = pr.celulasTI || 0;
            const bbFrameRate = pr.bbFrameRate || 0;
            const bitrateMbps = pr.bitrateMbps || 0;
            const cnr = pr.cnr;

            const bbFrameRateDisplay = bbFrameRate > 0 ? bbFrameRate + ' fps' : '—';
            const bitrateDisplay = bitrateMbps > 0 ? bitrateMbps.toFixed(3) + ' Mbps' : '—';

            plpBreakdownHTML += `
                <div class="sf-plp-card">
                    <div class="sf-plp-card-header">
                        <span class="sf-plp-label">PLP ${plpIdVal}</span>
                    </div>
                    <div class="sf-plp-card-fields">
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">ID do PLP:</span>
                            <span class="sf-plp-field-value">${plpIdVal}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">BBFramerate:</span>
                            <span class="sf-plp-field-value">${bbFrameRateDisplay}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">Bitrate:</span>
                            <span class="sf-plp-field-value">${bitrateDisplay}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">Quantidade de Celulas:</span>
                            <span class="sf-plp-field-value">${plp.size.toLocaleString()}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">Celula de Inicio:</span>
                            <span class="sf-plp-field-value">${startCell.toLocaleString()}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">Celulas TI:</span>
                            <span class="sf-plp-field-value">${celulasTI.toLocaleString()}</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">FEC Block Size:</span>
                            <span class="sf-plp-field-value">${fecBlockSize.toLocaleString()} cells</span>
                        </div>
                        <div class="sf-plp-field">
                            <span class="sf-plp-field-label">FEC Block Start:</span>
                            <span class="sf-plp-field-value">${fecDisplay}</span>
                        </div>
                    </div>
                    ${cnr ? `
                    <div class="sf-plp-cnr-section">
                        <div class="sf-plp-cnr-group">
                            <span class="sf-plp-cnr-title">BICM CNR</span>
                            <div class="sf-plp-cnr-fields">
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">AWGN [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.bicm?.awgn)}</span>
                                </div>
                            </div>
                        </div>
                        <div class="sf-plp-cnr-group">
                            <span class="sf-plp-cnr-title">Simulation CNR</span>
                            <div class="sf-plp-cnr-fields">
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">AWGN [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.simulation?.awgn)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RC20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.simulation?.rc20)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RL20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.simulation?.rl20)}</span>
                                </div>
                            </div>
                        </div>
                        <div class="sf-plp-cnr-group">
                            <span class="sf-plp-cnr-title">Lab CNR</span>
                            <div class="sf-plp-cnr-fields">
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">AWGN [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.lab?.awgn)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RC20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.lab?.rc20)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RL20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.lab?.rl20)}</span>
                                </div>
                            </div>
                        </div>
                        <div class="sf-plp-cnr-group">
                            <span class="sf-plp-cnr-title">Field CNR</span>
                            <div class="sf-plp-cnr-fields">
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">AWGN [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.field?.awgn)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RC20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.field?.rc20)}</span>
                                </div>
                                <div class="sf-plp-field">
                                    <span class="sf-plp-field-label">RL20 [dB]:</span>
                                    <span class="sf-plp-field-value">${f(cnr.field?.rl20)}</span>
                                </div>
                            </div>
                        </div>
                    </div>` : ''}
                </div>
            `;
        }

        const placeholder = container.querySelector('.resultados-placeholder');
        if (placeholder) placeholder.remove();

        let card = container.querySelector(`[data-sf="${subframeIndex}"]`);
        if (!card) {
            card = document.createElement('div');
            card.className = 'resultados-sf-card';
            card.dataset.sf = subframeIndex;
            container.appendChild(card);
        }

        const duracao = sfExtra.duracao ?? '—';
        const cellsFict = Math.max(0, totalCapacity - totalUsed);

        card.innerHTML = `
            <div class="resultados-sf-header">Subframe ${subframeIndex}</div>
            <div class="resultados-sf-fields">
                <div class="preamble-field">
                    <span class="preamble-field-label">Duracao:</span>
                    <span class="preamble-field-value" id="sf-duracao-${subframeIndex}">${duracao}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Celulas Disponiveis:</span>
                    <span class="preamble-field-value">${totalCapacity.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Celulas em DS:</span>
                    <span class="preamble-field-value" id="sf-cells-ds-${subframeIndex}">${cps.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Celulas em SBS:</span>
                    <span class="preamble-field-value" id="sf-cells-sbs-${subframeIndex}">${cellsSBSS.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Celulas Ficticias:</span>
                    <span class="preamble-field-value" id="sf-cells-fict-${subframeIndex}">${cellsFict.toLocaleString()}</span>
                </div>
            </div>
            <div class="plp-progress-bar ${isOverCapacity ? 'over-capacity' : ''}">
                <div class="plp-progress-fill" style="width: ${Math.min(100, usedPercent)}%"></div>
                <div class="plp-progress-text">${usedPercent.toFixed(1)}%</div>
            </div>
            <div class="plp-progress-info">
                <span class="info-used">${totalUsed.toLocaleString()} usado</span>
                <span class="info-capacity">${totalCapacity.toLocaleString()} total</span>
                <span class="info-remaining ${isOverCapacity ? 'negative' : ''}">${remaining.toLocaleString()} ${isOverCapacity ? 'excesso' : 'livre'}</span>
            </div>
            <div class="sf-plp-breakdown">${plpBreakdownHTML}</div>
        `;

        if (preambleFields && preambleFields.numSymbols != null) {
            window.__preambleNumSymbols__ = preambleFields.numSymbols - 1;
        }
        this.updateResultadosPreambleCells(preambleFields, resultsData);

        if (typeof applyFrameDurationToDOM === 'function') {
            applyFrameDurationToDOM();
        }
    }

    async updateAllPlpProgressBars() {
        const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
        for (let sf = 0; sf < numSubframes; sf++) {
            await this.updateResultadosSubframeProgress(sf);
        }
    }

    showSection(id) {
        document.querySelectorAll('.config-section, .dynamic-section').forEach(div => {
            div.style.display = 'none';
        });

        const target = document.getElementById(id);
        if (target) {
            target.style.display = 'block';

            if (window.location.hash !== '#' + id) {
                history.replaceState(null, null, '#' + id);
            }

            this.updateActiveMenu(id);

            if (typeof syncWizardWithSection === 'function') {
                syncWizardWithSection(id);
            }
        }
    }

    reloadPage() {
        location.reload();
    }

    updateResultadosPreambleCells(preambleFields, resultsData) {
        const container = document.getElementById('resultados-preamble-cells');
        if (!container || !preambleFields) return;

        const timeOffset = resultsData?.frame2LogData?.timeOffset ?? window.__frame2LogData__?.timeOffset;
        const timeOffsetDisplay = timeOffset != null ? timeOffset.toLocaleString() : '—';

        container.innerHTML = `
            <div class="preamble-fields-grid">
                <div class="preamble-field">
                    <span class="preamble-field-label">L1B Cells:</span>
                    <span class="preamble-field-value">${preambleFields.l1bCells.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Cells in First:</span>
                    <span class="preamble-field-value">${preambleFields.cellsInFirst.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Qtd. Simbolos:</span>
                    <span class="preamble-field-value">${preambleFields.numSymbols}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">L1D Bytes:</span>
                    <span class="preamble-field-value">${preambleFields.l1dBytes.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">L1D Cells:</span>
                    <span class="preamble-field-value">${preambleFields.l1dCells.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Cells in Next:</span>
                    <span class="preamble-field-value">${preambleFields.cellsInNext.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">PLP Cells:</span>
                    <span class="preamble-field-value">${preambleFields.plpCells.toLocaleString()}</span>
                </div>
                <div class="preamble-field">
                    <span class="preamble-field-label">Time Offset:</span>
                    <span class="preamble-field-value" id="resultados-time-offset">${timeOffsetDisplay}</span>
                </div>
            </div>
        `;
    }

    sendAction(action) {
        const xhr = new XMLHttpRequest();
        xhr.open("POST", "", true);
        xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        xhr.onreadystatechange = function () {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                }
            }
        };
        xhr.send("action=" + action);
    }
}

window.uiComponents = new UiComponents();
window.updateActiveMenu = (...args) => window.uiComponents.updateActiveMenu(...args);
window.promptPercentage = (...args) => window.uiComponents.promptPercentage(...args);
window.createPercentageModal = (...args) => window.uiComponents.createPercentageModal(...args);
window.updatePercentPreview = (...args) => window.uiComponents.updatePercentPreview(...args);
window.updatePercentFromInput = (...args) => window.uiComponents.updatePercentFromInput(...args);
window.setPercent = (...args) => window.uiComponents.setPercent(...args);
window.applyPercent = (...args) => window.uiComponents.applyPercent(...args);
window.closePercentModal = (...args) => window.uiComponents.closePercentModal(...args);
window.updatePlpProgressBar = (...args) => window.uiComponents.updatePlpProgressBar(...args);
window.updateResultadosSubframeProgress = (...args) => window.uiComponents.updateResultadosSubframeProgress(...args);
window.updateAllPlpProgressBars = (...args) => window.uiComponents.updateAllPlpProgressBars(...args);
window.showSection = (...args) => window.uiComponents.showSection(...args);
window.reloadPage = (...args) => window.uiComponents.reloadPage(...args);
window.updateResultadosPreambleCells = (...args) => window.uiComponents.updateResultadosPreambleCells(...args);
window.sendAction = (...args) => window.uiComponents.sendAction(...args);
