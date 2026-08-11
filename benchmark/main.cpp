/*
C++ CODEC FOR DIAMETER PROTOCOL (RFC 6733)
https://github.com/testillano/diametercodec
Licensed under the MIT License. Copyright (c) 2021 Eduardo Ramos
SPDX-License-Identifier: MIT
*/

// Reference micro-benchmark for the diametercodec hot path: per-message
// encode/decode plus the JSON conversions (toJson/fromJson) and a full
// roundtrip. It prints median and minimum ns/op per operation; capture the
// output yourself if you want to keep a record.
//
// It is NOT a correctness test (that is the unit-test suite); it only measures
// throughput/latency of the codec primitives on a representative base-protocol
// message.

// C
#include <libgen.h>  // basename

// Standard
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Project
#include <ert/diametercodec/codec/Message.hpp>
#include <ert/diametercodec/json/stacks.hpp>
#include <ert/diametercodec/stack/Dictionary.hpp>
#include <ert/tracing/Logger.hpp>
#include <nlohmann/json.hpp>

using ert::diametercodec::codec::Message;
using ert::diametercodec::stack::Dictionary;
using Buffer = ert::diametercodec::core::Buffer;

#ifndef BENCHMARK_BUILD_TYPE
#define BENCHMARK_BUILD_TYPE "Unknown"
#endif

namespace {

// Accumulator consumed at the end so the optimizer cannot elide the measured
// work. 'volatile' keeps the final read observable.
volatile std::uint64_t g_sink = 0;

struct OpResult {
    double median_ns = 0.0;
    double min_ns = 0.0;
};

// Runs 'fn' over 'iterations' a total of 'repeats' times and returns the median
// and minimum ns/op. The minimum is usually the most stable figure; the median
// resists occasional scheduler/turbo outliers.
template <typename Fn>
OpResult measure(std::size_t iterations, std::size_t repeats, Fn&& fn) {
    std::vector<double> samples;
    samples.reserve(repeats);
    for (std::size_t r = 0; r < repeats; ++r) {
        const auto start = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < iterations; ++i) fn();
        const auto end = std::chrono::steady_clock::now();
        const double totalNs = std::chrono::duration<double, std::nano>(end - start).count();
        samples.push_back(totalNs / static_cast<double>(iterations));
    }
    std::sort(samples.begin(), samples.end());

    OpResult res;
    res.min_ns = samples.front();
    const std::size_t n = samples.size();
    res.median_ns = (n % 2 == 1) ? samples[n / 2] : (samples[n / 2 - 1] + samples[n / 2]) / 2.0;
    return res;
}

// CPU model string (best effort) for the optional JSONL line, so a captured
// result stays interpretable. Read from /proc/cpuinfo; "unknown" if absent.
std::string cpuModel() {
    std::ifstream ifs("/proc/cpuinfo");
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.rfind("model name", 0) == 0) {
            const auto colon = line.find(':');
            if (colon != std::string::npos) {
                std::string model = line.substr(colon + 1);
                const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
                model.erase(model.begin(), std::find_if(model.begin(), model.end(), notSpace));
                model.erase(std::find_if(model.rbegin(), model.rend(), notSpace).base(), model.end());
                return model;
            }
        }
    }
    return "unknown";
}

}  // namespace

