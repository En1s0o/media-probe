/**
 * Probe media stream information
 *
 * @author Eniso
 */

#include <jni.h>

#include <cstdio>

#include <sstream>
#include <string>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/jni.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/avutil.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
} // extern "C"
#endif

#define CLASS_NAME "com/um/NativeMediaProbe"

#define LOGV(format, ...) printf(format, ##__VA_ARGS__)
#define LOGD(format, ...) printf(format, ##__VA_ARGS__)
#define LOGI(format, ...) printf(format, ##__VA_ARGS__)
#define LOGW(format, ...) printf(format, ##__VA_ARGS__)
#define LOGE(format, ...) printf(format, ##__VA_ARGS__)
#define LOGF(format, ...) printf(format, ##__VA_ARGS__)

/* -------------------------------------------------- */

using namespace std;


JNIEXPORT jstring JNICALL probe(JNIEnv *env, jobject instance, jstring _url, jstring _timeout) {
    auto start = std::chrono::steady_clock::now();
    const char *url = env->GetStringUTFChars(_url, 0);
    const char *timeout = env->GetStringUTFChars(_timeout, 0);

    AVFormatContext *inputContext = NULL;
    AVCodec *videoDecoder = NULL;
    AVCodec *audioDecoder = NULL;
    AVDictionary *dicts = NULL;
    av_dict_set(&dicts, "rtsp_transport", "tcp", 0);
    av_dict_set(&dicts, "max_delay", "500000", 0);
    av_dict_set(&dicts, "buffer_size", "2500000", 0);
    // av_dict_set(&dicts, "stimeout", "15000000", 0);
    av_dict_set(&dicts, "stimeout", timeout, 0);
    LOGI("start probe url (timeout: %s): %s\n", timeout, url);

    int err;
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    if ((err = avformat_open_input(&inputContext, url, NULL, &dicts)) != 0) {
        av_strerror(err, errbuf, AV_ERROR_MAX_STRING_SIZE);
        LOGE("avformat_open_input failed %s: %s\n", errbuf, url);
        av_dict_free(&dicts);
        ostringstream oss;
        // oss << "{\"code\": " << err << ", \"message\": \"" << errbuf << "\", \"url\": \"" << url << "\", \"ms\": 0}";
        oss << "{\"code\": " << err << ", \"message\": \"" << errbuf << "\", \"ms\": 0}";
        env->ReleaseStringUTFChars(_url, url);
        env->ReleaseStringUTFChars(_timeout, timeout);
        return env->NewStringUTF(oss.str().c_str());
    }

    if ((err = avformat_find_stream_info(inputContext, NULL)) != 0) {
        av_strerror(err, errbuf, AV_ERROR_MAX_STRING_SIZE);
        LOGW("avformat_find_stream_info failed %s: %s\n", errbuf, url);
        // ignored
    }

    int videoStreamIndex = av_find_best_stream(inputContext, AVMEDIA_TYPE_VIDEO,
                                               -1, -1, &videoDecoder, 0);
    int audioStreamIndex = av_find_best_stream(inputContext, AVMEDIA_TYPE_AUDIO,
                                               -1, -1, &audioDecoder, 0);
    if (videoStreamIndex < 0 && audioStreamIndex < 0) {
        LOGE("find video and/or audio stream failed %s\n", url);
        avformat_close_input(&inputContext);
        av_dict_free(&dicts);
        ostringstream oss;
        // oss << "{\"code\": " << err << ", \"message\": \"" << errbuf << "\", \"url\": \"" << url << "\", \"ms\": 0}";
        oss << "{\"code\": " << err << ", \"message\": \"" << errbuf << "\", \"ms\": 0}";
        env->ReleaseStringUTFChars(_url, url);
        env->ReleaseStringUTFChars(_timeout, timeout);
        return env->NewStringUTF(oss.str().c_str());
    }

    // put stream info
    ostringstream oss;
    // oss << "{\"code\": 0, \"message\": \"OK\", \"url\": \"" << url << "\", ";
    oss << "{\"code\": 0, \"message\": \"OK\", ";
    if (videoStreamIndex >= 0) {
        AVStream *stream = inputContext->streams[videoStreamIndex];
        AVCodecParameters *codecpar = stream->codecpar;
        const char *pix_fmt = NULL;

        /*
        // 海康 IPC 发送的 RTSP 指令中携带信息如下：
        // sprop-parameter-sets=Z0IAKpY1QPAET8s3AQEBQAABwgAAV+QB,aM48gA==
        // 逗号前面是 SPS(0x67)，后面是 PPS(0x68)
        // av_base64_decode
        uint8_t sps_pps[] = {
            // 起始码 0x00 0x00 0x00 0x01
            0x00, 0x00, 0x01,
            0x67, 0x42, 0x00, 0x2a,
            0x96, 0x35, 0x40, 0xf0,
            0x04, 0x4f, 0xcb, 0x37,
            0x01, 0x01, 0x01, 0x40,
            0x00, 0x01, 0xc2, 0x00,
            0x00, 0x57, 0xe4, 0x01,
            0x00, 0x00, 0x00, 0x01,
            0x68, 0xce, 0x3c, 0x80,
            0x00
        };
        codecpar->extradata_size = sizeof(sps_pps);
        codecpar->extradata = (uint8_t *) av_mallocz(codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        memset(codecpar->extradata, 0, sizeof(sps_pps) + AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(codecpar->extradata, sps_pps, sizeof(sps_pps));
        */
        AVCodecContext *decoderContext = avcodec_alloc_context3(videoDecoder);
        if (decoderContext != NULL) {
            if (avcodec_parameters_to_context(decoderContext, codecpar) >= 0) {
                pix_fmt = av_get_pix_fmt_name(decoderContext->pix_fmt);
                /*
                AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
                if (avcodec_open2(decoderContext, videoDecoder, NULL) == 0) {
                    for (int i = 0; i < 50; i++) {
                        if (av_read_frame(inputContext, packet) != 0) {
                            break;
                        }
                        if (packet->stream_index == videoStreamIndex) {
                            pix_fmt = av_get_pix_fmt_name(decoderContext->pix_fmt);
                            LOGI("%s %ld\n", av_get_pix_fmt_name(decoderContext->pix_fmt), inputContext->bit_rate);
                            // FIXME: decode ?
                        }
                        av_packet_unref(packet);
                    }
                }
                if (packet != NULL) {
                    av_free(packet);
                }
                */
            }
        }

        oss << "\"video\": {";
        oss << "\"index\": " << videoStreamIndex << ", ";
        oss << "\"codecId\": " << codecpar->codec_id << ", ";
        oss << "\"codecName\": \"" << videoDecoder->name << "\", ";
        oss << "\"codecLongName\": \"" << videoDecoder->long_name << "\", ";
        oss << "\"pixFormat\": \"" << (pix_fmt == NULL ? "" : pix_fmt) << "\", ";
        oss << "\"width\": " << codecpar->width << ", ";
        oss << "\"height\": " << codecpar->height << ", ";
        oss << "\"bitRate\": \"" << inputContext->bit_rate << "\", "; // FIXME: unusable
        oss << "\"frameRate\": \"" << av_q2d(stream->avg_frame_rate) << "\"";
        oss << "}, ";
        if (decoderContext != NULL) {
            avcodec_close(decoderContext);
            avcodec_free_context(&decoderContext);
        }
    } else {
        oss << "\"video\": {}, ";
    }
    if (audioStreamIndex >= 0) {
        AVStream *stream = inputContext->streams[audioStreamIndex];
        AVCodecParameters *codecpar = stream->codecpar;
        int bits_per_sample = av_get_bits_per_sample(codecpar->codec_id);
        int64_t bit_rate = bits_per_sample ? codecpar->sample_rate * (int64_t) codecpar->channels * bits_per_sample : codecpar->bit_rate;
        oss << "\"audio\": {";
        oss << "\"index\": " << audioStreamIndex << ", ";
        oss << "\"codecId\": " << codecpar->codec_id << ", ";
        oss << "\"codecName\": \"" << audioDecoder->name << "\", ";
        oss << "\"codecLongName\": \"" << audioDecoder->long_name << "\", ";
        oss << "\"channels\": " << codecpar->channels << ", ";
        oss << "\"bitRate\": \"" << bit_rate << "\", "; // b/s
        oss << "\"sampleRate\": \"" << codecpar->sample_rate << "\""; // Hz
        oss << "}, ";
    } else {
        oss << "\"audio\": {}, ";
    }
    auto end = std::chrono::steady_clock::now();
    oss << "\"ms\": " << std::chrono::duration<double, std::milli>(end - start).count() << "";
    oss << "}";
    const std::string json = oss.str();
    // LOGI("%s\n", json.c_str());

    avformat_close_input(&inputContext);
    av_dict_free(&dicts);
    LOGI("complete probe url: %s\n", url);

    env->ReleaseStringUTFChars(_url, url);
    env->ReleaseStringUTFChars(_timeout, timeout);
    return env->NewStringUTF(json.c_str());
}

