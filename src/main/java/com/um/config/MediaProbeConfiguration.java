package com.um.config;

import lombok.Data;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Configuration;

/**
 * MediaProbeConfiguration
 *
 * @author Eniso
 */
@Data
@Configuration
@ConfigurationProperties(prefix = MediaProbeConfiguration.PREFIX)
public class MediaProbeConfiguration {

    public static final String PREFIX = "server.media.probe";

    /**
     * 超时时间（单位：ms）
     */
    private long timeout;

}
