from flask import Flask, request, render_template, jsonify
import pymysql
from sqlalchemy import create_engine, select, func
from models import DeviceLog, Device
from sqlalchemy.orm import Session



app = Flask(__name__)

engine = create_engine("mysql+pymysql://project_user:project!%40#$@192.168.1.37/robot?charset=utf8mb4")


@app.route("/dashboard")
def dashboard():
    return render_template("dashboard.html")

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
                DeviceLog.recorded_point.label("point"),

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
                DeviceLog.recorded_point,
                func.hour(DeviceLog.recorded_at),
                func.minute(DeviceLog.recorded_at)
            )
            .order_by(
                func.hour(DeviceLog.recorded_at),
                func.minute(DeviceLog.recorded_at),
                DeviceLog.recorded_point
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
            "points": [
                row.point
                for row in rows
            ]
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

    chart = get_sensor_chart(2, "dust")

    return render_template(
        "dust.html",
        chart=chart
    )

@app.post("/api/dust")
def dustchart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        2,
        "dust",
        selected_date
    )

    return jsonify(chart=chart)

@app.get("/co2")
def co2():

    chart = get_sensor_chart(3, "co2")

    return render_template(
        "co2.html",
        chart=chart
    )

@app.post("/api/co2")
def co2_chart_date():

    selected_date = request.get_json()["date"]

    chart = get_sensor_chart(
        3,
        "co2",
        selected_date
    )

    return jsonify(chart=chart)


if __name__ == "__main__" :
    app.run(debug=True)