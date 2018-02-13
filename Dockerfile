FROM alpine

RUN apk add --no-cache gcc musl-dev make python3

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

RUN make PRODUCTION=1

EXPOSE 8080
CMD ["./server"]
