#include <setup.h>
// #include <speech_recognition_inferencing.h>

esp_now_peer_info_t Setup::peerInfo;

void Setup::ESPNOW(){
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
    
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void Setup::MPU6050(){
  GyroSensor::gyroSensor_Data.xAxisValue = 127; // Center
  GyroSensor::gyroSensor_Data.yAxisValue = 127; // Center 

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  #ifdef PRINT_DEBUG
    // initialize serial communication
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    // initialize device
    Serial.println(F("Initializing I2C devices..."));
  #endif
  
  GyroSensor::mpu.initialize();

  #ifdef PRINT_DEBUG  
    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(GyroSensor::mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    // wait for ready
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again
    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
  #endif
  
  GyroSensor::devStatus = GyroSensor::mpu.dmpInitialize();
  
  // make sure it worked (returns 0 if so)
  if (GyroSensor::devStatus == 0) 
  {
      // Calibration Time: generate offsets and calibrate our MPU6050
      GyroSensor::mpu.CalibrateAccel(6);
      GyroSensor::mpu.CalibrateGyro(6);
      
      #ifdef PRINT_DEBUG      
        GyroSensor::mpu.PrintActiveOffsets();
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
      #endif
      GyroSensor::mpu.setDMPEnabled(true);
  
      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      #ifdef PRINT_DEBUG      
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
      #endif
      GyroSensor::dmpReady = true;
  
      // get expected DMP packet size for later comparison
      GyroSensor::packetSize = GyroSensor::mpu.dmpGetFIFOPacketSize();
      
  } 
  else 
  {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      #ifdef PRINT_DEBUG       
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(GyroSensor::devStatus);
        Serial.println(F(")"));
      #endif
  }
}

void Setup::Button(){
  pinMode(buttonPin, INPUT);
}


void Setup::SpeechRecognition(){
  SpeechRecognition::setupSpeechRecognition();
}

void Setup::InitialTask(){
  // xTaskCreatePinnedToCore(
  //   GyroSensor::vTaskGestureControl,
  //   "Hand Gesture Control",
  //   STACK_SIZE, // Stack
  //   NULL, // Parameter to pass function
  //   1, // Task Priority
  //   NULL, // Task Handle
  //   0 // CPU core
  // );

  // xTaskCreatePinnedToCore(
  //   ButtonSensor::vTaskButtonControl,
  //   "Button Control",
  //   STACK_SIZE, // Stack
  //   NULL, // Parameter to pass function
  //   1, // Task Priority
  //   NULL, // Task Handle
  //   1 // CPU core
  // );

  xTaskCreatePinnedToCore(
    SpeechRecognition::vTaskSpeechRecognition,
    "Voice Recognition Task",
    STACK_SIZE * 8,
    NULL, 
    1,
    NULL,
    1
  );


}