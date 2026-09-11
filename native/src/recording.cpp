#include <napi.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <condition_variable>
#include <stdexcept>
#include <filesystem>
#include <system_error>

#include "ffmpeg.h"
#include "common.h"

namespace fs = std::filesystem;

#define USE_FORMAT_PARAM 0

// New multi-recording declarations
Napi::Value StartRecording(const Napi::CallbackInfo& info);
Napi::Value StopRecording(const Napi::CallbackInfo& info);
Napi::Value StopAllRecordings(const Napi::CallbackInfo& info);
Napi::Value GetRecordingStatus(const Napi::CallbackInfo& info);
Napi::Value GetActiveRecordings(const Napi::CallbackInfo& info);
Napi::Value PauseRecording(const Napi::CallbackInfo& info);
Napi::Value ResumeRecording(const Napi::CallbackInfo& info);

struct RecordingMeta {
	std::string title;
	std::string artist;
	std::string genre;
	std::string cover;

	void parse(Napi::Object);
};

// Recorder state for a single stream
struct RecorderState {
	std::string id;
	std::string streamUrl;
	std::string outputFile;
	std::string format; // mp3, aac, flac, etc.
	RecordingMeta meta;

	std::atomic<bool> isRecording{false};
	std::atomic<bool> shouldStop{false};
	std::atomic<bool> isPaused{false};

	std::thread recorderThread;

	std::mutex stateMutex;
	std::condition_variable pauseCV;
	
	// Stats
	std::atomic<int64_t> bytesWritten{0};
	std::atomic<int64_t> packetsProcessed{0};
	std::atomic<int64_t> durationMs{0};
	std::chrono::steady_clock::time_point startTime;
	
	// FFmpeg contexts
	AVFormatContext* inputCtx = nullptr;
	AVFormatContext* outputCtx = nullptr;
	int audioStreamIndex = -1;
	
	// Progress callback
	Napi::ThreadSafeFunction progressCallback;
	Napi::ThreadSafeFunction errorCallback;
	Napi::ThreadSafeFunction completionCallback;
	
	// ~RecorderState() {
	// 	printf("# Recording state destructor\n");

	// 	if (recorderThread.joinable()) {
	// 		shouldStop = true;
	// 		recorderThread.join();
	// 	}

	// 	printf("# Recording state destructor complete\n");
	// 	// cleanupFFmpeg();
	// }

	~RecorderState() = default;
	
	void cleanupFFmpeg() {
		printf("# Recording cleanup\n");

		if (inputCtx) {
			avformat_close_input(&inputCtx);
			inputCtx = nullptr;
		}
		if (outputCtx) {
			if (!(outputCtx->oformat->flags & AVFMT_NOFILE)) {
				avio_closep(&outputCtx->pb);
			}
			avformat_free_context(outputCtx);
			outputCtx = nullptr;
		}
	}
};

// Recording manager
class RecorderManager {
private:
	std::unordered_map<std::string, std::shared_ptr<RecorderState>> recorders;
	std::mutex managerMutex;
	std::atomic<int> nextId{0};
	
public:
	std::string createRecorder(const std::string& streamUrl, 
							   const std::string& outputFile,
							   const std::string& format = "mp3",
								const RecordingMeta& meta = {});
	
	bool stopRecorder(const std::string& id);
	void stopAllRecorders();
	bool pauseRecorder(const std::string& id);
	bool resumeRecorder(const std::string& id);
	std::shared_ptr<RecorderState> getRecorder(const std::string& id);
	std::vector<std::string> getActiveRecorderIds();
	Napi::Object getStatus(const std::string& id, Napi::Env env);
	
	static RecorderManager& getInstance() {
		static RecorderManager instance;
		return instance;
	}
	
private:
	RecorderManager() = default;
	~RecorderManager() = default;
	RecorderManager(const RecorderManager&) = delete;
	RecorderManager& operator=(const RecorderManager&) = delete;
};

