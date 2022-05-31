//Tensorflow 
#include <esp32-hal-log.h> //include board driver

#include <TensorFlowLite_ESP32.h> //include tensorflow lite library for converting microcode and let esp read the model
#include "main_functions.h" //declare 2 function setup and loop

#include "detection_responder.h" //
#include "image_provider.h"
#include "model_settings.h"
#include "classification_model_data.h"
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "soc/rtc_cntl_reg.h"

//ESP32 Cam
#include <dummy.h>

#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <base64.h>
#include <PubSubClient.h>

const char* ssid = "AIS_2.4GHz";
const char* password = "2543katw";
const char* mqttServer = "projecttech.thddns.net";
const int mqttPort = 5050;
const char* mqttUser = "";
const char* mqttPassword = "";

#define PUBLISH_TOPIC "esp32cam/image_data"
#define RAT_DETECTED "esp32cam/rat_detected"

WiFiClient espClient;
PubSubClient client(espClient);

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32      // POWER
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0      // XCLK
#define SIOD_GPIO_NUM     26      // SDA
#define SIOC_GPIO_NUM     27      // SCL

#define Y9_GPIO_NUM       35      // D7
#define Y8_GPIO_NUM       34      // D6
#define Y7_GPIO_NUM       39      // D5
#define Y6_GPIO_NUM       36      // D4
#define Y5_GPIO_NUM       21      // D3
#define Y4_GPIO_NUM       19      // D2
#define Y3_GPIO_NUM       18      // D1
#define Y2_GPIO_NUM        5      // D0

#define VSYNC_GPIO_NUM    25      // VSYNC
#define HREF_GPIO_NUM     23      // HREF
#define PCLK_GPIO_NUM     22      // PCLK
#define LAMP               4

const int timerInterval = 5000;    // time between each capture image
unsigned long previousMillis = 0;   // last time image was sent

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *interpreter = nullptr; //declare interpreter Tflite
  TfLiteTensor *input = nullptr;

  // An area of memory to use for input, output, and intermediate arrays.
  constexpr int kTensorArenaSize = 70 * 1024;
  static uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  pinMode(LAMP,OUTPUT);

  // Connect to Wifi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  /* Cam IP */
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init (error checking)
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  Serial.println("Successfully Connected\n");

  //Tensorflow Part
  // Disable brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  //Serial.setDebugOutput(true);
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
  log_i("Starting ESP32Cam...");

  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, false);

  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_classification_model_data);
  //Check if our model can read or not
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    error_reporter->Report("Model provided is schema version %d not equal to supported version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  //Get model will be in function set up()

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::ops::micro::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_AVERAGE_POOL_2D, tflite::ops::micro::Register_AVERAGE_POOL_2D());

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter; //pointer queue to run interpreter
  //Build interpreter, declare interpreter function in variable interpreter //reserve point address in C

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }
  // If one condition in setup fail, the whole process will not work, but if it all work it will go to function loop

  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0); //get input from image
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timerInterval) {
    sendPhoto();
    previousMillis = currentMillis;
  }

  /*
  //Tensorflow
  if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, input->data.uint8))
  {
    error_reporter->Report("Image capture failed.");
  }


  // Run the model on this input and make sure it succeeds.
  if (kTfLiteOk != interpreter->Invoke())
  {
    error_reporter->Report("Invoke failed.");
  }

  TfLiteTensor *output = interpreter->output(0);

  // Process the inference results.
  uint8_t Non_Animal = output->data.uint8[kPersonIndex]; 
  uint8_t Animal = output->data.uint8[kNotAPersonIndex];
  RespondToDetection(error_reporter, Non_Animal, Animal);*/
}

String sendPhoto() {
  String getAll;
  String getBody;

  /* Capture Image */
  digitalWrite(LAMP,HIGH);
  delay(300);
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();       // Image Capture
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  digitalWrite(LAMP,LOW);

  client.setServer(mqttServer, mqttPort);

  /* MQTT Part */
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      Serial.println("connected\n");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  /* Encode image */
  String encoded = base64::encode(fb->buf, fb->len);
  Serial.println(encoded);
  Serial.println();

  // string limit
  int payload_characters_limit = 100.00;
  int payloads_amount = (encoded.length())/payload_characters_limit;

  String sub_str;
  int begin_index;
  char char_buf[payload_characters_limit];

  // loop to split substring
  client.publish(PUBLISH_TOPIC, "data:image/jpeg;base64,");   // starting to send
  for (int i = 0; i <= payloads_amount; i++) {
    begin_index = i * payload_characters_limit;
    sub_str = encoded.substring(begin_index, begin_index + 100);
    Serial.println(sub_str);

    // Convert String to CharArray
    sub_str.toCharArray(char_buf, payload_characters_limit);
    client.publish(PUBLISH_TOPIC, char_buf);    // pub
  }
  client.publish(PUBLISH_TOPIC, " ");           // finish sending

  Serial.println("===========================================");

  // Return Frame Buffer
  esp_camera_fb_return(fb);
}
