from flask import Flask, Response, send_from_directory, request
# from waitress import serve
from utils.serial_communication import send_receive_serial_data
from utils.generate_frames import generate_frames

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


@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)
