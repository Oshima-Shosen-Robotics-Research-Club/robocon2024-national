// 他のファイルのプログラムを取得する
#include "Controller.h" // Controller.hを取得
#include <Arduino.h>
#include <MsTimer2.h> // MsTimer2.hを取得
#include <digitalWriteFast.h>
#include <liboshima.h> // liboshima.hを取得

// #defineでピン番号に別名をつける
#define CONNECT_LED_PIN PIN_PC0 // 受信中のランプのピン番号

// ボタンピン番号の配列
const uint8_t btnPins[NUM_MOTORS * 2] = {PIN_PD3, PIN_PD4, PIN_PD5, PIN_PD6,
                                         PIN_PD7, PIN_PB0, PIN_PB1, PIN_PB2,
                                         PIN_PC2, PIN_PC3, PIN_PC4, PIN_PC5};

// ImSender型のsender変数を宣言し、serial変数で初期化する
IM920SL im(Serial);

// 接続中のランプを消灯する関数
void onDisconnected() { digitalWriteFast(CONNECT_LED_PIN, LOW); }

// ロボットから返答があった時に呼び出される関数
void serialEvent() {
  char buf[sizeof("Success")];
  im.receive(buf);
  if (strcmp(buf, "Success") == 0) {
    // ランプを消灯
    digitalWriteFast(CONNECT_LED_PIN, HIGH);
    // タイマーのカウントを最初からやり直す
    MsTimer2::start();
  }
}

// 1度だけ実行される
void setup() {
  // 各ピンに対しボタンとして使うための設定を行う
  for (auto pin : btnPins) {
    pinModeFast(pin, INPUT_PULLUP);
  }

  // ランプを出力に設定
  pinModeFast(CONNECT_LED_PIN, OUTPUT);

  // IM920SLの初期化
  im.begin();

  // 500000マイクロ秒(0.5秒)のタイマーを設定
  MsTimer2::set(IM_RECEIVE_INTERVAL_MILLIS, onDisconnected);
  MsTimer2::start();
}

// 繰り返し実行される
void loop() {
  // ボタンの状態を取得し、コントローラーの状態を更新
  Controller controller;

  {
    uint8_t pinNum = 0;
    uint8_t motorNum = 0;
    while (pinNum < sizeof(btnPins)) {
      if (!digitalReadFast(btnPins[pinNum])) {
        controller.motors[motorNum] = MotorStateEnum::Forward;
      } else if (!digitalReadFast(btnPins[pinNum + 1])) {
        controller.motors[motorNum] = MotorStateEnum::Reverse;
      } else {
        controller.motors[motorNum] = MotorStateEnum::Stop;
      }
      pinNum += 2;
      motorNum++;
    }
  }

  // コントローラーの状態をIM920SLを使って送信
  im.send(controller);
  // 55ミリ秒待機
  delay(IM_SEND_INTERVAL);
}
