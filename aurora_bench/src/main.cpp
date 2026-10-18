// Aurora benchmark: renders a stress scene with the frame-rate limiter disabled,
// then prints a single JSON line with per-frame timing statistics.
//
// Usage: aurora_bench [--scene static|dynamic|panel|many] [--count N] [--frames N] [--warmup N] [--max-seconds S] [--capture out.ppm] [--msaa 1|2|4|8] [--verbose]

#include "aurora_ui/aurora_ui.hpp"
#include "aurora_ui/components/aurora_card.hpp"
#include "aurora_ui/components/aurora_circle.hpp"
#include "aurora_ui/components/aurora_panel.hpp"
#include "aurora_ui/components/aurora_rounded_rect.hpp"
#include "aurora_ui/components/aurora_text.hpp"
#include "aurora_ui/utils/aurora_theme_settings.hpp"
#include "aurora_engine/profiling/aurora_profiler.hpp"
#include "aurora_engine/utils/log.hpp"

#include <sys/resource.h>
#include <dirent.h>

#include <fstream>
#include <set>
#include <sstream>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {
    using Clock = std::chrono::steady_clock;
    const Clock::time_point processStart = Clock::now();

    double processCpuMs() {
        rusage usage{};
        getrusage(RUSAGE_SELF, &usage);
        return usage.ru_utime.tv_sec * 1e3 + usage.ru_utime.tv_usec / 1e3 +
               usage.ru_stime.tv_sec * 1e3 + usage.ru_stime.tv_usec / 1e3;
    }

    // GPU memory owned by this process, from the DRM fdinfo stats (drm-total-*), in MiB. 0 if unavailable.
    double gpuMemoryMiB() {
        double totalKiB = 0.0;
        std::set<std::string> seenClients;
        DIR* dir = opendir("/proc/self/fdinfo");
        if (!dir) return 0.0;
        while (dirent* entry = readdir(dir)) {
            if (entry->d_name[0] == '.') continue;
            std::ifstream file(std::string("/proc/self/fdinfo/") + entry->d_name);
            std::string line, clientId;
            double fdKiB = 0.0;
            while (std::getline(file, line)) {
                if (line.rfind("drm-client-id:", 0) == 0) {
                    clientId = line.substr(14);
                } else if (line.rfind("drm-total-", 0) == 0) {
                    std::istringstream values(line.substr(line.find(':') + 1));
                    double value = 0.0;
                    std::string unit;
                    values >> value >> unit;
                    if (unit == "MiB") value *= 1024.0;
                    else if (unit != "KiB") value /= 1024.0;
                    fdKiB += value;
                }
            }
            // Several fds can refer to the same DRM client; count each client once
            if (!clientId.empty() && seenClients.insert(clientId).second) {
                totalKiB += fdKiB;
            }
        }
        closedir(dir);
        return totalKiB / 1024.0;
    }

    struct Series {
        std::vector<double> values;

        void add(double v) { values.push_back(v); }

        double mean() const {
            if (values.empty()) return 0.0;
            double sum = 0.0;
            for (double v : values) sum += v;
            return sum / static_cast<double>(values.size());
        }

        double percentile(double p) const {
            if (values.empty()) return 0.0;
            std::vector<double> sorted = values;
            std::sort(sorted.begin(), sorted.end());
            size_t idx = static_cast<size_t>(std::round(p * static_cast<double>(sorted.size() - 1)));
            return sorted[idx];
        }
    };

    struct Options {
        std::string scene = "static";
        int frames = 1500;
        int warmup = 200;
        double maxSeconds = 60.0; // hard cap so the window always closes itself
        std::string capturePath;  // if set, the last measured frame is written there as PPM
        int msaa = 8;             // maximum MSAA sample count
        int count = 40000;        // number of shapes in the "many" scene
    };

    // Phases recorded through AURORA_PROFILE in AuroraUI::run / the renderer
    const char* PHASES[] = {"Poll Events", "Update", "Begin Frame", "GPU Wait", "Render Components", "End Frame", "GPU Frame"};
    constexpr size_t PHASE_COUNT = sizeof(PHASES) / sizeof(PHASES[0]);
}

class BenchApp : public aurora::AuroraUI {
    public:
        explicit BenchApp(Options options)
            : AuroraUI{"Aurora Bench", static_cast<VkSampleCountFlagBits>(options.msaa)}, options{std::move(options)} {}

