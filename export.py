from ultralytics import YOLO

# Load a model
model = YOLO("last.pt")  # load an official model

# Export the model
model.export(format="onnx")