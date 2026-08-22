#!/bin/sh
set -eu

if [ "$#" -ne 2 ]; then
    echo "Uso: $0 ORIGEM DESTINO" >&2
    exit 2
fi

origem=$1
destino=$2
if [ -r "$origem" ]; then
    mkdir -p "$(dirname "$destino")"
    install -m 0644 "$origem" "$destino"
    exit 0
fi

for instrumento in apt-get dpkg-deb dpkg-query; do
    command -v "$instrumento" >/dev/null 2>&1 || {
        echo "Não se pode extrahir kernel sem $instrumento." >&2
        exit 3
    }
done

pacote="linux-image-$(uname -r)"
versao=$(dpkg-query -W -f='${Version}' "$pacote")
temporario=$(mktemp -d /tmp/vramdisk-kernel-XXXXXX)

# Proposito: restituir pacote e extracção creados na árvore efêmera.
restituir_kernel_temporario()
{
    rm -rf -- "$temporario"
}
trap restituir_kernel_temporario EXIT HUP INT TERM

(cd "$temporario" && apt-get download "$pacote=$versao")
set -- "$temporario"/*.deb
test "$#" -eq 1
dpkg-deb -x "$1" "$temporario/extraido"
set -- "$temporario"/extraido/boot/vmlinuz-*
test "$#" -eq 1
test -r "$1"
mkdir -p "$(dirname "$destino")"
install -m 0644 "$1" "$destino"
