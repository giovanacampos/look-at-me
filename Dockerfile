FROM ubuntu:20.04

ENV TZ=America/Sao_Paulo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -y
RUN apt install -y sudo xauth

COPY scripts scripts
RUN bash scripts/install.sh

RUN mkdir /look-at-me
WORKDIR /look-at-me

COPY data data
COPY src src
COPY LookAtMe.h.in LookAtMe.h.in
COPY CMakeLists.txt CMakeLists.txt

RUN mkdir /look-at-me/build
WORKDIR /look-at-me/build

RUN cmake .. -DUSE_SSE2_INSTRUCTIONS=ON
RUN make -j4
RUN cp /look-at-me/bin/look-at-me /usr/local/bin

# Must change everything after "RUN xauth add " to local token from host
# Use the command `xauth list` to copy the hash.
RUN xauth add look-at-me/unix:1  MIT-MAGIC-COOKIE-1  a076adcfef9b24b6664d78b50ea99a4f
RUN xauth list

ENV LD_LIBRARY_PATH=/usr/local/lib
WORKDIR /look-at-me
# CMD ["bin/look-at-me", "-v", "/look-at-me/data/videos/man-1.mp4"]
CMD ["bin/look-at-me"]