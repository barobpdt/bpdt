from rembg import remove, new_session 
from PIL import Image

img_path = "c:/bpdt/data/sprites/ani01.jpg"
out_path = 'c:/bpdt/data/sprites/ani01.png'

img = Image.open(img_path)

model_name = "isnet-general-use"  # 여기에 모델 이름을 넣자
session = new_session(model_name)
out = remove(img, session=session)
out.save(out_path)