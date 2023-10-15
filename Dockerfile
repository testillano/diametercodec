ARG base_tag=latest
ARG scratch_img=ubuntu
ARG scratch_img_tag=latest
FROM ghcr.io/testillano/diametercodec_builder:${base_tag} as builder
MAINTAINER testillano

LABEL testillano.diametercodec.description="ert_diametercodec library image"

COPY . /code
WORKDIR /code

ARG make_procs=4
ARG build_type=Release

# ert_diametercodec
#COPY . /code/build/diametercodec/
RUN cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs}

FROM ${scratch_img}:${scratch_img_tag}
ARG build_type=Release
COPY --from=builder /code/build/${build_type}/bin/printHardcodedStacks /opt/

# Ubuntu has bash already installed, but vim is missing
ARG base_os=ubuntu
RUN if [ "${base_os}" = "alpine" ] ; then apk update && apk add bash jq && rm -rf /var/cache/apk/* ; elif [ "${base_os}" = "ubuntu" ] ; then apt-get update && apt-get install -y vim jq && apt-get clean ; fi

ENTRYPOINT ["/bin/bash"]
CMD []
