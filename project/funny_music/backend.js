https://ourhertz.tistory.com/117

eas build --platform android --profile production
출처: https://ourhertz.tistory.com/119 [우리들의 주파수:티스토리]

package com.example.app;

public class BaseWebView extends WebView {
	int inputType = EditorInfo.TYPE_NULL;

    public void setInputType(int type) {
        inputType = type;
    }

    public int getInputType() {
        return inputType;
    }
...

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        InputConnection ic = super.onCreateInputConnection(outAttrs);


        int original = outAttrs.imeOptions;

        int notMask = ~EditorInfo.IME_MASK_ACTION;

        int maskResult = original & notMask;

        if((outAttrs.inputType & EditorInfo.TYPE_MASK_CLASS) == EditorInfo.TYPE_CLASS_NUMBER){
            Log.d(TAG,IME Options is Number);
            outAttrs.privateImeOptions=defaultInputmode=numeric;;
        } else {
            Log.d(TAG,IME Options is English);
            outAttrs.privateImeOptions=defaultInputmode=english;;
        }

        return ic;
    }
	
/*
keyboardOpen — 키보드가 열림으로 전환된 즉시
keyboardClose — 키보드가 닫힘으로 전환된 즉시
keyboardStableClose — 닫힘 이후 연속 N 프레임(실험값: 6) 동안 뷰포트 높이가 동일해 완전히 안정되었을 때

2. 키보드를 감지하는 방법
1) 포커스 이벤트 + 리사이즈로 감지하는 방법
입력 가능한 요소의 focus/blur를 열림/닫힘 가능성 신호로 보고, 뷰포트 리사이즈 변화를 함께 관찰해 실제 열림 여부를 보정하는 방식이다.
(1) 아이디어
모바일 환경에서는 사용자가 input, textarea, 혹은 contenteditable 요소에 포커스를 주면 대체로 가상 키보드가 열린다.반대로 포커스를 잃으면(blur) 가상 키보드가 닫힌다고 추정할 수 있다.

focus 발생 → 키보드 열림으로 추정
blur 발생 → 키보드 닫힘으로 추정

 
또한 다음과 같은 보정 규칙을 덧붙이면 감지 시점이 더 명확해진다.

대상 필터링: 포커스된 요소가 실제로 키보드를 유도하는 입력기인지 확인한다.

허용: input[type=text|email|tel|number|search|url|password], textarea, contenteditable=true
제외: readonly, disabled, input[type=button|checkbox|radio] 등

function isTextInput(el: EventTarget | null): boolean {
  if (!(el instanceof HTMLElement)) return false;
  if (el instanceof HTMLTextAreaElement) return !el.readOnly && !el.disabled;
  if (el instanceof HTMLInputElement) {
    if (el.readOnly || el.disabled) return false;
    return ['text','email','tel','url','search','number','password'].includes(el.type);
  }
  return el.hasAttribute('contenteditable');
}
 

다음 포커스 대기: blur 직후 짧은 타임아웃(예: 100~200ms) 동안 다음 focus가 이어지면 여전히 키보드 열림 상태로 본다(인풋 간 포커스 이동 케이스).

let blurTimer: number | null = null;

document.addEventListener('focusin', (e) => {
  if (!isTextInput(e.target)) return;
  if (blurTimer) { clearTimeout(blurTimer); blurTimer = null; }
});

document.addEventListener('focusout', (e) => {
  if (!isTextInput(e.target)) return;
  if (blurTimer) clearTimeout(blurTimer);
  blurTimer = window.setTimeout(() => { blurTimer = null; }, 100);
});

const resizeTarget = window.visualViewport ?? window;
resizeTarget.addEventListener('resize', onResize, { passive: true });
출처: https://mong-blog.tistory.com/entry/JS-모바일-웹뷰에서-가상-키보드-감지하는-법-visualViewport·디바운스·rAF [Mong dev blog:티스토리]


//input을 이용한 show
05-24 09:53:41.362 13828 13828 D LatinIME: onStartInput = 24578: rkr.simplekeyboard.inputmethod
05-24 09:53:41.362 13828 13828 D LatinIME: onStartInput editorInfo.inputyType 24578: rkr.simplekeyboard.inputmethod
05-24 09:53:41.363 13828 13828 D LatinIME: executePendingImsCallback editorInfo.inputyType 24578: rkr.simplekeyboard.inputmethod
05-24 09:53:41.363 13828 13828 D LatinIME: onStartInputInternal editorInfo inputType24578: rkr.simplekeyboard.inputmethod
05-24 09:53:41.364 13828 13828 D LatinIME: onShowInputRequested: rkr.simplekeyboard.inputmethod
05-24 09:53:41.364 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
05-24 09:53:41.364 13828 13828 D LatinIME: onEvaluateFullscreenMode: rkr.simplekeyboard.inputmethod
05-24 09:53:41.364 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
05-24 09:53:41.371 13828 13828 D LatinIME: updateFullscreenMode: rkr.simplekeyboard.inputmethod
05-24 09:53:41.371 13828 13828 D LatinIME: updateSoftInputWindowLayoutParameters: rkr.simplekeyboard.inputmethod
05-24 09:53:41.375 13828 13828 D LatinIME: onStartInputView: rkr.simplekeyboard.inputmethod
05-24 09:53:41.376 13828 13828 D LatinIME: executePendingImsCallback editorInfo.inputyType 24578: rkr.simplekeyboard.inputmethod
05-24 09:53:41.395 13828 13828 I LatinIME: Starting input. Cursor position = 1,1: rkr.simplekeyboard.inputmethod
05-24 09:53:41.395 13828 13828 D LatinIME: onEvaluateFullscreenMode: rkr.simplekeyboard.inputmethod
05-24 09:53:41.395 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
05-24 09:53:41.395 13828 13828 D LatinIME: updateFullscreenMode: rkr.simplekeyboard.inputmethod
05-24 09:53:41.395 13828 13828 D LatinIME: updateSoftInputWindowLayoutParameters: rkr.simplekeyboard.inputmethod
05-24 09:53:41.396 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
05-24 09:53:41.411 13828 13828 D LatinIME: loadSettings: rkr.simplekeyboard.inputmethod
05-24 09:53:41.422 13828 13828 D LatinIME: getCurrentAutoCapsState: rkr.simplekeyboard.inputmethod
05-24 09:53:41.422 13828 13828 D LatinIME: getCurrentRecapitalizeState: rkr.simplekeyboard.inputmethod
05-24 09:53:41.426 13828 13828 D LatinIME: shouldShowLanguageSwitchKey: rkr.simplekeyboard.inputmethod
05-24 09:53:41.487 13828 13828 D LatinIME: onComputeInsets: rkr.simplekeyboard.inputmethod
05-24 09:53:41.487 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
05-24 09:53:41.498 13828 13828 D LatinIME: onComputeInsets: rkr.simplekeyboard.inputmethod
05-24 09:53:41.498 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod



//input hide
05-24 09:54:38.197 13828 13828 D LatinIME: hideWindow: rkr.simplekeyboard.inputmethod
05-24 09:54:38.197 13828 13828 D LatinIME: isShowingOptionDialog: rkr.simplekeyboard.inputmethod
05-24 09:54:38.197 13828 13828 D LatinIME: onFinishInputView: rkr.simplekeyboard.inputmethod
05-24 09:54:38.197 13828 13828 D LatinIME: onFinishInputViewInternal: rkr.simplekeyboard.inputmethod
05-24 09:54:38.200 13828 13828 D LatinIME: onWindowHidden: rkr.simplekeyboard.inputmethod
05-24 09:54:38.202 13828 13828 D LatinIME: updateFullscreenMode: rkr.simplekeyboard.inputmethod
05-24 09:54:38.202 13828 13828 D LatinIME: updateSoftInputWindowLayoutParameters: rkr.simplekeyboard.inputmethod
05-24 09:54:38.230 13828 13828 D LatinIME: onComputeInsets: rkr.simplekeyboard.inputmethod
05-24 09:54:38.230 13828 13828 D LatinIME: isImeSuppressedByHardwareKeyboard: rkr.simplekeyboard.inputmethod
*/
...	