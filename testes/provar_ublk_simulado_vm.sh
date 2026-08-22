#!/bin/sh
set -eu

directorio_das_provas=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
raiz_da_obra=$(CDPATH= cd -- "$directorio_das_provas/.." && pwd)
directorio_da_vm="$directorio_das_provas/vm"
. "$directorio_da_vm/configuracao.env"

kernel_da_vm=${KERNEL_DA_VM:-"/boot/vmlinuz-$(uname -r)"}
modulo_ublk=${MODULO_UBLK:-"/lib/modules/$(uname -r)/kernel/drivers/block/ublk_drv.ko.zst"}
directorio_do_cuda=${DIRECTORIO_DO_CUDA:-/usr/local/cuda}
pkg_config_do_ublk=${PKG_CONFIG_PATH:-"$HOME/.local/ublk-stack/lib/pkgconfig"}
artefactos="$raiz_da_obra/construcao/vm"
raiz_temporaria=$(mktemp -d /tmp/vramdisk-initramfs-XXXXXX)
raiz_do_initramfs="$raiz_temporaria/raiz"

# Proposito: restituir somente a árvore efêmera creada para o initramfs.
restituir_raiz_temporaria()
{
    rm -rf -- "$raiz_temporaria"
}
trap restituir_raiz_temporaria EXIT HUP INT TERM

for instrumento in qemu-system-x86_64 cpio gzip zstd cc make git ldd; do
    if ! command -v "$instrumento" >/dev/null 2>&1; then
        echo "Instrumento ausente: $instrumento" >&2
        exit 2
    fi
done

for caminho in "$kernel_da_vm" "$modulo_ublk" /bin/busybox; do
    if [ ! -r "$caminho" ]; then
        echo "Entrada da VM ausente: $caminho" >&2
        exit 3
    fi
done

mkdir -p "$artefactos" "$raiz_do_initramfs"
