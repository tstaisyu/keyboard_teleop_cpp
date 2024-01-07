#include <micro_ros_arduino.h>

#include <M5Stack.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#include "CytronMotorDriver.h"

rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

//rcl_init_options_t init_options; // Humble
//size_t domain_id = 117;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) {Serial.println("Error in " #fn); return;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

const int motorRPin1 = 25;
const int motorRPin2 = 2;
const int motorLPin1 = 26;
const int motorLPin2 = 5;

CytronMD motor_R(PWM_DIR, motorRPin1, motorRPin2);
CytronMD motor_L(PWM_DIR, motorLPin1, motorLPin2);

void subscription_callback(const void * msgin) {
  M5.Lcd.setCursor(0, 20);  
  M5.Lcd.println("Callback triggered");  // LCD出力
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;

  M5.Lcd.print("Received command: ");  // 受信したコマンドをシリアル出力
  M5.Lcd.println(msg->data);

//  M5.Lcd.clear();  // LCD画面をクリア
  M5.Lcd.setCursor(0, 20);  // テキスト表示位置を設定
  switch (msg->data) {
    case 1: // forward
      motor_R.setSpeed(255);
      motor_L.setSpeed(255);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("Moving Forward ");
      break;
    case 2: // backward
      motor_R.setSpeed(-255);
      motor_L.setSpeed(-255);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("Moving Backward");
      break;
    case 3: // left
      motor_R.setSpeed(255);
      motor_L.setSpeed(-255);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("Turning Left   ");
      break;
    case 4: // right
      motor_R.setSpeed(-255);
      motor_L.setSpeed(255);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("Turning Right  ");
      break;
    default:
      // stop or undefined command
      motor_R.setSpeed(0);
      motor_L.setSpeed(0);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.print("Stopped        ");
      break;
  }
}

void setup() {
//  Serial.begin(115200);  // シリアル通信の初期化
//  while(!Serial);  // シリアルポートが開くのを待つ

  M5.begin();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);  // ステータスメッセージの位置を設定
  M5.Lcd.print("micro ROS2 M5Stack START\n");  
	// Wi-Fi経由の場合
	//set_microros_wifi_transports("SSID", "PWD", "IP", Port);
	// USB経由の場合
	set_microros_transports();

//  delay(2000);

  allocator = rcl_get_default_allocator();

  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	//init_options = rcl_get_zero_initialized_init_options();
	//RCCHECK(rcl_init_options_init(&init_options, allocator));
	//RCCHECK(rcl_init_options_set_domain_id(&init_options, domain_id));		// ドメインIDの設定
	//RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator)); // 前のrclc_support_initは削除する
  RCCHECK(rclc_node_init_default(&node, "subscriber_node", "", &support));


  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "/motor_control"));

	int callback_size = 1;	// コールバックを行う数
//	executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, callback_size, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