// Global manager instance
static RecorderManager& g_recorderManager = RecorderManager::getInstance();

// Core recording function (runs in thread)
void RecordStream(std::shared_ptr<RecorderState> state);

// Recording Manager Implementation
std::string RecorderManager::createRecorder(const std::string& streamUrl, 
										   const std::string& outputFile,
										   const std::string& format,
											const RecordingMeta& meta) {
	
	auto state = std::make_shared<RecorderState>();
	state->id = "rec_" + std::to_string(nextId++);
	state->streamUrl = streamUrl;
	state->outputFile = outputFile;
	state->format = format;
	state->meta = meta;
	state->isRecording = true;
	state->shouldStop = false;
	state->startTime = std::chrono::steady_clock::now();

	{
        std::lock_guard<std::mutex> lock(managerMutex);
        recorders[state->id] = state;
    }
	
	
	// Start recording thread
	state->recorderThread = std::thread([state]() {
		RecordStream(state);
	state->isRecording.store(false);
	});
	
	return state->id;
}

bool RecorderManager::stopRecorder(const std::string& id) {

	std::shared_ptr<RecorderState> state;

    {
        std::lock_guard<std::mutex> lock(managerMutex);

        auto it = recorders.find(id);

        if (it == recorders.end()) {
            return false;
        }

        state = it->second;
        state->shouldStop.store(true);

        // Important if recording is paused
        state->pauseCV.notify_all();

        recorders.erase(it);
    }

    // Wait for RecordStream() to cleanly finish.
    if (state->recorderThread.joinable()) {
        state->recorderThread.join();
    }

    return true;
}

void RecorderManager::stopAllRecorders() {
	std::lock_guard<std::mutex> lock(managerMutex);
	
	for (auto& pair : recorders) {
		pair.second->shouldStop = true;
	}
}

bool RecorderManager::pauseRecorder(const std::string& id) {
	std::lock_guard<std::mutex> lock(managerMutex);
	
	auto it = recorders.find(id);
	if (it == recorders.end() || !it->second->isRecording) {
		return false;
	}
	
	it->second->isPaused = true;
	return true;
}

bool RecorderManager::resumeRecorder(const std::string& id) {
	std::lock_guard<std::mutex> lock(managerMutex);
	
	auto it = recorders.find(id);
	if (it == recorders.end() || !it->second->isRecording) {
		return false;
	}
	
	it->second->isPaused = false;
	it->second->pauseCV.notify_one();
	return true;
}

std::shared_ptr<RecorderState> RecorderManager::getRecorder(const std::string& id) {
	std::lock_guard<std::mutex> lock(managerMutex);
	
	auto it = recorders.find(id);
	if (it == recorders.end()) {
		return nullptr;
	}
	return it->second;
}

std::vector<std::string> RecorderManager::getActiveRecorderIds() {
	std::lock_guard<std::mutex> lock(managerMutex);
	
	std::vector<std::string> ids;
	for (const auto& pair : recorders) {
		if (pair.second->isRecording) {
			ids.push_back(pair.first);
		}
	}
	return ids;
}

Napi::Object RecorderManager::getStatus(const std::string& id, Napi::Env env) {
	auto state = getRecorder(id);
	Napi::Object result = Napi::Object::New(env);
	
	if (!state) {
		result.Set("exists", Napi::Boolean::New(env, false));
		return result;
	}
	
	result.Set("id", Napi::String::New(env, state->id));
	result.Set("streamUrl", Napi::String::New(env, state->streamUrl));
	result.Set("outputFile", Napi::String::New(env, state->outputFile));
	result.Set("isRecording", Napi::Boolean::New(env, state->isRecording.load()));
	result.Set("isPaused", Napi::Boolean::New(env, state->isPaused.load()));
	result.Set("bytesWritten", Napi::Number::New(env, state->bytesWritten.load()));
	result.Set("packetsProcessed", Napi::Number::New(env, state->packetsProcessed.load()));
	
	auto now = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - state->startTime);
	result.Set("durationMs", Napi::Number::New(env, duration.count()));
	
	return result;
}

