FROM debian

COPY ./src /app/src/
COPY ./include /app/include/
COPY ./Makefile /app/
WORKDIR /app
RUN apt update
RUN apt install build-essential -y
RUN make minesolve

ENTRYPOINT [ "/app/bin/minesolve" ]