1. 토큰 발급 받기

​

​우선 token으로 로그인 하기 위해서 github에서 token을 발급 받는다. github 홈페이지에서 아래 경로로 들어가야 한다.

[Settings] -> [Developer Settings] -> [Personal access tokens]

#!/bin/bash
today=`date +%Y-%m-%d`

# 저장소 초기화 할 때 주석 풀고 사용
#rm -rf .git
#git init
#git remote add origin https://github.com/[id]/[repository].git

git add .
git commit -m $today

#git push origin master -f # forced push
git push origin master


#로컬파일 폐기
git checkout -- 파일명경로