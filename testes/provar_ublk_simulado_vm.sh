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

for instrumento in qemu-system-x86_64 cpio gzip zstd cc make git ldd pkg-config; do
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

PKG_CONFIG_PATH="$pkg_config_do_ublk" make -C "$raiz_da_obra" \
    preparar_cuda DIRECTORIO_DO_CUDA="$directorio_do_cuda"
make -C "$raiz_da_obra" preparar_cliente
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -static \
    "$directorio_da_vm/provar_operacao_invalida.c" \
    -o "$artefactos/provar_operacao_invalida"
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -static \
    "$directorio_da_vm/encerrar_vm.c" -o "$artefactos/encerrar_vm"
"$directorio_da_vm/preparar_fio_estatico.sh" "$artefactos/fio"

mkdir -p "$raiz_do_initramfs/bin" "$raiz_do_initramfs/lib/modules"
install -m 0755 /bin/busybox "$raiz_do_initramfs/bin/busybox"
install -m 0755 "$directorio_da_vm/init" "$raiz_do_initramfs/init"
install -m 0644 "$directorio_da_vm/configuracao.env" \
    "$raiz_do_initramfs/configuracao.env"
install -m 0755 "$raiz_da_obra/construcao/vramdiskd" \
    "$raiz_do_initramfs/bin/vramdiskd"
install -m 0755 "$raiz_da_obra/construcao/vramdiskctl" \
    "$raiz_do_initramfs/bin/vramdiskctl"
install -m 0755 "$artefactos/fio" "$raiz_do_initramfs/bin/fio"
install -m 0755 "$artefactos/provar_operacao_invalida" \
    "$raiz_do_initramfs/bin/provar_operacao_invalida"
install -m 0755 "$artefactos/encerrar_vm" \
    "$raiz_do_initramfs/bin/encerrar_vm"
zstd -q -d -c "$modulo_ublk" > "$raiz_do_initramfs/lib/modules/ublk_drv.ko"

directorio_de_libublk=$(PKG_CONFIG_PATH="$pkg_config_do_ublk" \
    pkg-config --variable=libdir ublksrv)
LD_LIBRARY_PATH="$directorio_de_libublk:$directorio_do_cuda/lib64" \
    "$directorio_da_vm/copiar_elf_e_dependencias.sh" \
    "$raiz_do_initramfs" "$raiz_da_obra/construcao/vramdiskd"

mkdir -p "$raiz_do_initramfs/lib"
install -m 0755 "$directorio_de_libublk/libublksrv.so.0" \
    "$raiz_do_initramfs/lib/libublksrv.so.0"
install -m 0755 "$directorio_de_libublk/liburing.so.2" \
    "$raiz_do_initramfs/lib/liburing.so.2"
install -m 0755 "$directorio_do_cuda/lib64/libcudart.so.12" \
    "$raiz_do_initramfs/lib/libcudart.so.12"

imagem_do_initramfs="$artefactos/initramfs-vramdisk.cpio.gz"
(cd "$raiz_do_initramfs" &&
 find . -print0 | sort -z | cpio --null -o --format=newc 2>/dev/null |
 gzip -9 > "$imagem_do_initramfs")
test -s "$imagem_do_initramfs"
