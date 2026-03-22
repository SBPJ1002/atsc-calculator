function updateActiveMenu(activeId) {
    document.querySelectorAll('.wizard-tab, .sub-tab').forEach(item => {
        item.classList.remove('active');
    });
}

async function promptPercentage(subframeIndex, plpIndex) {
    const plpCapacity = await computePlpIndividualCapacity(subframeIndex, plpIndex);
    if (plpCapacity.capacity <= 0) {
        alert('Erro: ' + plpCapacity.reason);
        return;
    }

    createPercentageModal(subframeIndex, plpIndex, plpCapacity.capacity);
}

function createPercentageModal(subframeIndex, plpIndex, capacity) {
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
    updatePercentPreview(capacity);
}

function updatePercentPreview(capacity) {
    const slider = document.getElementById('percentSlider');
    const input = document.getElementById('percentInput');
    const preview = document.getElementById('percentPreview');

    if (!slider || !input || !preview) return;

    const percent = parseFloat(slider.value);
    input.value = percent;

    const cells = Math.floor(capacity * (percent / 100));
    preview.textContent = `${cells.toLocaleString()} cells`;
}

function updatePercentFromInput(capacity) {
    const slider = document.getElementById('percentSlider');
    const input = document.getElementById('percentInput');

    if (!slider || !input) return;

    const percent = Math.max(0, Math.min(100, parseFloat(input.value) || 0));
    slider.value = percent;
    input.value = percent;

    updatePercentPreview(capacity);
}

function setPercent(percent, capacity) {
    const slider = document.getElementById('percentSlider');
    const input = document.getElementById('percentInput');

    if (slider) slider.value = percent;
    if (input) input.value = percent;

    updatePercentPreview(capacity);
}

function applyPercent(subframeIndex, plpIndex, capacity) {
    const input = document.getElementById('percentInput');
    if (!input) return;

    const percent = parseFloat(input.value) || 0;
    const cells = Math.floor(capacity * (percent / 100));

    const sizeInput = document.getElementById(`size_${plpIndex}`);
    if (sizeInput) {
        sizeInput.value = cells;
        updatePlpProgressBar(subframeIndex, plpIndex);
    }

    closePercentModal();
}

function closePercentModal() {
    const modal = document.getElementById('percentModal');
    if (modal) modal.remove();
}

async function updatePlpProgressBar(subframeIndex, plpIndex) {
    // Trigger full subframe progress update in Resultados
    updateResultadosSubframeProgress(subframeIndex);
}

