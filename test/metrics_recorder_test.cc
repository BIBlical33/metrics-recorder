// All Rights Reserved
//
// Copyright (c) 2025, github.com/BIBlical33
//
// Metrics recorder test
//
// This software may not be modified without the explicit permission of the
// copyright holder. For permission requests, please contact:
// mag1str.kram@gmail.com

#include "include/metrics_recorder.h"

#include <cds/gc/hp.h>
#include <cds/init.h>
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

namespace fs = std::filesystem;

class MetricsRecorderTest : public ::testing::Test {
 protected:
  cds::gc::HP gc{67};

  void SetUp() override { cds::Initialize(); }

  void TearDown() override { cds::Terminate(); }

  std::string GetLastLine(const std::string& filename) {
    std::ifstream in(filename);
    std::string line, last_line;
    while (std::getline(in, line)) last_line = line;

    return last_line;
  }

  bool LineContainsMetric(const std::string& line,
                          const std::string& metric_name,
                          const std::string& metric_value) {
    std::regex pattern("\"" + metric_name + "\" " + metric_value + "\\b");
    return std::regex_search(line, pattern);
  }
};

TEST_F(MetricsRecorderTest, TestCase) {
  cds::threading::Manager::attachThread();

  std::string kTestFilename = "TestCase.log";

  metrics_recorder::MetricsRecorder<double> recorder(kTestFilename);

  recorder.Update("CPU", 0.97);
  recorder.Update("HTTP requests RPS", 42);

  recorder.Log();

  EXPECT_TRUE(LineContainsMetric(GetLastLine(kTestFilename), "CPU", "0.97"));
  EXPECT_TRUE(LineContainsMetric(GetLastLine(kTestFilename),
                                 "HTTP requests RPS", "42"));

  recorder.Update("CPU", 1.12);
  recorder.Update("HTTP requests RPS", 30);

  recorder.Log();

  EXPECT_TRUE(LineContainsMetric(GetLastLine(kTestFilename), "CPU", "1.12"));
  EXPECT_TRUE(LineContainsMetric(GetLastLine(kTestFilename),
                                 "HTTP requests RPS", "30"));

  cds::threading::Manager::detachThread();
}

TEST_F(MetricsRecorderTest, LogOutputMatchesFormatRegex) {
  cds::threading::Manager::attachThread();

  const std::string kTestFilename = "RegexTest.log";
  metrics_recorder::MetricsRecorder<int> recorder(kTestFilename);

  recorder.Update("CPU", 2);
  recorder.Update("HTTP requests RPS", 123);

  recorder.Log();

  std::string last_line = GetLastLine(kTestFilename);

  std::regex line_format(
      R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}( ".*?" \d+(\.\d+)?)+\s?$)");
  EXPECT_TRUE(std::regex_match(last_line, line_format));

  cds::threading::Manager::detachThread();
}

TEST_F(MetricsRecorderTest, ThrowsOnFileOpenFailure) {
  cds::threading::Manager::attachThread();

  fs::create_directory("test_dir");

  EXPECT_THROW(metrics_recorder::MetricsRecorder<int>("test_dir"),
               std::runtime_error);

  fs::remove("test_dir");

  cds::threading::Manager::detachThread();
}

TEST_F(MetricsRecorderTest, ValuesAreResetAfterLog) {
  cds::threading::Manager::attachThread();

  const std::string kTestFilename = "ResetTest.log";
  metrics_recorder::MetricsRecorder<int> recorder(kTestFilename);

  recorder.Update("CPU", 5);
  recorder.Update("HTTP requests RPS", 10);
  recorder.Log();

  recorder.Log();

  std::string last_line = GetLastLine(kTestFilename);

  EXPECT_TRUE(LineContainsMetric(last_line, "CPU", "0"));
  EXPECT_TRUE(LineContainsMetric(last_line, "HTTP requests RPS", "0"));

  cds::threading::Manager::detachThread();
}

TEST_F(MetricsRecorderTest, ConcurrentUpdateAndLog) {
  std::string kTestFilename = "ConcurrentTest.log";
  metrics_recorder::MetricsRecorder<double> recorder(kTestFilename);

  std::atomic<bool> stop{false};
  std::vector<std::thread> threads;

  threads.emplace_back([&recorder, &stop]() {
    cds::threading::Manager::attachThread();
    double cpu_value = 0;
    while (not stop.load()) {
      recorder.Update("CPU", cpu_value++);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    cds::threading::Manager::detachThread();
  });

  threads.emplace_back([&recorder, &stop]() {
    cds::threading::Manager::attachThread();
    int http_value = 0;
    while (not stop.load()) {
      recorder.Update("HTTP requests RPS", http_value++);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    cds::threading::Manager::detachThread();
  });

  threads.emplace_back([&recorder, &stop]() {
    cds::threading::Manager::attachThread();
    while (not stop.load()) {
      recorder.Log();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    cds::threading::Manager::detachThread();
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  stop.store(true);

  for (auto& t : threads) t.join();
}

}  // namespace
