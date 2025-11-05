(function() {
	function initForm(page, content) {
		content.addClass('form-reg')
		content.html(`
<div class="form-header">
            <h1>데이터베이스 필드 등록</h1>
            <div class="table-info">
                <label for="tableName">테이블 이름:</label>
                <input type="text" id="tableName" placeholder="테이블 이름을 입력하세요">
            </div>
        </div>

        <div class="form-container">
            <div class="field-form">
                <h2>필드 정보</h2>
                <div class="form-group">
                    <label for="fieldName">필드 이름:</label>
                    <input type="text" id="fieldName" placeholder="필드 이름을 입력하세요">
                </div>
                <div class="form-group">
                    <label for="fieldType">필드 타입:</label>
                    <select id="fieldType">
                        <option value="varchar">VARCHAR</option>
                        <option value="int">INT</option>
                        <option value="float">FLOAT</option>
                        <option value="double">DOUBLE</option>
                        <option value="date">DATE</option>
                        <option value="datetime">DATETIME</option>
                        <option value="text">TEXT</option>
                        <option value="boolean">BOOLEAN</option>
                        <option value="blob">BLOB</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="fieldLength">길이:</label>
                    <input type="number" id="fieldLength" placeholder="필드 길이를 입력하세요">
                </div>
                <div class="form-group">
                    <label for="fieldDefault">기본값:</label>
                    <input type="text" id="fieldDefault" placeholder="기본값을 입력하세요">
                </div>
                <div class="form-group checkbox-group">
                    <div class="checkbox-item">
                        <input type="checkbox" id="isPrimaryKey">
                        <label for="isPrimaryKey">기본키</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="isNotNull">
                        <label for="isNotNull">NOT NULL</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="isAutoIncrement">
                        <label for="isAutoIncrement">자동 증가</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="isUnique">
                        <label for="isUnique">UNIQUE</label>
                    </div>
                </div>
                <div class="form-group">
                    <label for="fieldDescription">설명:</label>
                    <textarea id="fieldDescription" placeholder="필드에 대한 설명을 입력하세요"></textarea>
                </div>
                <div class="button-group">
                    <button id="addFieldBtn" class="btn primary">필드 추가</button>
                    <button id="updateFieldBtn" class="btn secondary" disabled>필드 수정</button>
                    <button id="clearFormBtn" class="btn">폼 초기화</button>
                </div>
            </div>

            <div class="field-list">
                <h2>등록된 필드 목록</h2>
                <div class="table-container">
                    <table id="fieldTable">
                        <thead>
                            <tr>
                                <th>필드 이름</th>
                                <th>타입</th>
                                <th>길이</th>
                                <th>제약 조건</th>
                                <th>작업</th>
                            </tr>
                        </thead>
                        <tbody id="fieldTableBody">
                            <!-- 필드 목록이 여기에 동적으로 추가됩니다 -->
                        </tbody>
                    </table>
                </div>
                <div class="button-group">
                    <button id="saveTableBtn" class="btn primary">테이블 저장</button>
                    <button id="cancelBtn" class="btn">취소</button>
                </div>
            </div>
        </div>`)
		setFormEvent(page, content)
	}
	const pageImpl = {
		initPage: function() { initForm(this, this.contentEl) }
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent', {overflow:'auto'})
		, content: true
	}
	const pageInfo = {id:'form_db_field', layout}
	const app = cf.apps.currentApp
	app.createPage(pageInfo.id, pageInfo, pageImpl)
})()

function setFormEvent(page, contentEl) {
    // DOM 요소
	const content = contentEl[0]
	clog('>> setFormEvent content ', content, contentEl)
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
}
loadStyle(`
.form-header {
    background-color: #fff;
    padding: 20px;
    border-radius: 5px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
    margin-bottom: 20px;
}

.form-header h1 {
    margin-bottom: 15px;
    color: #2c3e50;
}

.table-info {
    display: flex;
    align-items: center;
    gap: 10px;
}

.table-info label {
    font-weight: bold;
    min-width: 100px;
}

.form-container {
    display: flex;
    gap: 20px;
}

.field-form, .field-list {
    background-color: #fff;
    padding: 20px;
    border-radius: 5px;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
}

.field-form {
    flex: 1;
}

.field-list {
    flex: 1;
}

h2 {
    margin-bottom: 20px;
    color: #2c3e50;
    border-bottom: 1px solid #eee;
    padding-bottom: 10px;
}

.form-group {
    margin-bottom: 15px;
}

label {
    display: block;
    margin-bottom: 5px;
    font-weight: 500;
}

input[type="text"],
input[type="number"],
select,
textarea {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 14px;
}

textarea {
    height: 100px;
    resize: vertical;
}

.checkbox-group {
    display: flex;
    flex-wrap: wrap;
    gap: 15px;
}

.checkbox-item {
    display: flex;
    align-items: center;
    gap: 5px;
}

.checkbox-item input[type="checkbox"] {
    width: 16px;
    height: 16px;
}

.button-group {
    display: flex;
    gap: 10px;
    margin-top: 20px;
}

.btn {
    padding: 10px 15px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-weight: 500;
    transition: background-color 0.3s;
}

.btn.primary {
    background-color: #3498db;
    color: white;
}

.btn.primary:hover {
    background-color: #2980b9;
}

.btn.secondary {
    background-color: #2ecc71;
    color: white;
}

.btn.secondary:hover {
    background-color: #27ae60;
}

.btn:not(.primary):not(.secondary) {
    background-color: #e0e0e0;
    color: #333;
}

.btn:not(.primary):not(.secondary):hover {
    background-color: #d0d0d0;
}

.btn:disabled {
    background-color: #ccc;
    cursor: not-allowed;
}

.table-container {
    overflow-x: auto;
    margin-bottom: 20px;
}

table {
    width: 100%;
    border-collapse: collapse;
}

th, td {
    padding: 12px 15px;
    text-align: left;
    border-bottom: 1px solid #ddd;
}

th {
    background-color: #f8f9fa;
    font-weight: 600;
}

tr:hover {
    background-color: #f5f5f5;
}

.action-btn {
    padding: 5px 10px;
    border: none;
    border-radius: 3px;
    cursor: pointer;
    font-size: 12px;
    margin-right: 5px;
}

.edit-btn {
    background-color: #f39c12;
    color: white;
}

.delete-btn {
    background-color: #e74c3c;
    color: white;
}

@media (max-width: 768px) {
    .form-container {
        flex-direction: column;
    }
}
`)