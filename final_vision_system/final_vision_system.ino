#include "esp_camera.h"
#include "esp_http_server.h"
#include <WiFi.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h" // Includes pin definitions for CAMERA_MODEL_AI_THINKER

// Wi-Fi credentials
const char* ssid = "WE66FFE7";
const char* password = "k9309278";

// Global HTTP server handle
httpd_handle_t server = NULL;

// Camera configuration
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// Initialize the Camera
void initCamera() {
    if (esp_camera_init(&camera_config) != ESP_OK) {
        Serial.println("Camera initialization failed!");
        while (true); // Halt execution if initialization fails
    }
    Serial.println("Camera initialized successfully!");
}

// Stream handler function
esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;

    // Set HTTP response type to multipart
    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        // Capture a frame from the camera
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            httpd_resp_send_500(req);
            break;
        }

        // Prepare the HTTP headers for the frame
        char buf[64];
        size_t hlen = snprintf(buf, sizeof(buf), "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
        res = httpd_resp_send_chunk(req, buf, hlen);
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            break;
        }

        // Send the JPEG frame
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        if (res != ESP_OK) {
            break;
        }

        // Send the boundary to indicate the end of this frame
        res = httpd_resp_send_chunk(req, "\r\n", 2);
        if (res != ESP_OK) {
            break;
        }

        delay(33); // ~30 FPS (33ms delay between frames)
    }

    return res;
}

// Start camera server function
static void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &stream_uri);
        Serial.println("HTTP server started successfully!");
    } else {
        Serial.println("Failed to start HTTP server!");
    }
}

void setup() {
    Serial.begin(115200); // Initialize Serial Monitor
    Serial.setDebugOutput(true);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    // Initialize the camera
    initCamera();

    // Start camera server
    startCameraServer();

    Serial.printf("Camera Ready! Stream at http://%s/stream\n", WiFi.localIP().toString().c_str());
}

void loop() {
    delay(10000); // Add optional tasks here
}
