ARG base_tag=latest
FROM ghcr.io/testillano/diametercodec_builder:${base_tag}
MAINTAINER testillano

LABEL testillano.diametercodec.description="ert_diametercodec library image"

WORKDIR /code/build

ARG make_procs=4
ARG build_type=Release

# ert_diametercodec
COPY . /code/build/diametercodec/
RUN set -x && \
    cd diametercodec && cmake -DCMAKE_BUILD_TYPE=${build_type} . && make -j${make_procs} && make install && \
    cd .. && rm -rf diametercodec && \
    set +x
