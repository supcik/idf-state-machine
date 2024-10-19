/**
 ******************************************************************************
 * @file        : state_machine.hpp
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

#pragma once

#include <inttypes.h>

#include <map>
#include <string>

#include "freertos/FreeRTOS.h"

using namespace std;

class StateMachine;

class State {
    friend class StateMachine;

   public:
    State(string name, StateMachine* machine) : name_(name), machine_(machine) {}
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Step() = 0;
    int id_;
    string name_;

   protected:
    StateMachine* machine_;
};

class StateMachine {
    friend class State;

   public:
    StateMachine() {
        currentState_ = nullptr;
        prevStateId_ = -1;
        nextStateId_ = -1;
        task_handle_ = nullptr;
        step_delay_ = pdTICKS_TO_MS(1000);
    }
    void RegisterState(int stateId, State* state);
    State* ThisState(int stateId);
    virtual void Start() = 0;
    void Step() {
        if (currentState_ != nullptr) {
            currentState_->Step();
        }
    }
    void Run(uint32_t stack_size, UBaseType_t priority, TickType_t step_delay);
    void Stop();
    void Pause();
    void Resume();

    void SwitchTo(int stateNum, bool force = false);
    State* CurrentState() { return currentState_; }
    virtual void SwitchToHook() {}

   protected:
    State* currentState_;
    int prevStateId_;
    int nextStateId_;
    TaskHandle_t task_handle_;
    TickType_t step_delay_;

   private:
    static void RunTaskForwarder(void* param) {
        StateMachine* state_machine = (StateMachine*)param;
        state_machine->RunTask();
    }
    void RunTask() {
        while (1) {
            Step();
            vTaskDelay(step_delay_);
        }
    }
    map<int, State*> states_;
};
