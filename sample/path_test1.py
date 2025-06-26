import os
import sys

localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(f'{localPath}/sample') 

s='"@aaa"'
if( s[0] == '"' or s[0] == "'" ):
	print("s[0]==",s[0])
else:
	print("s[0]!==",s[0])


 