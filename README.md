# bpdt
baro project developer tool

[diff] tool = winmerge

 

[merge] tool = winmerge

 

[difftool "WinMerge"]

    cmd = \"C:\\Program Files\\WinMerge\\WinMergeU.exe\" -e -u -dl \"Old $BASE\" -dr \"New $BASE\" \"$LOCAL\" \"$REMOTE\"

    trustExitCode = true

 

[mergetool "WinMerge"]

    cmd = \"C:\\Program Files\\WinMerge\\WinMergeU.exe\" -e -u -dl \"Local\" -dm \"Base\" -dr \"Remote\" \"$LOCAL\" \"$BASE\" \"$REMOTE\" -o \"$MERGED\"

    trustExitCode = true

    keepBackup = false

[노트패드 설정]
    git config --global core.editor "'C:/Program Files/Notepad++/notepad++.exe' -multiInst -nosession"


[설정 확인]
    git config --list
    git config user.name 

[저장소 생성]  
    git remote add origin [repository 주소]
    git remote -v
    git push -u orign master
    

[챠트 참조사이트]
    https://echarts.apache.org/examples/en/editor.html?c=line-race
    
[주식 참조사이트]
 https://github.com/hyunyulhenry/quant_py/blob/main/data_korea.ipynb
	
	
