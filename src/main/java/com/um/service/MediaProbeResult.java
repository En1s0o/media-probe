package com.um.service;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import lombok.Data;

/**
 * MediaProbeResult
 *
 * @author Eniso
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class MediaProbeResult {

    private Integer code;
    private String message;
    private String url;
    private Double ms;
    private Video video;
    private Audio audio;

    @Data
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Video {
        private Integer index;
        private Integer codecId;
        private String codecName;
        private String codecLongName;
        private String pixFormat;
        private Integer width;
        private Integer height;
        private String bitRate;
        private String frameRate;
    }

    @Data
    @JsonIgnoreProperties(ignoreUnknown = true)
    public static class Audio {
        private Integer index;
        private Integer codecId;
        private String codecName;
        private String codecLongName;
        private Integer channels;
        private String bitRate;
        private String sampleRate;
    }

}
