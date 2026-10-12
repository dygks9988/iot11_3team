# =========================================================
# Import
# =========================================================

from flask import Flask, request, render_template, jsonify, Response

from sqlalchemy import create_engine, select, func
from sqlalchemy.engine import URL
from sqlalchemy.orm import Session
from models import DeviceLog, Device
from datetime import datetime

import threading
import serial
import time

from collections import deque
import cv2
import numpy as np
from picamera2 import Picamera2


# =========================================================
# 공유 상태
# =========================================================

fire_detected = False

sensor_values_modbus = []
motor_rpm = [0, 0, 0, 0]
robot_state = 0
web_robot_cmd = None

_picam2 = None

latest_frame = None
frame_lock = threading.Lock()

web_robot_cmd = None
# =========================================================
# Flask / DB
# =========================================================

app = Flask(__name__)

db_url = URL.create(
    "mysql+pymysql",
    username="robot",
    password="robot123",
    host="127.0.0.1",
    database="robot_db",
    query={"charset": "utf8mb4"},
)
engine = create_engine(db_url)

# engine = create_engine("mysql+pymysql://robot:robot123@127.0.0.1/robot_db?charset=utf8mb4")


@app.route("/dashboard")
def dashboard():
    return render_template("dashboard.html")

@app.route("/video_feed")
def video_feed():

    def generate():

        while True:

            with frame_lock:
                frame = latest_frame

            if frame is None:
                time.sleep(0.05)
                continue

            yield (
                b"--frame\r\n"
                b"Content-Type: image/jpeg\r\n\r\n"
                + frame
                + b"\r\n"
            )

            time.sleep(0.03)

    return Response(
        generate(),
        mimetype="multipart/x-mixed-replace; boundary=frame"
    )


@app.get("/api/fire")
def fire_api():
    return jsonify({
        "fire": fire_detected
    })

@app.get("/api/modbus")
def modbus_api():
    return jsonify({
        "sensors": sensor_values_modbus,
        "motor_rpm": motor_rpm,
        "robot_state": robot_state
    })


# 로봇 제어 로직
@app.post("/api/robot/start")
def robot_start():
    global web_robot_cmd

    web_robot_cmd = MOTOR_ON

    return jsonify({
        "result": "start requested"
    })

@app.post("/api/robot/stop")
def robot_stop():

    global web_robot_cmd

    web_robot_cmd = MOTOR_OFF

    return jsonify({
        "result": "stop requested"
    })

@app.post("/api/robot/toggle")
def robot_toggle():
    global web_robot_cmd

    if robot_state == 1:
        web_robot_cmd = MOTOR_OFF
        target_state = 0
    else:
        web_robot_cmd = MOTOR_ON
        target_state = 1

    return jsonify({
        "result": "toggle requested",
        "target_state": target_state
    })


@app.get("/api/sensors")
def sensor_api():

    sensor_values = []

    with Session(engine) as db_session:

        devices = db_session.scalars(
            select(Device).limit(4)
        ).all()

        for device in devices:

            value_codes = db_session.scalars(
                select(DeviceLog.value_code)
                .where(DeviceLog.device_seq == device.device_seq)
                .distinct()
            ).all()

            for value_code in value_codes:

                value = get_latest_sensor_value(
                    device.device_seq,
                    value_code
                )

                sensor_values.append({
                    "device_seq": device.device_seq,
                    "device_name": device.device_name,
                    "value_code": value_code,
                    "value": value
                })

    return jsonify(sensor_values)


def get_latest_sensor_value(device_seq, value_code):

    with Session(engine) as db_session:

        value = db_session.scalar(
            select(DeviceLog.value)
            .where(
                DeviceLog.device_seq == device_seq,
                DeviceLog.value_code == value_code
            )
            .order_by(DeviceLog.recorded_at.desc())
            .limit(1)
        )

        if value is None:
            return None

        return float(value)


def get_sensor_chart(device_seq, value_name, selected_date=None):

    with Session(engine) as db_session:

        if selected_date is None:
            target_date = func.current_date()
        else:
            target_date = selected_date

        stmt = (
            select(

                func.hour(DeviceLog.recorded_at).label("hour"),
                func.minute(DeviceLog.recorded_at).label("minute"),

                func.round(
                    func.avg(DeviceLog.value),
                    1
                ).label("value")
            )
            .where(
                DeviceLog.device_seq == device_seq,
                DeviceLog.value_code == value_name,
                func.date(DeviceLog.recorded_at) == target_date
            )
            .group_by(
                func.hour(DeviceLog.recorded_at),
                func.minute(DeviceLog.recorded_at)
            )
            .order_by(
                func.hour(DeviceLog.recorded_at),
                func.minute(DeviceLog.recorded_at),
            )
        )

        rows = db_session.execute(stmt).all()

        return {
            "labels": [
                f"{row.hour:02d}:{row.minute:02d}"
                for row in rows
            ],

            "values": [
                float(row.value)
                for row in rows
            ],

        }


