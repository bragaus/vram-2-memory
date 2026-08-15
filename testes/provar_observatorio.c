#include "../src/monitor_do_observatorio.h"
#include "../src/observador_de_si.h"

#include <stdint.h>
#include <string.h>

/*
 * Proposito: demonstrar limites, proporção e terminação das figuras breves.
 * Pre-condições: nenhuma; os destinos pertencem á pilha da prova.
 * Effeitos: nenhum fora da pilha. Retorno: unidade ou zero na divergência.
 * Razão: zero, metade e saturação cercam o domínio visual da régua.
 */
static int provar_figuras_breves(void)
{
    const uint64_t amostras[] = {0, 1, 2, 4, 8};
    char figura[32];

    if (desenhar_regua_ascii(figura, sizeof(figura), 0, 8, 10) != 10 ||
        strcmp(figura, "[--------]") != 0) return 0;
    if (desenhar_regua_ascii(figura, sizeof(figura), 4, 8, 10) != 10 ||
        strcmp(figura, "[####----]") != 0) return 0;
    if (desenhar_regua_ascii(figura, sizeof(figura), 9, 8, 10) != 10 ||
        strcmp(figura, "[########]") != 0) return 0;
    if (desenhar_linha_scintillante(
            figura, sizeof(figura), amostras, 5, 3) != 3 ||
        figura[3] != '\0') return 0;
    return desenhar_regua_ascii(figura, 10, 1, 2, 10) == 0;
}

/*
 * Proposito: reunir as demonstrações do observatório num resultado exterior.
 * Pre-condições: nenhuma. Effeitos: nenhum além do código de saída.
 * Retorno: zero se todas as proposições resistem, unidade na primeira queda.
 * Razão: a receita de provas reclama uma porta singular e reproduzível.
 */
int main(void)
{
    return provar_figuras_breves() ? 0 : 1;
}
