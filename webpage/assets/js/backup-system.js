const BACKUP_CONFIG = {
    localStorageKey: 'atsc_config_backup',
    subframesKey: 'atsc_subframes_backup',
    timestampKey: 'atsc_backup_timestamp',
    serverCheckInterval: 5*60*1000,
    maxBackupAge: 7 * 24 * 60 * 60 * 1000
};

let serverStatus = {
    available: typeof serverAvailable !== 'undefined' ? serverAvailable : true,
    lastCheck: Date.now(),
    checkInterval: null,
    usingBackup: false
};

function showBackupNotification(message, type = 'info') {
    console.log(`ðŸ“± ${message}`);
    
    let notificationArea = document.getElementById('backup-notifications');
    if (!notificationArea) {
        notificationArea = document.createElement('div');
        notificationArea.id = 'backup-notifications';
        notificationArea.style.cssText = `
            position: fixed; top: 50px; left: 10px; z-index: 1001; max-width: 300px;
        `;
        document.body.appendChild(notificationArea);
    }
    
    const notification = document.createElement('div');
    notification.style.cssText = `
        padding: 10px; margin-bottom: 10px; border-radius: 5px; font-size: 12px;
        animation: slideIn 0.3s ease; box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        ${type === 'success' ? 'background: #4CAF50; color: white;' : ''}
        ${type === 'warning' ? 'background: #ff9800; color: white;' : ''}
        ${type === 'error' ? 'background: #f44336; color: white;' : ''}
        ${type === 'info' ? 'background: #2196F3; color: white;' : ''}
    `;
    notification.textContent = message;
    
    notificationArea.appendChild(notification);
    
    setTimeout(() => {
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 100);
    }, 1000);
}

function updateServerStatusIndicator() {
    let indicator = document.getElementById('server-status');
    
    if (!indicator) {
        console.log('Indicador de status do header não encontrado');
        return;
    }
    
    if (serverStatus.available) {
        indicator.textContent = 'Servidor Online';
        indicator.classList.add('online');
        indicator.classList.remove('offline');
    } else {
        indicator.textContent = 'Modo Offline (Backup)';
        indicator.classList.remove('online');
        indicator.classList.add('offline');
    }
}

function removeDuplicateStatusIndicator() {
    const duplicateIndicator = document.getElementById('server-status-indicator');
    if (duplicateIndicator) {
        duplicateIndicator.remove();
        console.log('Indicador de status duplicado removido');
    }
}

function updateBackupStatus(success) {
    const timestamp = new Date().toLocaleTimeString();
    const indicator = document.getElementById('server-status-indicator');
    
    if (indicator && success) {
        const originalBg = indicator.style.background;
        indicator.textContent = `Backup salvo ${timestamp}`;
        indicator.style.background = '#2196F3';
        
        setTimeout(() => {
            updateServerStatusIndicator();
        }, 3000);
    }
}

function getFormDataAsObject(formElement) {
    if (typeof formElement === 'string') {
        formElement = document.getElementById(formElement);
    }
    
    if (!formElement) {
        console.log(`Formulário não encontrado:`, formElement);
        return {};
    }
    
    const formData = {};
    const inputs = formElement.querySelectorAll('input, select, textarea');
    
    console.log(` Coletando dados de ${inputs.length} campos do formulário:`, formElement.id || formElement.tagName);
    
    inputs.forEach(input => {
        if (input.name || input.id) {
            const key = input.name || input.id;
            let value;
            
            if (input.type === 'checkbox') {
                value = input.checked;
            } else {
                value = input.value;
            }
            
            if (value !== '' && value !== null && value !== undefined) {
                formData[key] = value;
                console.log(`  âœ“ ${key} = ${value}`);
            }
        }
    });
    
    console.log(` Total de ${Object.keys(formData).length} campos coletados`);
    return formData;
}

function loadFormData(formElement, data) {
    if (typeof formElement === 'string') {
        formElement = document.getElementById(formElement);
    }
    
    if (!formElement || !data) {
        console.log(`Erro ao carregar dados:`, {formElement: !!formElement, data: !!data});
        return;
    }
    
    const inputs = formElement.querySelectorAll('input, select, textarea');
    console.log(`Aplicando dados em ${inputs.length} campos disponíveis`);
    
    let appliedCount = 0;
    inputs.forEach(input => {
        const key = input.name || input.id;
        if (data.hasOwnProperty(key)) {
            if (input.type === 'checkbox') {
                input.checked = data[key];
            } else {
                input.value = data[key];
            }
            console.log(`${key} = ${data[key]}`);
            appliedCount++;
        }
    });
    
    console.log(`${appliedCount} campos aplicados com sucesso`);
}

