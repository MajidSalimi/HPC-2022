FROM ubuntu:18.04

RUN apt-get --yes -qq update \
 && apt-get --yes -qq upgrade \
 && apt-get --yes -qq install \
                      cmake \
                      libhdf5-serial-dev \
                      curl \
                      g++ \
                      gcc \
                      python3-dev \
                      python3-pip \
                      virtualenv \
                      wget \
                      vim       \
            && apt update && apt install openssh-server sudo -y \
			&& apt-get --yes update  \								
			&& apt-get --yes install -y openjdk-8-jdk \
			&& apt-get --yes install -y ant \
			&& apt-get --yes clean \
			&& apt-get --yes -qq clean \

			 && rm -rf /var/lib/apt/lists/*


RUN wget https://download.open-mpi.org/release/open-mpi/v4.0/openmpi-4.0.1.tar.gz \
	&& tar -xvf openmpi-4.0.1.tar.gz \
  && cd openmpi-4.0.1 && ./configure --enable-mpi-java &&  make all install && ldconfig 


CMD [ "/bin/bash" ]