int main(int argc, char* argv[]) {
    const char* progname = basename(argv[0]);
    ert::tracing::Logger::initialize(progname);
    // Production-like logging: keep debug traces OFF so we measure the codec
    // itself and not the tracing subsystem (encode/decode emit LOGDEBUG lines).
    ert::tracing::Logger::setLevel("Warning");

    // Usage: diametercodec-benchmark [iterations] [--repeats N] [--jsonl]
    std::size_t iterations = 100000;
    std::size_t repeats = 5;
    bool jsonl = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--repeats" && i + 1 < argc) {
            repeats = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--jsonl") {
            jsonl = true;
        } else if (!arg.empty() && arg[0] != '-') {
            try {
                iterations = static_cast<std::size_t>(std::stoul(arg));
            } catch (const std::exception&) {
                std::cerr << "Invalid iteration count '" << arg << "', using default " << iterations << "\n";
            }
        } else {
            std::cerr << "Ignoring unknown argument '" << arg << "'\n";
        }
    }
    if (iterations == 0) iterations = 1;
    if (repeats == 0) repeats = 1;

    // The dictionary is parsed ONCE here (as consuming apps do at startup) and
    // reused by const reference in every timed operation. Parsing is never on
    // the measured hot path.
    Dictionary dictionary;
    dictionary.load(ert::diametercodec::json::stacks::base);

    // Representative base-protocol message: Capabilities-Exchange-Request (code
    // 257). Covers the common AVP formats: DiameterIdentity, Address, Unsigned32
    // and UTF8String.
    const nlohmann::json messageJson = {{"_header",
                                         {{"version", 1},
                                          {"flags", 128},
                                          {"command-code", 257},
                                          {"request", true},
                                          {"application-id", 0},
                                          {"hop-by-hop-id", 0x12345678},
                                          {"end-to-end-id", 0x87654321}}},
                                        {"Origin-Host", "benchmark.host.example.com"},
                                        {"Origin-Realm", "example.com"},
                                        {"Host-IP-Address", "127.0.0.1"},
                                        {"Vendor-Id", 10415},
                                        {"Product-Name", "ert_diametercodec_benchmark"},
                                        {"Origin-State-Id", 1699999999},
                                        {"Supported-Vendor-Id", 10415},
                                        {"Auth-Application-Id", 16777238},
                                        {"Firmware-Revision", 1}};

    Message baseMsg = Message::fromJson(messageJson, dictionary);
    Buffer wire = baseMsg.encode(dictionary);

    struct Op {
        const char* name;
        std::function<void()> fn;
    };
    const std::vector<Op> ops = {// encode: structured Message -> binary wire format.
                                 {"encode",
                                  [&]() {
                                      Buffer out = baseMsg.encode(dictionary);
                                      g_sink += out.size();
                                  }},
                                 // decode: binary wire format -> structured Message.
                                 {"decode",
                                  [&]() {
                                      Message m;
                                      m.decode(wire.data(), wire.size(), dictionary);
                                      g_sink += m.avps().size();
                                  }},
                                 // toJson: structured Message -> JSON.
                                 {"toJson",
                                  [&]() {
                                      nlohmann::json j = baseMsg.toJson(dictionary);
                                      g_sink += j.size();
                                  }},
                                 // fromJson: JSON -> structured Message.
                                 {"fromJson",
                                  [&]() {
                                      Message m = Message::fromJson(messageJson, dictionary);
                                      g_sink += m.avps().size();
                                  }},
                                 // roundtrip: fromJson -> encode -> decode -> toJson (full path).
                                 {"roundtrip", [&]() {
                                      Message m = Message::fromJson(messageJson, dictionary);
                                      Buffer b = m.encode(dictionary);
                                      Message d;
                                      d.decode(b.data(), b.size(), dictionary);
                                      nlohmann::json j = d.toJson(dictionary);
                                      g_sink += j.size() + b.size();
                                  }}};

    // Measure all operations first, then emit output.
    std::vector<std::pair<std::string, OpResult>> results;
    results.reserve(ops.size());
    for (const auto& op : ops) results.emplace_back(op.name, measure(iterations, repeats, op.fn));

    // Consume the accumulator so none of the loops above can be optimized away.
    if (g_sink == 0xFFFFFFFFFFFFFFFFULL) std::cerr << "unreachable\n";

    if (jsonl) {
        // One machine-readable line to STDOUT (no file is written; redirect the
        // process output yourself if you want to keep it). Git/commit context is
        // intentionally omitted (not reliably available from a container); tag it
        // yourself when capturing.
        nlohmann::json line = {{"build_type", BENCHMARK_BUILD_TYPE},
                               {"cpu_model", cpuModel()},
                               {"nproc", std::thread::hardware_concurrency()},
                               {"iterations", iterations},
                               {"repeats", repeats},
                               {"message", "Capabilities-Exchange-Request"},
                               {"avps", baseMsg.avps().size()},
                               {"wire_bytes", wire.size()}};
        nlohmann::json res = nlohmann::json::object();
        for (const auto& [name, r] : results) {
            const double opsPerSec = (r.median_ns > 0.0) ? (1e9 / r.median_ns) : 0.0;
            res[name] = {{"median_ns", r.median_ns}, {"min_ns", r.min_ns}, {"ops_per_s", opsPerSec}};
        }
        line["results"] = res;
        std::cout << line.dump() << "\n";
        return 0;
    }

    std::cout << "ert_diametercodec micro-benchmark\n"
              << "  build      : " << BENCHMARK_BUILD_TYPE << "\n"
              << "  iterations : " << iterations << " x " << repeats << " repeats\n"
              << "  message    : Capabilities-Exchange-Request (" << baseMsg.avps().size() << " AVPs, " << wire.size()
              << " wire bytes)\n\n"
              << "  operation  | median ns/op |    min ns/op |    throughput\n"
              << "  -----------+--------------+--------------+---------------\n";

    for (const auto& [name, r] : results) {
        const double opsPerSec = (r.median_ns > 0.0) ? (1e9 / r.median_ns) : 0.0;
        std::cout << "  " << std::left << std::setw(10) << name << " | " << std::right << std::setw(12) << std::fixed
                  << std::setprecision(1) << r.median_ns << " | " << std::setw(12) << r.min_ns << " | " << std::setw(10)
                  << std::setprecision(0) << opsPerSec << " ops/s\n";
    }

    return 0;
}
