FROM alpine

RUN apk add --no-cache gcc musl-dev make python3 git

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

RUN touch database.csv
RUN make

EXPOSE 8080
CMD ["./server"]
