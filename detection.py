import cv2
import socket
from ultralytics import YOLO

# Load the YOLO model for detection
model = YOLO("last.onnx", task="detect")

# Open the video stream (adjust the URL for your camera feed)
video_path = "http://192.168.105.22/stream"
cap = cv2.VideoCapture(video_path)

# UDP settings
udp_ip = "192.168.105.22"  # IP address of ESP32
udp_port = 12345        # UDP port number for ESP32

# Create UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Global variable to store coordinates
coordinates = []

# Function to send coordinates over UDP
def send_coordinates():
    if coordinates:
        # Get the latest coordinates
        x, y = coordinates[-1]
        data = f"{x},{y}"
        try:
            # Send the coordinates via UDP
            sock.sendto(data.encode(), (udp_ip, udp_port))
            print(f"Sent coordinates: x={x}, y={y}")
        except Exception as e:
            print(f"Error sending coordinates: {e}")

# Loop through the video frames
while cap.isOpened():
    # Read a frame from the video
    success, frame = cap.read()

    if success:
        # Run YOLO tracking on the frame
        results = model.track(frame, persist=True, stream=True)

        # Define annotated_frame to ensure it's always available
        annotated_frame = frame.copy()

        # Iterate through the results for tracking information
        for result in results:
            boxes = result.boxes.xyxy.cpu().numpy()  # Get bounding boxes [x1, y1, x2, y2]

            # Process all detected objects
            for box in boxes:
                x1, y1, x2, y2 = map(int, box)

                # Calculate the center coordinates of the bounding box
                cx = (x1 + x2) // 2
                cy = (y1 + y2) // 2

                # Store coordinates to send later
                coordinates.append((cx, cy))

                # Visualize the results on the frame
                cv2.rectangle(annotated_frame, (x1, y1), (x2, y2), (0, 0, 255), 2)
                label = f"Object ({cx}, {cy})"
                cv2.putText(annotated_frame, label, (x1, y1 - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # Send coordinates directly in the loop
        send_coordinates()

        # Display the annotated frame
        cv2.imshow("YOLO11 Tracking", annotated_frame)

        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
    else:
        # Break the loop if the end of the video is reached
        break

# Release the video capture object and close the display window
cap.release()
cv2.destroyAllWindows()
