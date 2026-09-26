# 🎬 AiraLang `youtube` Module
> **YouTube Video and Audio Downloader Engine (via APM)**

## Installation
```bash
aira install youtube
# or
airalang install youtube
```

## Import
```aira
import "youtube";
// or alias:
import "yt";
```

## Methods & Properties
- `youtube.get(url)`: Fetches video metadata and returns a `video` object.
  - `video.title`: Video title string.
  - `video.duration`: Duration in seconds.
  - `video.author`: Channel / uploader name.
  - `video.views`: Video view count.
  - `video.stream(format)`: Returns a `stream` object (format can be `"mp4"` or `"mp3"`).
    - `stream.download([save_path])`: Downloads the stream to specified file or default title.
- `os.save(filename)`: Configures and creates the destination file path.

## Example
```aira
import "youtube";
import "os";

let url = input("Enter Your URL Here : ");

let video = youtube.get(url);
say "Video Title : " + video.title;

let stream = video.stream("mp4");

let save = os.save("video.mp4");

say "Starting Download...";
stream.download(save);

say "Download Finished Successfully!";
```
