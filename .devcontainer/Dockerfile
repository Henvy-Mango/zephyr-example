FROM zephyrprojectrtos/ci:v0.26-branch

# Set default to bash instead sh
ENV SHELL /bin/bash

# Install packages
RUN apt-get -y update && \
  apt-get -y upgrade && \
  apt-get install --no-install-recommends -y \
  bash-completion \
  curl \
  doxygen \
  graphviz \
  mscgen \
  latexmk \
  librsvg2-bin \
  texlive-latex-base \
  texlive-latex-extra \
  texlive-fonts-recommended \
  tmux \
  usbutils \
  vim

# Clean up stale packages
RUN apt-get clean -y && \
  apt-get autoremove --purge -y && \
  rm -rf /var/lib/apt/lists/*

# Switch to 'user' context
USER user

RUN mkdir -p /home/user/.vscode-server/extensions

# Set working directory
WORKDIR /workspace
VOLUME ["/workspace"]

# Adjust 'user' home directory permissions
USER root
RUN chown -R user:user /home/user
RUN west completion bash > /etc/bash_completion.d/west-completion.bash

# Add 'user' to groups to access USB devices
RUN usermod -aG dialout user

# Make 'user' default on launch
USER user

RUN echo 'if [ -f "$WORKSPACE_DIR/zephyr/zephyr-env.sh" ]; then source "$WORKSPACE_DIR/zephyr/zephyr-env.sh"; fi' >> ~/.bashrc

# Launch bash shell by default
CMD ["/bin/bash"]