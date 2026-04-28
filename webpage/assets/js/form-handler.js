class FormHandler {
    saveFormData(formElement) {
        const formData = {};
        if (!formElement) return formData;

        const inputs = formElement.querySelectorAll('input, select, textarea');
        inputs.forEach(input => {
            if (input.name || input.id) {
                const key = input.name || input.id;
                if (input.type === 'checkbox') {
                    formData[key] = input.checked;
                } else {
                    formData[key] = input.value;
                }
            }
        });
        return formData;
    }

    restoreFormData(formElement, formData) {
        if (!formElement || !formData) return;

        const inputs = formElement.querySelectorAll('input, select, textarea');
        inputs.forEach(input => {
            const key = input.name || input.id;
            if (key && key.startsWith('plp_id_')) return;

            if (formData.hasOwnProperty(key)) {
                if (input.type === 'checkbox') {
                    input.checked = formData[key];
                } else {
                    input.value = formData[key];
                }
            }
        });
    }

    saveAllSubframesData() {
        const savedData = { subframes: {}, plps: {} };

        document.querySelectorAll('[id^="subframe"]').forEach(subframe => {
            const match = subframe.id.match(/subframe(\d+)/);
            if (match) {
                const subframeIndex = match[1];
                const form = subframe.querySelector('form');
                if (form) {
                    savedData.subframes[subframeIndex] = this.saveFormData(form);
                }
            }
        });

        document.querySelectorAll('[id^="plp-"]').forEach(plp => {
            const match = plp.id.match(/plp-(\d+)-(\d+)/);
            if (match) {
                const subframeIndex = match[1];
                const plpIndex = match[2];
                const form = plp.querySelector('form');
                if (form) {
                    if (!savedData.plps[subframeIndex]) {
                        savedData.plps[subframeIndex] = {};
                    }
                    savedData.plps[subframeIndex][plpIndex] = this.saveFormData(form);
                }
            }
        });

        return savedData;
    }

    restoreAllData(savedData) {
        if (!savedData) return;

        Object.keys(savedData.subframes || {}).forEach(subframeIndex => {
            const subframe = document.getElementById(`subframe${subframeIndex}`);
            if (subframe) {
                const form = subframe.querySelector('form');
                if (form) {
                    this.restoreFormData(form, savedData.subframes[subframeIndex]);
                }
            }
        });

        Object.keys(savedData.plps || {}).forEach(subframeIndex => {
            Object.keys(savedData.plps[subframeIndex] || {}).forEach(plpIndex => {
                const plp = document.getElementById(`plp-${subframeIndex}-${plpIndex}`);
                if (plp) {
                    const form = plp.querySelector('form');
                    if (form) {
                        this.restoreFormData(form, savedData.plps[subframeIndex][plpIndex]);
                    }
                }
            });
        });
    }

    handleSubframeCountChange() {
        const input = document.getElementById('number_of_subframes');
        const count = parseInt(input.value) || 1;

        const savedData = this.saveAllSubframesData();
        generateMenus(count);

        for (let i = 0; i < count; i++) {
            setTimeout(() => {
                const plpCountInput = document.getElementById(`plp-count-${i}`);
                if (plpCountInput && plpCountInput.value === "1") {
                    generatePLPMenusAndFields(i);
                }
            }, 100);
        }

        setTimeout(() => {
            this.restoreAllData(savedData);

            if (window.atscValidation && window.atscValidation.currentPreambleConfig) {
                window.atscValidation.applyPreambleConfigToAllSubframes();
                window.atscValidation.filterAllSubframePilotPatterns();
            }
        }, 250);
    }

    validateAllFields() {
        const emptyFields = [];

        function checkForm(form, formName) {
            if (!form) return;

            const inputs = form.querySelectorAll('input[required], select[required], textarea[required]');
            inputs.forEach(input => {
                if (!input.value || input.value.trim() === '') {
                    const label = form.querySelector(`label[for="${input.id}"]`);
                    const fieldName = label ? label.textContent.replace(':', '') : input.name || input.id;
                    emptyFields.push(`${formName}: ${fieldName}`);
                }
            });
        }

        const bootstrapForm = document.getElementById('botstrap_form');
        checkForm(bootstrapForm, 'Bootstrap');

        const preambleForm = document.getElementById('preamble_form');
        checkForm(preambleForm, 'Preamble');

        const subframeCount = parseInt(document.getElementById('number_of_subframes').value) || 0;

        for (let i = 0; i < subframeCount; i++) {
            const subframeForm = document.querySelector(`#subframe${i} form`);
            checkForm(subframeForm, `Subframe ${i}`);

            const plpCountInput = document.getElementById(`plp-count-${i}`);
            const plpCount = plpCountInput ? parseInt(plpCountInput.value) || 0 : 0;

            for (let j = 0; j < plpCount; j++) {
                const plpForm = document.querySelector(`#plp-${i}-${j} form`);
                checkForm(plpForm, `Subframe ${i} - PLP ${j}`);
            }
        }

        return emptyFields;
    }
}

window.formHandler = new FormHandler();
window.saveFormData = (...args) => window.formHandler.saveFormData(...args);
window.restoreFormData = (...args) => window.formHandler.restoreFormData(...args);
window.saveAllSubframesData = (...args) => window.formHandler.saveAllSubframesData(...args);
window.restoreAllData = (...args) => window.formHandler.restoreAllData(...args);
window.handleSubframeCountChange = (...args) => window.formHandler.handleSubframeCountChange(...args);
window.validateAllFields = (...args) => window.formHandler.validateAllFields(...args);