function saveConfigBackup() {
    try {
        console.log('Iniciando salvamento do backup...');
        
        const configData = {
            bootstrap: {},
            preamble: {},
            subframes: {},
            timestamp: Date.now()
        };

        console.log(' Salvando bootstrap...');
        const bootstrapForm = document.getElementById('botstrap_form');
        if (bootstrapForm) {
            configData.bootstrap = getFormDataAsObject(bootstrapForm);
        } else {
            console.log('Formulário bootstrap não encontrado');
        }
        
        console.log(' Salvando preamble...');
        const preambleForm = document.getElementById('preamble_form');
        if (preambleForm) {
            configData.preamble = getFormDataAsObject(preambleForm);
        } else {
            console.log('Formulário preamble não encontrado');
        }

        const subframeCountInput = document.getElementById('number_of_subframes');
        const subframeCount = subframeCountInput ? parseInt(subframeCountInput.value) || 0 : 0;
        console.log(`Salvando ${subframeCount} subframes...`);
        
        for (let i = 0; i < subframeCount; i++) {
            console.log(`Processando subframe ${i}...`);
            
            const subframeSection = document.getElementById(`subframe${i}`);
            if (subframeSection) {
                const subframeForm = subframeSection.querySelector('form');
                
                if (subframeForm) {
                    console.log(`Formulário do subframe ${i} encontrado`);
                    
                    const subframeConfig = getFormDataAsObject(subframeForm);
                    
                    configData.subframes[i] = {
                        config: subframeConfig,
                        plps: {}
                    };

                    const plpCountInput = document.getElementById(`plp-count-${i}`);
                    const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 0 : 0;
                    
                    console.log(`Subframe ${i} tem ${plpCount} PLPs configurados`);
                    
                    for (let j = 0; j < plpCount; j++) {
                        console.log(`Processando PLP ${j} do subframe ${i}...`);
                        
                        const plpSection = document.getElementById(`plp-${i}-${j}`);
                        if (plpSection) {
                            const plpForm = plpSection.querySelector('form');
                            
                            if (plpForm) {
                                console.log(`FormulÃ¡rio do PLP ${j} encontrado`);
                                configData.subframes[i].plps[j] = getFormDataAsObject(plpForm);
                            } else {
                                console.log(`Formulário do PLP ${j} não encontrado, tentando campos individuais...`);
                                
                                const plpData = {};
                                const plpFields = [
                                    'plp_id', 'lls_flag', 'layer', 'start', 'size', 'fec_type',
                                    'mod_order', 'code_rate', 'ti_mode', 'ti_extended', 'cti_depth',
                                    'mimo_plp', 'plp_mimo_stream_combining', 'plp_mimo_IQ_intervaling',
                                    'plp_mimo_PH', 'plp_type', 'num_subslice', 'subslice_interval', 
                                    'cell_intervaler', 'inter_subframe', 'num_ti_blocks', 
                                    'num_fec_blocks_max', 'num_fec_blocks', 'inter_ldm_injection_level'
                                ];
                                
                                plpFields.forEach(fieldName => {
                                    const fieldId = `${fieldName}_${j}`;
                                    const fieldElement = document.getElementById(fieldId);
                                    if (fieldElement && fieldElement.value) {
                                        plpData[fieldName] = fieldElement.value;
                                    }
                                });
                                
                                if (Object.keys(plpData).length > 0) {
                                    configData.subframes[i].plps[j] = plpData;
                                    console.log(`PLP ${j} salvo com ${Object.keys(plpData).length} campos individuais`);
                                }
                            }
                        } else {
                            console.log(`Seção do PLP ${j} não encontrada`);
                        }
                    }
                } else {
                    console.log(`Formulário do subframe ${i} não encontrado`);
                }
            } else {
                console.log(`Seção do subframe ${i} não encontrada`);
            }
        }

        localStorage.setItem(BACKUP_CONFIG.localStorageKey, JSON.stringify(configData));
        localStorage.setItem(BACKUP_CONFIG.timestampKey, Date.now().toString());
        
        console.log('Backup completo salvo!');
        console.log('Resumo:', {
            bootstrap: Object.keys(configData.bootstrap).length + ' campos',
            preamble: Object.keys(configData.preamble).length + ' campos',
            subframes: Object.keys(configData.subframes).length + ' subframes',
            timestamp: new Date(configData.timestamp).toLocaleString()
        });
        
        Object.keys(configData.subframes).forEach(i => {
            const sf = configData.subframes[i];
            console.log(`Subframe ${i}: ${Object.keys(sf.config).length} campos, ${Object.keys(sf.plps).length} PLPs`);
        });
        
        updateBackupStatus(true);
        
    } catch (error) {
        console.error('Erro ao salvar backup:', error);
        updateBackupStatus(false);
        showBackupNotification('Erro ao salvar backup!', 'error');
    }
}

