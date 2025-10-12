git pull > error
Updating 2ea653d..fa47e5b
error: Your local changes to the following files would be overwritten by merge:
        sample/test/xx_particle.py
Please commit your changes or stash them before you merge.
Aborting

폐기
C:\bpdt>git checkout -- sample/test/xx_particle.py

## 원격데슼크톱 설정
Set-ItemProperty -Path "HKLM:\System\CurrentControlSet\Control\Terminal Server" -Name "fDenyTSConnections" -Value 0

Enable-NetFirewallRule -DisplayGroup "원격 데스크톱" #영어일 경우 Remote Controll

Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\Terminal Server\Wds\rdpwd\Tds\tcp" -Name PortNumber 10002

Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\Terminal Server\WinStations\RDP-Tcp" -Name PortNumber 10002

New-NetFirewallRule -DisplayName "원격 데스크톱 연결(10002)" -Direction Inbound -Protocol TCP -LocalPort 10002 -Action Allow

net stop TermService

net start TermService

## webview
const webViewScript = `
  (function() {
    const inputs = document.getElementsByTagName('input');
    const textareas = document.getElementsByTagName('textarea');
    const focusableElements = [...inputs, ...textareas];
    window.ReactNativeWebView.postMessage(focusableElements.length > 0 ? 'allow-focus' : 'disallow-focus');
    return true;
  })();
`;

const [focusEnabled, setFocusEnabled] = useState<boolean>(false);

const onMessage = (event: WebViewMessageEvent) => {
  const message = event.nativeEvent.data;
  if (message === 'allow-focus') {
    setFocusEnabled(true);
  } else if (message === 'disallow-focus') {
    setFocusEnabled(false);
  }
}

<WebView
  focusEnabled={focusEnabled}
  injectedJavaScript={webViewScript}
/>

## webview 스크롤
<TouchableWithoutFeedback style={{ flex: 1, paddingHorizontal: 16 }}>
    <WebView
        originWhitelist={['http://', 'https://']}
        style={{ flex: 1, width: '100%' }}
        source={{ uri: '........' }}
        onError={(error) => {
            console.error(error);
        }}
    />
</TouchableWithoutFeedback>
 
const INJECTEDJAVASCRIPT = `
    const meta = document.createElement('meta');
    meta.setAttribute('name', 'viewport');
    meta.setAttribute('content', 'width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0');
    document.getElementsByTagName('head')[0].appendChild(meta);
`;

<WebView
    injectedJavaScript={INJECTEDJAVASCRIPT}
    source={{ uri: 'https://your-website.com' }}
    // Other WebView props
/>