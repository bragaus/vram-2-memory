#!/bin/sh
set -eu

directorio_do_rito=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
. "$directorio_do_rito/configuracao.env"

if [ "$#" -ne 1 ]; then
    echo "Uso: $0 DESTINO" >&2
    exit 2
fi

destino=$1
if [ -x "$destino" ] &&
   [ "$("$destino" --version)" = "$VERSAO_DO_FIO" ]; then
    exit 0
fi

temporario=$(mktemp -d /tmp/vramdisk-fio-XXXXXX)

# Proposito: restituir somente a árvore efêmera creada por mktemp.
restituir_fonte_do_fio()
{
    rm -rf -- "$temporario"
}
trap restituir_fonte_do_fio EXIT HUP INT TERM

git clone --quiet --depth 1 --branch "$VERSAO_DO_FIO" \
    "$REPOSITORIO_DO_FIO" "$temporario/fio"
commissao_observada=$(git -C "$temporario/fio" rev-parse HEAD)
if [ "$commissao_observada" != "$COMMISSAO_DO_FIO" ]; then
    echo "Commissão fio inesperada: $commissao_observada" >&2
    exit 3
fi

cd "$temporario/fio"
./configure --build-static --disable-native --disable-numa --disable-rdma \
    --disable-rados --disable-rbd --disable-http --disable-gfapi \
    --disable-pmem --disable-libnfs --disable-xnvme --disable-isal \
    --disable-isal64 --disable-libblkio --disable-libzbc --disable-tcmalloc
make -j2 fio
mkdir -p "$(dirname "$destino")"
install -m 0755 fio "$destino"
test "$("$destino" --version)" = "$VERSAO_DO_FIO"