function loadConfigBackup() {
    try {
        console.log('Iniciando carregamento do backup...');
        
        const backupData = localStorage.getItem(BACKUP_CONFIG.localStorageKey);
        const timestamp = localStorage.getItem(BACKUP_CONFIG.timestampKey);
        
        if (!backupData || !timestamp) {
            console.log('Nenhum backup encontrado');
            return false;
        }

        const backupAge = Date.now() - parseInt(timestamp);
        if (backupAge > BACKUP_CONFIG.maxBackupAge) {
            console.log('Backup muito antigo, removendo...');
            clearConfigBackup();
            return false;
        }

        const configData = JSON.parse(backupData);
        console.log('Dados do backup encontrados:', {
            bootstrap: configData.bootstrap ? Object.keys(configData.bootstrap).length + ' campos' : 'vazio',
            preamble: configData.preamble ? Object.keys(configData.preamble).length + ' campos' : 'vazio',
            subframes: configData.subframes ? Object.keys(configData.subframes).length + ' subframes' : 'vazio'
        });
        
        if (configData.bootstrap && Object.keys(configData.bootstrap).length > 0) {
            console.log('Carregando bootstrap...');
            loadFormData('botstrap_form', configData.bootstrap);
        }

        if (configData.preamble && Object.keys(configData.preamble).length > 0) {
            console.log('Carregando preamble...');
            loadFormData('preamble_form', configData.preamble);
            
            const subframeCount = configData.preamble.number_of_subframes || 1;
            const subframeInput = document.getElementById('number_of_subframes');
            if (subframeInput) {
                subframeInput.value = subframeCount;
                console.log(`Configurando ${subframeCount} subframes...`);
                
                if (typeof generateMenus === 'function') {
                    generateMenus(subframeCount);
                } else if (typeof handleSubframeCountChange === 'function') {
                    handleSubframeCountChange();
                } else {
                    console.log('Função de geração de subframes não encontrada');
                }
            }
        }

        if (configData.subframes && Object.keys(configData.subframes).length > 0) {
            setTimeout(() => {
                console.log('Carregando subframes...');
                
                Object.keys(configData.subframes).forEach(subframeIndex => {
                    const subframeData = configData.subframes[subframeIndex];
                    console.log(`Carregando subframe ${subframeIndex}...`);
                    
                    if (subframeData.config && Object.keys(subframeData.config).length > 0) {
                        const subframeForm = document.querySelector(`#subframe${subframeIndex} form`);
                        if (subframeForm) {
                            loadFormData(subframeForm, subframeData.config);
                            console.log(`Subframe ${subframeIndex} carregado`);
                        } else {
                            console.log(`Formulário do subframe ${subframeIndex} não encontrado ainda`);
                        }
                    }

                    const plpCount = Object.keys(subframeData.plps || {}).length;
                    if (plpCount > 0) {
                        console.log(`Carregando ${plpCount} PLPs para subframe ${subframeIndex}...`);
                        
                        const plpCountInput = document.getElementById(`plp-count-${subframeIndex}`);
                        if (plpCountInput) {
                            plpCountInput.value = plpCount;
                            
                            if (typeof generatePLPMenusAndFields === 'function') {
                                generatePLPMenusAndFields(parseInt(subframeIndex));
                                
                                setTimeout(() => {
                                    Object.keys(subframeData.plps).forEach(plpIndex => {
                                        const plpData = subframeData.plps[plpIndex];
                                        console.log(`Carregando PLP ${plpIndex}...`);
                                        
                                        const plpForm = document.querySelector(`#plp-${subframeIndex}-${plpIndex} form`);
                                        if (plpForm) {
                                            loadFormData(plpForm, plpData);
                                            console.log(`PLP ${plpIndex} carregado`);
                                        } else {
                                            console.log(`Formulário do PLP ${plpIndex} não encontrado, carregando campos individuais...`);
                                            
                                            Object.keys(plpData).forEach(fieldName => {
                                                const fieldId = `${fieldName}_${plpIndex}`;
                                                const fieldElement = document.getElementById(fieldId);
                                                if (fieldElement) {
                                                    fieldElement.value = plpData[fieldName];
                                                    console.log(`${fieldId} = ${plpData[fieldName]}`);
                                                }
                                            });
                                        }
                                    });
                                }, 800);
                            }
                        }
                    }
                });
            }, 1200);
        }

        const backupDate = new Date(parseInt(timestamp));
        console.log('Backup carregado com sucesso de:', backupDate.toLocaleString());
        showBackupNotification(`Configuração carregada do backup local (${backupDate.toLocaleString()})`, 'info');
        serverStatus.usingBackup = true;
        updateServerStatusIndicator();
        
        return true;
        
    } catch (error) {
        console.error('Erro ao carregar backup:', error);
        showBackupNotification('Erro ao carregar backup!', 'error');
        return false;
    }
}

