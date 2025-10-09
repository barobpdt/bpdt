include("classes/common/jobs")
@job.start()

@python.cmdExec(#[##>exec:
ac=ApiConfig()
cc=ac.CommCode
a=cc.children['test']
CommCode('01','codeValue',title='admin',parent=a)
CommCode('02','codeValue',title='user',parent=a)
CommCode('03','codeValue',title='other',parent=a)
log(f'print:a=={a.dump()}')
])
~~
@python.cmdExec(#[##>exec:
ac=ApiConfig()
val = ac.codeValue('test','03')
log(f'print:a=={ac.CommCode.dump()} {val}')
])
~~
@python.cmdExec(#[##>exec:
from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
import sqlalchemy as sa
import sqlalchemy.sql as ss
from sqlalchemy import create_engine
from sqlalchemy import ForeignKey
from sqlalchemy import select
from sqlalchemy import String

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm import selectinload
from sqlalchemy.orm import Session
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.pool import NullPool
from sqlalchemy.ext.mutable import MutableDict	
import threading

class Base(DeclarativeBase):
	pass
	
class CommCode(MappedAsDataclass, Base):
	__tablename__ = "comm_code"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("comm_code.id"), init=False
	)
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=sa.func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=sa.func.now())

	children: Mapped[Dict[str, CommCode]] = relationship(
		cascade="all, delete-orphan",
		back_populates="parent",
		collection_class=attribute_keyed_dict("code"),
		init=False,
		repr=False,
	)

	parent: Mapped[Optional[CommCode]] = relationship(
		back_populates="children", remote_side=id, default=None
	)

	def __init__(self, code, type, **kwargs):
		data = kwargs['data'] if 'data' in kwargs else None
		if isinstance(data, dict):
			self.data = data
		elif data:
			self.data = json.loads(data)
		self.code = code
		self.type = type
		self.title = kwargs['title']
		self.ref = kwargs['ref'] if 'ref' in kwargs else ''
		self.parent = kwargs['parent'] if 'parent' in kwargs else None
		print(f'init {self} {kwargs}')
	def childCode(self, code):
		try:
			return self.children[code]
		except Exception e:
			print(f'@@CC childCode code:{code} >>exception:{e}')
		return None
	def codeTitle(self, code):
		p=self
		for c in code.split('.'):
			p=p.childCode(c)
			if p:
				return p.title
	def __repr__(self):
		return f"CC(code={self.code},title={self.title})"

	def dump(self, indent: int = 0) -> str:
		return (
			"   " * indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(indent + 1) for c in self.children.values()])
		)

import os
import time 
import threading

class ApiConfig:
	_instance = None
	_lock = threading.Lock()
	def __new__(cls, *args, **kwargs):
		if not cls._instance:
			with cls._lock:
				if not cls._instance:
					cls._instance = super().__new__(cls)
		return cls._instance

	def __init__(self):
		if hasattr(self, '_initialized'):
			return
		self.commCode = None	# setCommCode
		self.logger = None
		self.fpIn = None		# setInPath
		self.inFileLastPos = 0
		self.inFilePath = ''
		self.nextCommand = ''
		self.fpOut = None		# setLogPath
		self.startTm = time.time()
		self._initialized = True

	def log(self, msg):
		try:
			if self.fpOut:
				self.fpOut.write(f"##> {msg}\n")
				self.fpOut.flush()
		except Exception as e:
			pass

	def setInPath(self, path)
		try:
			if self.fpIn:
				self.fpIn.close()
			self.fpIn=open(path, 'r', encoding='utf8')
			self.inFileLastPos = fpIn.seek(0, os.SEEK_END)
			self.inFilePath = path
		except Exception as e:
			pass
			
	def getInData(self):
		if not self.fpIn:
			return
		commands = ''
		try:
			fsize=os.stat(self.inFilePath).st_size
			if fsize > self.inFileLastPos:
				commands = self.fpIn.read().strip()
				self.inFileLastPos = fsize
		except Exception as e:
			self.log(f'print: getInData exception {e}')
		return commands
			
	def setLogPath(self, path):
		try:
			if self.fpOut:
				self.fpOut.close()
			self.fpOut=open(path, 'a', encoding='utf8')
		except Exception as e:
			pass
	def getLogger(self, name):
		if self.logger:
			return self.logger
			
		"""일자별 로그 설정"""
		# 로그 디렉토리 생성
		log_dir = "logs"
		if not os.path.exists(log_dir):
			os.makedirs(log_dir)
		
		# 로거 설정
		logger = logging.getLogger(name)
		logger.setLevel(logging.INFO)
		
		# 콘솔 핸들러
		console_handler = logging.StreamHandler()
		console_handler.setLevel(logging.INFO)
		console_formatter = logging.Formatter(
			'%(asctime)s - %(name)s - %(levelname)s - %(message)s'
		)
		console_handler.setFormatter(console_formatter)
		
		# 파일 핸들러 (일자별 로테이션)
		file_handler = TimedRotatingFileHandler(
			filename=os.path.join(log_dir, "fastapi_logger.log"),
			when="midnight",
			interval=1,
			backupCount=30,  # 30일간 보관
			encoding='utf-8'
		)
		file_handler.setLevel(logging.INFO)
		file_formatter = logging.Formatter(
			'%(asctime)s - %(name)s - %(levelname)s - %(funcName)s:%(lineno)d - %(message)s'
		)
		file_handler.setFormatter(file_formatter)
		
		# 핸들러 추가
		logger.addHandler(console_handler)
		logger.addHandler(file_handler)
		
		return logger	
			
	def setCommCode(self, root):
		self.commCode=root

	def getCodeTitle(self, code):
		if self.cc is None:
			return ''
		return self.cc.getTitle(code)

