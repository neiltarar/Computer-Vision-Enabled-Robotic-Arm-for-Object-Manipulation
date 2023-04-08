import cv2

def generate_webcam_frames():
    # Create a VideoCapture object to capture frames from the default camera
    cap = cv2.VideoCapture(2)

    # Check if the camera was opened successfully
    if not cap.isOpened():
        print("Could not open camera")
        exit()

    # Capture frames from the camera and display them
    while True:
        # Capture a frame from the camera
        ret, frame = cap.read()

        # Check if the frame was captured successfully
        if not ret:
            print("Could not capture frame")
            break

        # Display the frame
        # cv2.imshow("Frame", frame)

        # Wait for a key press to exit
        if cv2.waitKey(1) == ord("q"):
            break

        _, img_encoded = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(img_encoded) + b'\r\n')
    # Release the VideoCapture object and close all windows
    cap.release()
    cv2.destroyAllWindows()
