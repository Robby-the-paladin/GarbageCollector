FROM registry.altlinux.org/alt/alt:latest
#RUN npm install -g eslint prettier
RUN apt-get update
RUN apt-get install -y gcc gcc-c++ cmake make libcurl libcurl-devel json-cpp jsoncpp-devel libjsoncpp19 rpmspec su rpm-utils rpm-macros-branding libpqxx-devel librpm librpm-devel
RUN useradd builder