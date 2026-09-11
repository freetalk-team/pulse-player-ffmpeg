#include <napi.h>


Napi::Value ParseMetadata(const Napi::CallbackInfo&);
Napi::Value UpdateMetadata(const Napi::CallbackInfo&);
Napi::Value ExtractThumbnail(const Napi::CallbackInfo& info);
Napi::Value StartRecording(const Napi::CallbackInfo& info);
Napi::Value StopRecording(const Napi::CallbackInfo& info);
Napi::Value StopAllRecordings(const Napi::CallbackInfo& info);
Napi::Value GetRecordingStatus(const Napi::CallbackInfo& info);
Napi::Value GetActiveRecordings(const Napi::CallbackInfo& info);
Napi::Value PauseRecording(const Napi::CallbackInfo& info);
Napi::Value ResumeRecording(const Napi::CallbackInfo& info);

extern void init_ffmpeg();

Napi::Object Init(Napi::Env env, Napi::Object exports) {

    init_ffmpeg();

    exports.Set(Napi::String::New(env, "parseMeta"), Napi::Function::New(env, ParseMetadata));
    exports.Set(Napi::String::New(env, "updateMeta"), Napi::Function::New(env, UpdateMetadata));
    exports.Set(Napi::String::New(env, "generateThumbnail"), Napi::Function::New(env, ExtractThumbnail));
    exports.Set(Napi::String::New(env, "startRecording"), Napi::Function::New(env, StartRecording));
    exports.Set(Napi::String::New(env, "stopRecording"), Napi::Function::New(env, StopRecording));
    exports.Set(Napi::String::New(env, "getRecordingStatus"), Napi::Function::New(env, GetRecordingStatus));

    return exports;
}

NODE_API_MODULE(FFMpeg, Init)