@app.get("/temp")
def temp():

    chart = get_sensor_chart(1, "temp")

    return render_template(
        "temp.html",
        chart=chart
    )


@app.post("/api/temp")
def temp_chart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        1,
        "temp",
        selected_date
    )

    return jsonify(chart=chart)


@app.get("/hum")
def hum():

    chart = get_sensor_chart(1, "hum")

    return render_template(
        "hum.html",
        chart=chart
    )


@app.post("/api/hum")
def hum_chart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        1,
        "hum",
        selected_date
    )

    return jsonify(chart=chart)


@app.get("/dust")
def dust():

    chart = get_sensor_chart(3, "dust")

    return render_template(
        "dust.html",
        chart=chart
    )


@app.post("/api/dust")
def dust_chart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        3,
        "dust",
        selected_date
    )

    return jsonify(chart=chart)


@app.get("/co2")
def co2():

    chart = get_sensor_chart(2, "co2")

    return render_template(
        "co2.html",
        chart=chart
    )


@app.post("/api/co2")
def co2_chart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        2,
        "co2",
        selected_date
    )

    return jsonify(chart=chart)


# =========================================================
# Modbus
# =========================================================

BAUDRATE = 115200

SENSORS_BOARD = 0x01
MOTORS_BOARD = 0x02

READ_COILS = 0x01
READ_HOLDING_REGISTERS = 0x03
WRITE_SINGLE_COIL = 0x05

START_ADDR_SENSORS = 0x0000
START_ADDR_MOTORS = 0x0000

OUTPUT_ADDR_MOTORS = 0x0000

MOTOR_ON = 0xFF00
MOTOR_OFF = 0x0000


def calculate_crc16(data: bytes) -> bytes:

    crc = 0xFFFF

    for byte in data:

        crc ^= byte

        for _ in range(8):

            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001

            else:
                crc >>= 1

    return crc.to_bytes(2, byteorder="little")


def send_modbus_request(
        ser,
        slave_id,
        function_code,
        start_addr,
        registers_count
):

    pdu = bytes([
        slave_id,
        function_code,

        (start_addr >> 8) & 0xFF,
        start_addr & 0xFF,

        (registers_count >> 8) & 0xFF,
        registers_count & 0xFF
    ])

    crc = calculate_crc16(pdu)

    request_packet = pdu + crc

    ser.reset_input_buffer()
    ser.write(request_packet)

    print(
        f"[TX #{slave_id}] "
        f"{request_packet.hex(' ').upper()}"
    )


def receive_modbus_response(ser):


    time.sleep(0.01)

    header = ser.read(3)

    if len(header) < 3:

        print("[RX] 응답 없음")

        return None

    byte_count = header[2]

    remain = ser.read(byte_count + 2)

    if len(remain) < byte_count + 2:

        print("[RX] 데이터 길이 부족")

        return None

    actual_data = remain[:-2]
    received_crc = remain[-2:]

    expected_crc = calculate_crc16(
        header + actual_data
    )

    if received_crc != expected_crc:

        print("[RX] CRC ERROR")

        return None

    registers = []

    for i in range(0, len(actual_data), 2):

        value = int.from_bytes(
            actual_data[i:i + 2],
            byteorder="big"
        )

        registers.append(value)

    print("[RX]", registers)

    return registers

def save_sensor_values(values):

    if len(values) < 4:
        print("센서 데이터 부족")
        return

    logs = [
        DeviceLog(
            device_seq=1,
            value_code="temp",
            value=values[0],
            recorded_at=datetime.now(),
        ),

        DeviceLog(
            device_seq=1,
            value_code="hum",
            value=values[1],
            recorded_at=datetime.now(),
        ),

        DeviceLog(
            device_seq=3,
            value_code="dust",
            value=values[3],
            recorded_at=datetime.now(),
        ),

        DeviceLog(
            device_seq=2,
            value_code="co2",
            value=values[2],
            recorded_at=datetime.now(),
        )
    ]

    try:
        with Session(engine) as db_session:
            db_session.add_all(logs)
            db_session.commit()

        print("Sensor DB Save OK")

    except Exception as e:
        print("Sensor DB Save Error:", e)

