FROM docker.io/zmkfirmware/zmk-dev-arm:4.1-branch

# Set default to bash instead sh
ENV SHELL /bin/bash

RUN mkdir -p /root/.vscode-server/extensions

# Set working directory
WORKDIR /workspace
VOLUME ["/workspace"]

# Make 'user' default on launch
USER root

RUN echo 'if [ -f "$WORKSPACE_DIR/zephyr/zephyr-env.sh" ]; then source "$WORKSPACE_DIR/zephyr/zephyr-env.sh"; fi' >> ~/.bashrc

# Launch bash shell by default
CMD ["/bin/bash"]