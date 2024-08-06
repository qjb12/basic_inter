# Use Ubuntu as the base image
FROM alpine as builder

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials, CMake, and any other dependencies
RUN apk update && apk add --no-cache build-base cmake pkgconf

# Install essential command line tools and SSH client
RUN apk add --no-cache vim git curl openssh-client

# Create a directory for SSH keys and set permissions
RUN mkdir /root/.ssh && \
    chmod 0700 /root/.ssh

# Copy the SSH private key into the container
COPY /.ssh/id_rsa /root/.ssh
COPY ~/.ssh/id_rsa.pub /root/.ssh

# Setup SSH to use the key
RUN echo "Host github.com\n\tAddKeysToAgent yes\n\tIdentityFile /root/.ssh/id_rsa" > /root/.ssh/config

# Add GitHub to known hosts
RUN ssh-keyscan github.com >> /root/.ssh/known_hosts

# Start the ssh-agent and add the key
RUN eval "$(ssh-agent -s)" && ssh-add /root/.ssh/id_rsa

RUN git clone git@github.com:qjb12/basic_inter.git

# update for locales 
ENV MUSL_LOCALE_DEPS cmake make musl-dev gcc gettext-dev libintl
ENV MUSL_LOCPATH /usr/share/i18n/locales/musl

RUN apk add --no-cache \
    $MUSL_LOCALE_DEPS \
    && wget https://gitlab.com/rilian-la-te/musl-locales/-/archive/master/musl-locales-master.zip \
    && unzip musl-locales-master.zip \
      && cd musl-locales-master \
      && cmake -DLOCALE_PROFILE=OFF -D CMAKE_INSTALL_PREFIX:PATH=/usr . && make && make install \
      && cd .. && rm -r musl-locales-master