/** 获取 ffmpeg 编译配置，可以知道时候编译是否存在异常 */
JNIEXPORT jstring JNICALL getConfiguration(JNIEnv *env, jobject) {
    return env->NewStringUTF(avcodec_configuration());
}

/* -------------------------------------------------- */

/** 获取编译时采用的abi */
static inline const char *getCompiledABI() {
#if defined(__arm__)
#if defined(__ARM_ARCH_7A__)
#if defined(__ARM_NEON__)
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a/NEON (hard-float)"
#else
#define ABI "armeabi-v7a/NEON"
#endif
#else
#if defined(__ARM_PCS_VFP)
#define ABI "armeabi-v7a (hard-float)"
#else
#define ABI "armeabi-v7a"
#endif
#endif
#else
#define ABI "armeabi"
#endif
#elif defined(__i386__)
#define ABI "x86"
#elif defined(__x86_64__)
#define ABI "x86_64"
#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */
#define ABI "mips64"
#elif defined(__mips__)
#define ABI "mips"
#elif defined(__aarch64__)
#define ABI "arm64-v8a"
#else
#define ABI "unknown"
#endif

    return "Compiled with ABI: " ABI;
}

/** JNI 方法表 */
static const JNINativeMethod S_METHOD_TABLE[] = {
    {
        (char *) "getConfiguration",
        (char *) "()Ljava/lang/String;",
        (void *) getConfiguration
    }, {
        (char *) "probe",
        (char *) "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
        (void *) probe
    }
};