def save_motor_rpm(values):

    if len(values) < 4:
        print("RPM 데이터 부족")
        return

    logs = [
        DeviceLog(
            device_seq=4,
            value_code="rf",
            value=values[0],
            recorded_at=datetime.now()
        ),

        DeviceLog(
            device_seq=4,
            value_code="lf",
            value=values[1],
            recorded_at=datetime.now()
        ),

        DeviceLog(
            device_seq=4,
            value_code="rr",
            value=values[2],
            recorded_at=datetime.now()
        ),

        DeviceLog(
            device_seq=4,
            value_code="lr",
            value=values[3],
            recorded_at=datetime.now()
        )
    ]
    
    try:
        with Session(engine) as db_session:
            db_session.add_all(logs)
            db_session.commit()

        print("Motor RPM DB Save OK")

    except Exception as e:
        print("Motor RPM DB Save Error:", e)

def save_actuator_state(state):
    if state is None:
            print("STATE NONE")
            return
    
    log = DeviceLog(
        device_seq=4,
        value_code="mode",
        value=state,
        recorded_at=datetime.now()
        )

    try:
        with Session(engine) as db_session:
            db_session.add(log)
            db_session.commit()
        
            print("Motor State DB Save OK")
        
    except Exception as e:
        print("Motor State DB Save Error:", e)

# =========================================================
# Modbus Thread
# =========================================================

def modbus_thread():

    global sensor_values_modbus
    global motor_rpm
    global robot_state
    global web_robot_cmd

    last_sensor_db_save = 0
    last_rpm_db_save = 0

    prev_fire_detected = False
    prev_robot_state = None

    print("Modbus Thread Start")

    try:

        ser = serial.Serial(
            "/dev/serial0",
            baudrate=BAUDRATE,
            timeout=0.5
        )

        while True:

            # ------------------------------------------
            # Sensor Board
            # ------------------------------------------

            send_modbus_request(
                ser,
                SENSORS_BOARD,
                READ_HOLDING_REGISTERS,
                START_ADDR_SENSORS,
                4
            )

            result = receive_modbus_response(ser)

            if result is not None:
                sensor_values_modbus = result

                now = time.monotonic()

                if now - last_sensor_db_save >= 5.0:
                    save_sensor_values(result)
                    last_sensor_db_save = now


            # ------------------------------------------
            # Robot State
            # ------------------------------------------

            send_modbus_request(
                ser,
                MOTORS_BOARD,
                READ_COILS,
                START_ADDR_MOTORS,
                1
            )

            result = receive_modbus_response(ser)

            if result is not None and len(result) > 0:
                robot_state = result[0]

                if robot_state != prev_robot_state:
                    save_actuator_state(robot_state)
                    prev_robot_state = robot_state

            if web_robot_cmd is not None:

                send_modbus_request(
                    ser,
                    MOTORS_BOARD,
                    WRITE_SINGLE_COIL,
                    OUTPUT_ADDR_MOTORS,
                    web_robot_cmd
                )

                if web_robot_cmd == MOTOR_ON:
                    print("WEB -> ROBOT START")

                elif web_robot_cmd == MOTOR_OFF:
                    print("WEB -> ROBOT STOP")

                web_robot_cmd = None

                # Write 응답을 구현하지 않았으므로
                # 다음 Modbus 요청과 프레임을 분리
                time.sleep(0.01)

            # ------------------------------------------
            # Motor RPM
            # ------------------------------------------

            send_modbus_request(
                ser,
                MOTORS_BOARD,
                READ_HOLDING_REGISTERS,
                START_ADDR_MOTORS,
                4
            )

            result = receive_modbus_response(ser)

            if fire_detected and not prev_fire_detected:
                send_modbus_request(
                    ser,
                    MOTORS_BOARD,
                    WRITE_SINGLE_COIL,
                    START_ADDR_MOTORS,
                    MOTOR_OFF
                )

                print("FIRE -> ROBOT STOP")

            prev_fire_detected = fire_detected

            
            if result is not None:
                motor_rpm = result

                now = time.monotonic()
                
                if now - last_rpm_db_save >= 0.5:
                    save_motor_rpm(result)
                    last_rpm_db_save = now


            # Modbus 주기
            time.sleep(0.5)


    except Exception as e:

        print("Modbus Thread Error:", e)


    finally:

        try:
            ser.close()

        except:
            pass


