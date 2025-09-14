@python.cmdPip('pip install webdriver_manager')
@python.cmdPip('pip install selenium')
@python.cmdPip('pip install flask')
@python.cmdPip('pip list')

@python.cmdExec(#[##> exec:
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.action_chains import ActionChains

options = Options()
options.add_experimental_option("detach", True)
# options.add_argument("--window-size = x,y")
options.add_argument('--disable-popup-blocking')
 
driver = webdriver.Chrome(options=options)
driver.implicitly_wait(3)
driver.get(url='https://google.com')

# class name으로 찾기
driver.find_element(By.CLASS_NAME,'gLFyf')
# tag name으로 찾기
driver.find_element(By.TAG_NAME,'textarea')
# id로 찾기
el = driver.find_element(By.ID,'APjFqb')

# 클릭하기
el.click()
# 값 입력하기
el.send_keys("tistory")
# 키보드 입력하기
el.send_keys(Keys.ENTER)
# iframe 이동
driver.switch_to.frame(' iframe id ')
driver.switch_to.default_content()
# 붙여넣기
ActionChains(driver).key_down(Keys.COMMAND).send_keys('v').key_up(Keys.COMMAND).perform()

log(f'pageSource: url => {driver.page_source}')
])