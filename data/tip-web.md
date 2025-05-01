## SVG 
path 
<path d="M10 10 h 80 v 80 h -80 Z" fill="transparent" stroke="black"/>
<path d="M10 10 h 80 v 80 h -80 Z" fill-rule="evenodd" clip-rule="evenodd" />

// 버튼 안의 이미지 스타일 지정
export const ButtonIcon = styled.img<StyledButtonProps>`
  width: 20px;
  height: 20px;
  filter: ${({ isClicked }) => (isClicked ? "brightness(100%)" : "brightness(0%)")};

.icon {
  background: var(--primary-color);
  -webkit-mask-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 129 129'%3E%3Cpath d='m121.3,34.6c-1.6-1.6-4.2-1.6-5.8,0l-51,51.1-51.1-51.1c-1.6-1.6-4.2-1.6-5.8,0-1.6,1.6-1.6,4.2 0,5.8l53.9,53.9c0.8,0.8 1.8,1.2 2.9,1.2 1,0 2.1-0.4 2.9-1.2l53.9-53.9c1.7-1.6 1.7-4.2 0.1-5.8z' /%3E%3C/svg%3E");
  mask-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 129 129'%3E%3Cpath d='m121.3,34.6c-1.6-1.6-4.2-1.6-5.8,0l-51,51.1-51.1-51.1c-1.6-1.6-4.2-1.6-5.8,0-1.6,1.6-1.6,4.2 0,5.8l53.9,53.9c0.8,0.8 1.8,1.2 2.9,1.2 1,0 2.1-0.4 2.9-1.2l53.9-53.9c1.7-1.6 1.7-4.2 0.1-5.8z' /%3E%3C/svg%3E");
} 

## SVG 아이콘 스타일지정
.icon{ /*postion relative for absolute positioning to work*/
  position: relative; 
}
.icon>svg{
  position: absolute;
  top: 0px;
  right: 0px;
  left: 0px;
  bottom: 0px;
  z-index: -1;
}
.icon>svg>path{ /*target the image with css*/
  fill: var(--primary-color);
}

<div class="icon">
  <svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 129 129' id='background'><path d='m121.3,34.6c-1.6-1.6-4.2-1.6-5.8,0l-51,51.1-51.1-51.1c-1.6-1.6-4.2-1.6-5.8,0-1.6,1.6-1.6,4.2 0,5.8l53.9,53.9c0.8,0.8 1.8,1.2 2.9,1.2 1,0 2.1-0.4 2.9-1.2l53.9-53.9c1.7-1.6 1.7-4.2 0.1-5.8z'/> </svg>
  <p>Text goes here...</p>
</div>

3. filter
3.1. filter란?
요소(보통 <img>)에 대한 시각 효과 (예: 흐림/채도 등) 정의

기본값: none
상속여부: X
애니효과: O
CSS버전: CSS3
JS구문: object .style.filter = "grayscale (100 %)";



3.2. filter 구문
selector {filter: none | blur() | brightness() | contrast() | drop-shadow() | grayscale() | hue-rotate() | invert() | opacity() | saturate() | sepia() | url() | initial | inherit ; }




3.3. 속성값
none

효과 지정 안 함. (기본값)
blur (px)

이미지에 흐림 효과 적용
값이 클수록 더 흐려짐. 값 지정 안 하면 0 사용
brightness(%)

이미지의 밝기 조정
0 % : 이미지를 완전히 검정색으로 만듦
100 % (1) : 원본 이미지. (기본값)
100 % 초과 : 더 밝은 이미지
contrast(%)

이미지의 대비 조정
0 % : 이미지를 완전히 검정색으로 만듦
100 % (1) : 원본 이미지. (기본값)
100 % 초과 : 대비를 더 높여 결과를 제공.
grayscale(%)

이미지를 회색조로 변환. (흑백이미지 만듦)
0% (0): 원본이미지. (기본값)
100% : 완전 회색이미지. (흑백이미지에 사용)
hue-rotate(deg)

이미지에 색조 회전 적용.
이 값은 이미지 샘플이 조정될 색상 원 주위의 각도 수를 정의
0deg : 원본이미지. (기본값)
참고 : 최대값은 360deg
invert (%)

이미지의 샘플을 반전시킴
0 % (0) : 원본이미지. (기본값)
100 % : 이미지를 완전히 반전시킴
참고 : 음수값 비허용
invert (%)

이미지의 샘플을 반전시킴
0 % (0) : 원본이미지. (기본값)
100 % : 이미지를 완전히 반전시킴
참고 : 음수값 비허용
opacity(%)

이미지의 불투명도 수준
0 % : 완전 투명
100 % (1) : 완전 불투명 (=원본 이미지). (기본값)
참고 : 음수값 비허용
url ()

SVG 필터를 지정하는 XML 파일의 위치를 취하며, 특정 필터 요소에 대한 앵커를 포함 가능
filter: url (svg-url # element-id)
initial

이 속성의 기본값으로 설정
inherit

부모요소의 속성값 상속
