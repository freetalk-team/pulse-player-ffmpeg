/// <reference types="node" />

export interface CoverImage {
    mime?: string;
    data?: Buffer;
}

export interface Metadata {
    title?: string;
    artist?: string | string[];
    album?: string;
    genre?: string | string[];
    year?: string;
    track_no?: string;
    cover?: string | CoverImage | Buffer;
}

export interface MediaInfo {
    metadata: Metadata;
    duration: number;
    bitrate: number;
    size: number;
    contentHash: string;
    codec?: string;
    width?: number;
    height?: number;
}

export interface RecordingStatus {
    id: string;
    streamUrl: string;
    outputFile: string;
    status: string;
}

export interface FFmpegAddon {
    parseMeta(path: string): MediaInfo;
    updateMeta(path: string, metadata: Metadata, output?: string): boolean;
    generateThumbnail(path: string, outputFile: string, second: number, maxWidth?: number): boolean;

    startRecording(streamUrl: string, outputFile: string, metadata?: Metadata): RecordingStatus;
    stopRecording(id: string): boolean;
}

declare const ffmpeg: FFmpegAddon;

export default ffmpeg;
