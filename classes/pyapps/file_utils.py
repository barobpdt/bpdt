import os
import base64
import json
import csv
from pathlib import Path
from typing import Union, Optional, Any, Dict, List
import zipfile
import io
from datetime import datetime

def file_to_base64(file_path):
    """
    파일을 base64로 인코딩하여 반환합니다
    """
    try:
        # 파일이 존재하는지 확인
        if not os.path.exists(file_path):
            return f"오류: 파일 '{file_path}'을(를) 찾을 수 없습니다."
        
        # 파일이 디렉토리인지 확인
        if os.path.isdir(file_path):
            return f"오류: '{file_path}'은(는) 디렉토리입니다. 파일을 지정해주세요."
        
        # 파일을 바이너리 모드로 열고 base64로 인코딩
        with open(file_path, 'rb') as file:
            encoded_string = base64.b64encode(file.read()).decode('utf-8')
            return encoded_string
    
    except Exception as e:
        return f"오류: {str(e)}"

def check_path_exists(path: Union[str, Path], path_type: Optional[str] = None) -> tuple[bool, str]:
    """
    파일 또는 폴더의 존재 여부를 확인하는 함수
    
    Args:
        path (Union[str, Path]): 확인할 파일 또는 폴더 경로
        path_type (Optional[str]): 'file' 또는 'folder'로 지정하여 특정 타입만 체크. None이면 둘 다 체크
        
    Returns:
        tuple[bool, str]: (존재 여부, 메시지)
    """
    try:
        # Path 객체로 변환
        path_obj = Path(path) if isinstance(path, str) else path
        
        # 경로가 존재하지 않는 경우
        if not path_obj.exists():
            return False, f"경로가 존재하지 않습니다: {path}"
            
        # path_type이 지정된 경우 해당 타입만 체크
        if path_type:
            if path_type.lower() == 'file' and not path_obj.is_file():
                return False, f"파일이 아닙니다: {path}"
            elif path_type.lower() == 'folder' and not path_obj.is_dir():
                return False, f"폴더가 아닙니다: {path}"
                
        return True, "정상"
        
    except Exception as e:
        return False, f"오류 발생: {str(e)}"

def check_file_exists(file_path: Union[str, Path]) -> tuple[bool, str]:
    """
    파일 존재 여부를 확인하는 함수
    """
    return check_path_exists(file_path, 'file')

def check_folder_exists(folder_path: Union[str, Path]) -> tuple[bool, str]:
    """
    폴더 존재 여부를 확인하는 함수
    
    Args:
        folder_path (Union[str, Path]): 확인할 폴더 경로
        
    Returns:
        tuple[bool, str]: (존재 여부, 메시지)
    """
    return check_path_exists(folder_path, 'folder')

def change_extension(file_path: Union[str, Path], new_extension: str) -> str:
    """
    파일 경로의 확장자를 변경하는 함수
    
    Args:
        file_path (Union[str, Path]): 원본 파일 경로
        new_extension (str): 새로운 확장자 (예: 'txt', 'pdf', 'jpg')
        
    Returns:
        str: 확장자가 변경된 파일 경로
    """
    # 확장자 앞에 점이 없으면 추가
    if not new_extension.startswith('.'):
        new_extension = '.' + new_extension
    
    # Path 객체로 변환
    path_obj = Path(file_path) if isinstance(file_path, str) else file_path
    
    # 확장자 변경
    return str(path_obj.with_suffix(new_extension))

def save_file(file_path: Union[str, Path], data: Any, file_format: str = 'text', 
              encoding: str = 'utf-8', create_dirs: bool = True) -> tuple[bool, str]:
    """
    데이터를 파일로 저장하는 함수
    
    Args:
        file_path (Union[str, Path]): 저장할 파일 경로
        data (Any): 저장할 데이터 (문자열, 바이너리, 딕셔너리, 리스트 등)
        file_format (str): 파일 형식 ('text', 'binary', 'json', 'csv', 'base64')
        encoding (str): 텍스트 파일 인코딩 (기본값: 'utf-8')
        create_dirs (bool): 필요한 디렉토리를 자동 생성할지 여부 (기본값: True)
        
    Returns:
        tuple[bool, str]: (성공 여부, 메시지)
    """
    try:
        # Path 객체로 변환
        path_obj = Path(file_path) if isinstance(file_path, str) else file_path
        
        # 디렉토리 생성
        if create_dirs:
            path_obj.parent.mkdir(parents=True, exist_ok=True)
        
        # 파일 형식에 따라 저장
        if file_format.lower() == 'text':
            # 텍스트 파일로 저장
            with open(path_obj, 'w', encoding=encoding) as f:
                f.write(str(data))
                
        elif file_format.lower() == 'binary':
            # 바이너리 파일로 저장
            if isinstance(data, str):
                # 문자열인 경우 인코딩하여 저장
                with open(path_obj, 'wb') as f:
                    f.write(data.encode(encoding))
            else:
                # 바이너리 데이터인 경우 그대로 저장
                with open(path_obj, 'wb') as f:
                    f.write(data)
                    
        elif file_format.lower() == 'json':
            # JSON 파일로 저장
            with open(path_obj, 'w', encoding=encoding) as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
                
        elif file_format.lower() == 'csv':
            # CSV 파일로 저장
            with open(path_obj, 'w', encoding=encoding, newline='') as f:
                writer = csv.writer(f)
                if isinstance(data, list):
                    writer.writerows(data)
                else:
                    writer.writerow(data)
                    
        elif file_format.lower() == 'base64':
            # Base64 디코딩하여 저장
            if isinstance(data, str):
                binary_data = base64.b64decode(data)
                with open(path_obj, 'wb') as f:
                    f.write(binary_data)
            else:
                return False, "Base64 형식은 문자열 데이터만 지원합니다."
                
        else:
            return False, f"지원하지 않는 파일 형식입니다: {file_format}"
            
        return True, f"파일이 성공적으로 저장되었습니다: {path_obj}"
        
    except Exception as e:
        return False, f"파일 저장 중 오류 발생: {str(e)}"

