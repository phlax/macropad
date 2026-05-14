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

# System-wide venv, world-readable+executable so any UID can run it.
RUN python3 -m venv /opt/qmk-venv \
    && /opt/qmk-venv/bin/pip install --upgrade pip \
    && /opt/qmk-venv/bin/pip install qmk

# Pre-install every dependency QMK pulls in at runtime so the CLI never tries
# to call sudo/pip from a non-root container user.  These are taken from QMK's
# own requirements.txt; pinning a known-good ref keeps the image reproducible.
RUN /opt/qmk-venv/bin/pip install \
        appdirs \
        argcomplete \
        colorama \
        dotty-dict \
        hid \
        hjson \
        jsonschema \
        milc \
        pillow \
        pygments \
        pyserial \
        pyusb \
    && chmod -R a+rwX /opt/qmk-venv

ENV PATH=/opt/qmk-venv/bin:${PATH}

WORKDIR /workspace
CMD ["/bin/bash"]