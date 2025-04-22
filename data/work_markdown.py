class MarkdownLinkGUI:
    """Markdown 링크 처리를 위한 GUI 애플리케이션"""
    
    def __init__(self, root):
        """
        초기화
        
        Args:
            root (tk.Tk): Tkinter 루트 윈도우
        """
        self.root = root
        self.root.title("Markdown 링크 처리기")
        self.root.geometry("800x600")
        
        # 링크 처리기 초기화
        self.link_processor = MarkdownLinkProcessor()
        
        # UI 구성
        self.create_widgets()
    
    def create_widgets(self):
        """UI 위젯 생성"""
        # 메인 프레임
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # 입력 영역
        input_frame = ttk.LabelFrame(main_frame, text="Markdown 입력", padding="5")
        input_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.input_text = tk.Text(input_frame, wrap=tk.WORD)
        self.input_text.pack(fill=tk.BOTH, expand=True)
        
        # 버튼 영역
        button_frame = ttk.Frame(main_frame)
        button_frame.pack(fill=tk.X, pady=5)
        
        ttk.Button(button_frame, text="파일 열기", command=self.open_file).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="HTML 변환", command=self.convert_to_html).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="링크 기록 보기", command=self.show_link_history).pack(side=tk.LEFT, padx=5)
        
        # 상태 표시줄
        self.status_var = tk.StringVar()
        self.status_var.set("준비")
        status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(fill=tk.X, pady=5)
    
    def open_file(self):
        """파일 열기"""
        file_path = filedialog.askopenfilename(
            title="Markdown 파일 선택",
            filetypes=[("Markdown 파일", "*.md"), ("모든 파일", "*.*")]
        )
        
        if file_path:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    self.input_text.delete(1.0, tk.END)
                    self.input_text.insert(tk.END, content)
                
                # 기본 디렉토리 설정
                self.link_processor = MarkdownLinkProcessor(os.path.dirname(file_path))
                
                self.status_var.set(f"파일 로드됨: {file_path}")
            except Exception as e:
                messagebox.showerror("오류", f"파일 열기 실패: {e}")
    
    def convert_to_html(self):
        """HTML 변환"""
        markdown_text = self.input_text.get(1.0, tk.END)
        
        if not markdown_text.strip():
            messagebox.showwarning("경고", "변환할 Markdown 텍스트가 없습니다.")
            return
        
        # HTML 변환
        html = self.link_processor.markdown_to_html(markdown_text)
        
        # 파일 저장 대화상자
        file_path = filedialog.asksaveasfilename(
            title="HTML 파일 저장",
            defaultextension=".html",
            filetypes=[("HTML 파일", "*.html"), ("모든 파일", "*.*")]
        )
        
        if file_path:
            try:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(html)
                
                self.status_var.set(f"HTML 저장됨: {file_path}")
                
                # 저장 후 브라우저로 열기
                if messagebox.askyesno("완료", "HTML 파일이 저장되었습니다. 브라우저로 열겠습니까?"):
                    webbrowser.open(file_path)
            except Exception as e:
                messagebox.showerror("오류", f"HTML 저장 실패: {e}")
    
    def show_link_history(self):
        """링크 기록 보기"""
        # 새 창 생성
        history_window = tk.Toplevel(self.root)
        history_window.title("링크 기록")
        history_window.geometry("600x400")
        
        # 링크 기록 표시
        history_frame = ttk.Frame(history_window, padding="10")
        history_frame.pack(fill=tk.BOTH, expand=True)
        
        # 링크 기록 목록
        history_list = ttk.Treeview(history_frame, columns=("timestamp", "link", "text"), show="headings")
        history_list.heading("timestamp", text="시간")
        history_list.heading("link", text="링크")
        history_list.heading("text", text="텍스트")
        
        history_list.column("timestamp", width=150)
        history_list.column("link", width=250)
        history_list.column("text", width=200)
        
        history_list.pack(fill=tk.BOTH, expand=True)
        
        # 스크롤바 추가
        scrollbar = ttk.Scrollbar(history_frame, orient=tk.VERTICAL, command=history_list.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        history_list.configure(yscrollcommand=scrollbar.set)
        
        # 링크 기록 로드
        history = self.link_processor.link_handler.get_history()
        
        for item in history:
            history_list.insert("", tk.END, values=(item['timestamp'], item['link'], item['text']))
        
        # 버튼 영역
        button_frame = ttk.Frame(history_window)
        button_frame.pack(fill=tk.X, pady=5)
        
        ttk.Button(button_frame, text="기록 지우기", command=lambda: self.clear_link_history(history_list)).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="닫기", command=history_window.destroy).pack(side=tk.RIGHT, padx=5)
    
    def clear_link_history(self, history_list):
        """링크 기록 지우기"""
        if messagebox.askyesno("확인", "모든 링크 기록을 지우시겠습니까?"):
            self.link_processor.link_handler.clear_history()
            history_list.delete(*history_list.get_children())
            self.status_var.set("링크 기록이 지워졌습니다.")
