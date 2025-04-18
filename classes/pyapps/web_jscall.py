import os
import sys
from PyQt5 import QtCore, QtWidgets, QtWebEngineWidgets, QtWebChannel
from jinja2 import Template

''' form.html
<html>
    <body>
        <form>
            Number 1:<input type="text" id="num1">
            <br>
            Number 2:<input type="text" id="num2">
            <br>
            <input type="submit" id="sub1">
        </form>
    </body>
</html>
'''

class Element(QtCore.QObject):
    def __init__(self, name, parent=None):
        super(Element, self).__init__(parent)
        self._name = name

    @property
    def name(self):
        return self._name

    def script(self):
        raise NotImplementedError


class FormObject(Element):
    numbersChanged = QtCore.pyqtSignal(str, str)

    def script(self):
        _script = r"""
        var btn = document.getElementById('sub1');
        btn.addEventListener("click", function(event){
            var number1 = document.getElementById('num1');
            var number2 = document.getElementById('num2');
            {{name}}.update(number1.value , number2.value);
        });
        """
        return Template(_script).render(name=self.name)

    @QtCore.pyqtSlot(str, str)
    def update(self, number1, number2):
        self.numbersChanged.emit(number1, number2)


class WebEnginePage(QtWebEngineWidgets.QWebEnginePage):
    def __init__(self, *args, **kwargs):
        super(WebEnginePage, self).__init__(*args, **kwargs)
        self.loadFinished.connect(self.onLoadFinished)
        self._objects = []

    def add_object(self, obj):
        self._objects.append(obj)
    def javaScriptConsoleMessage(self, level, msg, linenumber, source_id):
        try:
            print('%s:%s: %s' % (source_id, linenumber, msg))
        except OSError:
            pass

    @QtCore.pyqtSlot(str)
    def print(self, text):
        print('From JS:', text)
        
    @QtCore.pyqtSlot(bool)
    def onLoadFinished(self, ok):
        if ok:
            self.load_qwebchannel()
            self.load_objects()

    def load_qwebchannel(self):
        file = QtCore.QFile(":/qtwebchannel/qwebchannel.js")
        if file.open(QtCore.QIODevice.ReadOnly):
            content = file.readAll() 
            file.close()
            self.runJavaScript(content.data().decode())
        if self.webChannel() is None:
            channel = QtWebChannel.QWebChannel(self)
            self.setWebChannel(channel)

    def load_objects(self):
        if self.webChannel() is not None:
            objects = {obj.name: obj for obj in self._objects}
            self.webChannel().registerObjects(objects)
            _script = r"""
            {% for obj in objects %}
            var {{obj}};
            {% endfor %}
            new QWebChannel(qt.webChannelTransport, function (channel) {
            {% for obj in objects %}
                {{obj}} = channel.objects.{{obj}};
            {% endfor %}
            }); 
            """
            self.runJavaScript(Template(_script).render(objects=objects.keys()))
            for obj in self._objects:
                if isinstance(obj, Element):
                    self.runJavaScript(obj.script())


class WebPage(QtWebEngineWidgets.QWebEngineView):
    def __init__(self, parent=None):
        super().__init__(parent)

        page = WebEnginePage(self)
        self.setPage(page)

        formobject = FormObject("formobject", self)
        formobject.numbersChanged.connect(self.on_numbersChanged)
        page.add_object(formobject)
 
        filepath = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "form.html")
        )
        print(f'filepath={filepath}')
        self.load(QtCore.QUrl.fromLocalFile(filepath))

    @QtCore.pyqtSlot(str, str)
    def on_numbersChanged(self, number1, number2):
        print(number1, number2)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    web = WebPage()
    web.show()
    sys.exit(app.exec_())