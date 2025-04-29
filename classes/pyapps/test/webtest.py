import os
import sys
from PyQt5.QtWidgets import QApplication, QVBoxLayout, QWidget
from PyQt5.QtCore import QUrl, QEventLoop
from PyQt5.QtWebEngineWidgets import QWebEngineView
from PyQt5.QtWebEngineCore import QWebEngineUrlRequestInterceptor

class WebPage(QWebEngineView):
    def __init__(self):
        QWebEngineView.__init__(self)
        self.current_url = ''
        self.load(QUrl("https://facebook.com"))
        self.loadFinished.connect(self._on_load_finished)
        self.urlChanged.connect(self._on_url_change)
        self.value = '';

    def _on_load_finished(self):
        self.current_url = self.url().toString()

    def _on_url_change(self):
        self.page().runJavaScript("document.getElementsByName('email')[0]", self.store_value)

    def store_value(self, param):
        print(f"store value {param} URL:{self.current_url}");
        self.value = param
        print("Param: " +str(param))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    web = WebPage()
    web.show()
    sys.exit(app.exec_()) 