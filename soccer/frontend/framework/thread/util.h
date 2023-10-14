#pragma once
#include <string_view>
#include <thread>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

void set_current_thread_name(const std::string_view name);