        void printResults(double setupMs) const {
            if (frameTimes.values.empty()) {
                std::printf("{\"scene\":\"%s\",\"frames\":0,\"error\":\"no frames measured\"}\n", options.scene.c_str());
                return;
            }
            double cpuPerFrame = (cpuEndMs - cpuStartMs) / static_cast<double>(frameTimes.values.size());
            double wallMs = std::chrono::duration<double, std::milli>(measureEnd - measureStart).count();

            std::printf("{\"scene\":\"%s\",\"frames\":%zu,\"components\":%zu,\"setup_ms\":%.3f,"
                        "\"fps\":%.2f,\"frame_ms_mean\":%.4f,\"frame_ms_p50\":%.4f,\"frame_ms_p95\":%.4f,\"frame_ms_p99\":%.4f,"
                        "\"process_cpu_ms_per_frame\":%.4f,\"cpu_work_ms_mean\":%.4f,\"cpu_work_ms_p50\":%.4f,\"gpu_mem_mib\":%.1f,\"first_frame_ms\":%.3f",
                        options.scene.c_str(), frameTimes.values.size(), componentCount, setupMs,
                        1000.0 * static_cast<double>(frameTimes.values.size()) / wallMs,
                        frameTimes.mean(), frameTimes.percentile(0.5), frameTimes.percentile(0.95), frameTimes.percentile(0.99),
                        cpuPerFrame, cpuWork.mean(), cpuWork.percentile(0.5), gpuMemory, firstFrameMs);
            for (size_t i = 0; i < PHASE_COUNT; ++i) {
                std::string key = PHASES[i];
                std::replace(key.begin(), key.end(), ' ', '_');
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                std::printf(",\"%s_ms\":%.4f", key.c_str(), phases[i].mean());
            }
            std::printf("}\n");
            std::fflush(stdout);
        }

    protected:
        void onSetup() override {
            if (options.scene == "panel") {
                buildPanelScene();
            } else if (options.scene == "many") {
                buildManyScene();
            } else {
                buildGridScene();
            }
        }

        void onUpdate(float) override {
            // Capture the final measured frame (rendered after this update)
            if (!options.capturePath.empty() && frameIndex == options.warmup + options.frames) {
                getRenderer().requestCapture(options.capturePath);
            }
            if (options.scene == "dynamic") {
                updateDynamicScene();
            } else if (options.scene == "panel") {
                updatePanelScene();
            }
        }

        void onFrameEnd() override {
            auto now = Clock::now();
            auto& profiler = aurora::AuroraProfiler::instance();

            if (frameIndex == 0) {
                firstFrameMs = std::chrono::duration<double, std::milli>(now - processStart).count();
            }

            if (frameIndex == options.warmup) {
                measureStart = now;
                cpuStartMs = processCpuMs();
            } else if (frameIndex > options.warmup) {
                frameTimes.add(std::chrono::duration<double, std::milli>(now - lastFrameEnd).count());

                double work = 0.0;
                for (size_t i = 0; i < PHASE_COUNT; ++i) {
                    double value = profiler.getStats(PHASES[i]).current;
                    phases[i].add(value);
                }
                // CPU-side work: everything except waiting on the GPU and the GPU itself
                work = phases[0].values.back() + phases[1].values.back() + phases[2].values.back()
                     - phases[3].values.back() + phases[4].values.back() + phases[5].values.back();
                cpuWork.add(work);
            }

            lastFrameEnd = now;
            frameIndex++;

            bool timedOut = std::chrono::duration<double>(now - runStart).count() > options.maxSeconds;
            if (timedOut && !closing) {
                aurora::log::engine()->warn("Benchmark exceeded {}s, stopping early", options.maxSeconds);
            }
            if ((frameIndex > options.warmup + options.frames || timedOut) && !closing) {
                closing = true;
                measureEnd = now;
                cpuEndMs = processCpuMs();
                gpuMemory = gpuMemoryMiB();
                requestClose();
            }
        }

    private:
        void buildGridScene() {
            const auto& theme = aurora::AuroraThemeSettings::get();

            // 60 cards (each card = 3 child shapes)
            for (int i = 0; i < 60; ++i) {
                auto card = create<aurora::AuroraCard>(glm::vec2{280.f, 140.f}, theme.PURPLE);
                card->setPosition(20.f + static_cast<float>(i % 6) * 310.f, 20.f + static_cast<float>(i / 6) * 105.f);
                componentCount += 4;
            }

            // 400 small rounded rectangles
            for (int i = 0; i < 400; ++i) {
                auto rect = create<aurora::AuroraRoundedRectangle>(glm::vec2{36.f, 24.f}, 6.f);
                rect->color = (i % 2) ? theme.BLUE : theme.ORANGE;
                rect->setPosition(10.f + static_cast<float>(i % 40) * 47.f, 700.f + static_cast<float>(i / 40) * 30.f);
                rects.push_back(rect);
                componentCount++;
            }

            // 300 circles
            for (int i = 0; i < 300; ++i) {
                auto circle = create<aurora::AuroraCircle>(8.f, theme.GRAY);
                circle->setPosition(20.f + static_cast<float>(i % 60) * 31.f, 1010.f + static_cast<float>(i / 60) * 12.f);
                componentCount++;
            }

            // 1200 text labels, half of them updated every frame in the dynamic scene
            for (int i = 0; i < 1200; ++i) {
                auto text = create<aurora::AuroraText>("Label " + std::to_string(i) + " value 0.000", 14.f);
                text->setPosition(30.f + static_cast<float>(i % 8) * 235.f, 40.f + static_cast<float>(i / 8) * 6.5f);
                texts.push_back(text);
                componentCount++;
            }
        }

