# remove_bg.py
import sys
import argparse
import cv2
import numpy as np
from PIL import Image

def estimate_border_bg_mask(img, sample_width=10, color_thresh=30):
    h, w = img.shape[:2]
    # 샘플링 영역: 상하좌우 가장자리 strip
    strips = []
    strips.append(img[0:sample_width, :, :])         # top
    strips.append(img[h-sample_width:h, :, :])       # bottom
    strips.append(img[:, 0:sample_width, :])         # left
    strips.append(img[:, w-sample_width:w, :])       # right
    samples = np.vstack([s.reshape(-1, 3) for s in strips])
    # 평균 색상 (BGR)
    mean_color = samples.mean(axis=0)
    # 색상 거리
    diff = np.linalg.norm(img.reshape(-1,3) - mean_color, axis=1)
    diff = diff.reshape(h, w)
    # 초기 확률적 배경: 색 거리 작으면 배경일 가능성 높음
    bg_mask = diff < color_thresh
    return bg_mask.astype('uint8')

def remove_background_grabcut(img_bgr, iter_count=5):
    h, w = img_bgr.shape[:2]
    # 초기 배경 추정 마스크
    prob_bg = estimate_border_bg_mask(img_bgr, sample_width=max(5, min(h,w)//40), color_thresh=30)
    # GrabCut 마스크 초기화 (0,1,2,3 used)
    mask = np.full((h, w), cv2.GC_PR_FGD, dtype=np.uint8)  # default probable fg
    mask[prob_bg == 1] = cv2.GC_PR_BGD
    # 모델들
    bgdModel = np.zeros((1,65), np.float64)
    fgdModel = np.zeros((1,65), np.float64)
    # run grabCut using mask mode
    cv2.grabCut(img_bgr, mask, None, bgdModel, fgdModel, iterCount=iter_count, mode=cv2.GC_INIT_WITH_MASK)
    # 결과: 확정 전경/확정 혹은 잠정 전경
    mask2 = np.where((mask==cv2.GC_FGD) | (mask==cv2.GC_PR_FGD), 255, 0).astype('uint8')
    return mask2

def apply_alpha_save(img_bgr, alpha_mask, out_path):
    # alpha_mask: 0/255 single channel
    b, g, r = cv2.split(img_bgr)
    rgba = cv2.merge((b, g, r, alpha_mask))
    # OpenCV.imwrite can write PNG with alpha, but to be safe use PIL
    rgba = cv2.cvtColor(rgba, cv2.COLOR_BGRA2RGBA)
    pil = Image.fromarray(rgba)
    pil.save(out_path, format='PNG')

def main():
    parser = argparse.ArgumentParser(description='이미지 배경 제거 (GrabCut 자동 초기화)')
    parser.add_argument('input', help='입력 이미지 파일 경로')
    parser.add_argument('output', nargs='?', default='output.png', help='출력 PNG 파일 경로 (기본: output.png)')
    parser.add_argument('--iter', type=int, default=5, help='GrabCut 반복 횟수 (기본: 5)')
    args = parser.parse_args()

    img = cv2.imread(args.input, cv2.IMREAD_COLOR)
    if img is None:
        print('이미지를 열 수 없습니다:', args.input)
        sys.exit(1)

    mask = remove_background_grabcut(img, iter_count=args.iter)
    apply_alpha_save(img, mask, args.output)
    print('저장됨:', args.output)

if __name__ == '__main__':
    main()