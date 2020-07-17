## 用途

探测媒体信息，例如：分辨率、帧率、比特率（未实现）



### 接口示例

POST http://127.0.0.1:62062/api/v1/probe?timeout=10000

```json
[
    "rtsp://admin:ABCabc123@192.168.205.64",
    "rtsp://127.0.0.1:554/test.mp4"
]
```

回复

```json
[
    {
        "code": 0,
        "message": "OK",
        "url": "rtsp://admin:ABCabc123@192.168.205.64",
        "ms": 1271.61,
        "video": {
            "index": 0,
            "codecId": 27,
            "codecName": "h264",
            "codecLongName": "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
            "pixFormat": "yuvj420p",
            "width": 1920,
            "height": 1080,
            "bitRate": "0",
            "frameRate": "25"
        },
        "audio": {
            "index": 1,
            "codecId": 65543,
            "codecName": "pcm_alaw",
            "codecLongName": "PCM A-law / G.711 A-law",
            "channels": 1,
            "bitRate": "64000",
            "sampleRate": "8000"
        }
    },
    {
        "code": -111,
        "message": "Connection refused",
        "url": "rtsp://127.0.0.1:554/test.mp4",
        "ms": 0.0,
        "video": null,
        "audio": null
    }
]
```



### 编译

- Linux

  ```shell
  g++ -Iinclude -I/home/eniso/jdk1.8.0_231/include -I/home/eniso/jdk1.8.0_231/include/linux native-lib.cpp \
  -fPIC -shared -o libprobe.so -Llibs -lavformat -lavcodec -lavutil -lswresample \
  -L/usr/lib -lm -lpthread -std=c++11
  ```



- Win32-x86

  ```shell
  ./configure --cross-prefix=i686-w64-mingw32- --enable-cross-compile --target-os=mingw32 --arch=x86 \
  --enable-gpl --enable-version3 --enable-nonfree --enable-runtime-cpudetect --enable-pic \
  --enable-w32threads --disable-x86asm --disable-static --enable-shared
  
  i686-w64-mingw32-g++ -Iinclude -Iinclude/win32 native-lib.cpp -shared -o probe.dll \
  -Llib -lavformat -lavcodec -lavutil -lswresample -lpthread -lm -std=c++11
  依赖：libgcc_s_sjlj-1.dll libstdc++-6.dll（-static-libstdc++ -static-libgcc 选项可以去除依赖）
  ```



- Win32-x86_64

  ```shell
  ./configure --cross-prefix=x86_64-w64-mingw32- --enable-cross-compile --target-os=mingw32 --arch=x86_64 \
  --enable-gpl --enable-version3 --enable-nonfree --enable-runtime-cpudetect --enable-pic \
  --enable-w32threads --disable-x86asm --disable-static --enable-shared
  
  x86_64-w64-mingw32-g++ -Iinclude -Iinclude/win32 native-lib.cpp -shared -o probe.dll \
  -Llib -lavformat -lavcodec -lavutil -lswresample -std=c++11
  依赖：libgcc_s_seh-1.dll libstdc++-6.dll（-static-libstdc++ -static-libgcc 选项可以去除依赖）
  ```



