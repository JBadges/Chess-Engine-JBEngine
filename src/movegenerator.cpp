#include "movegenerator.h"

using namespace JACEA;

int JACEA::mvv_lva[12][12];

void JACEA::init_mvv_lva()
{
    for (Piece attacker = P; attacker <= k; attacker++)
    {
        for (Piece victim = P; victim <= k; victim++)
        {
            mvv_lva[attacker][victim] = piece_to_value[attacker] * 5 - piece_to_value[victim];
        }
    }
}