        // Overlapping rectangles and circles, alternated so neighbours live in different render systems
        // and only the depth values keep later shapes on top of earlier ones
        void buildManyScene() {
            const auto& theme = aurora::AuroraThemeSettings::get();
            const glm::vec4 colors[] = {theme.BLUE, theme.ORANGE, theme.PURPLE, theme.GRAY};
            const int columns = 240;
            const float cell = 1900.f / static_cast<float>(columns);
            const float rowHeight = 1060.f * static_cast<float>(columns) / static_cast<float>(std::max(options.count, 1));

            for (int i = 0; i < options.count; ++i) {
                float x = 10.f + static_cast<float>(i % columns) * cell;
                float y = 10.f + static_cast<float>(i / columns) * rowHeight;
                const glm::vec4& color = colors[i % 4];
                if (i % 2) {
                    auto circle = create<aurora::AuroraCircle>(cell * 0.9f, color);
                    circle->setPosition(x, y);
                } else {
                    auto rect = create<aurora::AuroraRoundedRectangle>(glm::vec2{cell * 1.8f, cell * 1.8f}, 2.f);
                    rect->color = color;
                    rect->setPosition(x, y);
                }
                componentCount++;
            }
        }

        void updateDynamicScene() {
            char buffer[64];
            for (size_t i = 0; i < texts.size(); i += 2) {
                std::snprintf(buffer, sizeof(buffer), "Label %zu value %.3f", i, static_cast<double>(frameIndex % 1000) * 0.001 * static_cast<double>(i));
                texts[i]->setText(buffer);
            }

            float t = static_cast<float>(frameIndex) * 0.05f;
            for (size_t i = 0; i < rects.size(); ++i) {
                float dx = std::sin(t + static_cast<float>(i)) * 5.f;
                rects[i]->setPosition(10.f + static_cast<float>(i % 40) * 47.f + dx, 700.f + static_cast<float>(i / 40) * 30.f);
            }
        }

        void buildPanelScene() {
            auto panel = create<aurora::AuroraPanel>(900.f);
            for (int s = 0; s < 4; ++s) {
                auto& section = panel->addSection("SECTION " + std::to_string(s));
                for (int e = 0; e < 6; ++e) {
                    bool enclosed = (e % 2) == 0;
                    entries.push_back(section.addEntry("ENTRY " + std::to_string(e), "0", enclosed, aurora::AuroraThemeSettings::get().SUCCESS));
                    componentCount += 2;
                }
            }
        }

        void updatePanelScene() {
            for (size_t i = 0; i < entries.size(); ++i) {
                entries[i].setValue(std::to_string(frameIndex * static_cast<int>(i + 1)));
            }
        }

        Options options;

        std::vector<std::shared_ptr<aurora::AuroraText>> texts;
        std::vector<std::shared_ptr<aurora::AuroraRoundedRectangle>> rects;
        std::vector<aurora::AuroraEntryHandle> entries;
        size_t componentCount = 0;

        int frameIndex = 0;
        bool closing = false;
        Clock::time_point runStart = Clock::now();
        Clock::time_point lastFrameEnd{};
        Clock::time_point measureStart{};
        Clock::time_point measureEnd{};
        double cpuStartMs = 0.0;
        double cpuEndMs = 0.0;
        double gpuMemory = 0.0;
        double firstFrameMs = 0.0;

        Series frameTimes;
        Series cpuWork;
        Series phases[PHASE_COUNT];
};

int main(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--scene") == 0 && i + 1 < argc) options.scene = argv[++i];
        else if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) options.frames = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--warmup") == 0 && i + 1 < argc) options.warmup = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--max-seconds") == 0 && i + 1 < argc) options.maxSeconds = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--capture") == 0 && i + 1 < argc) options.capturePath = argv[++i];
        else if (std::strcmp(argv[i], "--msaa") == 0 && i + 1 < argc) options.msaa = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--count") == 0 && i + 1 < argc) options.count = std::atoi(argv[++i]);
    }

    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--verbose") == 0) verbose = true;
    }
    aurora::log::init(verbose ? spdlog::level::debug : spdlog::level::warn);

    auto setupStart = Clock::now();
    BenchApp app{options};
    double setupMs = std::chrono::duration<double, std::milli>(Clock::now() - setupStart).count();

    try {
        app.run();
    } catch (const std::exception& e) {
        aurora::log::engine()->error("{}", e.what());
        return EXIT_FAILURE;
    }

    app.printResults(setupMs);
    return EXIT_SUCCESS;
}
