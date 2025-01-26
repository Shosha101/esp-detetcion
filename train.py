from ultralytics import YOLO
model=YOLO("yolo11n.pt")
model.train(data="dataset_custom.yaml", imgsz=640, epochs=100, device="cpu")
