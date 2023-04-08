from flask import Flask, Response, send_from_directory, request
# from waitress import serve
from utils.serial_communication import send_receive_serial_data
from utils.generate_frames.generate_frames import generate_comp_vision_frames
from utils.generate_frames.generate_webcam_frames import generate_webcam_frames


app = Flask(__name__, static_folder="../webclient/build")

@app.route('/')
def serve():
    return send_from_directory(app.static_folder, "index.html")


@app.route("/<path:path>")
def serve_static(path):
    return send_from_directory(app.static_folder, path)


@app.route("/command", methods=["POST"])
def handle_command():
    command = request.json.get("command")
    # Handle the command here
    print(f"Received command: {command}")
    send_receive_serial_data(command)
    return {"status": "success"}


@app.route('/computer_vision_video_feed')
def computer_vision_video_feed():
    lm_threshold = 0.2
    return Response(generate_comp_vision_frames(lm_threshold), mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/robot_arm_video_feed')
def robot_arm_video_feed():
    return Response(generate_webcam_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
