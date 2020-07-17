package com.um.controller;

import com.um.service.MediaProbeResult;
import com.um.service.MediaProbeService;
import com.um.util.AsyncCallbackTemplate;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.context.request.async.DeferredResult;

import java.util.Collection;
import java.util.List;

/**
 * MediaProbeController
 *
 * @author Eniso
 */
@Slf4j
@RestController
@RequestMapping("/api/v1/probe")
public class MediaProbeController {

    private final MediaProbeService service;

    @Autowired
    public MediaProbeController(MediaProbeService service) {
        this.service = service;
    }

    @PostMapping
    public DeferredResult<List<MediaProbeResult>> probe(
            @RequestParam(value = "timeout", required = false) Long timeout,
            @RequestBody Collection<String> urls) {
        DeferredResult<List<MediaProbeResult>> responseWriter = new DeferredResult<>();
        AsyncCallbackTemplate.withCallback(service.submit(timeout, urls),
                responseWriter::setResult,
                responseWriter::setErrorResult);
        return responseWriter;
    }

}
