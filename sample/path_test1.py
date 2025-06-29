import os
import sys
import io

path = 'c:/temp/hellofile.txt'
sys.stdout = open(path, 'w')

localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(f'{localPath}/sample') 
 
sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(f'{localPath}/sample') 

from rich.console import Console
console = Console()
console.input("What is [i]your[/i] [bold red]name[/]? :smiley: ")


print('page end')
