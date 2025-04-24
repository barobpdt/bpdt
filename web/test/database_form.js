document.addEventListener('DOMContentLoaded', function() {
    // DOM 요소
    const tableNameInput = document.getElementById('tableName');
    const fieldNameInput = document.getElementById('fieldName');
    const fieldTypeSelect = document.getElementById('fieldType');
    const fieldLengthInput = document.getElementById('fieldLength');
    const fieldDefaultInput = document.getElementById('fieldDefault');
    const isPrimaryKeyCheckbox = document.getElementById('isPrimaryKey');
    const isNotNullCheckbox = document.getElementById('isNotNull');
    const isAutoIncrementCheckbox = document.getElementById('isAutoIncrement');
    const isUniqueCheckbox = document.getElementById('isUnique');
    const fieldDescriptionTextarea = document.getElementById('fieldDescription');
    const addFieldBtn = document.getElementById('addFieldBtn');
    const updateFieldBtn = document.getElementById('updateFieldBtn');
    const clearFormBtn = document.getElementById('clearFormBtn');
    const saveTableBtn = document.getElementById('saveTableBtn');
    const cancelBtn = document.getElementById('cancelBtn');
    const fieldTableBody = document.getElementById('fieldTableBody');

    // 필드 목록 저장
    let fields = [];
    let editingIndex = -1;

    // 필드 타입에 따라 길이 입력 필드 활성화/비활성화
    fieldTypeSelect.addEventListener('change', function() {
        const selectedType = this.value;
        const lengthTypes = ['varchar', 'int', 'float', 'double'];
        
        if (lengthTypes.includes(selectedType)) {
            fieldLengthInput.disabled = false;
            fieldLengthInput.required = true;
        } else {
            fieldLengthInput.disabled = true;
            fieldLengthInput.required = false;
            fieldLengthInput.value = '';
        }
    });

    // 필드 추가 버튼 클릭 이벤트
    addFieldBtn.addEventListener('click', function() {
        if (!validateForm()) {
            return;
        }

        const field = {
            name: fieldNameInput.value,
            type: fieldTypeSelect.value,
            length: fieldLengthInput.value,
            defaultValue: fieldDefaultInput.value,
            isPrimaryKey: isPrimaryKeyCheckbox.checked,
            isNotNull: isNotNullCheckbox.checked,
            isAutoIncrement: isAutoIncrementCheckbox.checked,
            isUnique: isUniqueCheckbox.checked,
            description: fieldDescriptionTextarea.value
        };

        fields.push(field);
        updateFieldTable();
        clearForm();
    });

    // 필드 수정 버튼 클릭 이벤트
    updateFieldBtn.addEventListener('click', function() {
        if (!validateForm()) {
            return;
        }

        const field = {
            name: fieldNameInput.value,
            type: fieldTypeSelect.value,
            length: fieldLengthInput.value,
            defaultValue: fieldDefaultInput.value,
            isPrimaryKey: isPrimaryKeyCheckbox.checked,
            isNotNull: isNotNullCheckbox.checked,
            isAutoIncrement: isAutoIncrementCheckbox.checked,
            isUnique: isUniqueCheckbox.checked,
            description: fieldDescriptionTextarea.value
        };

        fields[editingIndex] = field;
        updateFieldTable();
        clearForm();
        editingIndex = -1;
        updateFieldBtn.disabled = true;
        addFieldBtn.disabled = false;
    });

    // 폼 초기화 버튼 클릭 이벤트
    clearFormBtn.addEventListener('click', clearForm);

    // 테이블 저장 버튼 클릭 이벤트
    saveTableBtn.addEventListener('click', function() {
        const tableName = tableNameInput.value.trim();
        
        if (!tableName) {
            alert('테이블 이름을 입력하세요.');
            return;
        }
        
        if (fields.length === 0) {
            alert('최소 하나 이상의 필드를 추가하세요.');
            return;
        }

        // 여기에 테이블 저장 로직 구현
        // 예: API 호출 또는 로컬 스토리지에 저장
        console.log('테이블 저장:', { tableName, fields });
        alert('테이블이 저장되었습니다.');
    });

    // 취소 버튼 클릭 이벤트
    cancelBtn.addEventListener('click', function() {
        if (confirm('작업을 취소하시겠습니까? 모든 데이터가 삭제됩니다.')) {
            window.location.href = 'index.html'; // 메인 페이지로 이동
        }
    });

    // 필드 테이블 업데이트
    function updateFieldTable() {
        fieldTableBody.innerHTML = '';
        
        fields.forEach((field, index) => {
            const row = document.createElement('tr');
            
            // 필드 이름
            const nameCell = document.createElement('td');
            nameCell.textContent = field.name;
            row.appendChild(nameCell);
            
            // 필드 타입
            const typeCell = document.createElement('td');
            typeCell.textContent = field.type.toUpperCase();
            if (field.length) {
                typeCell.textContent += `(${field.length})`;
            }
            row.appendChild(typeCell);
            
            // 필드 길이
            const lengthCell = document.createElement('td');
            lengthCell.textContent = field.length || '-';
            row.appendChild(lengthCell);
            
            // 제약 조건
            const constraintsCell = document.createElement('td');
            const constraints = [];
            
            if (field.isPrimaryKey) constraints.push('PK');
            if (field.isNotNull) constraints.push('NOT NULL');
            if (field.isAutoIncrement) constraints.push('AUTO_INCREMENT');
            if (field.isUnique) constraints.push('UNIQUE');
            
            constraintsCell.textContent = constraints.join(', ') || '-';
            row.appendChild(constraintsCell);
            
            // 작업 버튼
            const actionsCell = document.createElement('td');
            
            const editBtn = document.createElement('button');
            editBtn.textContent = '수정';
            editBtn.className = 'action-btn edit-btn';
            editBtn.addEventListener('click', () => editField(index));
            
            const deleteBtn = document.createElement('button');
            deleteBtn.textContent = '삭제';
            deleteBtn.className = 'action-btn delete-btn';
            deleteBtn.addEventListener('click', () => deleteField(index));
            
            actionsCell.appendChild(editBtn);
            actionsCell.appendChild(deleteBtn);
            row.appendChild(actionsCell);
            
            fieldTableBody.appendChild(row);
        });
    }

    // 필드 수정
    function editField(index) {
        const field = fields[index];
        
        fieldNameInput.value = field.name;
        fieldTypeSelect.value = field.type;
        fieldLengthInput.value = field.length || '';
        fieldDefaultInput.value = field.defaultValue || '';
        isPrimaryKeyCheckbox.checked = field.isPrimaryKey;
        isNotNullCheckbox.checked = field.isNotNull;
        isAutoIncrementCheckbox.checked = field.isAutoIncrement;
        isUniqueCheckbox.checked = field.isUnique;
        fieldDescriptionTextarea.value = field.description || '';
        
        // 필드 타입에 따라 길이 입력 필드 활성화/비활성화
        const lengthTypes = ['varchar', 'int', 'float', 'double'];
        if (lengthTypes.includes(field.type)) {
            fieldLengthInput.disabled = false;
            fieldLengthInput.required = true;
        } else {
            fieldLengthInput.disabled = true;
            fieldLengthInput.required = false;
        }
        
        editingIndex = index;
        updateFieldBtn.disabled = false;
        addFieldBtn.disabled = true;
    }

    // 필드 삭제
    function deleteField(index) {
        if (confirm('이 필드를 삭제하시겠습니까?')) {
            fields.splice(index, 1);
            updateFieldTable();
            
            if (editingIndex === index) {
                clearForm();
                editingIndex = -1;
                updateFieldBtn.disabled = true;
                addFieldBtn.disabled = false;
            } else if (editingIndex > index) {
                editingIndex--;
            }
        }
    }

    // 폼 초기화
    function clearForm() {
        fieldNameInput.value = '';
        fieldTypeSelect.value = 'varchar';
        fieldLengthInput.value = '';
        fieldDefaultInput.value = '';
        isPrimaryKeyCheckbox.checked = false;
        isNotNullCheckbox.checked = false;
        isAutoIncrementCheckbox.checked = false;
        isUniqueCheckbox.checked = false;
        fieldDescriptionTextarea.value = '';
        
        // 필드 타입에 따라 길이 입력 필드 활성화/비활성화
        fieldLengthInput.disabled = false;
        fieldLengthInput.required = true;
    }

    // 폼 유효성 검사
    function validateForm() {
        const fieldName = fieldNameInput.value.trim();
        
        if (!fieldName) {
            alert('필드 이름을 입력하세요.');
            fieldNameInput.focus();
            return false;
        }
        
        // 필드 이름 중복 검사
        const isDuplicate = fields.some((field, index) => 
            field.name === fieldName && index !== editingIndex
        );
        
        if (isDuplicate) {
            alert('이미 존재하는 필드 이름입니다.');
            fieldNameInput.focus();
            return false;
        }
        
        // 필드 타입에 따른 길이 검사
        const selectedType = fieldTypeSelect.value;
        const lengthTypes = ['varchar', 'int', 'float', 'double'];
        
        if (lengthTypes.includes(selectedType) && !fieldLengthInput.value) {
            alert('필드 길이를 입력하세요.');
            fieldLengthInput.focus();
            return false;
        }
        
        return true;
    }

    // 초기 설정
    clearForm();
}); 