package com.um.service;

import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.ListeningExecutorService;
import com.google.common.util.concurrent.MoreExecutors;
import com.um.NativeMediaProbe;
import com.um.config.MediaProbeConfiguration;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

/**
 * 媒体探测服务实现
 *
 * @author Eniso
 */
@Slf4j
@Service
public class MediaProbeServiceImpl implements MediaProbeService {

    private final ObjectMapper mapper = new ObjectMapper()
            .disable(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES);

    private final String defaultTimeout;

    private ExecutorService executor;

    private ListeningExecutorService listeningExecutor;

    @Autowired
    public MediaProbeServiceImpl(MediaProbeConfiguration configuration) {
        defaultTimeout = String.valueOf(configuration.getTimeout() * 1000L);
        log.info("default timeout: {}", defaultTimeout);
    }

    @PostConstruct
    public void init() {
        executor = Executors.newWorkStealingPool(32);
        listeningExecutor = MoreExecutors.listeningDecorator(Executors.newWorkStealingPool(128));
    }

    @PreDestroy
    public void destroy() {
        if (executor != null) {
            executor.shutdownNow();
        }
        if (listeningExecutor != null) {
            listeningExecutor.shutdownNow();
        }
    }

    @Override
    public ListenableFuture<List<MediaProbeResult>> submit(Long timeout, Collection<String> urls) {
        String micros;
        if (timeout != null && timeout >= 0L) {
            micros = String.valueOf(timeout * 1000L);
        } else {
            micros = this.defaultTimeout;
        }
        return listeningExecutor.submit(() -> {
            List<Future<MediaProbeResult>> futures = new HashSet<>(urls).parallelStream()
                    .filter(Objects::nonNull)
                    .map(url -> submit(url, micros))
                    .collect(Collectors.toList());
            List<MediaProbeResult> results = new ArrayList<>();
            for (Future<MediaProbeResult> future : futures) {
                results.add(future.get());
            }
            return results;
        });
    }

    private Future<MediaProbeResult> submit(String url, String timeout) {
        return executor.submit(() -> {
            MediaProbeResult result = mapper.readValue(
                    NativeMediaProbe.probe(url, timeout), MediaProbeResult.class);
            result.setUrl(url);
            log.info(mapper.writeValueAsString(result));
            return result;
        });
    }

}
