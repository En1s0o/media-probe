FROM openjdk:8
MAINTAINER Eniso
VOLUME /tmp
ADD target/media-probe-1.0.0.jar app.jar
EXPOSE 62062
ENTRYPOINT ["java", "-Djava.security.egd=file:/dev/./urandom", "-jar", "/app.jar"]
