FROM debian:trixie

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        git \
        git-lfs \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        python3 \
        python3-pip \
        python3-venv \
        dfu-util \
        dfu-programmer \
        unzip \
        wget \
        xxd \
    && rm -rf /var/lib/apt/lists/* \
    && git lfs install --system --skip-repo

RUN useradd --create-home --shell /bin/bash builder

USER builder
RUN python3 -m venv /home/builder/.venv \
    && /home/builder/.venv/bin/pip install --upgrade pip \
    && /home/builder/.venv/bin/pip install qmk

USER root
ENV PATH=/home/builder/.venv/bin:${PATH}

WORKDIR /workspace
CMD ["/bin/bash"]
