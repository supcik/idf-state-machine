/**
 ******************************************************************************
 * @file        : main.cpp
 * @brief       : State machine example
 * @author      : Jacques Supcik <jacques@supcik.net>
 * @date        : 8 July 2024
 ******************************************************************************
 * @copyright   : Copyright (c) 2024 Jacques Supcik
 * @attention   : SPDX-License-Identifier: MIT
 ******************************************************************************
 * @details
 *
 ******************************************************************************
 */

#include "esp_log.h"
#include "state_machine.hpp"

const char* kTag = "state_machine (example)";

extern "C" {
void app_main(void);
}

typedef struct context {
    int id;
} context;

// State machine declaration (must be declared before states)

class MyStateMachine : public StateMachine {
   public:
    MyStateMachine(context* ctx);

    void Start();
    void SwitchToHook();

    context* ctx_;
};

// States implementation

enum states { kInit, kIdle };

class MyState : public State {
   public:
    MyState(const string& name, MyStateMachine* machine) : State(name, machine) {}
    MyStateMachine* machine() { return static_cast<MyStateMachine*>(machine_); }
};

class InitState : public MyState {
   public:
    InitState(MyStateMachine* machine) : MyState("init", machine) {}
    void Enter() override {
        ESP_LOGI(kTag, "Entering InitState");
        machine()->ctx_->id = 0;
    }
    void Exit() override {}
    void Step() override { machine_->SwitchTo(kIdle); }
};

class IdleState : public MyState {
   public:
    IdleState(MyStateMachine* machine) : MyState("idle", machine) {}
    void Enter() override {
        ESP_LOGI(kTag, "Entering IdleState");
        machine()->ctx_->id = 1;
    }
    void Exit() override {}
    void Step() override {}
};

MyStateMachine::MyStateMachine(context* ctx) : StateMachine(), ctx_(ctx) {
    RegisterState(kInit, new InitState(this));
    RegisterState(kIdle, new IdleState(this));
}
void MyStateMachine::Start() { SwitchTo(kInit); }
void MyStateMachine::SwitchToHook() {
    ESP_LOGI(kTag,
             "Switching from %s to %s",
             ThisState(prevStateId_)->name_.c_str(),
             ThisState(nextStateId_)->name_.c_str());
}

void app_main(void) {
    context ctx;
    MyStateMachine machine(&ctx);
    machine.Start();
    while (true) {
        machine.Step();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
