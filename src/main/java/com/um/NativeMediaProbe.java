package com.um;

import com.sun.jna.Native;
import com.sun.jna.Platform;
import lombok.extern.slf4j.Slf4j;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * NativeMediaProbe
 *
 * @author Eniso
 */
@SuppressWarnings("unused")
@Slf4j
public class NativeMediaProbe {

    static {
        List<String> linux_libs = Arrays.asList(
                "libavutil.so.56",
                "libswresample.so.3",
                "libavcodec.so.58",
                "libavformat.so.58",
                "libprobe.so");
        List<String> win32_libs = Arrays.asList(
                "libstdc++-6.dll",
                "avutil-56.dll",
                "swresample-3.dll",
                "avcodec-58.dll",
                "avformat-58.dll",
                "probe.dll");
        try {
            List<String> libs = new ArrayList<>();
            if (Platform.isLinux()) {
                libs.addAll(linux_libs);
            } else if (Platform.isWindows() || Platform.isWindowsCE()) {
                if (Platform.is64Bit()) {
                    libs.add("libgcc_s_seh-1.dll");
                } else {
                    libs.add("libgcc_s_sjlj-1.dll");
                }
                libs.addAll(win32_libs);
            }

            String fileSep = System.getProperty("file.separator", "/");
            ClassLoader loader = NativeMediaProbe.class.getClassLoader();
            for (String lib : libs) {
                log.info("load library: {}", lib);
                String libName = "/libs/" + Platform.RESOURCE_PREFIX + "/" + lib;
                File temp = Native.extractFromResourcePath(libName, loader);
                File file = new File(temp.getParentFile().getAbsolutePath() + fileSep + lib);
                if (!temp.equals(file)) {
                    boolean delete = file.delete();
                    boolean rename = temp.renameTo(file);
                    log.info("delete={}, rename={}", delete, rename);
                }
                System.load(file.getAbsolutePath());
            }
        } catch (Exception e) {
            log.error("load probe failed", e);
        }
    }

    /**
     * 获取编译配置
     *
     * @return 编译配置
     */
    public static native String getConfiguration();

    /**
     * 媒体探测
     *
     * @param url     媒体地址
     * @param timeout 超时时间（单位：微秒）
     * @return 探测结果 JSON
     */
    public static native String probe(String url, String timeout);

}