function clearConfigBackup() {
    localStorage.removeItem(BACKUP_CONFIG.localStorageKey);
    localStorage.removeItem(BACKUP_CONFIG.timestampKey);
    console.log(' Backup local removido');
    showBackupNotification('Backup local removido', 'info');
}

function checkServerStatus() {
    return fetch('?check_server=1', {
        method: 'GET',
        headers: { 'Content-Type': 'application/json' }
    })
    .then(response => response.json())
    .then(data => {
        const wasAvailable = serverStatus.available;
        serverStatus.available = data.server_available;
        serverStatus.lastCheck = Date.now();
        
        if (!wasAvailable && serverStatus.available) {
            console.log('Servidor reconectado!');
            showBackupNotification('Servidor reconectado! Dados sincronizados.', 'success');
            serverStatus.usingBackup = false;
            
        } else if (wasAvailable && !serverStatus.available) {
            console.log('Servidor desconectado!');
            showBackupNotification('Servidor desconectado. Usando backup local.', 'warning');
            serverStatus.usingBackup = true;
        }
        
        updateServerStatusIndicator();
        return serverStatus.available;
    })
    .catch(error => {
        console.log('Erro ao verificar servidor:', error);
        serverStatus.available = false;
        serverStatus.usingBackup = true;
        updateServerStatusIndicator();
        return false;
    });
}

function startServerMonitoring() {
    if (serverStatus.checkInterval) {
        clearInterval(serverStatus.checkInterval);
    }
    
    serverStatus.checkInterval = setInterval(() => {
        checkServerStatus();
    }, BACKUP_CONFIG.serverCheckInterval);
    
    console.log('Monitoramento do servidor iniciado');
}

function debugBackup() {
    const backupData = localStorage.getItem(BACKUP_CONFIG.localStorageKey);
    if (backupData) {
        const configData = JSON.parse(backupData);
        console.log(' DEBUG - Backup completo:');
        console.log('Bootstrap:', configData.bootstrap);
        console.log('Preamble:', configData.preamble);
        console.log('Subframes:', configData.subframes);
        console.log('Timestamp:', new Date(configData.timestamp).toLocaleString());
        
        Object.keys(configData.subframes || {}).forEach(i => {
            const sf = configData.subframes[i];
            console.log(`Subframe ${i}:`);
            console.log(`Config (${Object.keys(sf.config).length} campos):`, sf.config);
            console.log(`PLPs (${Object.keys(sf.plps).length}):`, sf.plps);
            
            Object.keys(sf.plps).forEach(j => {
                console.log(`PLP ${j} (${Object.keys(sf.plps[j]).length} campos):`, sf.plps[j]);
            });
        });
        
        return configData;
    } else {
        console.log('DEBUG - Nenhum backup encontrado');
        return null;
    }
}

function initializeBackupSystem() {
    
    removeDuplicateStatusIndicator();
    
    updateServerStatusIndicator();
    
    checkServerStatus().then(serverAvailable => {
        if (!serverAvailable) {
            console.log('Servidor indisponível - carregando backup');
            setTimeout(() => {
                loadConfigBackup();
            }, 1000);
        }
        
        startServerMonitoring();
    });
    
    setInterval(() => {
        const subframeInput = document.getElementById('number_of_subframes');
        if (subframeInput && subframeInput.value) {
            console.log('Backup automático executado');
            saveConfigBackup();
        }
    }, 5 * 60 * 1000);
    
    window.addEventListener('beforeunload', function() {
        const subframeInput = document.getElementById('number_of_subframes');
        if (subframeInput && subframeInput.value) {
            console.log('Salvando backup antes de fechar pÃ¡gina');
            saveConfigBackup();
        }
    });
    
}