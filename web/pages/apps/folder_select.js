let pageModule = null

document.addEventListener('DOMContentLoaded', () => {
    // 초기화	
    pageModule = initTree('#folderTree')
	initData()
	console.log('page module => ', pageModule)
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
            const node = await response.json()
			pageModule.rootNode = node
			pageModule.initTreeData(node.children)
             
        } catch (error) {
            console.error('폴더 트리 로드 실패:', error);
        }
    }
	initDrives()
	return impl(pageModule, {initDrives, loadTree})
}
 
function initTree(treeId) {
	const cf = {}
	const tree = $(treeId)
	const btnAdd = $('#addNode')
	const btnDelete = $('#deleteNode')
	let jstree = null
	
	function treeEvent() {
		tree.on('select_node.jstree', (e, d) => cf.treeChangeNode(d.node) )		
		// 드래그 앤 드롭 이벤트
		tree.on('move_node.jstree', (e, d) => cf.treeMoveNode(e,d) )		
		// 이벤트 open_node
		tree.on('open_node.jstree', (e,d)=> cf.treeOpen(d.node, d.node.original) )
		
		// 컨텍스트 메뉴 이벤트
		tree.on('contextmenu', '.jstree-anchor', function(e) {
			e.preventDefault();
			// 컨텍스트 메뉴 생성
			const menu = cf.makeMenu($(this))
			// 메뉴 위치 설정
			menu.css({top: e.pageY, left: e.pageX})
			
			// 메뉴 표시
			$('body').append(menu)			
			// 메뉴 항목 클릭 이벤트
			menu.on('click', '.context-menu-item', function() {
				if ($(this).hasClass('disabled')) return;				
				const action = $(this).data('action');
				cf.menuAction(action)
				// 메뉴 제거
				menu.remove();
			});			
			// 다른 곳 클릭시 메뉴 제거
			$(document).one('click', e=>menu.remove())
		});

		// 버튼 이벤트
		btnAdd.on('click',() => cf.treeAddChild() )
		btnDelete.on('click', () => cf.treeDelete() )

		// 버튼 상태 업데이트
		tree.on('select_node.jstree', () => cf.updateButtonStates() )
		tree.on('deselect_node.jstree', () => cf.updateButtonStates() )
	}

	cf.initTreeData = function(data) {
		const newData = data||[]
		tree.jstree({
			core: {
				data: newData,
				check_callback: true,
				dblclick_toggle: false,
				themes: {
					'name': 'proton',
					'responsive': false,
					'variant': 'small',
					'stripes': false
				}
			},
			plugins: ['dnd','checkbox','types','search','changed'],
			types: {
				default: {icon:'fa fa-folder'}
			}
		})
		jstree = tree.jstree()
		treeEvent()
		cf.updateButtonStates();
		cf.jstree = jstree
		if( newData.length ) { 
			setTimeout(()=>jstree.open_node(newData[0].id), 250 )
		}
	}

	cf.refreshTreeData = function(data) {
		if( jstree ) {
			// tree.jstree('destroy')
			const a = tree.jstree(true)
			console.log('@@ jstree destory => ',a)
			a.settings.core.data = data
			console.log('@@ jstree data => ', jstree)
			a.refresh()
			console.log('@@ jstree refresh => ' )
			return;
		}
	}
	
	cf.setTreeChild = function(node) {
		const rootNode = pageModule.rootNode
		const params={
			idx:rootNode.idx,
			children:[]
		}
		node.children.forEach(pid=>{
			const {id,fullPath,checkChild} = jstree.get_node(pid).original
			if( !checkChild ) {
				params.children.push({id,pid,fullPath})
			}
		})
		console.log('@@ setTreeChild => ', params)
		if( params.children.length ) {
			fetch('/api/fetchTreeChild', {method:'POST',body:JSON.stringify(params)}).then(res=>res.json()).then(result=> {
				if( !Array.isArray(result.children) ) {
					console.log('@@ setTreeChild fetch result 유효하지 않습니다 => ', result)
					return;
				}
				result.children.forEach(cur=>{					
					if( cur.children) {
						const nodeId = cur.pid
						console.log('@@ setTreeChild 부모 노드:'+nodeId+' 자식추가=>', cur)						
						cur.children.forEach( sub => {
							sub.id = 'K'+rootNode.idx++
							sub.parent = nodeId
							jstree.create_node(nodeId, sub)
							console.log('sub=>', sub)
						})
					}
				})
			})
		}
	}
	
	cf.makeMenu = function(node) {
		const menu = $('<div class="context-menu"></div>');
		const nodeId = node.attr('id');
		const isLeaf = node.parent().hasClass('jstree-leaf');
		menu.append('<div class="context-menu-item" data-action="add-child">자식 추가</div>');
		menu.append('<div class="context-menu-item" data-action="add-sibling">형제 추가</div>');

		// 삭제 메뉴 항목 추가 (자식이 없는 노드만 활성화)
		const deleteItem = $('<div class="context-menu-item" data-action="delete">삭제</div>');
		if (!isLeaf) {
			deleteItem.addClass('disabled')
		}
		menu.append(deleteItem)
		return menu
	}
	
	cf.menuAction = function(action) {
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
		default:
			break;
		}
	}
	
	cf.treeOpen = function(node, cur) {
		if( !cur.checkIcons) {
			node.children.forEach(tid=>cf.treeIcon(tid))
			cur.checkIcons = true
		}
		if( !cur.checkTree) {
			cf.setTreeChild(node)
			cur.checkTree = true
		}
	}
	cf.treeAddChild = function(tid, node) { 		
		if( !tid ) {
			const a = jstree.get_selected()
			if( a.length==0 ) return console.log('자식추가 오류 부모노드 미정의')
			tid = a[0]
		}
		if( !node ) {
			node={type:'file', text:'새파일'}
		}
		jstree.create_node(tid, node , 'last', cur => $.jstree.reference(cur).edit(cur) )
	}
	cf.treeDelete = function(tid) { 	
		if( !tid ) {
			const a = jstree.get_selected()
			if( a.length==0 ) return console.log('삭제 노드 미정의')
			tid = a[0]
		}
		const node = jstree.get_node(tid)
		if ( node.children.length || !confirm('선택된 노드를 삭제하시겠습니까?') ) return;
		tree.delete_node(tid)
	}
	cf.treeChangeNode = function(node) { 	
		console.log('노드가 선택되었습니다:', node);
	}
	cf.treeMoveNode = function(e,d) {
		const parent = tree.get_node(d.parent)
		tree.open_node(parent)
		console.log('노드가 선택되었습니다:', node);
	}
		
	cf.treeIcon = function(tid, cur) {
		if(!cur) cur = jstree.get_node(tid)
		const icon = qs('#'+tid+' .jstree-themeicon')
		// console.log('icon=>',tid, cur, icon )
	}
	
	cf.updateButtonStates = function() {
		const a = jstree.get_selected()
		const hasSelection = a.length > 0;
		const isLeaf = hasSelection && jstree.get_node(a[0]).children.length === 0;
		
		btnAdd.prop('disabled', !hasSelection);
		btnDelete.prop('disabled', !hasSelection || !isLeaf);
	}
	
	// 특정 노드의 자식들만 새로고침
	cf.treeRender = function(tid, newData) {
		const node = jstree.get_node(tid)
		jstree.delete_node( node.children )
		newData.forEach( cur => jstree.create_node(tid, cur, 'last') )
	}
	
	return cf
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