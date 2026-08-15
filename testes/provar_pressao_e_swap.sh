#!/bin/sh
set -eu

# § I. ADVERTENCIA — esta experiência destrói todo conteúdo do dispositivo.
dispositivo=${1:-}
if [ -z "$dispositivo" ] || [ ! -b "$dispositivo" ]; then
    echo "Indique exactamente um dispositivo de blocos existente." >&2
    exit 2
fi
if [ "${CONFIRMAR_DESTRUICAO:-NAO}" != "SIM" ]; then
    echo "Defina CONFIRMAR_DESTRUICAO=SIM para consentir a experiência." >&2
    exit 2
fi
if [ "$(id -u)" -ne 0 ]; then
    echo "A prova de swap reclama privilégios de administrador." >&2
    exit 2
fi
for instrumento in fio mkswap swapon swapoff stress-ng; do
    if ! command -v "$instrumento" >/dev/null 2>&1; then
        echo "Falta o instrumento exterior: $instrumento" >&2
        exit 2
    fi
done

# § II. A integridade precede o offício de swap e ocupa toda a grandeza.
fio --name=prova_integral --filename="$dispositivo" --direct=1 \
    --ioengine=io_uring --rw=randrw --rwmixread=50 --bs=128k \
    --iodepth=32 --size=100% --verify=crc32c --do_verify=1 \
    --verify_fatal=1 --group_reporting

# § III. A armadilha restitue o swap mesmo se a pressão romper a prova.
swap_activado=0
restituir_swap()
{
    if [ "$swap_activado" -eq 1 ]; then
        swapoff "$dispositivo"
    fi
}
trap restituir_swap EXIT INT TERM

mkswap -f "$dispositivo"
swapon --priority -2 "$dispositivo"
swap_activado=1
stress-ng --vm 2 --vm-bytes 80% --verify --timeout 60s
swapoff "$dispositivo"
swap_activado=0
trap - EXIT INT TERM

echo "A integridade e a pressão convergiram. Q.E.D."
