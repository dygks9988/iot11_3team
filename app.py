from flask import Flask, request, render_template, jsonify
import pymysql
from sqlalchemy import create_engine, select, func
from models import DeviceLog, Device
from sqlalchemy.orm import Session



app = Flask(__name__)

engine = create_engine("mysql+pymysql://project_user:project!%40#$@192.168.1.37/robot?charset=utf8mb4")

@app.route("/dashboard")
def dashboard():
    with Session(engine) as db_session:

        devices = db_session.scalars(
            select(Device).limit(4)
        ).all()

        

        charts = []

        for device in devices:
            value_codes = db_session.scalars(
                select(DeviceLog.value_code)
                .where(DeviceLog.device_seq == device.device_seq)
                .distinct()
            ).all()
            
            for value_code in value_codes:
                chart = get_sensor_chart(device.device_seq,value_code)
                
                charts.append({
                    "device_seq": device.device_seq,
                    "device_name": device.device_name,
                    "value_code": value_code,
                    "labels": chart["labels"],
                    "values": chart["values"]
                })

        return render_template("dashboard.html",charts=charts)


def get_sensor_chart(device_seq,value_name,selected_date = None):
    with Session(engine) as db_session:

        if selected_date is None:
            target_date = func.current_date()
        else:
            target_date = selected_date

        

        stmt = (
            select(
                func.hour(DeviceLog.recorded_at).label("hour"),
                func.minute(DeviceLog.recorded_at).label("minute"),
                func.round(func.avg(DeviceLog.value), 1).label("value")
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
                func.minute(DeviceLog.recorded_at)
            )
        )

        
        rows = db_session.execute(stmt).all()
        
        for row in rows:
            print(row.hour, row.minute, row.value)

        return { "labels": [f"{row.hour:02d}:{row.minute:02d}"for row in rows],
                 "values": [float(row.value)for row in rows]
                }

@app.get("/temp")
def temp():
    chart = get_sensor_chart(1,'temp')
    

    return render_template("temp.html",labels=chart["labels"],value=chart["values"])

@app.post("/api/temp")
def temp_chart_date():

    selected_date = request.get_json()['date'];

    chart = get_sensor_chart(1,'temp',selected_date)

    return jsonify(labels=chart["labels"],value=chart["values"])

@app.get("/hum")
def hum():
    chart = get_sensor_chart(1,'hum')

    return render_template("hum.html",labels=chart["labels"],value=chart["values"])

@app.post("/api/hum")
def hum_chart_date():

    selected_date = request.get_json()['date'];

    chart = get_sensor_chart(1,'hum',selected_date)

    return jsonify(labels=chart["labels"],value=chart["values"])

@app.get("/dust")
def dust():
    chart = get_sensor_chart(2,'dust')
    
    return render_template("dust.html",labels=chart["labels"],value=chart["values"])

@app.post("/api/dust")
def dust_chart_date():

    selected_date = request.get_json()['date'];

    chart = get_sensor_chart(2,'dust',selected_date)

    return jsonify(labels=chart["labels"],value=chart["values"])

@app.get("/co2")
def co2():
    chart = get_sensor_chart(3,'co2')
        
    return render_template("co2.html",labels=chart["labels"],value=chart["values"])

@app.post("/api/co2")
def co2_chart_date():

    selected_date = request.get_json()['date'];

    chart = get_sensor_chart(3,'co2',selected_date)

    return jsonify(labels=chart["labels"],value=chart["values"])


@app.get("/pm25")
def pm25():
    chart = get_sensor_chart(3,'pm25')
        
    return render_template("pm25.html",labels=chart["labels"],value=chart["values"])

@app.post("/api/pm25")
def pm25_chart_date():

    selected_date = request.get_json()['date'];

    chart = get_sensor_chart(3,'pm25',selected_date)

    return jsonify(labels=chart["labels"],value=chart["values"])

if __name__ == "__main__" :
    app.run(debug=True)