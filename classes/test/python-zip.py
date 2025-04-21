import os
import zipfile
import rarfile
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from pathlib import Path
import json
import datetime

class ZipPreviewApp:
    def __init__(self, root):
        self.root = root
        self.root.title("압축 파일 미리보기")
        self.root.geometry("800x600")
        self.root.minsize(600, 400)
        
        # 변수 초기화
        self.current_file = None
        self.file_list = []
        
        # UI 구성
        self.create_widgets()
        
    def create_widgets(self):
        # 메인 프레임
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # 상단 버튼 프레임
        button_frame = ttk.Frame(main_frame)
        button_frame.pack(fill=tk.X, pady=(0, 10))
        
        # 파일 선택 버튼
        self.select_button = ttk.Button(button_frame, text="압축 파일 선택", command=self.select_file)
        self.select_button.pack(side=tk.LEFT, padx=(0, 5))
        
        # 현재 파일 경로 표시
        self.file_label = ttk.Label(button_frame, text="선택된 파일: 없음")
        self.file_label.pack(side=tk.LEFT, padx=5)
        
        # JSON 출력 버튼
        self.json_button = ttk.Button(button_frame, text="JSON 출력", command=self.export_json)
        self.json_button.pack(side=tk.LEFT, padx=5)
        self.json_button.config(state=tk.DISABLED)
        
        # 파일 목록 프레임
        list_frame = ttk.LabelFrame(main_frame, text="압축 파일 내용")
        list_frame.pack(fill=tk.BOTH, expand=True)
        
        # 트리뷰 (파일 목록 표시)
        self.tree = ttk.Treeview(list_frame, columns=("size", "date"), show="headings")
        self.tree.heading("size", text="크기")
        self.tree.heading("date", text="수정 날짜")
        self.tree.column("size", width=100, anchor=tk.E)
        self.tree.column("date", width=150, anchor=tk.W)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # 스크롤바
        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.tree.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree.configure(yscrollcommand=scrollbar.set)
        
        # 하단 프레임
        bottom_frame = ttk.Frame(main_frame)
        bottom_frame.pack(fill=tk.X, pady=(10, 0))
        
        # 파일 정보 표시
        self.info_label = ttk.Label(bottom_frame, text="압축 파일을 선택하세요")
        self.info_label.pack(side=tk.LEFT)
        
        # 추출 버튼
        self.extract_button = ttk.Button(bottom_frame, text="선택 항목 추출", command=self.extract_selected)
        self.extract_button.pack(side=tk.RIGHT)
        self.extract_button.config(state=tk.DISABLED)
        
        # 트리뷰 선택 이벤트
        self.tree.bind("<<TreeviewSelect>>", self.on_select)
        
    def select_file(self):
        file_path = filedialog.askopenfilename(
            title="압축 파일 선택",
            filetypes=[
                ("압축 파일", "*.zip;*.rar;*.7z"),
                ("ZIP 파일", "*.zip"),
                ("RAR 파일", "*.rar"),
                ("7Z 파일", "*.7z"),
                ("모든 파일", "*.*")
            ]
        )
        
        if file_path:
            self.load_archive(file_path)
    
    def load_archive(self, file_path):
        self.current_file = file_path
        self.file_label.config(text=f"선택된 파일: {os.path.basename(file_path)}")
        
        # 트리뷰 초기화
        for item in self.tree.get_children():
            self.tree.delete(item)
        
        self.file_list = []
        
        # 파일 확장자 확인
        ext = os.path.splitext(file_path)[1].lower()
        
        try:
            if ext == '.zip':
                self._load_zip(file_path)
            elif ext == '.rar':
                self._load_rar(file_path)
            elif ext == '.7z':
                self._load_7z(file_path)
            else:
                messagebox.showerror("오류", "지원하지 않는 압축 파일 형식입니다.")
                return
            
            # 파일 정보 업데이트
            self.info_label.config(text=f"총 {len(self.file_list)}개 파일")
            self.extract_button.config(state=tk.NORMAL)
            self.json_button.config(state=tk.NORMAL)
            
        except Exception as e:
            messagebox.showerror("오류", f"압축 파일을 열 수 없습니다: {str(e)}")
    
    def _load_zip(self, file_path):
        with zipfile.ZipFile(file_path, 'r') as zip_ref:
            for file_info in zip_ref.infolist():
                # 디렉토리인 경우 건너뛰기
                if file_info.filename.endswith('/'):
                    continue
                
                # 파일 정보 추출
                name = file_info.filename
                size = file_info.file_size
                date = self._format_date(file_info.date_time)
                
                # 트리뷰에 추가
                self.tree.insert("", tk.END, values=(name, self._format_size(size), date))
                self.file_list.append({
                    'name': name,
                    'size': size,
                    'date': date,
                    'index': len(self.file_list)
                })
    
    def _load_rar(self, file_path):
        with rarfile.RarFile(file_path, 'r') as rar_ref:
            for file_info in rar_ref.infolist():
                # 디렉토리인 경우 건너뛰기
                if file_info.is_dir():
                    continue
                
                # 파일 정보 추출
                name = file_info.filename
                size = file_info.file_size
                date = self._format_date(file_info.date_time)
                
                # 트리뷰에 추가
                self.tree.insert("", tk.END, values=(name, self._format_size(size), date))
                self.file_list.append({
                    'name': name,
                    'size': size,
                    'date': date,
                    'index': len(self.file_list)
                })
    
    def _load_7z(self, file_path):
        # 7z 파일 처리를 위한 코드
        # py7zr 라이브러리가 필요합니다
        try:
            import py7zr
            with py7zr.SevenZipFile(file_path, 'r') as sz_ref:
                for name, info in sz_ref.files.items():
                    # 디렉토리인 경우 건너뛰기
                    if info.is_directory:
                        continue
                    
                    # 파일 정보 추출
                    size = info.size
                    date = self._format_date(info.creationtime)
                    
                    # 트리뷰에 추가
                    self.tree.insert("", tk.END, values=(name, self._format_size(size), date))
                    self.file_list.append({
                        'name': name,
                        'size': size,
                        'date': date,
                        'index': len(self.file_list)
                    })
        except ImportError:
            messagebox.showerror("오류", "7z 파일 지원을 위해 py7zr 라이브러리가 필요합니다.")
    
    def _format_size(self, size):
        """파일 크기를 읽기 쉬운 형식으로 변환"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if size < 1024.0 or unit == 'GB':
                return f"{size:.1f} {unit}"
            size /= 1024.0
    
    def _format_date(self, date_time):
        """날짜 시간을 읽기 쉬운 형식으로 변환"""
        if isinstance(date_time, tuple):
            year, month, day, hour, minute, second = date_time
            return f"{year}-{month:02d}-{day:02d} {hour:02d}:{minute:02d}:{second:02d}"
        return str(date_time)
    
    def on_select(self, event):
        """트리뷰 항목 선택 시 호출되는 함수"""
        selected_items = self.tree.selection()
        if selected_items:
            self.extract_button.config(state=tk.NORMAL)
        else:
            self.extract_button.config(state=tk.DISABLED)
    
    def extract_selected(self):
        """선택한 파일 추출"""
        selected_items = self.tree.selection()
        if not selected_items:
            return
        
        # 추출할 디렉토리 선택
        extract_dir = filedialog.askdirectory(title="추출할 위치 선택")
        if not extract_dir:
            return
        
        try:
            # 파일 확장자 확인
            ext = os.path.splitext(self.current_file)[1].lower()
            
            if ext == '.zip':
                self._extract_zip(extract_dir, selected_items)
            elif ext == '.rar':
                self._extract_rar(extract_dir, selected_items)
            elif ext == '.7z':
                self._extract_7z(extract_dir, selected_items)
            
            messagebox.showinfo("완료", "선택한 파일이 추출되었습니다.")
            
        except Exception as e:
            messagebox.showerror("오류", f"파일 추출 중 오류가 발생했습니다: {str(e)}")
    
    def _extract_zip(self, extract_dir, selected_items):
        """ZIP 파일에서 선택한 항목 추출"""
        with zipfile.ZipFile(self.current_file, 'r') as zip_ref:
            for item in selected_items:
                values = self.tree.item(item, 'values')
                file_name = values[0]
                
                # 파일 경로 생성
                file_path = os.path.join(extract_dir, file_name)
                
                # 디렉토리 생성
                os.makedirs(os.path.dirname(file_path), exist_ok=True)
                
                # 파일 추출
                with zip_ref.open(file_name) as source, open(file_path, 'wb') as target:
                    target.write(source.read())
    
    def _extract_rar(self, extract_dir, selected_items):
        """RAR 파일에서 선택한 항목 추출"""
        with rarfile.RarFile(self.current_file, 'r') as rar_ref:
            for item in selected_items:
                values = self.tree.item(item, 'values')
                file_name = values[0]
                
                # 파일 경로 생성
                file_path = os.path.join(extract_dir, file_name)
                
                # 디렉토리 생성
                os.makedirs(os.path.dirname(file_path), exist_ok=True)
                
                # 파일 추출
                with rar_ref.open(file_name) as source, open(file_path, 'wb') as target:
                    target.write(source.read())
    
    def _extract_7z(self, extract_dir, selected_items):
        """7Z 파일에서 선택한 항목 추출"""
        try:
            import py7zr
            with py7zr.SevenZipFile(self.current_file, 'r') as sz_ref:
                for item in selected_items:
                    values = self.tree.item(item, 'values')
                    file_name = values[0]
                    
                    # 파일 경로 생성
                    file_path = os.path.join(extract_dir, file_name)
                    
                    # 디렉토리 생성
                    os.makedirs(os.path.dirname(file_path), exist_ok=True)
                    
                    # 파일 추출
                    with sz_ref.open(file_name) as source, open(file_path, 'wb') as target:
                        target.write(source.read())
        except ImportError:
            messagebox.showerror("오류", "7z 파일 추출을 위해 py7zr 라이브러리가 필요합니다.")
    
    def export_json(self):
        """압축 파일 목록을 JSON 형태로 출력"""
        if not self.file_list:
            messagebox.showinfo("알림", "출력할 파일 목록이 없습니다.")
            return
        
        # JSON 파일 저장 경로 선택
        file_path = filedialog.asksaveasfilename(
            title="JSON 파일 저장",
            defaultextension=".json",
            filetypes=[("JSON 파일", "*.json"), ("모든 파일", "*.*")]
        )
        
        if not file_path:
            return
        
        try:
            # JSON 데이터 준비
            json_data = {
                "archive_name": os.path.basename(self.current_file),
                "archive_path": self.current_file,
                "file_count": len(self.file_list),
                "files": self.file_list,
                "export_date": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
            
            # JSON 파일로 저장
            with open(file_path, 'w', encoding='utf-8') as f:
                json.dump(json_data, f, ensure_ascii=False, indent=2)
            
            messagebox.showinfo("완료", f"JSON 파일이 저장되었습니다: {file_path}")
            
        except Exception as e:
            messagebox.showerror("오류", f"JSON 파일 저장 중 오류가 발생했습니다: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    app = ZipPreviewApp(root)
    root.mainloop() 