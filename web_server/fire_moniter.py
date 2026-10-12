from collections import deque
import cv2
import numpy as np
from picamera2 import Picamera2


# 노출 고정 (마이크로초). 얼굴/천장은 어둡고 불꽃만 밝게 보이도록 조정
EXPOSURE_US = 2000

# 이 밝기(0~255) 이상인 픽셀만 "빛나는 것"으로 인정
BRIGHT_MIN = 230

# 빛나는 픽셀 수 범위: 너무 적으면 노이즈, 너무 많으면 불이 아닌 큰 물체
MIN_PIXELS = 30
MAX_PIXELS = 8000

# 최근 HISTORY 프레임 중 VOTE_MIN 프레임 이상이 불꽃이면 True (깜빡임 방지)
HISTORY = 10
VOTE_MIN = 4

# 모니터링 화면 밝기
DISPLAY_GAMMA = 0.3

_picam2 = None
_show_window = True
_last_display = None
_vote_hist = deque(maxlen=HISTORY)
_kernel = np.ones((3, 3), np.uint8)
_gamma_lut = np.array([((i / 255.0) ** DISPLAY_GAMMA) * 255 for i in range(256)],
                      dtype="uint8")


def _get_camera():
    """카메라는 처음 한 번만 켜고 계속 재사용"""
    global _picam2
    if _picam2 is None:
        _picam2 = Picamera2()
        config = _picam2.create_preview_configuration(main={"size": (640, 480)})
        _picam2.configure(config)
        _picam2.start()
        _picam2.set_controls({"AeEnable": False,
                              "ExposureTime": EXPOSURE_US,
                              "AnalogueGain": 1.0})
    return _picam2


def get_display_frame():
    """가장 최근 프레임의 밝게 보정된 모니터링용 화면(BGR)을 반환
    check_flame()을 한 번 이상 호출한 뒤에 사용 가능, 없으면 None"""
    return _last_display


def check_flame(show=False):
    
    global _show_window, _last_display

    frame = _get_camera().capture_array()
    frame_bgr = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
    gray = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)   # 감지는 어두운 원본으로

    _, mask = cv2.threshold(gray, BRIGHT_MIN, 255, cv2.THRESH_BINARY)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, _kernel)
    n_pixels = cv2.countNonZero(mask)

    candidate = MIN_PIXELS <= n_pixels <= MAX_PIXELS

    _vote_hist.append(candidate)
    detected = sum(_vote_hist) >= VOTE_MIN

    display = cv2.LUT(frame_bgr, _gamma_lut)
    _last_display = display

    # 창 표시 + q 키로 종료
    if show and _show_window:
        color = (0, 0, 255) if detected else (0, 255, 0)
        cv2.putText(display, f"px:{n_pixels}", (10, 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
        if detected:
            cv2.putText(display, "FIRE DETECTED", (10, 55),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
        cv2.imshow("moniter", display)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            cv2.destroyAllWindows()
            _show_window = False

    return {"fire": detected}


def close_camera():
    global _picam2
    if _picam2 is not None:
        _picam2.stop()
        _picam2 = None


if __name__ == "__main__":
    try:
        while _show_window:
            print(check_flame(show=True))
    finally:
        close_camera()
        cv2.destroyAllWindows()