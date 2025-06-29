
import os
from utils.mylog import logger

current_dir = os.path.dirname(os.path.abspath(__file__))
jkj_model_path = os.path.join(current_dir, "jkj_model.py")

print("## ", os.path)
print("## ", current_dir, jkj_model_path)
logger.info('## test program start')