async function updateResultadosSubframeProgress(subframeIndex) {
    const container = document.getElementById('resultados-subframe-progress');
    if (!container) return;

    const plpCapacity = await computePlpIndividualCapacity(subframeIndex, 0);
    const preambleFields = await calculatePreambleFields(subframeIndex);

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

    let plpBreakdownHTML = '';
    plpRows.sort((a, b) => a.index - b.index);
    const fecStarts = window.__frame2LogData__?.fecBlockStarts || [];
    const sfDurMs = parseFloat(sfExtra.duracao) || 0;
    const sfDurSec = sfDurMs / 1000;

    for (let plpIdx = 0; plpIdx < plpRows.length; plpIdx++) {
        const plp = plpRows[plpIdx];
        const plpPercent = totalCapacity > 0 ? (plp.size / totalCapacity) * 100 : 0;
        const fecEntry = fecStarts[plpIdx];
        const fecDisplay = fecEntry?.fecBlockStart != null ? fecEntry.fecBlockStart.toLocaleString() : '—';

        // Read PLP parameters from form
        const plpIdVal = document.getElementById(`plp_id_${plp.index}`)?.value ?? plp.index;
        const fecType = parseInt(document.getElementById(`fec_type_${plp.index}`)?.value || '0', 10);
        const modOrder = parseInt(document.getElementById(`mod_order_${plp.index}`)?.value || '0', 10);
        const codeRateIdx = parseInt(document.getElementById(`code_rate_${plp.index}`)?.value || '0', 10);
        const tiMode = parseInt(document.getElementById(`ti_mode_${plp.index}`)?.value || '0', 10);
        const ctiDepthIdx = parseInt(document.getElementById(`cti_depth_${plp.index}`)?.value || '0', 10);
        const numFecBlocksMax = parseInt(document.getElementById(`num_fec_blocks_max_${plp.index}`)?.value || '0', 10);
        const startCell = parseInt(document.getElementById(`start_${plp.index}`)?.value || '0', 10);

        // LDPC size: odd fec_type → 64800 (long), even → 16200 (short)
        const ldpcSize = (fecType % 2 === 1) ? 64800 : 16200;

        // Bits per cell from modulation order
        const bitsPerCell = (modOrder + 1) * 2;

        // FEC Block Size in cells
        const fecBlockSize = ldpcSize / bitsPerCell;

        // Number of FEC blocks that fit in this PLP
        const numFecBlocks = plp.size > 0 ? Math.floor(plp.size / fecBlockSize) : 0;

        // Code rate fraction
        const codeRateNum = codeRateIdx + 2;
        const codeRateDen = 15;

        // Células TI (Time Interleaver cells)
        // Fórmula: número triangular T = N_rows * (N_rows - 1) / 2
        // Para TI-Mode None ou CTI: usa CTI depth (default 512 quando None)
        // Para HTI: usa numFecBlocksMax * fecBlockSize
        const ctiDepthValues = [512, 724, 887, 1024];
        const ctiDepthValuesExtended = [512, 724, 1254, 1448];
        const tiExtended = parseInt(document.getElementById(`ti_extended_${plp.index}`)?.value || '0', 10);
        let celulasTI;
        if (tiMode === 0) {
            // TI-Mode None: usa default CTI depth 512
            celulasTI = 512 * 511 / 2;
        } else if (tiMode === 1) {
            // CTI: usa o CTI depth configurado
            const depthValues = tiExtended ? ctiDepthValuesExtended : ctiDepthValues;
            const N_rows = depthValues[ctiDepthIdx] || 512;
            celulasTI = N_rows * (N_rows - 1) / 2;
        } else {
            // HTI
            celulasTI = numFecBlocksMax * fecBlockSize;
        }

        // Outer code parity bits (BCH/CRC overhead per FEC block)
        // fec_type: 0=BCH+16K, 1=BCH+64K, 2=CRC+16K, 3=CRC+64K, 4=16K only, 5=64K only
        let outerCodeParity = 0;
        if (fecType === 0) outerCodeParity = 168;           // BCH + 16K LDPC
        else if (fecType === 1) outerCodeParity = 192;      // BCH + 64K LDPC
        else if (fecType === 2 || fecType === 3) outerCodeParity = 32; // CRC-32

        const bbpHeader = 16; // 2 bytes BBP header
        const kLdpc = ldpcSize * (codeRateNum / codeRateDen);
        const kPayload = kLdpc - outerCodeParity - bbpHeader;
        const payloadEff = (kLdpc > 0) ? (kPayload / kLdpc) : 1;

        // BBFramerate and Bitrate (uses frame duration for average data rate)
        let bbFrameRateDisplay = '—';
        let bitrateDisplay = '—';
        const frameDurSec = (window.__frameDurationMs__ || 0) / 1000;
        if (frameDurSec > 0 && plp.size > 0) {
            const bbfr = Math.trunc(numFecBlocks / frameDurSec);
            bbFrameRateDisplay = bbfr + ' fps';
            // Data Rate = PLP_cells × bits_per_cell × code_rate × payload_efficiency / frame_duration
            const bitrateBps = plp.size * bitsPerCell * (codeRateNum / codeRateDen) * payloadEff / frameDurSec;
            bitrateDisplay = (bitrateBps / 1e6).toFixed(3) + ' Mbps';
        }

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
                        <span class="sf-plp-field-label">Quantidade de Células:</span>
                        <span class="sf-plp-field-value">${plp.size.toLocaleString()}</span>
                    </div>
                    <div class="sf-plp-field">
                        <span class="sf-plp-field-label">Célula de Início:</span>
                        <span class="sf-plp-field-value">${startCell.toLocaleString()}</span>
                    </div>
                    <div class="sf-plp-field">
                        <span class="sf-plp-field-label">Células TI:</span>
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
                ${(() => {
                    const cnr = (typeof getCnrValues === 'function') ? getCnrValues(modOrder, codeRateIdx, fecType) : null;
                    const f = (typeof formatCnr === 'function') ? formatCnr : (v) => v != null ? v.toFixed(2) : '—';
                    return `
                <div class="sf-plp-cnr-section">
                    <div class="sf-plp-cnr-group">
                        <span class="sf-plp-cnr-title">BICM CNR</span>
                        <div class="sf-plp-cnr-fields">
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">AWGN [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.bicm.awgn) : '—'}</span>
                            </div>
                        </div>
                    </div>
                    <div class="sf-plp-cnr-group">
                        <span class="sf-plp-cnr-title">Simulation CNR</span>
                        <div class="sf-plp-cnr-fields">
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">AWGN [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.simulation.awgn) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RC20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.simulation.rc20) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RL20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.simulation.rl20) : '—'}</span>
                            </div>
                        </div>
                    </div>
                    <div class="sf-plp-cnr-group">
                        <span class="sf-plp-cnr-title">Lab CNR</span>
                        <div class="sf-plp-cnr-fields">
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">AWGN [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.lab.awgn) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RC20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.lab.rc20) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RL20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.lab.rl20) : '—'}</span>
                            </div>
                        </div>
                    </div>
                    <div class="sf-plp-cnr-group">
                        <span class="sf-plp-cnr-title">Field CNR</span>
                        <div class="sf-plp-cnr-fields">
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">AWGN [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.field.awgn) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RC20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.field.rc20) : '—'}</span>
                            </div>
                            <div class="sf-plp-field">
                                <span class="sf-plp-field-label">RL20 [dB]:</span>
                                <span class="sf-plp-field-value">${cnr ? f(cnr.field.rl20) : '—'}</span>
                            </div>
                        </div>
                    </div>
                </div>`;
                })()}
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
                <span class="preamble-field-label">Duração:</span>
                <span class="preamble-field-value" id="sf-duracao-${subframeIndex}">${duracao}</span>
            </div>
            <div class="preamble-field">
                <span class="preamble-field-label">Células Disponíveis:</span>
                <span class="preamble-field-value">${totalCapacity.toLocaleString()}</span>
            </div>
            <div class="preamble-field">
                <span class="preamble-field-label">Células em DS:</span>
                <span class="preamble-field-value" id="sf-cells-ds-${subframeIndex}">${cps.toLocaleString()}</span>
            </div>
            <div class="preamble-field">
                <span class="preamble-field-label">Células em SBS:</span>
                <span class="preamble-field-value" id="sf-cells-sbs-${subframeIndex}">${cellsSBSS.toLocaleString()}</span>
            </div>
            <div class="preamble-field">
                <span class="preamble-field-label">Células Fictícias:</span>
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

    // Update preamble cells and store calculated numSymbols for config export
    if (preambleFields && preambleFields.numSymbols != null) {
        window.__preambleNumSymbols__ = preambleFields.numSymbols - 1;
    }
    updateResultadosPreambleCells(preambleFields);

    // Re-apply frame duration values after re-render
    if (typeof applyFrameDurationToDOM === 'function') {
        applyFrameDurationToDOM();
    }
}

async function updateAllPlpProgressBars() {
    const numSubframes = parseInt(document.getElementById('number_of_subframes')?.value || '1', 10);
    for (let sf = 0; sf < numSubframes; sf++) {
        await updateResultadosSubframeProgress(sf);
    }
}

function showSection(id) {
    document.querySelectorAll('.config-section, .dynamic-section').forEach(div => {
        div.style.display = 'none';
    });

    const target = document.getElementById(id);
    if (target) {
        target.style.display = 'block';

        if (window.location.hash !== '#' + id) {
            history.replaceState(null, null, '#' + id);
        }

        updateActiveMenu(id);

        if (typeof syncWizardWithSection === 'function') {
            syncWizardWithSection(id);
        }
    }
}

function reloadPage() {
    location.reload();
}

function updateResultadosPreambleCells(preambleFields) {
    const container = document.getElementById('resultados-preamble-cells');
    if (!container || !preambleFields) return;

    const timeOffset = window.__frame2LogData__?.timeOffset;
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
                <span class="preamble-field-label">Qtd. Símbolos:</span>
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

function sendAction(action) {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                console.log(xhr.responseText);
            } else {
                console.error("Erro ao enviar a acao: " + action);
            }
        }
    };
    xhr.send("action=" + action);
}
