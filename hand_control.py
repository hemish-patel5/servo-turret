# Check USB PORT Connected: ls -l /dev/ttyUSB* 2>/dev/null

import argparse
import time
import urllib.request
from pathlib import Path

import cv2
import mediapipe as mp
import numpy as np
import serial

MODEL_URL = ("https://storage.googleapis.com/mediapipe-models/hand_landmarker/"
             "hand_landmarker/float16/1/hand_landmarker.task")
MODEL_PATH = Path(__file__).with_name("hand_landmarker.task")
SERIAL_PORT = "/dev/ttyUSB0"
HORIZONTAL_RANGE = (5, 175)
VERTICAL_RANGE = (5, 130)
SMOOTHING = 0.50
SEND_INTERVAL = 0.02


def ensure_model() -> None:
    if not MODEL_PATH.exists():
        print("Downloading the MediaPipe hand model (once only)...")
        urllib.request.urlretrieve(MODEL_URL, MODEL_PATH)


def map_angle(value: float, angle_range: tuple[int, int], reverse: bool = False) -> float:
    value = float(np.clip(value, 0.0, 1.0))
    if reverse:
        value = 1.0 - value
    return angle_range[0] + value * (angle_range[1] - angle_range[0])


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--camera", type=int, default=0, help="OpenCV camera number")
    args = parser.parse_args()

    ensure_model()
    board = serial.Serial(SERIAL_PORT, 115200, timeout=0.1, write_timeout=0.1)
    time.sleep(2.0)
    board.reset_input_buffer()

    camera = cv2.VideoCapture(args.camera)
    if not camera.isOpened():
        board.close()
        raise RuntimeError(f"Could not open camera {args.camera}")

    options = mp.tasks.vision.HandLandmarkerOptions(
        base_options=mp.tasks.BaseOptions(model_asset_path=str(MODEL_PATH)),
        running_mode=mp.tasks.vision.RunningMode.VIDEO,
        num_hands=1,
        min_hand_detection_confidence=0.6,
        min_tracking_confidence=0.6,
    )
    horizontal, vertical = 90.0, 60.0
    last_send = 0.0
    started = time.monotonic()

    try:
        with mp.tasks.vision.HandLandmarker.create_from_options(options) as detector:
            while True:
                ok, frame = camera.read()
                if not ok:
                    break
                frame = cv2.flip(frame, 1)
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                timestamp_ms = int((time.monotonic() - started) * 1000)
                result = detector.detect_for_video(image, timestamp_ms)

                if result.hand_landmarks:
                    tip = result.hand_landmarks[0][8]
                    # Reverse pan so moving your hand right turns the turret right.
                    target_h = map_angle(tip.x, HORIZONTAL_RANGE, True)
                    target_v = map_angle(tip.y, VERTICAL_RANGE, True)
                    horizontal += SMOOTHING * (target_h - horizontal)
                    vertical += SMOOTHING * (target_v - vertical)
                    now = time.monotonic()
                    if now - last_send >= SEND_INTERVAL:
                        board.write(f"{round(horizontal)},{round(vertical)}\n".encode("ascii"))
                        last_send = now

                    height, width = frame.shape[:2]
                    point = (int(tip.x * width), int(tip.y * height))
                    cv2.circle(frame, point, 12, (0, 255, 0), -1)
                    cv2.putText(frame, f"pan {horizontal:.0f}  tilt {vertical:.0f}",
                                (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.8,
                                (0, 255, 0), 2)
                else:
                    cv2.putText(frame, "Show one hand", (20, 40),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 180, 255), 2)

                cv2.imshow("Hand turret control - Q to quit", frame)
                if cv2.waitKey(1) & 0xFF in (ord("q"), 27):
                    break
    finally:
        camera.release()
        board.close()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
