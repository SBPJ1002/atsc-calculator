class BackupSystem {

    static BACKUP_CONFIG = {
        localStorageKey: 'atsc_config_backup',
        subframesKey: 'atsc_subframes_backup',
        timestampKey: 'atsc_backup_timestamp',
        serverCheckInterval: 5*60*1000,
        maxBackupAge: 7 * 24 * 60 * 60 * 1000
    };

    constructor() {
        this.serverStatus = {
            available: typeof serverAvailable !== 'undefined' ? serverAvailable : true,
            lastCheck: Date.now(),
            checkInterval: null,
            usingBackup: false
        };
    }

    showBackupNotification(message, type = 'info') {
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

    updateServerStatusIndicator() {
        let indicator = document.getElementById('server-status');

        if (!indicator) {
            return;
        }

        if (this.serverStatus.available) {
            indicator.textContent = 'Servidor Online';
            indicator.classList.add('online');
            indicator.classList.remove('offline');
        } else {
            indicator.textContent = 'Modo Offline (Backup)';
            indicator.classList.remove('online');
            indicator.classList.add('offline');
        }
    }

    removeDuplicateStatusIndicator() {
        const duplicateIndicator = document.getElementById('server-status-indicator');
        if (duplicateIndicator) {
            duplicateIndicator.remove();
        }
    }

    updateBackupStatus(success) {
        const timestamp = new Date().toLocaleTimeString();
        const indicator = document.getElementById('server-status-indicator');

        if (indicator && success) {
            indicator.textContent = `Backup salvo ${timestamp}`;
            indicator.style.background = '#2196F3';

            setTimeout(() => {
                this.updateServerStatusIndicator();
            }, 3000);
        }
    }

    getFormDataAsObject(formElement) {
        if (typeof formElement === 'string') {
            formElement = document.getElementById(formElement);
        }

        if (!formElement) {
            return {};
        }

        const formData = {};
        const inputs = formElement.querySelectorAll('input, select, textarea');

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
                }
            }
        });

        return formData;
    }

    loadFormData(formElement, data) {
        if (typeof formElement === 'string') {
            formElement = document.getElementById(formElement);
        }

        if (!formElement || !data) {
            return;
        }

        const inputs = formElement.querySelectorAll('input, select, textarea');

        inputs.forEach(input => {
            const key = input.name || input.id;
            if (data.hasOwnProperty(key)) {
                if (input.type === 'checkbox') {
                    input.checked = data[key];
                } else {
                    input.value = data[key];
                }
            }
        });
    }

    saveConfigBackup() {
        try {
            const configData = {
                bootstrap: {},
                preamble: {},
                subframes: {},
                timestamp: Date.now()
            };

            const bootstrapForm = document.getElementById('botstrap_form');
            if (bootstrapForm) {
                configData.bootstrap = this.getFormDataAsObject(bootstrapForm);
            }

            const preambleForm = document.getElementById('preamble_form');
            if (preambleForm) {
                configData.preamble = this.getFormDataAsObject(preambleForm);
            }

            const subframeCountInput = document.getElementById('number_of_subframes');
            const subframeCount = subframeCountInput ? parseInt(subframeCountInput.value) || 0 : 0;

            for (let i = 0; i < subframeCount; i++) {
                const subframeSection = document.getElementById(`subframe${i}`);
                if (subframeSection) {
                    const subframeForm = subframeSection.querySelector('form');

                    if (subframeForm) {
                        const subframeConfig = this.getFormDataAsObject(subframeForm);

                        configData.subframes[i] = {
                            config: subframeConfig,
                            plps: {}
                        };

                        const plpCountInput = document.getElementById(`plp-count-${i}`);
                        const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 0 : 0;

                        for (let j = 0; j < plpCount; j++) {
                            const plpSection = document.getElementById(`plp-${i}-${j}`);
                            if (plpSection) {
                                const plpForm = plpSection.querySelector('form');

                                if (plpForm) {
                                    configData.subframes[i].plps[j] = this.getFormDataAsObject(plpForm);
                                } else {
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
                                    }
                                }
                            }
                        }
                    }
                }
            }

            localStorage.setItem(BackupSystem.BACKUP_CONFIG.localStorageKey, JSON.stringify(configData));
            localStorage.setItem(BackupSystem.BACKUP_CONFIG.timestampKey, Date.now().toString());

            this.updateBackupStatus(true);

        } catch (error) {
            this.updateBackupStatus(false);
            this.showBackupNotification('Erro ao salvar backup!', 'error');
        }
    }

    loadConfigBackup() {
        try {
            const backupData = localStorage.getItem(BackupSystem.BACKUP_CONFIG.localStorageKey);
            const timestamp = localStorage.getItem(BackupSystem.BACKUP_CONFIG.timestampKey);

            if (!backupData || !timestamp) {
                return false;
            }

            const backupAge = Date.now() - parseInt(timestamp);
            if (backupAge > BackupSystem.BACKUP_CONFIG.maxBackupAge) {
                this.clearConfigBackup();
                return false;
            }

            const configData = JSON.parse(backupData);
            if (configData.bootstrap && Object.keys(configData.bootstrap).length > 0) {
                this.loadFormData('botstrap_form', configData.bootstrap);
            }

            if (configData.preamble && Object.keys(configData.preamble).length > 0) {
                this.loadFormData('preamble_form', configData.preamble);

                const subframeCount = configData.preamble.number_of_subframes || 1;
                const subframeInput = document.getElementById('number_of_subframes');
                if (subframeInput) {
                    subframeInput.value = subframeCount;

                    if (typeof generateMenus === 'function') {
                        generateMenus(subframeCount);
                    } else if (typeof handleSubframeCountChange === 'function') {
                        handleSubframeCountChange();
                    }
                }
            }

            if (configData.subframes && Object.keys(configData.subframes).length > 0) {
                setTimeout(() => {
                    Object.keys(configData.subframes).forEach(subframeIndex => {
                        const subframeData = configData.subframes[subframeIndex];

                        if (subframeData.config && Object.keys(subframeData.config).length > 0) {
                            const subframeForm = document.querySelector(`#subframe${subframeIndex} form`);
                            if (subframeForm) {
                                this.loadFormData(subframeForm, subframeData.config);
                            }
                        }

                        const plpCount = Object.keys(subframeData.plps || {}).length;
                        if (plpCount > 0) {
                            const plpCountInput = document.getElementById(`plp-count-${subframeIndex}`);
                            if (plpCountInput) {
                                plpCountInput.value = plpCount;

                                if (typeof generatePLPMenusAndFields === 'function') {
                                    generatePLPMenusAndFields(parseInt(subframeIndex));

                                    setTimeout(() => {
                                        Object.keys(subframeData.plps).forEach(plpIndex => {
                                            const plpData = subframeData.plps[plpIndex];

                                            const plpForm = document.querySelector(`#plp-${subframeIndex}-${plpIndex} form`);
                                            if (plpForm) {
                                                this.loadFormData(plpForm, plpData);
                                            } else {
                                                Object.keys(plpData).forEach(fieldName => {
                                                    const fieldId = `${fieldName}_${plpIndex}`;
                                                    const fieldElement = document.getElementById(fieldId);
                                                    if (fieldElement) {
                                                        fieldElement.value = plpData[fieldName];
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
            this.showBackupNotification(`Configuração carregada do backup local (${backupDate.toLocaleString()})`, 'info');
            this.serverStatus.usingBackup = true;
            this.updateServerStatusIndicator();

            return true;

        } catch (error) {
            this.showBackupNotification('Erro ao carregar backup!', 'error');
            return false;
        }
    }

    clearConfigBackup() {
        localStorage.removeItem(BackupSystem.BACKUP_CONFIG.localStorageKey);
        localStorage.removeItem(BackupSystem.BACKUP_CONFIG.timestampKey);
        this.showBackupNotification('Backup local removido', 'info');
    }

    checkServerStatus() {
        return fetch('?check_server=1', {
            method: 'GET',
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => response.json())
        .then(data => {
            const wasAvailable = this.serverStatus.available;
            this.serverStatus.available = data.server_available;
            this.serverStatus.lastCheck = Date.now();

            if (!wasAvailable && this.serverStatus.available) {
                this.showBackupNotification('Servidor reconectado! Dados sincronizados.', 'success');
                this.serverStatus.usingBackup = false;
            } else if (wasAvailable && !this.serverStatus.available) {
                this.showBackupNotification('Servidor desconectado. Usando backup local.', 'warning');
                this.serverStatus.usingBackup = true;
            }

            this.updateServerStatusIndicator();
            return this.serverStatus.available;
        })
        .catch(error => {
            this.serverStatus.available = false;
            this.serverStatus.usingBackup = true;
            this.updateServerStatusIndicator();
            return false;
        });
    }

    startServerMonitoring() {
        if (this.serverStatus.checkInterval) {
            clearInterval(this.serverStatus.checkInterval);
        }

        this.serverStatus.checkInterval = setInterval(() => {
            this.checkServerStatus();
        }, BackupSystem.BACKUP_CONFIG.serverCheckInterval);
    }

    debugBackup() {
        const backupData = localStorage.getItem(BackupSystem.BACKUP_CONFIG.localStorageKey);
        if (backupData) {
            const configData = JSON.parse(backupData);
            return configData;
        } else {
            return null;
        }
    }

    initializeBackupSystem() {
        this.removeDuplicateStatusIndicator();
        this.updateServerStatusIndicator();

        this.checkServerStatus().then(serverAvailable => {
            if (!serverAvailable) {
                setTimeout(() => {
                    this.loadConfigBackup();
                }, 1000);
            }

            this.startServerMonitoring();
        });

        setInterval(() => {
            const subframeInput = document.getElementById('number_of_subframes');
            if (subframeInput && subframeInput.value) {
                this.saveConfigBackup();
            }
        }, 5 * 60 * 1000);

        window.addEventListener('beforeunload', () => {
            const subframeInput = document.getElementById('number_of_subframes');
            if (subframeInput && subframeInput.value) {
                this.saveConfigBackup();
            }
        });
    }
}

window.backupSystem = new BackupSystem();
window.serverStatus = window.backupSystem.serverStatus;
window.showBackupNotification = (...args) => window.backupSystem.showBackupNotification(...args);
window.updateServerStatusIndicator = (...args) => window.backupSystem.updateServerStatusIndicator(...args);
window.removeDuplicateStatusIndicator = (...args) => window.backupSystem.removeDuplicateStatusIndicator(...args);
window.updateBackupStatus = (...args) => window.backupSystem.updateBackupStatus(...args);
window.getFormDataAsObject = (...args) => window.backupSystem.getFormDataAsObject(...args);
window.loadFormData = (...args) => window.backupSystem.loadFormData(...args);
window.saveConfigBackup = (...args) => window.backupSystem.saveConfigBackup(...args);
window.loadConfigBackup = (...args) => window.backupSystem.loadConfigBackup(...args);
window.clearConfigBackup = (...args) => window.backupSystem.clearConfigBackup(...args);
window.checkServerStatus = (...args) => window.backupSystem.checkServerStatus(...args);
window.startServerMonitoring = (...args) => window.backupSystem.startServerMonitoring(...args);
window.debugBackup = (...args) => window.backupSystem.debugBackup(...args);
window.initializeBackupSystem = (...args) => window.backupSystem.initializeBackupSystem(...args);
