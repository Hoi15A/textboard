FROM alpine

RUN apk add --no-cache gcc musl-dev make python3

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

RUN make

EXPOSE 8080
CMD ["./server"]