/** 注册 JNI 方法 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env = NULL;
    auto start = std::chrono::steady_clock::now();

    LOGI("%s %s\n", __func__, getCompiledABI());
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("cannot get env\n");
        return -1;
    }

    jclass clazz = env->FindClass(CLASS_NAME);
    if (clazz == NULL) {
        LOGE("cannot get class: %s\n", CLASS_NAME);
        return -1;
    }

    size_t size = sizeof(S_METHOD_TABLE) / sizeof(S_METHOD_TABLE[0]);
    if (env->RegisterNatives(clazz, S_METHOD_TABLE, size) != JNI_OK) {
        LOGE("register native methods failed, class: %s\n", CLASS_NAME);
        return -1;
    }

    // av_jni_set_java_vm(vm, NULL);
    // 4.0 之后可以省略 omit
    // av_register_all();
    // avfilter_register_all();
    // avcodec_register_all();
    avformat_network_init();

    auto end = std::chrono::steady_clock::now();
    LOGI("%s success (%f ms)\n", __func__, std::chrono::duration<double, std::milli>(end - start).count());
    return JNI_VERSION_1_6;
}

/** 卸载 JNI 方法 */
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *) {
    JNIEnv *env = NULL;
    auto start = std::chrono::steady_clock::now();

    LOGI("%s\n", __func__);
    avformat_network_deinit();

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("cannot get env\n");
        return;
    }

    jclass clazz = env->FindClass(CLASS_NAME);
    if (clazz == NULL) {
        LOGE("cannot get class: %s\n", CLASS_NAME);
        return;
    }

    if (env->UnregisterNatives(clazz) != JNI_OK) {
        LOGE("unregister native methods failed, class: %s\n", CLASS_NAME);
        return;
    }

    auto end = std::chrono::steady_clock::now();
    LOGI("%s success (%f ms)\n", __func__, std::chrono::duration<double, std::milli>(end - start).count());
}