root=CommCode('root','base', title="rootnode")
CommCode('test','base', title="test", parent=root)
ac.setCommCode(root)
log(f'print:bi=={bi}')

])


##
from __future__ import annotations
from typing import Dict
from typing import Optional
from typing import List
from datetime import datetime 
import sqlalchemy as sa
import sqlalchemy.sql as ss
from sqlalchemy import create_engine
from sqlalchemy import ForeignKey
from sqlalchemy import select
from sqlalchemy import String

from sqlalchemy.orm import DeclarativeBase
from sqlalchemy.orm import Mapped
from sqlalchemy.orm import mapped_column
from sqlalchemy.orm import MappedAsDataclass
from sqlalchemy.orm import relationship
from sqlalchemy.orm import selectinload
from sqlalchemy.orm import Session
from sqlalchemy.orm.collections import attribute_keyed_dict
from sqlalchemy.pool import NullPool
from sqlalchemy.ext.mutable import MutableDict	
import threading
import json

class Base(DeclarativeBase):
	pass
	
class CC(MappedAsDataclass, Base):
	__tablename__ = "cc"

	id: Mapped[int] = mapped_column(primary_key=True, init=False)
	parent_id: Mapped[Optional[int]] = mapped_column(
		ForeignKey("cc.id"), init=False
	)
	code: Mapped[str]
	title: Mapped[str]
	type: Mapped[str]
	ref: Mapped[Optional[str]] = mapped_column(nullable=True)
	data: Mapped[dict|None] = mapped_column(MutableDict.as_mutable(sa.JSON))
	created_at: Mapped[datetime] = mapped_column(server_default=sa.func.now())
	updated_at: Mapped[datetime | None] = mapped_column(onupdate=sa.func.now())

	children: Mapped[Dict[str, CC]] = relationship(
		cascade="all, delete-orphan",
		back_populates="parent",
		collection_class=attribute_keyed_dict("code"),
		init=False,
		repr=False,
	)

	parent: Mapped[Optional[CC]] = relationship(
		back_populates="children", remote_side=id, default=None
	)

	def __init__(self, code, type, **kwargs):
		data = kwargs['data'] if 'data' in kwargs else None
		if isinstance(data, dict):
			self.data = data
		elif data:
			self.data = json.loads(data)
		self.code = code
		self.type = type
		self.title = kwargs['title']
		self.ref = kwargs['ref'] if 'ref' in kwargs else ''
		self.parent = kwargs['parent'] if 'parent' in kwargs else None
		print(f'init {self} {kwargs}')

	def getChild(self, code):
		try:
			return self.children[code]
		except Exception as e:
			print(f'@@CC childCode code:{code} >>exception:{e}')
		return None
	
	def getTitle(self, code):
		p=self
		try:
			for c in code.split('.'):
				p=p.getChild(c)
		except Exception as e:
			pass
		if p:
			return p.title
		
	def __repr__(self):
		return f"CC(code={self.code},title={self.title})"

	def dump(self, indent: int = 0) -> str:
		return (
			"   " * indent
			+ repr(self)
			+ "\n"
			+ "".join([c.dump(indent + 1) for c in self.children.values()])
		)