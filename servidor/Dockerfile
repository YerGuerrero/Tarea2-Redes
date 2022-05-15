FROM gcc:4.9
COPY . /usr/src/servidor
WORKDIR /usr/src/servidor
RUN gcc -std=c99 -o servidor servidor.c
EXPOSE 9666
CMD ["./servidor"]