void RecordStream(std::shared_ptr<RecorderState> state) {
	AVFormatContext* inputCtx = nullptr;
	AVFormatContext* outputCtx = nullptr;
	AVBSFContext* bsfCtx = nullptr;
	int ret;
	
	try {
		// Open input stream
		inputCtx = avformat_alloc_context();
		if (!inputCtx) {
			throw std::runtime_error("Failed to allocate input context");
		}
		
		// Set timeout options
		AVDictionary* options = nullptr;
		av_dict_set(&options, "timeout", "30000000", 0);
		av_dict_set(&options, "stimeout", "30000000", 0);
		av_dict_set(&options, "reconnect", "1", 0);
		av_dict_set(&options, "reconnect_streamed", "1", 0);
		av_dict_set(&options, "reconnect_delay_max", "3000", 0);
		
		if ((ret = avformat_open_input(&inputCtx, state->streamUrl.c_str(), nullptr, &options)) < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			throw std::runtime_error("Failed to open input: " + std::string(errbuf));
		}
		av_dict_free(&options);
		
		// Find stream info
		if ((ret = avformat_find_stream_info(inputCtx, nullptr)) < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			throw std::runtime_error("Failed to find stream info: " + std::string(errbuf));
		}
		
		// Find audio stream
		state->audioStreamIndex = -1;
		for (unsigned int i = 0; i < inputCtx->nb_streams; i++) {
			if (inputCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				state->audioStreamIndex = i;
				break;
			}
		}
		
		if (state->audioStreamIndex == -1) {
			throw std::runtime_error("No audio stream found");
		}
		
#if USE_FORMAT_PARAM
		// Allocate output context based on format
		std::string formatName = state->format;
		const AVOutputFormat* outputFormat = nullptr;
		
		if (formatName == "mp3") {
			outputFormat = av_guess_format("mp3", nullptr, nullptr);
		} else if (formatName == "aac") {
			outputFormat = av_guess_format("adts", nullptr, nullptr);
		} else if (formatName == "flac") {
			outputFormat = av_guess_format("flac", nullptr, nullptr);
		} else if (formatName == "ogg") {
			outputFormat = av_guess_format("ogg", nullptr, nullptr);
		} else if (formatName == "wav") {
			outputFormat = av_guess_format("wav", nullptr, nullptr);
		} else {
			outputFormat = av_guess_format(nullptr, state->outputFile.c_str(), nullptr);
		}
		
		if (!outputFormat) {
			// Default to mp3
			outputFormat = av_guess_format("mp3", nullptr, nullptr);
		}
		
		ret = avformat_alloc_output_context2(&outputCtx, outputFormat, nullptr, state->outputFile.c_str());
#else
		ret = avformat_alloc_output_context2(&outputCtx, nullptr, nullptr, state->outputFile.c_str());

#endif
		
		if (ret < 0 || !outputCtx) {
			throw std::runtime_error("Failed to allocate output context");
		}

		AVStream* inStream = inputCtx->streams[state->audioStreamIndex];
		
		// Create output stream
		AVStream* outStream = avformat_new_stream(outputCtx, nullptr);
		if (!outStream) {
			throw std::runtime_error("Failed to create output stream");
		}
		
		// Copy codec parameters
		ret = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
		if (ret < 0) {
			throw std::runtime_error("Failed to copy codec parameters");
		}

		if (outStream->codecpar->codec_id == AV_CODEC_ID_AAC &&
            outputCtx->oformat->name &&
            (strcmp(outputCtx->oformat->name, "ipod") == 0 || strcmp(outputCtx->oformat->name, "mp4"))) {

            const AVBitStreamFilter* filter =
                av_bsf_get_by_name("aac_adtstoasc");

            if (!filter) {
                throw std::runtime_error(
                    "aac_adtstoasc bitstream filter not available");
            }

            ret = av_bsf_alloc(filter, &bsfCtx);
            if (ret < 0) {
                throw std::runtime_error(
                    "Failed to allocate aac_adtstoasc");
            }

            ret = avcodec_parameters_copy(
                bsfCtx->par_in,
                outStream->codecpar
            );

            if (ret < 0) {
                av_bsf_free(&bsfCtx);
                throw std::runtime_error(
                    "Failed to configure aac_adtstoasc");
            }

            bsfCtx->time_base_in = outStream->time_base;

            ret = av_bsf_init(bsfCtx);
            if (ret < 0) {
                av_bsf_free(&bsfCtx);
                throw std::runtime_error(
                    "Failed to initialize aac_adtstoasc");
            }

            // The BSF has now generated the MPEG-4 AAC
            // codec parameters, including extradata.
            ret = avcodec_parameters_copy(
                outStream->codecpar,
                bsfCtx->par_out
            );

            if (ret < 0) {
                av_bsf_free(&bsfCtx);
                throw std::runtime_error(
                    "Failed to copy AAC BSF parameters");
            }

            outStream->time_base = bsfCtx->time_base_out;
        }
        else {
		
			outStream->time_base = inStream->time_base;
		}
		
		// Open output file
		if (!(outputCtx->oformat->flags & AVFMT_NOFILE)) {
			ret = avio_open(&outputCtx->pb, state->outputFile.c_str(), AVIO_FLAG_WRITE);
			if (ret < 0) {
				char errbuf[AV_ERROR_MAX_STRING_SIZE];
				av_strerror(ret, errbuf, sizeof(errbuf));
				throw std::runtime_error("Failed to open output: " + std::string(errbuf));
			}
		}

		auto& meta = state->meta;

		bool isOgg = strcmp(outputCtx->oformat->name, "ogg") == 0;
		bool isMp3 = strcmp(outputCtx->oformat->name, "mp3") == 0;

		// log("Metadata pointer: %p", metadata);
		/*
		For ogg add use stream->matadata !!!
		*/
		AVDictionary* metadata = nullptr;

		if (!meta.title.empty()) 
			av_dict_set(&metadata, "title", meta.title.c_str(), 0);

		if (!meta.artist.empty())
			av_dict_set(&metadata, "artist", meta.artist.c_str(), 0);

		if (!meta.genre.empty())
			av_dict_set(&metadata, "genre", meta.genre.c_str(), 0);

		if (isOgg) outStream->metadata = metadata;
		else outputCtx->metadata = metadata;

		std::vector<uint8_t> cover;
		int coverStreamIndex = -1;

		if (!meta.cover.empty()) {
			cover = readFile(meta.cover);
			if (cover.size() > 0) {

				coverStreamIndex = isOgg 
					? addCoverImageStream(outStream, cover)
					: addCoverImageStream(outputCtx, cover);

				log("Cover stream added: index=%d, size=%lu", coverStreamIndex, cover.size());
			}
		}

		options = nullptr;

		if (isMp3)
			av_dict_set(&options, "id3v2_version", "3", 0);
	
		// Write header
		ret = avformat_write_header(outputCtx, &options);
		av_dict_free(&options);

		if (ret < 0) {
			throw std::runtime_error("Failed to write header");
		}
		
		// Store contexts in state
		state->inputCtx = inputCtx;
		state->outputCtx = outputCtx;

		if (coverStreamIndex >= 0) {
			// do not add for ogg
			addCoverImage(outputCtx, coverStreamIndex, cover);
		}
		
		// Main recording loop
		AVPacket packet;
		av_init_packet(&packet);
		packet.data = nullptr;
		packet.size = 0;
		
		int64_t lastProgressUpdate = 0;
		const int64_t PROGRESS_INTERVAL = 1000000; // 1 second in microseconds

		int64_t firstTimestamp = AV_NOPTS_VALUE;
		
		while (!state->shouldStop.load()) {
			// Handle pause
			if (state->isPaused.load()) {
				std::unique_lock<std::mutex> lock(state->stateMutex);
				state->pauseCV.wait(lock, [&state]() { 
					return !state->isPaused.load() || state->shouldStop.load(); 
				});
				if (state->shouldStop.load()) {
					break;
				}
				continue;
			}
			
			// Read packet
			ret = av_read_frame(inputCtx, &packet);
			if (ret < 0) {
				if (ret == AVERROR_EOF) {
					// Stream ended, try to reconnect if not stopping
					if (!state->shouldStop.load()) {
						// Wait and try to reconnect
						std::this_thread::sleep_for(std::chrono::seconds(5));
						continue;
					}
					break;
				}
				// Other error, might be recoverable
				av_packet_unref(&packet);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			
			// Process audio packets only
			if (packet.stream_index == state->audioStreamIndex) {

				auto inTimeBase = inStream->time_base;
				auto outTimeBase = outStream->time_base;

				av_packet_rescale_ts(&packet, inTimeBase, outTimeBase);

				int64_t timestamp = packet.dts != AV_NOPTS_VALUE
            		? packet.dts
            		: packet.pts;

				if (firstTimestamp == AV_NOPTS_VALUE &&
					timestamp != AV_NOPTS_VALUE) {

					firstTimestamp = timestamp;
				}

				if (firstTimestamp != AV_NOPTS_VALUE) {
					if (packet.pts != AV_NOPTS_VALUE)
						packet.pts -= firstTimestamp;

					if (packet.dts != AV_NOPTS_VALUE)
						packet.dts -= firstTimestamp;
				}

				// Convert timestamps
				// packet.pts = av_rescale_q_rnd(packet.pts,
				// 	inputCtx->streams[packet.stream_index]->time_base,
				// 	outStream->time_base,
				// 	(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				// packet.dts = av_rescale_q_rnd(packet.dts,
				// 	inputCtx->streams[packet.stream_index]->time_base,
				// 	outStream->time_base,
				// 	(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
				// packet.duration = av_rescale_q(packet.duration,
				// 	inputCtx->streams[packet.stream_index]->time_base,
				// 	outStream->time_base);

				packet.stream_index = 0;
				packet.pos = -1;

				if (bsfCtx) {
                    ret = av_bsf_send_packet(bsfCtx, &packet);

                    if (ret < 0) {
                        char errbuf[AV_ERROR_MAX_STRING_SIZE];
                        av_strerror(ret, errbuf, sizeof(errbuf));

                        fprintf(stderr,
                                "Error sending packet to BSF: %s\n",
                                errbuf);

                        av_packet_unref(&packet);
                        continue;
                    }

                    while ((ret = av_bsf_receive_packet(bsfCtx, &packet)) >= 0) {

                        packet.stream_index = 0;
                        packet.pos = -1;

                        ret = av_interleaved_write_frame(outputCtx, &packet);

                        av_packet_unref(&packet);

                        if (ret < 0) {
                            char errbuf[AV_ERROR_MAX_STRING_SIZE];
                            av_strerror(ret, errbuf, sizeof(errbuf));

                            fprintf(stderr,
                                    "Error writing packet: %s\n",
                                    errbuf);

                            break;
                        }
                    }

					state->packetsProcessed++;
					state->bytesWritten += packet.size;
					
					// Update progress
					auto now = av_gettime();
					if (now - lastProgressUpdate > PROGRESS_INTERVAL) {
						lastProgressUpdate = now;
						// Could trigger progress callback here
					}

                    if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF && ret < 0) {
                        // BSF error
                    }

                } else {
				
					// Write packet
					ret = av_interleaved_write_frame(outputCtx, &packet);
					if (ret < 0) {
						// Log error but continue
					} else {
						state->packetsProcessed++;
						state->bytesWritten += packet.size;
						
						// Update progress
						auto now = av_gettime();
						if (now - lastProgressUpdate > PROGRESS_INTERVAL) {
							lastProgressUpdate = now;
							// Could trigger progress callback here
						}
					}
				}
			}
			
			av_packet_unref(&packet);
		}

		if (bsfCtx) {
            ret = av_bsf_send_packet(bsfCtx, nullptr);

            if (ret >= 0) {
                while ((ret = av_bsf_receive_packet(
                            bsfCtx, &packet)) >= 0) {

                    packet.stream_index = 0;
                    packet.pos = -1;

                    av_interleaved_write_frame(outputCtx, &packet);
                    av_packet_unref(&packet);
                }
            }

			av_bsf_free(&bsfCtx);
        }
		
		// Write trailer
		av_write_trailer(outputCtx);
		
		// Mark as stopped
		state->isRecording = false;
	} catch (const std::exception& e) {
		fprintf(stderr, "Recording error for %s: %s\n", state->id.c_str(), e.what());
		state->isRecording = false;
	}
	
	// Cleanup
	state->cleanupFFmpeg();
}