def list_zip_contents(zip_file: Union[str, Path, bytes]) -> tuple[bool, str]:
    """
    압축 파일의 내용을 JSON 형태로 출력하는 함수
    
    Args:
        zip_file (Union[str, Path, bytes]): 압축 파일 경로 또는 바이트 데이터
        output_file (Optional[Union[str, Path]]): 출력 JSON 파일 경로 (기본값: None, 콘솔에 출력)
        
    Returns:
        tuple[bool, str]: (성공 여부, 메시지)
    """
    try:
        # 압축 파일 데이터 준비
        if isinstance(zip_file, (str, Path)):
            zip_path = Path(zip_file) if isinstance(zip_file, str) else zip_file
            
            # 파일이 존재하는지 확인
            if not zip_path.exists():
                return False, f"압축 파일이 존재하지 않습니다: {zip_path}"
            
            # 압축 파일 열기
            zip_data = zipfile.ZipFile(zip_path)
        elif isinstance(zip_file, bytes):
            # 바이트 데이터에서 압축 파일 열기
            zip_data = zipfile.ZipFile(io.BytesIO(zip_file))
        else:
            return False, f"지원하지 않는 압축 파일 형식입니다: {type(zip_file)}"
        
        # 압축 파일 정보 수집
        zip_info = {
            "filename": zip_data.filename if hasattr(zip_data, 'filename') else "memory_zip",
            "file_count": len(zip_data.namelist()),
            "total_size": sum(info.file_size for info in zip_data.filelist),
            "compressed_size": sum(info.compress_size for info in zip_data.filelist),
            "compression_ratio": round((1 - sum(info.compress_size for info in zip_data.filelist) / 
                                      sum(info.file_size for info in zip_data.filelist)) * 100, 2) if zip_data.filelist else 0,
            "files": []
        }
        
        # 각 파일 정보 수집
        for file_info in zip_data.filelist:
            file_data = {
                "name": file_info.filename,
                "size": file_info.file_size,
                "compressed_size": file_info.compress_size,
                "date_time": datetime(*file_info.date_time).isoformat() if file_info.date_time else None,
                "is_dir": file_info.is_dir(),
                "is_file": not file_info.is_dir(),
                "compression_method": file_info.compress_type,
                "compression_name": {
                    zipfile.ZIP_STORED: "저장됨 (압축 없음)",
                    zipfile.ZIP_DEFLATED: "Deflate",
                    zipfile.ZIP_BZIP2: "BZIP2",
                    zipfile.ZIP_LZMA: "LZMA"
                }.get(file_info.compress_type, "알 수 없음")
            }
            
            # 파일 확장자 추출
            if file_data["is_file"]:
                file_path = Path(file_data["name"])
                file_data["extension"] = file_path.suffix.lower() if file_path.suffix else None
                
                # 파일 유형 분류
                if file_data["extension"] in [".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".webp"]:
                    file_data["type"] = "이미지"
                elif file_data["extension"] in [".doc", ".docx", ".xls", ".xlsx", ".ppt", ".pptx", ".pdf", ".txt", ".rtf"]:
                    file_data["type"] = "문서"
                elif file_data["extension"] in [".mp3", ".wav", ".ogg", ".flac", ".aac"]:
                    file_data["type"] = "오디오"
                elif file_data["extension"] in [".mp4", ".avi", ".mov", ".wmv", ".flv", ".mkv"]:
                    file_data["type"] = "비디오"
                elif file_data["extension"] in [".zip", ".rar", ".7z", ".tar", ".gz"]:
                    file_data["type"] = "압축 파일"
                elif file_data["extension"] in [".exe", ".msi", ".bat", ".sh", ".py", ".js", ".html", ".css", ".php"]:
                    file_data["type"] = "실행 파일/스크립트"
                else:
                    file_data["type"] = "기타"
            else:
                file_data["extension"] = None
                file_data["type"] = "디렉토리"
            
            zip_info["files"].append(file_data)
        
        # 파일 유형별 통계 추가
        type_stats = {}
        for file_data in zip_info["files"]:
            file_type = file_data.get("type", "기타")
            if file_type not in type_stats:
                type_stats[file_type] = {"count": 0, "total_size": 0}
            type_stats[file_type]["count"] += 1
            type_stats[file_type]["total_size"] += file_data["size"]
        
        zip_info["type_statistics"] = type_stats
         
        # 출력 파일이 지정된 경우 파일로 저장
        return True, json.dumps(zip_info, ensure_ascii=False, indent=2)
    
    except Exception as e:
        return False, f"압축 파일 내용 출력 중 오류 발생: {str(e)}"

def extract_zip_to_directory(zip_file: Union[str, Path, bytes], output_dir: Optional[Union[str, Path]] = None) -> tuple[bool, str]:
    """
    압축 파일을 지정된 디렉토리에 압축 해제하는 함수
    
    Args:
        zip_file (Union[str, Path, bytes]): 압축 파일 경로 또는 바이트 데이터
        output_dir (Optional[Union[str, Path]]): 출력 디렉토리 경로 (기본값: 압축 파일과 같은 이름의 디렉토리)
        
    Returns:
        tuple[bool, str]: (성공 여부, 메시지)
    """
    try:
        # 압축 파일 데이터 준비
        if isinstance(zip_file, (str, Path)):
            zip_path = Path(zip_file) if isinstance(zip_file, str) else zip_file
            
            # 파일이 존재하는지 확인
            if not zip_path.exists():
                return False, f"압축 파일이 존재하지 않습니다: {zip_path}"
            
            # 압축 파일 열기
            zip_data = zipfile.ZipFile(zip_path)
            
            # 출력 디렉토리 설정
            if output_dir is None:
                output_dir = zip_path.parent / zip_path.stem
        elif isinstance(zip_file, bytes):
            # 바이트 데이터에서 압축 파일 열기
            zip_data = zipfile.ZipFile(io.BytesIO(zip_file))
            
            # 출력 디렉토리 설정
            if output_dir is None:
                output_dir = Path("extracted_files")
        else:
            return False, f"지원하지 않는 압축 파일 형식입니다: {type(zip_file)}"
        
        # 출력 디렉토리 생성
        output_path = Path(output_dir) if isinstance(output_dir, str) else output_dir
        output_path.mkdir(parents=True, exist_ok=True)
        
        # 압축 해제
        zip_data.extractall(output_path)
        
        # 압축 파일 닫기
        zip_data.close()
        
        return True, f"압축 파일이 성공적으로 압축 해제되었습니다: {output_path}"
    
    except Exception as e:
        return False, f"압축 해제 중 오류 발생: {str(e)}"

# 사용 예시
if __name__ == "__main__":
    print("file utils start")
    # 파일 체크 예시
    '''
    test_file = "test.txt"
    exists, message = check_file_exists(test_file)
    print(f"파일 체크 결과: {message}")
    
    # 폴더 체크 예시
    test_folder = "test_folder"
    exists, message = check_folder_exists(test_folder)
    print(f"폴더 체크 결과: {message}")
    
    # 일반 경로 체크 예시
    test_path = "some/path"
    exists, message = check_path_exists(test_path)
    print(f"경로 체크 결과: {message}")
    
    # 확장자 변경 예시
    file_path = "example/document.pdf"
    new_path = change_extension(file_path, "txt")
    print(f"확장자 변경: {file_path} -> {new_path}")
    
    # 파일 저장 예시
    # 텍스트 파일 저장
    success, msg = save_file("example/text.txt", "Hello, World!")
    print(f"텍스트 파일 저장: {msg}")
    
    # JSON 파일 저장
    data = {"name": "John", "age": 30, "city": "New York"}
    success, msg = save_file("example/data.json", data, file_format="json")
    print(f"JSON 파일 저장: {msg}")
    
    # CSV 파일 저장
    data = [["Name", "Age", "City"], ["John", 30, "New York"], ["Jane", 25, "Los Angeles"]]
    success, msg = save_file("example/data.csv", data, file_format="csv")
    print(f"CSV 파일 저장: {msg}")
 
    # 압축 파일 예시
    # 샘플 ZIP 파일 생성
    with zipfile.ZipFile("example/sample.zip", "w") as zipf:
        # 텍스트 파일 추가
        with zipf.open("readme.txt", "w") as f:
            f.write(b"This is a sample ZIP file for testing.")
        
        # JSON 파일 추가
        with zipf.open("data.json", "w") as f:
            f.write(json.dumps({"test": "data"}).encode('utf-8'))
    
    # 압축 파일 내용 출력
    success, msg = list_zip_contents("example/sample.zip")
    print(f"압축 파일 내용 출력: {msg}")

    # 압축 파일 압축 해제
    success, msg = extract_zip_to_directory("example/sample.zip", "example/extracted")
    print(f"압축 파일 압축 해제: {msg}") 
    '''