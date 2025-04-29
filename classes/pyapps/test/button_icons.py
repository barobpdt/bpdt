import os
from pathlib import Path
from typing import Dict, List, Optional, Union, Tuple
import hashlib
import logging
import requests
import shutil
import json
from datetime import datetime

from .icon_utils import IconManager

logger = logging.getLogger('button_icons')

class ButtonIconManager(IconManager):
    """
    자주 사용하는 버튼 아이콘을 다운로드하고 관리하는 클래스
    """
    
    # 기본 버튼 아이콘 URL (Material Design Icons)
    DEFAULT_BUTTON_ICONS = {
        # 기본 동작
        'add': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/content/add/materialicons/24px.svg',
        'edit': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/image/edit/materialicons/24px.svg',
        'delete': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/delete/materialicons/24px.svg',
        'save': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/content/save/materialicons/24px.svg',
        'cancel': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/cancel/materialicons/24px.svg',
        'close': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/close/materialicons/24px.svg',
        'search': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/search/materialicons/24px.svg',
        'refresh': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/refresh/materialicons/24px.svg',
        
        # 파일 관련
        'upload': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/file/upload/materialicons/24px.svg',
        'download': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/file/download/materialicons/24px.svg',
        'attach_file': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/file/attach_file/materialicons/24px.svg',
        'folder_open': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/file/folder_open/materialicons/24px.svg',
        
        # 미디어 제어
        'play': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/play_arrow/materialicons/24px.svg',
        'pause': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/pause/materialicons/24px.svg',
        'stop': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/stop/materialicons/24px.svg',
        'volume_up': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/volume_up/materialicons/24px.svg',
        'volume_down': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/volume_down/materialicons/24px.svg',
        'volume_mute': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/av/volume_off/materialicons/24px.svg',
        
        # 탐색
        'home': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/home/materialicons/24px.svg',
        'menu': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/menu/materialicons/24px.svg',
        'more': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/more_horiz/materialicons/24px.svg',
        'arrow_back': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/arrow_back/materialicons/24px.svg',
        'arrow_forward': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/navigation/arrow_forward/materialicons/24px.svg',
        
        # 소셜/커뮤니케이션
        'share': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/social/share/materialicons/24px.svg',
        'chat': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/communication/chat/materialicons/24px.svg',
        'email': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/communication/email/materialicons/24px.svg',
        'notifications': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/social/notifications/materialicons/24px.svg',
        
        # 설정/도구
        'settings': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/settings/materialicons/24px.svg',
        'info': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/info/materialicons/24px.svg',
        'help': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/help/materialicons/24px.svg',
        'warning': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/alert/warning/materialicons/24px.svg',
        'error': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/alert/error/materialicons/24px.svg',
        
        # 사용자
        'account': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/account_circle/materialicons/24px.svg',
        'group': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/social/group/materialicons/24px.svg',
        'person_add': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/social/person_add/materialicons/24px.svg',
        'person_remove': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/social/person_remove/materialicons/24px.svg',
        
        # 기타
        'favorite': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/favorite/materialicons/24px.svg',
        'star': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/toggle/star/materialicons/24px.svg',
        'print': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/action/print/materialicons/24px.svg',
        'link': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/content/link/materialicons/24px.svg',
        'copy': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/content/content_copy/materialicons/24px.svg',
        'paste': 'https://raw.githubusercontent.com/google/material-design-icons/master/src/content/content_paste/materialicons/24px.svg',
    }
    
    def __init__(self, cache_dir: Optional[Union[str, Path]] = None):
        """
        버튼 아이콘 매니저 초기화
        
        Args:
            cache_dir (Optional[Union[str, Path]]): 아이콘 캐시 디렉토리 경로 (기본값: ~/.button_icon_cache)
        """
        if cache_dir is None:
            cache_dir = os.path.expanduser('~/.button_icon_cache')
        
        super().__init__(cache_dir)
        
        # 버튼 아이콘 URL 추가
        self.icon_urls.update(self.DEFAULT_BUTTON_ICONS)
    
    def get_button_icon_path(self, button_name: str, force_download: bool = False) -> Optional[Path]:
        """
        버튼 아이콘 파일 경로 가져오기
        
        Args:
            button_name (str): 버튼 이름 (예: 'add', 'edit', 'delete' 등)
            force_download (bool): 강제 다운로드 여부 (기본값: False)
            
        Returns:
            Optional[Path]: 아이콘 파일 경로 (없으면 None)
        """
        return self.get_icon_path(button_name, force_download)
    
    def get_all_button_names(self) -> List[str]:
        """
        지원하는 모든 버튼 이름 목록 가져오기
        
        Returns:
            List[str]: 버튼 이름 목록
        """
        return list(self.DEFAULT_BUTTON_ICONS.keys())
    
    def get_button_icon_info(self, button_name: str) -> Dict:
        """
        버튼 아이콘 정보 가져오기
        
        Args:
            button_name (str): 버튼 이름 (예: 'add', 'edit', 'delete' 등)
            
        Returns:
            Dict: 아이콘 정보
        """
        return self.get_icon_info(button_name)

# 사용 예시
if __name__ == "__main__":
    # 버튼 아이콘 매니저 초기화
    button_manager = ButtonIconManager()
    
    # 기본 버튼 아이콘 다운로드
    add_icon_path = button_manager.get_button_icon_path('add')
    print(f"추가 버튼 아이콘 경로: {add_icon_path}")
    
    edit_icon_path = button_manager.get_button_icon_path('edit')
    print(f"편집 버튼 아이콘 경로: {edit_icon_path}")
    
    delete_icon_path = button_manager.get_button_icon_path('delete')
    print(f"삭제 버튼 아이콘 경로: {delete_icon_path}")
    
    # 지원하는 모든 버튼 목록
    button_names = button_manager.get_all_button_names()
    print(f"지원하는 버튼 수: {len(button_names)}")
    print("지원하는 버튼 목록:")
    for name in button_names:
        print(f"- {name}")
    
    # 버튼 아이콘 정보 가져오기
    add_icon_info = button_manager.get_button_icon_info('add')
    print(f"추가 버튼 아이콘 정보: {add_icon_info}") 