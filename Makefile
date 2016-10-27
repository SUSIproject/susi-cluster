all: build docker

build:
	docker run \
		-w /go/src/github.com/SUSIproject/cluster \
		-v $(shell pwd):/go/src/github.com/SUSIproject/cluster \
		golang bash -c \
			"go get -v github.com/SUSIproject/cluster; \
			 cp /go/bin/cluster /go/src/github.com/SUSIproject/cluster/susi-cluster-linux-amd64"

docker:
	docker build -t quay.io/trusch/susi-cluster:$(shell git describe) .
	docker push quay.io/trusch/susi-cluster:$(shell git describe)
