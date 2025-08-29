## 유튜브에서 mp3추출
```python
import os
from pytubefix import YouTube

def downloadYouTube(videourl, path):
    yt = YouTube(videourl)
    yt = yt.streams.filter(progressive=True, file_extension='mp4').order_by('resolution').desc().first()
    if not os.path.exists(path):
        os.makedirs(path)
    yt.download(path)

video_path = 'https://youtu.be/D9syciL3Xsg?si=ek8gwbWBnvZniotV'
downloadYouTube(video_path, 'videos'
```
