FROM docker.io/zmkfirmware/zmk-dev-arm:4.1-branch

# Set default to bash instead sh
ENV SHELL /bin/bash

RUN mkdir -p /root/.vscode-server/extensions

RUN apt update && apt install -y \
    python3.12-venv usbutils linux-tools-virtual hwdata \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/local/bin/usbip usbip $(command -v ls /usr/lib/linux-tools/*/usbip | tail -n1) 20

# Set working directory
WORKDIR /workspace
VOLUME ["/workspace"]

# Make 'user' default on launch
USER root

RUN echo 'if [ -f "$WORKSPACE_DIR/zephyr/zephyr-env.sh" ]; then source "$WORKSPACE_DIR/zephyr/zephyr-env.sh"; fi' >> ~/.bashrc

# Launch bash shell by default
CMD ["/bin/bash"]