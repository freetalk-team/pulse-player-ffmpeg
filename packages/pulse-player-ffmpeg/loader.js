'use strict';

const supported = {
    linux: ['x64', 'arm64'],
    win32: ['x64', 'arm64']
};

const arches = supported[process.platform];

if (!arches || !arches.includes(process.arch)) {
    throw new Error(
        `Unsupported platform: ${process.platform}/${process.arch}`
    );
}

const pkg =
    `@freetalk-team/pulse-player-ffmpeg-${process.platform}-${process.arch}`;

module.exports = require(pkg);

