FROM archlinux:base

# Install OS dependencies
RUN pacman -Syu --noconfirm --needed \
  clang xmake git make cmake ninja unzip zsh \ 
  nodejs yarn python \ 
  bear go

# Create a user called nodepp with UID 1000
RUN useradd -u 1000 -m -s /bin/zsh nodepp

# Set the user as the current user
USER nodepp

# Copy the project files
COPY --chown=nodepp:nodepp . /home/nodepp/nodepp

# Install go to get stomlGO111MODULE=on go install github.com/freshautomations/stoml@latest
ENV GOPATH="/home/nodepp/go"
RUN GO112MODULE=on go install github.com/freshautomations/stoml@latest

WORKDIR /home/nodepp/nodepp

# Export $HOME/go/bin to $PATH
ENV PATH="${GOPATH}/bin:/usr/lib/bin:${PATH}"
ENV TERM="tmux-256color"

# Run the dependencies script
RUN ./scripts/deps.sh --install

# Install dependencies
RUN yarn install --ignore-scripts

# Build the project
RUN ./scripts/build.sh --build
