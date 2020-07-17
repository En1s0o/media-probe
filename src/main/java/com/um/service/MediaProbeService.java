package com.um.service;

import com.google.common.util.concurrent.ListenableFuture;

import java.util.Collection;
import java.util.List;

/**
 * 媒体探测服务
 *
 * @author Eniso
 */
public interface MediaProbeService {

    /**
     * 提交媒体探测请求
     *
     * @param timeout 探测超时时间（单位：ms）
     * @param urls    媒体地址集合
     * @return 探测程序执行结束的返回值的 ListenableFuture 对象
     */
    ListenableFuture<List<MediaProbeResult>> submit(Long timeout, Collection<String> urls);

}