# =========================================================
# OpenCV
# =========================================================

EXPOSURE_US = 2000

BRIGHT_MIN = 230

MIN_PIXELS = 30
MAX_PIXELS = 8000

HISTORY = 10
VOTE_MIN = 4

DISPLAY_GAMMA = 0.3

# CO2 임계값
CO2_THRESHOLD = 1000   


_picam2 = None

_vote_hist = deque(
    maxlen=HISTORY
)

_kernel = np.ones(
    (3, 3),
    np.uint8
)

_gamma_lut = np.array(
    [
        ((i / 255.0) ** DISPLAY_GAMMA) * 255
        for i in range(256)
    ],
    dtype="uint8"
)


def get_camera():

    global _picam2

    if _picam2 is None:

        _picam2 = Picamera2()

        config = _picam2.create_preview_configuration(
            main={
                "size": (640, 480)
            }
        )

        _picam2.configure(config)

        _picam2.start()

        _picam2.set_controls({
            "AeEnable": False,
            "ExposureTime": EXPOSURE_US,
            "AnalogueGain": 1.0
        })

    return _picam2


def check_flame():

    global latest_frame

    frame = get_camera().capture_array()

    frame_bgr = cv2.cvtColor(
        frame,
        cv2.COLOR_RGB2BGR
    )

    gray = cv2.cvtColor(
        frame_bgr,
        cv2.COLOR_BGR2GRAY
    )

    _, mask = cv2.threshold(
        gray,
        BRIGHT_MIN,
        255,
        cv2.THRESH_BINARY
    )

    mask = cv2.morphologyEx(
        mask,
        cv2.MORPH_OPEN,
        _kernel
    )

    n_pixels = cv2.countNonZero(mask)

    candidate = (
        MIN_PIXELS
        <= n_pixels
        <= MAX_PIXELS
    )

    _vote_hist.append(candidate)

    detected = (
        sum(_vote_hist)
        >= VOTE_MIN
    )

    display = cv2.LUT(
    frame_bgr,
    _gamma_lut
)

    color = (
        (0, 0, 255)
        if detected
        else (0, 255, 0)
    )

    cv2.putText(
        display,
        f"px:{n_pixels}",
        (10, 25),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.7,
        color,
        2
    )

    if detected:
        cv2.putText(
            display,
            "CV DETECTED",
            (10, 55),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            (0, 0, 255),
            2
        )

    success, encoded = cv2.imencode(
        ".jpg",
        display,
        [cv2.IMWRITE_JPEG_QUALITY, 80]
    )

    if success:
        with frame_lock:
            latest_frame = encoded.tobytes()

    return detected


def close_camera():

    global _picam2

    if _picam2 is not None:

        _picam2.stop()
        _picam2 = None

def fire_process(): 
    # Modbus 센서 데이터가 아직 안 들어온 경우
    sensor_values = sensor_values_modbus

    if len(sensor_values_modbus) < 4:
        print("Modbus Sensor Data NONE")
        return False

    co2_value = sensor_values_modbus[2]

    if co2_value is None:
        print("CO2 Data NONE")
        return False

    if co2_value >= CO2_THRESHOLD:
        print("CV 화재 감지 + CO2 임계값 초과 ")
        return True
    else:
        print(f"CO2 임계값 미만,화재 프로세스 종료 {co2_value}")
        return False

# =========================================================
# OpenCV Thread
# =========================================================

def opencv_thread():

    global fire_detected

    cv_fire_detected = False

    print("OpenCV Thread Start")
    
    try:

        while True:

            cv_fire_detected = check_flame()

            if cv_fire_detected:
                print("CV DETECTED")

                fire_detected = fire_process()
            else:
                fire_detected = False

            # CPU 너무 많이 먹지 않도록 약간 대기
            time.sleep(0.05)


    except Exception as e:

        print("OpenCV Thread Error:", e)


    finally:

        close_camera()


# =========================================================
# Main
# =========================================================

if __name__ == "__main__":

    print("================================")
    print(" Robot System Start")
    print("================================")


    # Modbus
    t_modbus = threading.Thread(
        target=modbus_thread,
        daemon=True
    )

    t_modbus.start()


    # OpenCV
    t_opencv = threading.Thread(
        target=opencv_thread,
        daemon=True
    )

    t_opencv.start()


    # Flask는 Main Thread
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True,
        use_reloader=False
    )
