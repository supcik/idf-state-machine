/**
 ******************************************************************************
 * @file        : state_machine.cpp
 * @brief       : State machine
 * @author      : Jacques Supcik <jacques@supcik.net>
 * @date        : 22 June 2024
 ******************************************************************************
 * @copyright   : Copyright (c) 2024 Jacques Supcik
 * @attention   : SPDX-License-Identifier: MIT
 ******************************************************************************
 * @details     : This file contains the implementation of the generic
 *                state machine.
 ******************************************************************************
 */

#include "state_machine.hpp"

#include <stdlib.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* kTag = "StateMachine";

using namespace std;

void StateMachine::RegisterState(int stateId, State* state) {
    ESP_LOGI(kTag, "Registering state \"%s\" with id = %d", state->name_.c_str(), stateId);
    state->id_ = stateId;
    states_[stateId] = state;
}

State* StateMachine::ThisState(int stateId) { return states_[stateId]; }

void StateMachine::SwitchTo(int stateNum, bool force) {
    if (currentState_ != nullptr && stateNum == currentState_->id_ && !force) {
        ESP_LOGD(kTag, "SwitchTo: already in state %d", stateNum);
        return;
    }
    nextStateId_ = stateNum;
    if (currentState_ != nullptr) {
        prevStateId_ = currentState_->id_;
        currentState_->Exit();
    }
    currentState_ = ThisState(stateNum);
    SwitchToHook();
    currentState_->Enter();
}

void StateMachine::Run(uint32_t stack_size, UBaseType_t priority, TickType_t step_delay) {
    if (task_handle_ != nullptr) {
        ESP_LOGE(kTag, "Task already running");
        return;
    }
    ESP_LOGI(kTag, "Running state machine");
    step_delay_ = pdMS_TO_TICKS(step_delay);
    xTaskCreate(RunTaskForwarder, "state_machine", stack_size, this, priority, &task_handle_);
}

void StateMachine::Stop() {
    if (task_handle_ == nullptr) {
        ESP_LOGE(kTag, "Task not running");
        return;
    }
    ESP_LOGI(kTag, "Stopping state machine");
    vTaskDelete(task_handle_);
    task_handle_ = nullptr;
}

void StateMachine::Pause() {
    if (task_handle_ == nullptr) {
        ESP_LOGE(kTag, "Task not running");
        return;
    }
    ESP_LOGI(kTag, "Pausing state machine");
    vTaskSuspend(task_handle_);
}

void StateMachine::Resume() {
    if (task_handle_ == nullptr) {
        ESP_LOGE(kTag, "Task not running");
        return;
    }
    ESP_LOGI(kTag, "Resuming state machine");
    vTaskResume(task_handle_);
}