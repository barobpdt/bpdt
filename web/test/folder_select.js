document.addEventListener('DOMContentLoaded', () => {
    // 초기화
	const treeModule = initTree('#folderTree')
    initData(treeModule)
}); 

function initData(treeModule) {
	const driveSelect = document.getElementById('driveSelect');
    const filterInput = document.getElementById('filterInput');
    const selectedPathInput = document.getElementById('selectedPath');
    const selectButton = document.getElementById('selectButton');
    const cancelButton = document.getElementById('cancelButton');

    let currentPath = '';
    let selectedFolder = null;
    let expandedFolders = new Set();

    // 드라이브 목록 초기화
    async function initDrives() {
        try {
            const response = await fetch('/api/drives');
            const node = await response.json();
            
            node.children.forEach(drive => {
                const option = document.createElement('option');
                option.value = drive.path;
                option.textContent = drive.name+' 드라이브';
                driveSelect.appendChild(option);
            });
            // 첫 번째 드라이브 선택 시 폴더 트리 로드 
			$(driveSelect).on('change', e=>loadTree())
			loadTree();
        } catch (error) {
            console.error('드라이브 목록 로드 실패:', error);
        }
    }

    // 폴더 트리 로드
    async function loadTree() {
		const path = $(driveSelect).val()
		if( !path ) {
			console.log('load tree 오류 폴더경로 미정의')
			return;
		}
        try {
            const response = await fetch('/api/folders?path='+encodeURIComponent(path))
            const node = await response.json();
			treeModule.initTree(node.children)
             
        } catch (error) {
            console.error('폴더 트리 로드 실패:', error);
        }
    }

}

function initTree(treeId) {
	const tree = $(treeId)
	const btnAdd = $('#addNode')
	const btnDelete = $('#deleteNode')
	
	function treeInit(data) {
		if(tree.jstree ) tree.jstree('destroy')
		tree.jstree({
			'core': {
				'data': data||[],
				'check_callback': true,
				'themes': {
					'name': 'proton',
					'responsive': false,
					'variant': 'small',
					'stripes': false
				}
			},
			'plugins': ['dnd','checkbox'],
		})
		console.log('tree create ', tree)
	}
	
	function treeEvent(tree) {
		tree.on('select_node.jstree', function (e, data) {
			console.log('노드가 선택되었습니다:', data.node.text);
		});			
		// 드래그 앤 드롭 이벤트
		tree.on('move_node.jstree', function(e, data) {
			console.log('Node moved:', data);
			// 여기에 노드 이동 후 추가 로직을 구현할 수 있습니다
			const parent = tree.get_node(data.parent)
			tree.open_node(parent)
		});
		
		// 이벤트 open_node
		tree.on('open_node.jstree', (e,d)=>{
			if( !d.node.checkIcons) {
				d.node.children.forEach(k=>treeIcon(k))
				d.node.checkIcons = true
			}
		})
		

		// 컨텍스트 메뉴 이벤트
		tree.on('contextmenu', '.jstree-anchor', function(e) {
			e.preventDefault();
			const node = $(this);
			const nodeId = node.attr('id');
			const isLeaf = node.parent().hasClass('jstree-leaf');
			
			// 컨텍스트 메뉴 생성
			const menu = $('<div class="context-menu"></div>');
			menu.append('<div class="context-menu-item" data-action="add-child">자식 추가</div>');
			menu.append('<div class="context-menu-item" data-action="add-sibling">형제 추가</div>');
			
			// 삭제 메뉴 항목 추가 (자식이 없는 노드만 활성화)
			const deleteItem = $('<div class="context-menu-item" data-action="delete">삭제</div>');
			if (!isLeaf) {
				deleteItem.addClass('disabled');
			}
			menu.append(deleteItem);
			
			// 메뉴 위치 설정
			menu.css({
				top: e.pageY,
				left: e.pageX
			});
			
			// 메뉴 표시
			$('body').append(menu);
			
			// 메뉴 항목 클릭 이벤트
			menu.on('click', '.context-menu-item', function() {
				if ($(this).hasClass('disabled')) return;
				
				const action = $(this).data('action');
				const newNodeId = 'new_' + Date.now();
				
				switch(action) {
					case 'add-child':
						tree.jstree('create_node', nodeId, 'last', {
							'id': newNodeId,
							'text': '새 폴더'
						});
						break;
					case 'add-sibling':
						tree.jstree('create_node', tree.jstree('get_parent', nodeId), 'last', {
							'id': newNodeId,
							'text': '새 폴더'
						});
						break;
					case 'delete':
						if (isLeaf) {
							tree.jstree('delete_node', nodeId);
						}
						break;
				}
				
				// 메뉴 제거
				menu.remove();
			});
			
			// 다른 곳 클릭시 메뉴 제거
			$(document).one('click', function() {
				menu.remove();
			});
		});

		// 버튼 이벤트
		btnAdd.on('click', function() {
			const a = tree.get_selected()
			if ( a.length > 0) { 
				tree.create_node(a[0]
					, {type:'file', text:'새파일'}, 'last'
					, cur => $.jstree.reference(cur).edit(cur) 
				)
			}
		});

		btnDelete.on('click', function() {
			const a = tree.get_selected()
			if ( a.length > 0) {
				const node = tree.get_node(a[0])
				if ( node.children.length === 0 && !confirm()) return;
				tree.delete_node(a[0])
			}
		});

		// 버튼 상태 업데이트
		tree.on('select_node.jstree', function() {
			updateButtonStates();
		});

		tree.on('deselect_node.jstree', function() {
			updateButtonStates();
		});
		
	}
	
	function treeIcon(tid, cur) {
		if(!cur) cur = tree.get_node(tid)
		const icon = qs('#'+tid+' .jstree-themeicon')
		console.log('icon=>',tid, cur, icon )
	}
	

	function updateButtonStates() {
		const a = tree ? tree.get_selected():[]
		const hasSelection = a.length > 0;
		const isLeaf = hasSelection && tree.get_node(a[0]).children.length === 0;
		
		btnAdd.prop('disabled', !hasSelection);
		btnDelete.prop('disabled', !hasSelection || !isLeaf);
	}
	
	// 특정 노드의 자식들만 새로고침
	function treeLoad(nodeId, newData) {
		const parent = tree.get_node(nodeId)
		tree.delete_node( parent.children )
		newData.forEach( cur => tree.create_node(nodeId, cur, 'last') )
	}
	
	// 초기 트리 설정
	treeInit()
	treeEvent()
	updateButtonStates();

	return {tree, treeData, treeEvent, treeIcon, treeLoad, updateButtonStates }	
}

function useTreeHtml() {
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

}