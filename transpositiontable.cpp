#include "transpositiontable.h"

void JACEA::clear_table(std::vector<TTEntry> &table, const int size)
{
    for (int i = 0; i < size; i++)
    {
        table[i].key = 0;
        table[i].depth = 0;
        table[i].flags = 0;
        table[i].value = 0;
        table[i].best_move = 0;
    }
}