Napi::Value StartRecording(const Napi::CallbackInfo& info) {

	// dump_version();

	Napi::Env env = info.Env();
	
	if (info.Length() < 2) {
		Napi::TypeError::New(env, "Expected stream URL and output file path")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
	
	if (!info[0].IsString() || !info[1].IsString()) {
		Napi::TypeError::New(env, "Both arguments must be strings")
			.ThrowAsJavaScriptException();
		return env.Null();
	}

	// In StartRecording, allow callback function
	// if (info.Length() >= 4 && info[3].IsFunction()) {
	//     state->progressCallback = Napi::ThreadSafeFunction::New(
	//         env,
	//         info[3].As<Napi::Function>(),
	//         "ProgressCallback",
	//         0,
	//         1
	//     );
	// }

	// // Add segment duration parameter
	// int segmentDuration = 3600; // 1 hour default
	// if (info.Length() >= 4 && info[3].IsNumber()) {
	//     segmentDuration = info[3].As<Napi::Number>().Int32Value();
	// }

	// // In RecordStream, segment the output
	// int64_t segmentStartTime = 0;
	// int segmentCounter = 0;

	// // When duration exceeds segmentDuration, start new file
	// if (durationMs > segmentStartTime + (segmentDuration * 1000)) {
	//     // Close current file
	//     av_write_trailer(outputCtx);
	//     // Open new file with segment number
	//     std::string newFile = state->outputFile + "." + std::to_string(++segmentCounter);
	//     // Reopen output context...
	//     segmentStartTime = durationMs;
	// }
	
	std::string streamUrl = info[0].As<Napi::String>().Utf8Value();
	std::string outputFile = info[1].As<Napi::String>().Utf8Value();

	log("Recording: stream=%s, file=%s", streamUrl.c_str(), outputFile.c_str());

	fs::path outputPath(outputFile);
	fs::path outputDir = outputPath.parent_path();

	if (!outputDir.empty()) {
		std::error_code ec;

		fs::create_directories(outputDir, ec);

		if (ec) {
			// throw std::runtime_error(
			// 	"Failed to create output directory '" +
			// 	outputDir.string() + "': " +
			// 	ec.message()
			// );

			Napi::Error::New(env, "Failed to create output directory").ThrowAsJavaScriptException();
			return env.Null();
		}
	}
	
	// Optional format parameter
	std::string format = "mp3";
	RecordingMeta meta;

#if USE_FORMAT_PARAM
	if (info.Length() >= 3 && info[2].IsString()) {
		format = info[2].As<Napi::String>().Utf8Value();
	}

	if (info.Length() >= 4 && info[3].IsString()) {
		station = info[3].As<Napi::String>().Utf8Value();
	}
#else
	if (info.Length() >= 3) {
		if (info[2].IsString()) {
			meta.title = info[2].As<Napi::String>().Utf8Value();
		}
		else if (info[2].IsObject()) {

			log("Recording: meta info provided");

			Napi::Object obj = info[2].As<Napi::Object>();

			meta.parse(obj);

		}
	}
#endif
	
	// Create recorder
	std::string id = g_recorderManager.createRecorder(streamUrl, outputFile, format, meta);
	
	Napi::Object result = Napi::Object::New(env);
	result.Set("id", Napi::String::New(env, id));
	result.Set("streamUrl", Napi::String::New(env, streamUrl));
	result.Set("outputFile", Napi::String::New(env, outputFile));
	result.Set("format", Napi::String::New(env, format));
	result.Set("status", Napi::String::New(env, "recording"));
	
	return result;
}

Napi::Value StopRecording(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	if (info.Length() < 1 || !info[0].IsString()) {
		Napi::TypeError::New(env, "Expected recorder ID string")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
	
	std::string id = info[0].As<Napi::String>().Utf8Value();
	bool success = g_recorderManager.stopRecorder(id);
	
	if (!success) {
		Napi::Error::New(env, "Recorder not found").ThrowAsJavaScriptException();
		return env.Null();
	}
	
	return Napi::Boolean::New(env, true);
}

Napi::Value StopAllRecordings(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	g_recorderManager.stopAllRecorders();
	return Napi::Boolean::New(env, true);
}

Napi::Value GetRecordingStatus(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	if (info.Length() < 1 || !info[0].IsString()) {
		Napi::TypeError::New(env, "Expected recorder ID string")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
	
	std::string id = info[0].As<Napi::String>().Utf8Value();
	return g_recorderManager.getStatus(id, env);
}

Napi::Value GetActiveRecordings(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	auto ids = g_recorderManager.getActiveRecorderIds();
	Napi::Array result = Napi::Array::New(env, ids.size());
	
	for (size_t i = 0; i < ids.size(); i++) {
		result[i] = Napi::String::New(env, ids[i]);
	}
	
	return result;
}

Napi::Value PauseRecording(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	if (info.Length() < 1 || !info[0].IsString()) {
		Napi::TypeError::New(env, "Expected recorder ID string")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
	
	std::string id = info[0].As<Napi::String>().Utf8Value();
	bool success = g_recorderManager.pauseRecorder(id);
	
	return Napi::Boolean::New(env, success);
}

Napi::Value ResumeRecording(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	
	if (info.Length() < 1 || !info[0].IsString()) {
		Napi::TypeError::New(env, "Expected recorder ID string")
			.ThrowAsJavaScriptException();
		return env.Null();
	}
	
	std::string id = info[0].As<Napi::String>().Utf8Value();
	bool success = g_recorderManager.resumeRecorder(id);
	
	return Napi::Boolean::New(env, success);
}

void RecordingMeta::parse(Napi::Object obj) {
	auto title = obj.Get("title");
	if (title.IsString())
		this->title = title.As<Napi::String>().Utf8Value();

	auto artist = obj.Get("artist");
	if (artist.IsString())
		this->artist = artist.As<Napi::String>().Utf8Value();

	auto genre = obj.Get("genre");
	if (genre.IsString())
		this->genre = genre.As<Napi::String>().Utf8Value();

	auto cover = obj.Get("cover");
	if (cover.IsString()) {

		auto path = cover.As<Napi::String>().Utf8Value();
		std::filesystem::path p(path);

		std::string ext = p.extension().string();

		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) {
				return std::tolower(c);
			});

		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
			this->cover = path;
		}
	}


	log("Recording: meta info => title=%s, artist=%s, genre=%s, cover=%s"
		, this->title.c_str()
		, this->artist.c_str()
		, this->genre.c_str()
		, this->cover.c_str()
	);
}

/*

ffmpeg -i "http://icecast.radiofrance.fr/franceinfo-hifi.aac" -c copy radiofrance1.m4a
ffmpeg -i "https://icecast.walmradio.com:8443/jazz_opus" -c copy jazz1.ogg

*/
