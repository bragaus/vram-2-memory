#!/bin/sh
set -eu

if [ "$#" -lt 2 ]; then
    echo "Uso: $0 RAIZ ELF..." >&2
    exit 2
fi

raiz_do_initramfs=$1
shift

# Proposito: copiar um caminho absoluto preservando sua morada na raiz.
copiar_caminho_absoluto()
{
    caminho=$1
    destino="$raiz_do_initramfs$caminho"

    mkdir -p "$(dirname "$destino")"
    cp -L "$caminho" "$destino"
}

# Proposito: copiar um ELF e cada dependência absoluta declarada por ldd.
copiar_elf()
{
    elf=$1

    copiar_caminho_absoluto "$elf"
    for dependencia in $(ldd "$elf" | awk '
        /=> \// { print $3 }
        /^[[:space:]]*\// { print $1 }
    '); do
        copiar_caminho_absoluto "$dependencia"
    done
}

for elf_recebido in "$@"; do
    case "$elf_recebido" in
        /*) copiar_elf "$elf_recebido" ;;
        *) echo "ELF não absoluto: $elf_recebido" >&2; exit 3 ;;
    esac
done
