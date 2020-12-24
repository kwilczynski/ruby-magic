#!/bin/bash

set -e
set -o pipefail

PREFIX=${PREFIX:-/tmp/magic}

export PATH="/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:${PATH}"

cat <<'EOF' | tee /etc/apt/apt.conf.d/99apt &>/dev/null
DPkg::options { "--force-confdef"; "--force-confnew"; }
EOF

export DEBIAN_FRONTEND='noninteractive'
export DEBCONF_NONINTERACTIVE_SEEN='true'

apt-get update

apt-get install --assume-yes \
    wget            \
    git             \
    mercurial       \
    make            \
    bison           \
    flex            \
    build-essential \
    autoconf        \
    automake        \
    autotools-dev   \
    libltdl-dev     \
    libtool         \
    libtool-doc


rm -Rf "file-${VERSION}"

CACHE_DIRECTORY="${HOME}/cache"
if [[ $USER == 'root' ]]; then
    eval HOME_DIRECTORY="~${SUDO_USER}"
    CACHE_DIRECTORY="${HOME_DIRECTORY}/cache"
fi

ARCHIVE_NAME="file-${VERSION}.tar.gz"

mkdir -p "$CACHE_DIRECTORY"

if [[ -f "${CACHE_DIRECTORY}/${ARCHIVE_NAME}" ]]; then
    cp -f "${CACHE_DIRECTORY}/${ARCHIVE_NAME}" .
fi

set +e
if ! echo "$SHA1 *${ARCHIVE_NAME}" | sha1sum -c; then
    rm -f "file-${VERSION}.tar.gz"

    MIRRORS=(
        'https://fossies.org/linux/misc'
        'http://ftp.clfs.org/pub/clfs/conglomeration/file'
        'http://ftp.uni-stuttgart.de/pub/mirrors/mirror.slitaz.org/slitaz/sources/packages/f'
        'ftp://ftp.astron.com/pub/file'
    )

    for mirror in "${MIRRORS[@]}"; do
        wget -O "$ARCHIVE_NAME" "${mirror}/${ARCHIVE_NAME}" && break
    done

    if echo "$SHA1 *${ARCHIVE_NAME}" | sha1sum -c; then
        cp -f "$ARCHIVE_NAME" "${CACHE_DIRECTORY}/${ARCHIVE_NAME}"
        chown -R "$SUDO_USER" "$CACHE_DIRECTORY"
    fi
fi
set -e

tar -zxf "$ARCHIVE_NAME"

pushd "file-${VERSION}" &> /dev/null

rm -f src/magic.h

for action in clean distclean; do
    make "$action" || true
done

libtoolize
aclocal
autoheader
automake --add-missing
autoconf

if ! ./configure --prefix="$PREFIX" --enable-fsect-man5; then
  $CC --version
  cat config.log
  declare -p
  exit $?
fi

make -j 4
make install

cat <<EOF > /etc/ld.so.conf.d/libmagic.conf
${PREFIX}
${PREFIX}/lib
EOF

ldconfig

popd &> /dev/null

rm -Rf \
    "file-${VERSION}" \
    "file-${VERSION}.tar.gz"
