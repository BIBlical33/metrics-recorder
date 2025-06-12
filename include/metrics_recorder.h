// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Metrics recorder
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#ifndef INCLUDE_METRICS_RECORDER_H_
#define INCLUDE_METRICS_RECORDER_H_

#include <cds/container/skip_list_map_hp.h>
#include <cds/gc/hp.h>

#include <atomic>
#include <chrono>
#include <format>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace metrics_recorder {

template <class ValueType>
class MetricsRecorder final {
 public:
  // Lock free map for recording metrics in lexicographic order
  cds::container::SkipListMap<cds::gc::HP, std::string, ValueType> metrics_;

  std::ofstream log_file_;

  MetricsRecorder() = delete;

  // Construсtor to link to a file
  explicit MetricsRecorder(const std::string& log_filename)
      : log_file_(log_filename) {
    if (not log_file_.is_open())
      throw std::runtime_error(
          std::format("Cannot open file: {}", log_filename));
  }

  // Creates or updates metric
  void Update(const std::string& name, const ValueType& value) {
    metrics_.update(
        name, [&value](bool, std::pair<const std::string, ValueType>& item) {
          item.second = value;
        });
  }

  // Records metrics to a log file and resets them
  void Log();

 private:
   // Thread id that currently logs
  std::atomic<std::thread::id> logging_thread_id_;

  // Resets metrics
  void ClearValues() {
    for (auto it = metrics_.begin(); it != metrics_.end(); ++it)
      Update(it->first, ValueType{});
  }
};

// Records metrics to a log file and resets them
template <class ValueType>
void MetricsRecorder<ValueType>::Log() {
  const auto current_thread_id = std::this_thread::get_id();
  std::thread::id expected_thread_id{};

  if (!logging_thread_id_.compare_exchange_strong(expected_thread_id,
                                                  current_thread_id))
    throw std::runtime_error(
        "Log() can only be called from a single dedicated thread!");

  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) %
                1000;

  log_file_ << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
  log_file_ << '.' << std::setfill('0') << std::setw(3) << now_ms.count()
            << " ";

  for (auto it = metrics_.begin(); it != metrics_.end();) {
    log_file_ << "\"" << it->first << "\" " << it->second;
    if (++it != metrics_.end()) log_file_ << " ";
  }

  log_file_ << "\n";
  log_file_.flush();

  ClearValues();

  logging_thread_id_ = std::thread::id{};
}

}  // namespace metrics_recorder

#endif  // INCLUDE_METRICS_RECORDER_H_
