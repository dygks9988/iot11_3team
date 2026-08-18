from flask import Flask, request, render_template
import pymysql
from sqlalchemy import create_engine, select, func
from models import DeviceLog, Device
from sqlalchemy.orm import Session



app = Flask(__name__)

engine = create_engine("mysql+pymysql://ktech:ktech!%40#$@192.168.1.37/robot?charset=utf8mb4")


@app.route("/dashboard")
def dashboard():
    with Session(engine) as db_session :

        devices = db_session.scalars(
                    select(Device).limit(4)
                ).all()
        
        charts = []

        for device in devices:
            stmt = (
                select(
                    func.hour(DeviceLog.recorded_at).label("hour"),
                    func.round(func.avg(DeviceLog.value)).label("value")
                )
                .where(
                    DeviceLog.device_seq == device.device_seq,
                    func.date(DeviceLog.recorded_at) == func.curdate()
                )
                .group_by(
                    func.hour(DeviceLog.recorded_at)
                )
                .order_by(
                    func.hour(DeviceLog.recorded_at)
                )
            )
            rows = db_session.execute(stmt).all()

            charts.append({
                "device_seq": device.device_seq,
                "device_name": device.device_name,
                "labels":[f"{row.hour}:00" for row in rows],
                "values": [float(row.value) for row in rows]
            })


        return render_template("dashboard.html", charts = charts)

def get_sensor_chart(device_seq):
    with Session(engine) as db_session:
        stmt = (
            select(
                func.hour(DeviceLog.recorded_at).label("hour"),
                func.round(func.avg(DeviceLog.value)).label("value")
            )
            .where(
                DeviceLog.device_seq == device_seq,
                func.date(DeviceLog.recorded_at) == func.curdate()
            )
            .group_by(
                func.hour(DeviceLog.recorded_at)
            )
            .order_by(
                func.hour(DeviceLog.recorded_at)
            )
        )

        
        rows = db_session.execute(stmt).all()
        
        print(rows)

        return { "labels": [f"{row.hour}:00"for row in rows],
                 "values": [float(row.value)for row in rows] }

@app.get("/temp")
def temp():
    chart = get_sensor_chart(2)

    return render_template("temp.html",labels=chart["labels"],value=chart["values"])


@app.get("/hum")
def hum():
    return render_template("hum.html")


@app.get("/dust")
def dust():
    return render_template("dust.html")


@app.get("/co2")
def co2():
    return render_template("co2.html")

if __name__ == "__main__" :
    app.run(debug=True)