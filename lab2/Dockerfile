FROM alpine
RUN apk add -q util-linux
COPY s.sh /s
RUN chmod +x /s
CMD ["/s"]
