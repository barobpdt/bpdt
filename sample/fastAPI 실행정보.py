## fastAPI 실행정보
conf('python.path', " %userprofile%/AppData/Local/Programs/Python/Python313", true)
 
c=cmd()
path=conf('python.path' )
pp="$path/python"
papi = "c:/bpdt/sample/fapi.py"
ptest= "c:/bpdt/sample/path_test.py"
c.run("$pp -m pip install fastapi pydantic uvicorn")
c.run("$pp -m pip list")
c.run("$pp $papi")
c.run("netstat -ano | findstr 8000")
c.run("taskkill /f /pid 5080")
c.run("$pp $ptest")

cc=cmd('test')
cc.run('ping 192.168.219.1')
cc.run("$pp $papi")

cm = Baro.process('fapi')
cb = call('cb_fapi')
setCallback(cm, cb)
cm.run("c:/Users/user/AppData/Local/Programs/Python/Python313/python $papi")

c.run("cd ")


~~
<func>
	cb_fapi(type, data) {
		print("fapi $type>>$data ")
	}
</func>
 