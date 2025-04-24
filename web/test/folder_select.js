document.addEventListener('DOMContentLoaded', () => {
    const driveSelect = document.getElementById('driveSelect');
    const filterInput = document.getElementById('filterInput');
    const folderTree = document.getElementById('folderTree');
    const selectedPathInput = document.getElementById('selectedPath');
    const selectButton = document.getElementById('selectButton');
    const cancelButton = document.getElementById('cancelButton');

    let currentPath = '';
    let selectedFolder = null;
    let expandedFolders = new Set();

    // 드라이브 목록 초기화
    async function initializeDrives() {
        try {
            const response = await fetch('/api/drives');
            const drives = await response.json();
            
            drives.forEach(drive => {
                const option = document.createElement('option');
                option.value = drive.path;
                option.textContent = `${drive.name} (${drive.path})`;
                driveSelect.appendChild(option);
            });

            // 첫 번째 드라이브 선택 시 폴더 트리 로드
            if (drives.length > 0) {
                loadFolderTree(drives[0].path);
            }
        } catch (error) {
            console.error('드라이브 목록 로드 실패:', error);
        }
    }

    // 폴더 트리 로드
    async function loadFolderTree(path, filter = '') {
        try {
            const response = await fetch(`/api/folders?path=${encodeURIComponent(path)}&filter=${encodeURIComponent(filter)}`);
            const folders = await response.json();
            
            folderTree.innerHTML = '';
            currentPath = path;

            // 루트 폴더 생성
            const rootFolder = createFolderItem({
                name: path.split('\\').pop() || path,
                fullPath: path,
                hasChildren: folders.length > 0
            }, 0);
            folderTree.appendChild(rootFolder);

            // 하위 폴더들 생성
            folders.forEach(folder => {
                const folderItem = createFolderItem(folder, 1);
                folderTree.appendChild(folderItem);
            });
        } catch (error) {
            console.error('폴더 트리 로드 실패:', error);
        }
    }

    // 폴더 아이템 생성
    function createFolderItem(folder, level) {
        const div = document.createElement('div');
        div.className = 'folder-item';
        div.style.paddingLeft = `${level * 20}px`;

        const hasChildren = folder.hasChildren;
        const isExpanded = expandedFolders.has(folder.fullPath);

        div.innerHTML = `
            <span class="folder-icon">${hasChildren ? (isExpanded ? '📂' : '📁') : '📁'}</span>
            <span class="folder-name">${folder.name}</span>
            ${hasChildren ? `<span class="expand-icon">${isExpanded ? '▼' : '▶'}</span>` : ''}
        `;

        div.addEventListener('click', (e) => {
            if (e.target.classList.contains('expand-icon')) {
                toggleFolder(folder, div);
            } else {
                selectFolder(folder, div);
            }
        });

        return div;
    }

    // 폴더 토글 (확장/축소)
    async function toggleFolder(folder, element) {
        const isExpanded = expandedFolders.has(folder.fullPath);
        const expandIcon = element.querySelector('.expand-icon');
        const folderIcon = element.querySelector('.folder-icon');

        if (isExpanded) {
            // 폴더 축소
            expandedFolders.delete(folder.fullPath);
            expandIcon.textContent = '▶';
            folderIcon.textContent = '📁';
            
            // 하위 폴더들 제거
            const nextSibling = element.nextElementSibling;
            while (nextSibling && nextSibling.style.paddingLeft > element.style.paddingLeft) {
                nextSibling.remove();
            }
        } else {
            // 폴더 확장
            expandedFolders.add(folder.fullPath);
            expandIcon.textContent = '▼';
            folderIcon.textContent = '📂';

            try {
                const response = await fetch(`/api/folders?path=${encodeURIComponent(folder.fullPath)}`);
                const subFolders = await response.json();
                
                const currentLevel = parseInt(element.style.paddingLeft) / 20;
                subFolders.forEach(subFolder => {
                    const subFolderItem = createFolderItem(subFolder, currentLevel + 1);
                    element.insertAdjacentElement('afterend', subFolderItem);
                });
            } catch (error) {
                console.error('하위 폴더 로드 실패:', error);
            }
        }
    }

    // 폴더 선택
    function selectFolder(folder, element) {
        // 이전 선택 제거
        const previousSelected = folderTree.querySelector('.selected');
        if (previousSelected) {
            previousSelected.classList.remove('selected');
        }

        // 새로운 선택 표시
        element.classList.add('selected');
        selectedFolder = folder;
        selectedPathInput.value = folder.fullPath;
    }

    // 이벤트 리스너 설정
    driveSelect.addEventListener('change', (e) => {
        loadFolderTree(e.target.value, filterInput.value);
    });

    filterInput.addEventListener('input', (e) => {
        loadFolderTree(currentPath, e.target.value);
    });

    selectButton.addEventListener('click', () => {
        if (selectedFolder) {
            // 선택된 폴더 정보를 부모 창으로 전달
            window.opener.postMessage({
                type: 'folderSelected',
                folder: selectedFolder
            }, '*');
            window.close();
        }
    });

    cancelButton.addEventListener('click', () => {
        window.close();
    });

    // 초기화
    initializeDrives();
}); 