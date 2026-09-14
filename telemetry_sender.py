import time
import json
import threading
from datetime import datetime, timedelta
import paho.mqtt.client as mqtt

# ============================================================
# ThingsBoard MQTT Configuration
# ============================================================
MQTT_SERVER = "mqtt.thingsboard.cloud"
MQTT_PORT = 1883
ACCESS_TOKEN = "BiTechX_Token_2026"

TELEMETRY_TOPIC = "v1/devices/me/telemetry"
RPC_REQUEST_TOPIC = "v1/devices/me/rpc/request/+"
RPC_RESPONSE_TOPIC = "v1/devices/me/rpc/response/{}"

# ============================================================
# System State & Initial Schedule
# ============================================================
current_dose_datetime = datetime(2026, 9, 14, 17, 0, 0)
FIXED_HOUR = "05:00 PM"

system_state = {
    "pills": 20,
    "battery": 90,
    "med_name": "Paracetamol",
    "ultrasonic_distance": 50.0,
    "ir_distance": 10.0,
    "person_detected": False,
    "door_status": "Closed",
    "conveyor_status": "IDLE",
    "is_dispensing": False
}

client = mqtt.Client(client_id="BiTechX_DualSlider_Simulator")
client.username_pw_set(ACCESS_TOKEN)

def get_current_dose_string():
    return f"{current_dose_datetime.strftime('%d/%m/%Y')} {FIXED_HOUR}"

def push_data(payload):
    try:
        msg = json.dumps(payload)
        client.publish(TELEMETRY_TOPIC, msg, qos=1)
        print(f"[MQTT PUSH] -> {msg}")
    except Exception as e:
        print(f"[ERROR] Push failed: {e}")

def run_auto_dispensing_sequence():
    """التسلسل الكامل التلقائي للصرف مع التوقيتات"""
    global current_dose_datetime

    # المرحلة 1: فتح الباب وتشغيل السير
    system_state["is_dispensing"] = True
    system_state["person_detected"] = True
    system_state["door_status"] = "Open"
    system_state["conveyor_status"] = "RUNNING"

    print("\n>>> [PHASE 1] Patient Detected (< 20 cm): Door OPEN | Conveyor RUNNING <<<")
    push_data({
        "device_status": "Dispensing Dose",
        "person_detected": True,
        "door_status": "Open",
        "conveyor_status": "RUNNING",
        "ultrasonic_distance": system_state["ultrasonic_distance"]
    })

    # الانتظار 5 ثوانٍ حتى تصل الحبة للحساس
    print(">>> Waiting 5 seconds for pill to reach IR sensor...")
    time.sleep(5)

    # المرحلة 2: محاكاة نزول الحبة أمام الـ IR
    system_state["ir_distance"] = 1.5
    print("\n>>> [PHASE 2] IR Triggered (1.5 cm): Pill Dispensed! Stopping Conveyor & Closing Door <<<")
    push_data({"ir_distance": 1.5})

    dose_being_taken = get_current_dose_string()

    # خصم الحبة وإنقاص شحن البطارية 1% مع كل جرعة
    system_state["pills"] = max(0, system_state["pills"] - 1)
    system_state["battery"] = max(10, system_state["battery"] - 1)

    # إيقاف المشغلات وقفل الباب
    system_state["door_status"] = "Closed"
    system_state["conveyor_status"] = "IDLE"

    current_dose_datetime += timedelta(days=1)
    next_dose_string = get_current_dose_string()

    # تحديث بيانات الداشبورد والـ History فوراً
    push_data({
        "device_status": "Dose Delivered",
        "door_status": "Closed",
        "conveyor_status": "IDLE",
        "medication_name": system_state["med_name"],
        "dose_time": dose_being_taken,
        "dose_status": "Taken",
        "pills_remaining": system_state["pills"],
        "battery_level": system_state["battery"],
        "next_dose_time": next_dose_string
    })
    print(f">>> Recorded [{dose_being_taken}] as Taken. Next: [{next_dose_string}] <<<")

    # الانتظار 5 ثوانٍ قبل إعادة تصفير السلايدرز وحالة الاستعداد
    print(">>> Waiting 5 seconds before resetting system to IDLE...")
    time.sleep(5)

    # إعادة ضبط القيم الافتراضية
    system_state["is_dispensing"] = False
    system_state["person_detected"] = False
    system_state["ultrasonic_distance"] = 50.0
    system_state["ir_distance"] = 10.0

    push_data({
        "device_status": "Waiting for Person",
        "person_detected": False,
        "ultrasonic_distance": 50.0,
        "ir_distance": 10.0
    })
    print(">>> [RESET] System returned to Idle. Sliders reset to (50 cm / 10 cm). Ready for next cycle! <<<\n")

def on_connect(c, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Connected to ThingsBoard!")
        client.subscribe(RPC_REQUEST_TOPIC, qos=1)
        push_data({
            "device_status": "Waiting for Person",
            "person_detected": False,
            "door_status": "Closed",
            "conveyor_status": "IDLE",
            "ultrasonic_distance": 50.0,
            "ir_distance": 10.0,
            "pills_remaining": system_state["pills"],
            "battery_level": system_state["battery"],
            "medication_name": system_state["med_name"],
            "next_dose_time": get_current_dose_string()
        })

def on_message(c, userdata, msg):
    try:
        req_id = msg.topic.split("/")[-1]
        data = json.loads(msg.payload.decode())
        method = data.get("method")
        params = data.get("params")
        print(f"[RPC] Method: {method} | Value: {params}")

        # رد فوري لمنع ظهور Request Timeout على الشاشة
        res = {"success": True}
        client.publish(RPC_RESPONSE_TOPIC.format(req_id), json.dumps(res), qos=1)

        # استقبال حركة سلايدر المريض (Ultrasonic)
        if method in ["setDistance", "setUltrasonic"]:
            val = float(params)
            system_state["ultrasonic_distance"] = val
            if val < 20.0 and not system_state["is_dispensing"]:
                # بدء السلسلة في Thread منفصل لعدم تجميد استقبال رسائل الـ MQTT
                threading.Thread(target=run_auto_dispensing_sequence, daemon=True).start()
            else:
                push_data({"ultrasonic_distance": val})

        # استقبال حركة سلايدر الـ IR (في حال تم تحريكه يدوياً)
        elif method in ["setIRDistance", "setIR"]:
            val = float(params)
            system_state["ir_distance"] = val
            push_data({"ir_distance": val})

    except Exception as e:
        print(f"[ERROR in on_message] {e}")

client.on_connect = on_connect
client.on_message = on_message

def main():
    print("==================================================")
    print(" BiTechX Auto-Dispensing Simulation Running       ")
    print(" Move Ultrasonic Slider < 20 cm to start cycle!   ")
    print("==================================================")
    client.connect(MQTT_SERVER, MQTT_PORT, keepalive=60)
    client.loop_forever()

if __name__ == "__main__":
    main()